#ifdef VH_TAG_FIRMWARE

#include <stdint.h>
#include <limits.h>
#include <time.h>

#include <xdc/std.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <ti/devices/cc13x0/driverlib/rf_mailbox.h>
#include <ti/devices/cc13x0/driverlib/rf_prop_mailbox.h>
#include <ti/devices/cc13x0/driverlib/rf_common_cmd.h>
#include <ti/devices/cc13x0/driverlib/rf_prop_cmd.h>
#include <ti/drivers/rf/RF.h>

#include <ti/devices/cc13x0/driverlib/sys_ctrl.h>


#include "config.h"
#include "tag.h"
#include "radio_setup.h"
#include "leds.h"
#include "board.h"
#include "receive.h"

#include "watchdog.h"
#include "sensors_batmon.h"
#include "vildehaye.h"
#include "sensor_controller.h"

/*****************************************************************************/
/* WRITE CONFIGURATION TO FLASH                                              */
/*****************************************************************************/

static uint8_t* flashConfigurationData = (uint8_t*) 0x0001e000;
#define flashPageSize 0x1000
// this should be adjusted when we find out how much space there is past the configuration packet
static uint32_t flashConfigurationEnd  = 0x0001e000 + (flashPageSize-1);

#include <ti/devices/cc13x0/driverlib/rom.h>
#include <ti/devices/cc13x0/driverlib/vims.h>
//extern uint32_t ROM_FlashProgram(uint8_t* buffer, uint32_t address, uint32_t count);
//#include <ti/devices/cc13x0/driverlib/flash.h>

void flashSetConfigurationLimit(uint16_t length) {
	flashConfigurationEnd = 0x0001e000 + length;
}

uint8_t flashGetLastConfiguration() {
	uint8_t* p = flashConfigurationData + (flashPageSize - 1);
	uint8_t  last = 255; // nothing found yet;

	while ((uint32_t) p > flashConfigurationEnd) {
		uint8_t current = *p;
		System_printf("checking address 0x%08x = %d\n",(uint32_t) p,current);
		if (current == 255) break; // no more programmed bytes here
		last = current;
		p--;
	}
	return last;
}

void flashWriteLastConf(int8_t c) {
	uint8_t* p = flashConfigurationData + (flashPageSize - 1);
	while ((uint32_t) p > flashConfigurationEnd) {
		uint8_t current = *p;
		if (current == 255 && current != c) {
			//ROM_FlashProgram(&configuration, (uint32_t) p, 1);
		    VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);
		    while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
		    CPUcpsid();
			HapiProgramFlash(&c, (uint32_t) p, 1);
			// uint32_t rc = HapiProgramFlash(&c, (uint32_t) p, 1);
			CPUcpsie();
		    VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
		    //System_printf("*** flash write result %d ***\n",rc);
			return;
		}
		p--;
	}
}

/*****************************************************************************/
/* PROTOCOL HANDLERS                                                         */
/*****************************************************************************/
uint64_t tagId = 0xFFFFFFFF;
int32_t tag_setId(uint64_t id) {
	if (tagId==0xFFFFFFFF) {
		//System_printf("setting tag id to %d (lower digits)\n",(uint32_t) (id%1000000));
		//System_printf("setting tag id to %8x:%08x\n",(uint32_t) (id>>32), (uint32_t) id);
		//System_exit(1);
		tagId = id;
		return 0; // all okay
	}
	if (tagId == id) return 0; // packet destination is us
	return -1; // packet for some other tag
}
/*****************************************************************************/
/* PROTOCOL HANDLERS                                                         */
/*****************************************************************************/

#define DATA_PROTOCOL_ATLAS                     16
#define DATA_PROTOCOL_VILDEHAYE                  8
#define DATA_PROTOCOL_VILDEHAYE_BEACON           3
#define DATA_PROTOCOL_VILDEHAYE_BEACON_RESPONSE  2
#define DATA_PROTOCOL_VILDEHAYE_SESSION          1

#define SLOT_ACTION_TX     1
#define SLOT_ACTION_TX_ADV 2
#define SLOT_ACTION_RX     3
#define SLOT_ACTION_NOP    4

typedef struct slot_action_st {
	uint8_t  action;
	uint8_t  opts;
	uint8_t* packet;
	uint16_t packet_len;
} slot_action_t;

//typedef slot_action_t* (*slot_preparer_t)(uint8_t configuration, uint8_t setup, uint16_t slot);

uint16_t tagPeriodMs = 1000;

#ifdef obsolete
/*****************************************************************************/
/* RANDOM BEACON                                                             */
/*****************************************************************************/

uint16_t randomBeaconExpectation = 0xFFFF;
uint16_t randomBeaconSetup;
uint8_t  randomBeaconState;
 int8_t  randomBeaconDbm  ;

void random_beacon_init(const uint16_t* configurationData, uint16_t configurationDataLength) {
  tagPeriodMs             = configurationData[ 0 ];
  randomBeaconExpectation = configurationData[ 1 ];
  randomBeaconSetup       = configurationData[ 2 ];
  uint16_t dbmo           = configurationData[ 3 ];
	 int8_t dbm  = ( dbmo     & 0xFF);
	uint8_t opts = ((dbmo>>8) & 0xFF);
  System_printf("tag period=%d ms beacon expectation %d setup %d\n",tagPeriodMs,randomBeaconExpectation,randomBeaconSetup);
}
#endif

/*****************************************************************************/
/* TAG SCHEDULE                                                              */
/*****************************************************************************/

#ifndef MAX_CONFIGURATIONS
#define MAX_CONFIGURATIONS 4
#endif

//static uint8_t radioSetupsCount;

slot_action_t* vildehayeGetSlotAction(uint8_t configuration, uint8_t setup, uint16_t slot);
slot_action_t* atlasGetSlotAction(uint8_t configuration, uint8_t setup, uint16_t slot);

	// skip radio setups
//slot_preparer_t setupPreparers[ MAX_RADIO_SETUPS ];

/*
 * Each configuration is descibed by an array that specifies frame length (in tag-period units)
  * and the slot allocation (start slot, step, and count limit) for each radio setup.
 * Note that the limit is on the number of slots used by the setup, it is not the largest
 * slot index allowed (the end index).
 */
#define CONF_FRAME_LEN     0
#define SETUP_START(SETUP) (1+((SETUP)*5)+0)
#define SETUP_STEP(SETUP)  (1+((SETUP)*5)+1)
#define SETUP_LIMIT(SETUP) (1+((SETUP)*5)+2)
#define SETUP_DBM(SETUP)   (1+((SETUP)*5)+3)
#define SETUP_OPTS(SETUP)  (1+((SETUP)*5)+4)
uint16_t configurations[MAX_CONFIGURATIONS][1+(MAX_RADIO_SETUPS*5)];

uint8_t configurationsCount;
 int8_t  currentConfiguration = 0; // default
//uint16_t currentFrameLength;
uint16_t slotInFrameIndex;
 int16_t nextSlotForSetup [ MAX_RADIO_SETUPS ];
uint16_t slotStepForSetup [ MAX_RADIO_SETUPS ];
uint16_t slotCountForSetup[ MAX_RADIO_SETUPS ];
uint16_t slotLimitForSetup[ MAX_RADIO_SETUPS ];
 int8_t  slotDbmForSetup  [ MAX_RADIO_SETUPS ];
uint8_t  slotOptsForSetup [ MAX_RADIO_SETUPS ];

 int8_t slotRadioSetup; // which radio setup is used by this slot

 slot_action_t* slotActionPtr; // pointer to what to do in this slot

 /*
  *
  */

#ifdef obsolete
static const uint32_t radioData[] = {
	RPE(DATA_PROTOCOL_ATLAS,            MODULATION_TYPE_FSK, 10,1,32,DATA_ENCODING_PLAIN  ), 0x333C3C33, 915000000, 380859,  999756, 2200000,
	RPE(DATA_PROTOCOL_VILDEHAYE_BEACON, MODULATION_TYPE_FSK, 10,1,32,DATA_ENCODING_CC1310_FASTLRM), 0x930B51DE, 915000000, 175000,  500000, 1410000,
  RPE(DATA_PROTOCOL_VILDEHAYE_SESSION,MODULATION_TYPE_GFSK,10,1,32,DATA_ENCODING_PLAIN  ), 0x333C3C33, 915000000, 253906,  499878, 1100000,
};
static const int radioDataLength = RADIO_SETUP_BUFFER_SIZE(3);

static const uint16_t confData[] = {
  1000,
	  60,
		   0,  3, 20, 10,
			 1,  3, 20, 10,
			 2,  3, 20, 10,
    60,
		   0, 20,  3, 10,
			55,  2,  1, 10,
			57,  2,  2, 10,
};
const int confDataLength = (1 + 2*(1+3*4)) * sizeof(uint16_t);
#endif

void tag_gotoConfiguration(uint8_t c) {
	// TODO: right now only works in initial configuration
	//System_printf("goto configuration %d\n",c);
	if (currentConfiguration==c) return;

	currentConfiguration = c;
	slotInFrameIndex=65535; // this causes a new frame to start
}

uint8_t wakeupTriggers[4][4];
uint8_t wakeupTriggersCount = 0;
void tag_triggerWakeupSetup(const uint8_t* wtarray) {
	System_printf("wakeup trigger (%d,%d) -> %d\n",wtarray[0], wtarray[1], wtarray[2]);
	wakeupTriggers[ wakeupTriggersCount ][0] = wtarray[0]; // fromConf
	wakeupTriggers[ wakeupTriggersCount ][1] = wtarray[1]; // fromSetup
	wakeupTriggers[ wakeupTriggersCount ][2] = wtarray[2]; // toConf
	wakeupTriggers[ wakeupTriggersCount ][3] = wtarray[3]; // trigger index
	wakeupTriggersCount++;
}

uint32_t lastWakeupSecs = 0;
uint32_t noWakeupTriggerSecs;
 int8_t  noWakeupTriggerConf = -1;
uint8_t  noWakeupTriggerArmed = 0;
void tag_triggerNoWakeupSetup(const uint16_t* nwtarray) {
	noWakeupTriggerConf  = nwtarray[0]; // toConf
	noWakeupTriggerSecs  = nwtarray[1]; // secs
	noWakeupTriggerSecs += (nwtarray[2] * 3600 ); // hours
	noWakeupTriggerSecs += (nwtarray[2] * 3600 * 24 ); // days
	//lastWakeupSecs = 0;
	noWakeupTriggerArmed = 1;
	System_printf("no wakeup trigger %d after %ds\n",noWakeupTriggerConf, noWakeupTriggerSecs);
}

static int8_t wakeupTriggerConf = -1; // configuration to go to
void tag_triggerWakeup(uint8_t trigger) {
	uint8_t i;
	lastWakeupSecs = Seconds_get();
	if (noWakeupTriggerConf!=-1) {
		noWakeupTriggerArmed = 1; // start counting
	}
	for (i=0; i<wakeupTriggersCount; i++) {
		if (wakeupTriggers[i][0] == currentConfiguration
				&& wakeupTriggers[i][1] == slotRadioSetup
				&& wakeupTriggers[i][3] == trigger) {
			wakeupTriggerConf = wakeupTriggers[i][2];
			System_printf("wakup -> %d\n",wakeupTriggerConf);
			return;
		}
	}
}

/*
 * This function is called at the end of a slot to prepare the schedule for the next slot.
 */

void scheduleAdvance() {
	uint8_t i;

	 //System_printf("0 conf %d slot %d setup %d\n",currentConfiguration, slotInFrameIndex,slotRadioSetup);
	uint8_t newFrame = 0;
	if (slotInFrameIndex>=configurations[currentConfiguration][CONF_FRAME_LEN]-1) newFrame = 1;
	if (wakeupTriggerConf != -1) { // we got a wakeup packet
		flashWriteLastConf(wakeupTriggerConf);

		newFrame = 1;
		currentConfiguration = wakeupTriggerConf;
		wakeupTriggerConf = -1;
		System_printf("wup->%d!\n",currentConfiguration);
	}
	if (noWakeupTriggerArmed && (Seconds_get() - lastWakeupSecs) > noWakeupTriggerSecs) {
		noWakeupTriggerArmed = 0; // disarm
		newFrame = 1;
		currentConfiguration = noWakeupTriggerConf;
		wakeupTriggerConf = -1;
		System_printf("wdn->%d!\n",currentConfiguration);
	}

	//if (slotInFrameIndex>=configurations[currentConfiguration][CONF_FRAME_LEN]-1) { // this frame is done
	if (newFrame) { // this frame is done
		System_printf("nf: %d!\n",currentConfiguration);
		// apply triggers here

		// now currentConfiguration is setup correctly, initialize the frame.
		slotInFrameIndex = 0; // we start a new frame
		for (i=0; i<radioSetupsCount; i++) {
			nextSlotForSetup [i] = configurations[currentConfiguration][SETUP_START(i)];
			slotStepForSetup [i] = configurations[currentConfiguration][SETUP_STEP (i)];
			slotLimitForSetup[i] = configurations[currentConfiguration][SETUP_LIMIT(i)];
			slotCountForSetup[i] = 0;
			//System_printf("* %d\n",i);
		}
	} else {
		if (slotRadioSetup >= 0) { // -1 means no radio setup uses the slot
			slotCountForSetup[slotRadioSetup]++;
			if (slotCountForSetup[slotRadioSetup] >= slotLimitForSetup[slotRadioSetup]) { // we reached the limit, drop the setup from frame
				nextSlotForSetup[slotRadioSetup] = -1;
			} else {
				nextSlotForSetup[slotRadioSetup] += slotStepForSetup[slotRadioSetup];
			}
			//System_printf("next[%d] = %d\n",slotRadioSetup,nextSlotForSetup[slotRadioSetup]);
		}

		//System_printf("inc\n");
	  slotInFrameIndex++;
	}

	 //System_printf("1 conf %d slot %d setup %d\n",currentConfiguration, slotInFrameIndex,slotRadioSetup);

	 /*
	 * Now the frame is set up, the slot index is correct for the next slot,
	 * and the radio setup for the last slot has been updated.
	 * Find out which setup will use the next slot.
	 */
	// the next slot and tell it to prepare.

	for (i = 0; i<radioSetupsCount; i++) {
		if (slotInFrameIndex == nextSlotForSetup[i]) {
			// this is the setup we will use!
			slotRadioSetup = i;

			System_printf("c%d s%d r%d\n",currentConfiguration,slotInFrameIndex,slotRadioSetup);
			switch (radioSetupDataProtocol[ slotRadioSetup ]) {
			case DATA_PROTOCOL_ATLAS:
				slotActionPtr = atlasGetSlotAction(currentConfiguration, slotRadioSetup, slotInFrameIndex);
				//System_printf("3A conf %d slot %d setup %d\n",currentConfiguration, slotInFrameIndex,slotRadioSetup);
				return;
			case DATA_PROTOCOL_VILDEHAYE:
			case DATA_PROTOCOL_VILDEHAYE_BEACON:
			case DATA_PROTOCOL_VILDEHAYE_BEACON_RESPONSE:
			case DATA_PROTOCOL_VILDEHAYE_SESSION:
				slotActionPtr = vildehayeGetSlotAction(currentConfiguration, slotRadioSetup, slotInFrameIndex);
				//System_printf("3V conf %d slot %d setup %d\n",currentConfiguration, slotInFrameIndex,slotRadioSetup);
				return;
			default:
				//System_printf("3X conf %d slot %d setup %d\n",currentConfiguration, slotInFrameIndex,slotRadioSetup);
				return;
			}
		}
	}

	slotRadioSetup = -1; // no radio setup uses the next slot.
	System_printf("oops c%d s%d r%d\n",currentConfiguration,slotInFrameIndex,slotRadioSetup);
	//System_printf("3Y conf %d slot %d setup %d\n",currentConfiguration, slotInFrameIndex,slotRadioSetup);
}

void schedule_init(const uint16_t* configurationData, uint16_t configurationDataLength) {
	 tagPeriodMs = configurationData[ 0 ];
	 configurationsCount  = 0;
	 uint8_t confIndex = 0; // may come from gotoConfiguration

	 System_printf("tag period=%d ms initial configuration %d\n",tagPeriodMs,confIndex);

	 uint16_t i = 1;
	 uint16_t l = configurationDataLength / sizeof(uint16_t);
	 while (i<l) {
		 //configurations[ currentConfiguration ][0] = configurationData[ i++ ];
		 uint8_t j;
		 for (j=0; j<(1+(radioSetupsCount*5)); j++) {
			 //System_printf(":: %d -> %d\n",j,configurationData[ i ]);
			 configurations[ confIndex ][ j ] = configurationData[ i++ ];
		 }
		 confIndex++;
	 }
	 configurationsCount = confIndex;

	 System_printf("#conf=%d #setups=%d\n",configurationsCount,radioSetupsCount);

	 for (confIndex = 0; confIndex<configurationsCount; confIndex++) {
		 System_printf("%d frame len=%d\n",confIndex,configurations[confIndex][CONF_FRAME_LEN]);
		 for (i=0; i<radioSetupsCount; i++)
			 System_printf("  start=%d step=%d limit=%d dbm=%d opts=%x\n",
					 configurations[confIndex][SETUP_START(i)],
					 configurations[confIndex][SETUP_STEP(i)],
					 configurations[confIndex][SETUP_LIMIT(i)],
					 configurations[confIndex][SETUP_DBM(i)],
					 configurations[confIndex][SETUP_OPTS(i)]
					 );
	 }

	 /*** now setup initial state ***/

	 //currentConfiguration = 0;
	 slotInFrameIndex = 65535; // over the frame length, will cause frame initialization
	 scheduleAdvance();
	 System_printf("schedule init ended\n");
}

/*****************************************************************************/
/* PROTOCOL HANDLERS: ATLAS                                                  */
/*****************************************************************************/

static uint8_t atlas_code[2048];

static uint8_t* atlas_codes[4];
static uint16_t atlas_code_lengths[4];
static uint16_t atlas_code_ptr = 0;

void atlas_setCode(uint8_t index, const uint8_t* p, uint16_t len) {
	int32_t l = len;
	if (l > sizeof(atlas_code) - atlas_code_ptr) l = sizeof(atlas_code) - atlas_code_ptr;
	if (l < 0) return;

	System_printf("setting atlas code %d to length %d (requested %d)\n",index,l,len);
	memcpy(atlas_code + atlas_code_ptr, p, l);
	atlas_codes       [index] = atlas_code + atlas_code_ptr; //p;
	// TODO: check if the address is in flash and modify only then
	//atlas_codes       [index] = (uint8_t*) (((uint32_t) p) | 0xA0000000); // modify the flash address for the radio core
	atlas_code_lengths[index] = l;
	atlas_code_ptr += l;
}

static slot_action_t atlasSlotAction = {
		.action     = SLOT_ACTION_TX_ADV,
		.opts       = 0,
		.packet     = 0,
		.packet_len = 0,
};

slot_action_t* atlasGetSlotAction(uint8_t configuration, uint8_t setup, uint16_t slot) {
	// int8_t dbm  = ( configurations[configuration][SETUP_DBMO(setup)]     & 0xFF);
	uint8_t opts        = ((configurations[configuration][SETUP_OPTS(setup)]) & 0xFF);
	uint8_t index       = opts & VH_TAGSTATE_ATLAS_CODE_INDEX_MASK;
	uint8_t invert      = (opts & VH_TAGSTATE_ATLAS_INVERTED)    != 0;
	uint8_t random_sign = (opts & VH_TAGSTATE_ATLAS_RANDOM_SIGN) != 0;

	//System_printf("atlas code %d len %d invert %d\n",index,atlas_code_lengths[index],invert);
	atlasSlotAction.packet = atlas_codes[index];
	atlasSlotAction.packet_len = atlas_code_lengths[index];

	atlasSlotAction.opts = opts & (~VH_TAGSTATE_ATLAS_CODE_INDEX_MASK);

	System_printf("atlas code %02x %02x %02x len %d opts %02x\n",
			(atlasSlotAction.packet)[0],
			(atlasSlotAction.packet)[1],
			(atlasSlotAction.packet)[2],
			atlas_code_lengths[index],atlasSlotAction.opts);

	return &atlasSlotAction;
}

/*****************************************************************************/
/* PROTOCOL HANDLERS: VILDE HAYE                                             */
/*****************************************************************************/
#include <vildehaye.h>
#include <random.h>

//uint8_t vildehayeIdPacket[] = { 0x62, 0x80, 0xc8, 0xe8, 0x03, 0x60, 0x09, 0xd6, 0xd7, 0x3b, 0x25, 0x4d, };
uint8_t vildehayePayload[256]; // TODO mark it as in use or not, or switch to buffers

static slot_action_t vildehayeSlotAction = {
		.action     = SLOT_ACTION_TX,
		.opts       = 0,
		.packet     = vildehayePayload, // vildehayeIdPacket,
		.packet_len = 12,
};

static vildehaye_packet_t vildehayePacket;
//static uint8_t            vildehayePacketValid = 0;
static uint8_t            vildehayeInSession   = 0;
static uint32_t           vildehayeSeqno       = 0; // last sequence number we transmitted
static uint8_t            vildehayeRetransmissions;

typedef enum {
	SYN = 0,
	ACK = 1,
	NACK = 2,
	UNKNOWN = 3,
} vildehayeProtocolState;

uint8_t tag_sessionSeqno(uint32_t seqno) {
	vildehayeProtocolState state = UNKNOWN;

	if (!vildehayeInSession) state = SYN;
	if ( vildehayeInSession && seqno == vildehayeSeqno+1) state = ACK;
	if ( vildehayeInSession && seqno == vildehayeSeqno-1) state = NACK;

	switch (state) {
	case UNKNOWN:
		// wrong response, ignore
		break;
	case SYN:
		vildehayeInSession = 1;
		vildehayeSeqno = seqno + 1;
		vildehayeRetransmissions = 0;
		break;
	case ACK:
		// continue with the protocol
		break;
	case NACK:
		break;
	}

	if (state==SYN) {
	} else {

	}
	return 0; // incorrect seq number
}

//uint32_t xxxI2CTest;

static void vildehayeCreateBeaconPacket(uint16_t opts, uint16_t period) {
	uint8_t flags = opts & 0xFF;
	flags |= (currentConfiguration & 0x03); // we can announce configurations 0 to 3
	System_printf("flags %02x %d\n",flags,currentConfiguration);
	vildehayeInitPacket(&vildehayePacket, vildehayePayload, 0, 256);
	vildehayeAddTagState(&vildehayePacket, flags, 256-8, period); // tag state
	vildehayeAddUIntWithHeader(&vildehayePacket, VH_SOURCE_ID, tagId);

	if ((opts & VH_OPTIONS_BEACON_WAKEUP_MASK) != 0) {
		System_printf("beaconing wakeup %d\n", (opts & VH_OPTIONS_BEACON_WAKEUP_MASK) >> 8);
		vildehayeAddUIntWithHeader(&vildehayePacket, VH_WAKEUP, (opts & VH_OPTIONS_BEACON_WAKEUP_MASK) >> 8);
	}

	if ((opts & VH_OPTIONS_BEACON_CLOCK) != 0) {
		vildehayeAddHeader (&vildehayePacket, VH_LOCAL_CLOCK, 4); // local clock
		vildehayeAddUInt32 (&vildehayePacket, Seconds_get());
		//vildehayeAddUInt32 (&vildehayePacket, xxxI2CTest);
	}

	if ((opts & VH_OPTIONS_BEACON_SECS_SINCE_WAKEUP) != 0) {
		vildehayeAddHeader (&vildehayePacket, VH_SECS_SINCE_WAKEUP, 4); // local clock
		vildehayeAddUInt32 (&vildehayePacket, (Seconds_get() - lastWakeupSecs));
	}
	//System_printf("exiting\n");
	//System_exit(1);
}

slot_action_t* vildehayeGetSlotAction(uint8_t configuration, uint8_t setup, uint16_t slot) {

	//uint16_t count  = configurations[configuration][SETUP_LIMIT(setup)];
	uint16_t step   = configurations[configuration][SETUP_STEP(setup)];
	uint16_t start  = configurations[configuration][SETUP_START(setup)];

	//uint16_t dbm  = configurations[configuration][SETUP_DBM(setup)];
	uint16_t opts = configurations[configuration][SETUP_OPTS(setup)];

	//System_printf("==> %04x\n",opts);

	uint16_t period = tagPeriodMs * step;

	uint16_t parity = ((slot-start)/step) & 0x0001;

	if (opts & VH_TAGSTATE_LISTEN_ONLY) {
		vildehayeSlotAction.action = SLOT_ACTION_RX;
		return &vildehayeSlotAction;
	}

	if ( (opts & VH_OPTIONS_BEACON_RANDOM_SLOT)) {
		uint16_t frameLen = configurations[configuration][CONF_FRAME_LEN];
		// transmit probability is 1/frameLen
		int32_t r = random() % frameLen; // between 0 and frameLen-1
		//System_printf("random %d expectation %d\n",r,frameLen);
		// now do it (or skip)
		if (r<=0) {
			System_printf("!\n");
			vildehayeCreateBeaconPacket(opts, period);

			vildehayeSlotAction.packet_len = vildehayePacket.length;
			vildehayeSlotAction.action = SLOT_ACTION_TX;
		} else {
			vildehayeSlotAction.action = SLOT_ACTION_NOP;
		}
		return &vildehayeSlotAction;
	}

	//System_printf("*** *** conf %d setup %d period %d slot %d parity %d\n",configuration,setup,period,slot,parity);

	uint8_t reasonToReceive = 0;
	reasonToReceive |= (opts & VH_TAGSTATE_SESSION_OK);
	reasonToReceive |= (opts & VH_TAGSTATE_NON_SESSION_CMD_OK);

	if (parity==0 || !reasonToReceive) {
		vildehayeCreateBeaconPacket(opts, period);

		//vildehayeInitPacket(&vildehayePacket, vildehayePayload, 0, 256);
		//vildehayeAddTagState(&vildehayePacket, opts & 0xFF, 256-8, period); // tag state
		//vildehayeAddUIntWithHeader(&vildehayePacket, VH_SOURCE_ID, tagId);
		//vildehayeAddHeader (&vildehayePacket, VH_LOCAL_CLOCK, 4); // local clock
		//vildehayeAddUInt32 (&vildehayePacket, Seconds_get());

		vildehayeSlotAction.packet_len = vildehayePacket.length;
		vildehayeSlotAction.action = SLOT_ACTION_TX;
	} else {
		vildehayeSlotAction.action = SLOT_ACTION_RX;
	}
	return &vildehayeSlotAction;
}

//void vildehayeHandlePacket(uint8_t setup, uint8_t packet, uint16_t len) {
//}

/*****************************************************************************/
/* GLOBAL DEFINITIONS                                                        */
/*****************************************************************************/

// #define PACKET_INTERVAL     (uint32_t)(4000000*5.0f) /* Set packet interval to 5000ms */

/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;

#if 0
//uint32_t time;
//static uint8_t packet[PAYLOAD_LENGTH];
#define ID_PACKET_LENGTH   16
#define COMM_PACKET_LENGTH 16
//static uint8_t id_packet[ID_PACKET_LENGTH+1] =     { ID_PACKET_LENGTH, 2, 3   };
#define ID_PACKET_LENGTH   12
static uint8_t id_packet[] = { ID_PACKET_LENGTH, 0x62, 0x80, 0xc8, 0xe8, 0x03, 0x60, 0x09, 0xd6, 0xd7, 0x3b, 0x25, 0x4c, };
static uint8_t comm_packet[COMM_PACKET_LENGTH+1] = { COMM_PACKET_LENGTH, 2, 3 };
//const uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//static uint16_t seqNumber;


typedef struct slot_st {
	int8_t   setup;
	uint8_t* packet;
	uint16_t packet_length;
} slot_t;

slot_t slots[] = {
	{ RADIO_SETUP_TOA,  atlas_code , 1024                 },
	{ RADIO_SETUP_ID,   id_packet  , ID_PACKET_LENGTH  +1 },
	{ RADIO_SETUP_COMM, comm_packet, COMM_PACKET_LENGTH+1 },
	{ -1 },
};

uint8_t slot_index = 0;

uint8_t comm_state = 0; // tx

#endif

/*****************************************************************************/
/* PERIODIC WAKEUPS                                                          */
/*****************************************************************************/

//uint8_t tagPeriod = 1;

#ifdef SEMWAIT
Semaphore_Handle wakeupSemaphore;

static void wakeupFunction(UArg arg) {
	Semaphore_post(wakeupSemaphore);
}
#endif

#define UNKNOWN 99
#define TX_OK   0
#define TX_FAIL 1
#define RX_OK   2
#define RX_FAIL 3
#define RX_TIMEOUT 4

/*****************************************************************************/
/* VOLTAGE THRESHOLD                                                         */
/*****************************************************************************/

static uint32_t voltageThreshold = 0;
void tag_setVoltageThreshold( uint32_t vth ) { voltageThreshold = vth; }

/*****************************************************************************/
/* MAIN TAG TASK FUNCTION                                                    */
/*****************************************************************************/

/*
	Error handling function – replaces the default while(1) in RF driver.
	This is necessary for certain versions of TI-RTOS, not all; see
	http://processors.wiki.ti.com/index.php/CC1310_rev_B_PCN_information
*/
static void errorCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e) {
    if ((int32_t)ch == RF_ERROR_CMDFS_SYNTH_PROG) {
        // Handle error
        // If CMD_FS is executed standalone, re-start CMD_FS in this function if there is time. If not, handle as in previous example
        // When CMD_FS is chained with RX/TX commands, the RX/TX is executed in parallel with this callback by the radio and will subsequently fail. Error can be handled as in previous example.
    }
}

#include <i2c_sensors.h>

static void tagTaskFunction(UArg arg0, UArg arg1) {

	/*
	i2cSensorsInit();
	uint8_t xxx;
	bool i2cSuccess;
	i2cSuccess = mpu9150WhoAmI(&xxx,0 ); // this one works with Alex's MPU9
	xxxI2CTest = xxx;
	xxxI2CTest |= (i2cSuccess <<  8);
	xxxI2CTest |= (xxx        << 16);
  i2cSuccess = mpu9150WhoAmI(&xxx,1 ); // this one does not.
	xxxI2CTest |= (i2cSuccess << 24);
*/

	//uint8_t i;
	//for (i = 0; i < PAYLOAD_LENGTH; i++) {
	//	packet[i] = rand();
	//}
  // setup periodic wakeup

	//uint8_t outcome;

#ifdef SEMWAIT
	Semaphore_Params semParams;
	semParams.mode = Semaphore_Mode_BINARY;
  Semaphore_Params_init(&semParams);
  wakeupSemaphore = Semaphore_create(0, &semParams, NULL);

  Clock_Params clockParams;
	Clock_Handle sleepClock;

	Clock_Params_init(&clockParams);
	clockParams.period = (uint32_t) ((double) tagPeriodMs / (1e-3*Clock_tickPeriod));
	clockParams.startFlag = TRUE;
	clockParams.arg = (UArg) 0;
	sleepClock = Clock_create(wakeupFunction, clockParams.period, &clockParams, NULL);
	if (sleepClock == NULL) {
	 System_abort("Clock create failed");
	}

	System_printf("period = %d ms = %d ticks * %dus per tick\n", tagPeriodMs, clockParams.period, Clock_tickPeriod);
  /* Request access to the radio */
#else
	System_printf("period = %d\n", tagPeriodMs);
#endif

  RF_Params rfParams;
  RF_Params_init(&rfParams);
  rfParams.nInactivityTimeout = 100; // in usecs
  rfParams.pErrCb = errorCallback; // only necessary in certain versions of TI-RTOS

  Types_FreqHz tsf;
  Timestamp_getFreq(&tsf);

  //uint32_t prev_time;
  uint32_t rat_time  = 0;
  uint8_t  rat_known = 0;
  //uint16_t rat_slack      =  400; // 100us
  //uint16_t rat_slack_step =  400; // 100us
  //uint16_t rat_slack_max  = 4000; // 1ms

  int8_t previousSetup = -1;
  uint8_t radioOpen = 0;

  //leds_setBlinkCounter(0); // XXX debug random tag

  while (1) {
#ifdef SEMWAIT
		Semaphore_pend(wakeupSemaphore, BIOS_WAIT_FOREVER);
		System_printf("tag wakeup setup %d ...\n",slotRadioSetup);

#endif
	  //leds_off(LEDS_RX); // XXX debug random tag
	  //leds_on (LEDS_TX); // XXX debug random tag

#ifdef HALL_SENSOR
		if (Semaphore_getCount(scStopSemaphore)==1) { // we need to stop
		  Semaphore_pend(scStopSemaphore, BIOS_NO_WAIT);

			//System_printf("we are told to go to deep sleep\n");
		  if (radioOpen) RF_close(rfHandle);
		  radioOpen = 0;
		  rat_known = 0; // the timer will be useless when we wake up.
		  sensorcontroller_ack(); // tell the sensor controller that the system CPU is going down
		  Semaphore_pend(scStartSemaphore, BIOS_WAIT_FOREVER); // wait for wake up.

		  sensorcontroller_ack(); // tell the sensor controller that the system CPU is going down

			//System_printf("woke up!\n");
		}
#endif

		watchdog_pacify();

	  while ((slotRadioSetup == -1) || (slotActionPtr -> action)==SLOT_ACTION_NOP) {
  		rat_time += 4000*tagPeriodMs;
  		scheduleAdvance();
  		continue;
		}

		//System_printf("tag wakeup conf %d setup %d slot action %d len %d\n",currentConfiguration, slotRadioSetup, slotActionPtr->action, slotActionPtr->packet_len);
		//outcome = UNKNOWN;

	  //radio_time = RF_getCurrentTime();

		//System_printf("1 tag %d\n",slot_index);

	  //uint32_t rt0 = Timestamp_get32();

	  //leds_on(LEDS_RX); // XXX debug random tag

	  if ((radioOpen==0)
	  		|| (slotRadioSetup != previousSetup)
				|| ((slotActionPtr -> opts) != 0)     // for ATLAS inversions
				) {

		  if (radioOpen) RF_close(rfHandle);
		  radioOpen = 1;

	  	(radio_cmd_prop_div_setup[slotRadioSetup]).status = IDLE;
	  	rfHandle = RF_open(&rfObject, &(radio_mode[slotRadioSetup]), (RF_RadioSetup*)&(radio_cmd_prop_div_setup[slotRadioSetup]), &rfParams);

	  	//System_printf("2@ %d->%d\n",previousSetup,slotRadioSetup);

	  	(radio_cmd_prop_div_setup[slotRadioSetup]).status = IDLE;
	  	// ATLAS reversal
	  	if (((slotActionPtr -> opts) & VH_TAGSTATE_ATLAS_INVERTED) != 0) {
	  		(radio_cmd_prop_div_setup[slotRadioSetup]).formatConf.bBitReversal = 0x1;
	  	} else {
	  		(radio_cmd_prop_div_setup[slotRadioSetup]).formatConf.bBitReversal = 0x0;
	  	}
	  	if (((slotActionPtr -> opts) & VH_TAGSTATE_ATLAS_RANDOM_SIGN) != 0) {
	  		uint8_t invert = (random() & 0x04) != 0; // pick a bit in a random word
	  		System_printf("*** %d ***\n",invert);
	  		(radio_cmd_prop_div_setup[slotRadioSetup]).formatConf.bBitReversal = invert;
	  	}
	  	RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_div_setup[slotRadioSetup]), RF_PriorityNormal, NULL, 0);
	  	if ((radio_cmd_prop_div_setup[slotRadioSetup]).status != PROP_DONE_OK) System_printf("CMD_SETUP> %04x\n",(radio_cmd_prop_div_setup[slotRadioSetup]).status);
			//System_printf("3@\n");
	  //}
		//uint32_t rt1 = Timestamp_get32();
	  //if (slotRadioSetup != previousSetup) {
	  	/* Set the frequency */
	  	(radio_cmd_fs[slotRadioSetup]).status = IDLE;
	  	RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_fs[slotRadioSetup]), RF_PriorityNormal, NULL, 0);
	  	if ((radio_cmd_fs[slotRadioSetup]).status != DONE_OK) System_printf("CMD_FS> %04x\n",(radio_cmd_fs[slotRadioSetup]).status);
	  	//System_printf("CMD_FS> %04x\n",(radio_cmd_fs[slotRadioSetup]).status);
			//System_printf("4@\n");

		  previousSetup = slotRadioSetup;
	  }

	  uint32_t rat_now = RF_getCurrentTime();
	  uint32_t rat_next;

	  if (rat_known==0) {
	  	rat_time  = rat_now + 4000000; // rat_slack; // 20000; // 5ms; rat_slack; // add 100us
	  	rat_known = 1;
	  }

	  uint32_t wait_us = ratDiffToUs(rat_now,rat_time);

	  //uint32_t rt2 = Timestamp_get32();

	  rfc_propRxOutput_t rxout;
	  //RF_EventMask result;

	  // NOP never gets here
#if 0
	  //if (slotActionPtr -> action != SLOT_ACTION_NOP)
	  System_printf("* %d %d %d\n",RF_getCurrentTime(), rat_time, slotInFrameIndex);

	  // Do we have enough time until the next slot deadline?
	  // If not, skip. Time is 2.5ms to let the synthesizer stabilize
	  if (RF_getCurrentTime() >= rat_time - 10000) { // RAT runs at 4MHz * 2.5ms = 10k
	  	previousSetup = slotRadioSetup;
  		rat_time = rat_time + 4000*tagPeriodMs;
	  	scheduleAdvance();
			System_printf("5@\n");
	  	continue;
	  }
#endif

	  //leds_off(LEDS_TX); // XXX debug random tag

	  //System_printf("t=%d v=%f\n",batmonTemp(), ((float) batmonVoltage()) / 256.0f);

	  /*
	   * If there is more than 10ms until the next action time,
	   * and if a voltage threshold is set (not zero), we sleep until 10ms prior and then
	   * test the voltage. If it is below the threshold, we skip the cycle.
	   * This is meant mostly for Silver Oxide batteries, in which a capacitor is really
	   * powering the tag during receive and transmit operations.
	   */
	  //float voltageThresholdF = 3.0f;
	  //uint32_t voltageThreshold = (uint32_t) ( voltageThresholdF * 256.0f );
	  /*
	  System_printf("t=%d v=%f %d %d\n",batmonTemp(), ((float) batmonVoltage()) / 256.0f,RF_getCurrentTime(),rat_time);
	  if (voltageThreshold != 0 && RF_getCurrentTime() + 10*4000 < rat_time) { // 10ms
	  	uint32_t sleep = (rat_time - 10*4000 - RF_getCurrentTime()) / 4; // microseconds because RAT runs at 4MHz
	  	System_printf("sleep=%d us\n",sleep);
	  	if (sleep > 10000000) sleep = 10000000; // 10s
	  	sleep = sleep / Clock_tickPeriod;
	  	System_printf("sleep=%d ticks\n",sleep);
	  	Task_sleep(sleep);
	  	// after wake up
	  	if (batmonVoltage() < voltageThreshold) {
	  	  System_printf("voltage too low t=%d v=%f\n",batmonTemp(), ((float) batmonVoltage()) / 256.0f);
	  	  rat_next = rat_time + 4000*tagPeriodMs;
	  	  scheduleAdvance();
	  	  rat_time = rat_next;
	  	  continue;
	  	} else {
	  	  System_printf("voltage high enough t=%d v=%f\n",batmonTemp(), ((float) batmonVoltage()) / 256.0f);
	  	}
	  }
	  */
	  //System_exit(1);

//#ifndef TESTING_SO_FAILURE
//	  SysCtrl_DCDC_VoltageConditionalControl();
//#endif

	  /* Make sure no RAT wrap around occurs while we wait for RAT; it does not work */
	  if (rat_now > rat_time) { // wrap around
	  	if (wait_us > 10000) // more than 10ms? if so, sleep until 10ms before deadline
	  		Task_sleep((wait_us-10000) / Clock_tickPeriod);
	  }

	  uint8_t action = slotActionPtr -> action;

	  // If less than 10ms, skip the slot; this also takes care of the case of wraparound of less than 10ms
	  if (wait_us <= 10000) action = SLOT_ACTION_NOP;

	  switch (action) {
	  case SLOT_ACTION_NOP:
  		rat_next = rat_time + 4000*tagPeriodMs;
  		break;
	  case SLOT_ACTION_RX:
	  	// just skip RAT time
  		rat_next = rat_time + 4000*tagPeriodMs;

  		(radio_cmd_prop_rx[slotRadioSetup]).status = IDLE;
#ifdef SEMWAIT
	  	(radio_cmd_prop_rx[slotRadioSetup]).startTrigger.triggerType = TRIG_NOW; // no reason to wait
	  	(radio_cmd_prop_rx[slotRadioSetup]).startTrigger.pastTrig = 1;
#else
	  	(radio_cmd_prop_rx[slotRadioSetup]).startTrigger.triggerType = TRIG_ABSTIME;
	  	(radio_cmd_prop_rx[slotRadioSetup]).startTrigger.pastTrig = 0;
	  	(radio_cmd_prop_rx[slotRadioSetup]).startTime = rat_time;
#endif
	  	(radio_cmd_prop_rx[slotRadioSetup]).pQueue = &dataQueue;
	    (radio_cmd_prop_rx[slotRadioSetup]).pOutput = (uint8_t*) &rxout;
	    (radio_cmd_prop_rx[slotRadioSetup]).maxPktLen = 100;
	    (radio_cmd_prop_rx[slotRadioSetup]).rxConf.bAutoFlushIgnored = 1;  /* Discard ignored packets from Rx queue */
	    (radio_cmd_prop_rx[slotRadioSetup]).rxConf.bAutoFlushCrcErr = 1;   /* Discard packets with CRC error from Rx queue */
	    (radio_cmd_prop_rx[slotRadioSetup]).pktConf.bRepeatOk = 0; // return if we received a packet
	    (radio_cmd_prop_rx[slotRadioSetup]).pktConf.bRepeatNok = 1;
	    (radio_cmd_prop_rx[slotRadioSetup]).pktConf.endType = 1; // stop rx when end trigger arrives
	    (radio_cmd_prop_rx[slotRadioSetup]).endTrigger.triggerType  = TRIG_ABSTIME;
	    (radio_cmd_prop_rx[slotRadioSetup]).endTime                 = rat_time + (uint32_t) (0.015 * 4e6); // listen time
	    //(radio_cmd_prop_rx[slotRadioSetup]).endTrigger.triggerType  = TRIG_REL_START;
	    //(radio_cmd_prop_rx[slotRadioSetup]).endTime                 = (uint32_t) (0.008 * 4e6); // 2ms

	    /* Enter RX mode  */
	    //RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radio_setup]), RF_PriorityNormal, &callback, IRQ_RX_ENTRY_DONE);
	    RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[slotRadioSetup]), RF_PriorityNormal, NULL, 0);
	    switch (radio_cmd_prop_rx[slotRadioSetup].status) {
	    	case PROP_DONE_OK:
	    		//outcome = RX_OK;
	  	  	receiveGetBuffer();
	  	  	{
	  	  		uint8_t* rxpacket = buffers[ incomingBuffer.id ];
	  	  		//uint8_t elementLength = rxpacket[0];
	  	  		uint8_t payloadLength = rxpacket[1];
	  	  		//uint8_t rssi          = rxpacket[1+1+payloadLength];
	  	  		uint8_t tsp = 1+1+payloadLength+1;
	  	  	  uint32_t rx_ts;
	  	  		rx_ts =  rxpacket[tsp] | (rxpacket[tsp+1]<<8) | (rxpacket[tsp+2]<<16) | (rxpacket[tsp+3]<<24);
	  	  		//uint8_t  status       = rxpacket[1+1+payloadLength+1+4];

	  	  		//vildehayeHandlePacket(slotRadioSetup, rxpacket+2, payloadLength);

	  	  		vildehayeHandlePacketNaked(rxpacket+2,payloadLength);

	  	  		Mailbox_post(freeMailbox, &incomingBuffer, BIOS_WAIT_FOREVER);
	  	  		System_printf("rxout status=%04x ts=%lu %d %lu\n",radio_cmd_prop_rx[slotRadioSetup].status,rx_ts,rxout.nRxOk, rxout.timeStamp);
	  	  	}
//#if defined(CC1310_LAUNCHPAD)
	  	  	leds_blink(LEDS_RX, 1);
//#endif
	  	  	//tagId = 12345678900ll;
	    		break;
	    	case ERROR_PAST_START:
		  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs;
	  	  	leds_blink(LEDS_RX, 2);
	  	  	//tagId = 23456789100ll;
		  		break;
	    	case PROP_DONE_RXTIMEOUT:
	    		//outcome = RX_TIMEOUT;
//#if defined(CC1310_LAUNCHPAD)
	  	  	leds_blink(LEDS_RX, 2);
//#endif
	  	  	//tagId = 34567891200ll;
	    		break;
	    	default:
	    		//outcome = RX_FAIL;
//#if defined(CC1310_LAUNCHPAD)
	  	  	leds_blink(LEDS_RX, 3);
//#endif
	  	  	//tagId = 45678912300ll;
	    		break;
	    }
		  //if (( != PROP_DONE_OK)
		  //	System_printf("CMD_RX> %04x\n",(radio_cmd_prop_div_setup[slotRadioSetup]).status);
	  	break;

	  	// else we are in a transmit state, just go to transmit case
	  case SLOT_ACTION_TX:
	  	/* Send packet */
	  	(radio_cmd_prop_tx[slotRadioSetup]).status = IDLE;
	  	(radio_cmd_prop_tx[slotRadioSetup]).pktLen = slotActionPtr->packet_len;
	  	(radio_cmd_prop_tx[slotRadioSetup]).pPkt   = slotActionPtr->packet;
	  	(radio_cmd_prop_tx[slotRadioSetup]).startTrigger.triggerType = TRIG_ABSTIME;
	  	(radio_cmd_prop_tx[slotRadioSetup]).startTrigger.pastTrig = 0;
	  	(radio_cmd_prop_tx[slotRadioSetup]).startTime = rat_time;

	  	//System_printf("1\n");
	  	//RF_EventMask result =
	  	RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_tx[slotRadioSetup]), RF_PriorityNormal, NULL, 0);
	  	//System_printf("2\n");
	  	switch ((radio_cmd_prop_tx[slotRadioSetup]).status) {
	  	case PROP_DONE_OK:
	  		rat_next = rat_time + 4000*tagPeriodMs;
	  		//outcome = TX_OK;
//#if defined(CC1310_LAUNCHPAD)
		  	leds_blink(LEDS_TX, 1);
//#endif
	  		break;
	  	case ERROR_PAST_START:
#ifdef SEMWAIT
	  		if (rat_slack < rat_slack_max) rat_slack += rat_slack_step;
	  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs + rat_slack;
#else
	  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs;
	  		//rat_next = rat_time + 4000*tagPeriodMs;
#endif
	  		//System_printf("+100us\n");
	  		//outcome = TX_FAIL;
//#if defined(CC1310_LAUNCHPAD)
		  	leds_blink(LEDS_TX, 3);
//#endif
	  		break;
	  	default:
#ifdef SEMWAIT
	  		if (rat_slack < rat_slack_max) rat_slack += rat_slack_step;
	  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs + rat_slack;
#else
	  		rat_next = rat_time + 4000*tagPeriodMs;
#endif
	  		System_printf("CMD_TX> %04x\n",(radio_cmd_prop_tx[slotRadioSetup]).status); // ADV
	  		//outcome = TX_FAIL;
		  	leds_blink(LEDS_TX, 2);
	  		break;
	  	}

	  	//flashWriteLastConf(1);

	  	break;

		  case SLOT_ACTION_TX_ADV:
		  	(radio_cmd_prop_tx_adv[slotRadioSetup]).status = IDLE;
		  	(radio_cmd_prop_tx_adv[slotRadioSetup]).pPkt   = slotActionPtr->packet;
		  	(radio_cmd_prop_tx_adv[slotRadioSetup]).pktLen = slotActionPtr->packet_len;

		  	(radio_cmd_prop_tx_adv[slotRadioSetup]).startTrigger.triggerType = TRIG_ABSTIME;
		  	//(radio_cmd_prop_tx_adv[slotRadioSetup]).startTrigger.triggerType = TRIG_NOW;
		  	(radio_cmd_prop_tx_adv[slotRadioSetup]).startTrigger.pastTrig    = 0;
		  	(radio_cmd_prop_tx_adv[slotRadioSetup]).startTime = rat_time;

		  	//result =
		  	RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_tx_adv[slotRadioSetup]), RF_PriorityNormal, receiveCallback, 0);
		  	switch ((radio_cmd_prop_tx_adv[slotRadioSetup]).status) {
		  	case PROP_DONE_OK:
		  		rat_next = rat_time + 4000*tagPeriodMs;
//#if defined(CC1310_LAUNCHPAD)
			  	leds_blink(LEDS_TX, 1);
//#endif
		  		//outcome = TX_OK;
		  		break;
		  	case ERROR_PAST_START:
#ifdef SEMWAIT
	  		if (rat_slack < rat_slack_max) rat_slack += rat_slack_step;
	  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs + rat_slack;
#else
	  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs;
	  		//rat_next = rat_time + 4000*tagPeriodMs;
#endif
		  		//outcome = TX_FAIL;
//#if defined(CC1310_LAUNCHPAD)
			  	leds_blink(LEDS_TX, 3);
//#endif
		  		//System_printf("+100us\n");
		  		break;
		  	default:
#ifdef SEMWAIT
	  		if (rat_slack < rat_slack_max) rat_slack += rat_slack_step;
	  		rat_next = RF_getCurrentTime() + 4000*tagPeriodMs + rat_slack;
#else
	  		rat_next = rat_time + 4000*tagPeriodMs;
#endif
		  		System_printf("CMD_TX_ADV> %04x\n",(radio_cmd_prop_tx_adv[slotRadioSetup]).status); // ADV
		  		//outcome = TX_FAIL;
			  	leds_blink(LEDS_TX, 2);
		  		break;
		  	}

		  	break;
    } // switch on what type of slot we are in

	  //leds_off(LEDS_RX); // XXX debug random tag

	  //uint32_t rat_debug = RF_getCurrentTime();

	  uint32_t rt3 = Timestamp_get32();

	  scheduleAdvance();

	  //System_printf("= %d -> %d\n",previousSetup, slotRadioSetup);

	  //if (slotRadioSetup != previousSetup)
	  //	RF_close(rfHandle);

	  uint32_t rt4 = Timestamp_get32();

	  //RF_yield(rfHandle);
#if 0
	  double t  = (rt0 - prev_time) / (double) tsf.lo;
	  double t1 = (rt1 - rt0) / (double) tsf.lo;
	  double t2 = (rt2 - rt1) / (double) tsf.lo;
	  double t3 = (rt3 - rt2) / (double) tsf.lo;
	  double t4 = (rt4 - rt3) / (double) tsf.lo;
	  System_printf("ts diff=%.6f RAT diff=%.6f %.6f %.6f %.6f %0.6f slack %d\n",
	  		          t,(rat_time-rat_prev)/4e6,t1,t2,t3,t4,rat_slack);
	  prev_time = rt0;
#endif
	  //rat_prev = rat_time;
	  rat_time = rat_next;

		//slot_index++;
		//if (slots[slot_index].setup < 0) slot_index = 0; // wrap around

	}

}

/*****************************************************************************/
/* TRANSMIT TASK                                                             */
/*****************************************************************************/

#define TAG_TASK_STACK_SIZE 1024

static Task_Params tagTaskParams;
Task_Struct tagTask;    /* not static so you can see in ROV */
static uint8_t tagTaskStack[TAG_TASK_STACK_SIZE];

void tagTask_init() {

	uint8_t* cdata = (uint8_t*) 0x0001e000; // configuration page
	uint16_t length = *((uint16_t*) cdata);
	if (length == 0xFFFF) {
		//System_printf("configuration data missing, halting\n");
		leds_on(LEDS_TX);
		System_abort("configuration data missing, halting\n");
	} else {
		System_printf("configuration length %d\n",length);
		uint16_t len = vildehayeHandlePacketNaked(cdata+2, length);
		flashSetConfigurationLimit(len);
		uint8_t c = flashGetLastConfiguration();
		if (c != 255) tag_gotoConfiguration(c);
		//flashWriteLastConf(1); // for testing only
	}

	randomSetState((uint32_t) tagId);

  //if (tagPeriodMs < 1000) leds_setBlinkCounter(0); // don't blink on short periods (random beacons)

  System_printf("tag %d starting (lower digits)\n",(uint32_t) (tagId%1000000ll));
	/*
	radioSetup_frequency(1, 150000000);
	radioSetup_modulation(0, MODULATION_TYPE_GFSK,
	                      100000,
			                  2500,
										    25000);
*/
	//radioSetup_txPower(0, 13);
	//radioSetup_txPower(1, 13);
	//radioSetup_txPower(2, 13);
	//radioSetupsCount = radioSetup_configureFromBuffer(radioData, radioDataLength);
	//schedule_init(confData, confDataLength);


	Task_Params_init(&tagTaskParams);
	tagTaskParams.stackSize = TAG_TASK_STACK_SIZE;
	tagTaskParams.priority  = TAG_TASK_PRIORITY;
	tagTaskParams.stack     = &tagTaskStack;
	tagTaskParams.arg0      = (UInt)1000000;

	Task_construct(&tagTask, tagTaskFunction, &tagTaskParams, NULL);
}

#endif // tag firmware
