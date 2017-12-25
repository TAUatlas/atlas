#ifdef VH_BASESTATION_FIRMWARE

#include <stdlib.h>
#include <time.h>

#include <xdc/std.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>

#include <ti/sysbios/family/arm/cc26xx/Seconds.h>

#include <ti/devices/cc13x0/driverlib/rf_mailbox.h>
#include <ti/devices/cc13x0/driverlib/rf_prop_mailbox.h>
#include <ti/devices/cc13x0/driverlib/rf_common_cmd.h>
#include <ti/devices/cc13x0/driverlib/rf_prop_cmd.h>
#include <ti/drivers/rf/RF.h>

#include "config.h"
#include "radio_setup.h"
#include "buffers.h"
#include "uart.h"

#include "leds.h"
#include "board.h"


#include "rf_queue_pointer.h"

#include "receive.h"
#include "vildehaye.h"

#include "watchdog.h"

// for uint32_t ratDiffToUs(uint32_t now, uint32_t rat_time):
#include "tag.h"

/* buffer configuration */
//#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
//#define MAX_LENGTH             30 /* Max length byte the radio will accept */
//#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
//#define NUM_APPENDED_BYTES     (1+1+1+4+1)  /* The Data Entries data field will contain:
//                                             * 1 element length byte
//                                             * 1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
//                                             * Max 30 payload bytes
//                                             * 1 RSSI byte
//                                             * 4 timestamp bytes
//                                             * 1 status byte (RF_cmdPropRx.rxConf.bAppendStatus = 0x1) */

// repetition!!!
#define DATA_PROTOCOL_ATLAS                     16
#define DATA_PROTOCOL_VILDEHAYE_BEACON           3
#define DATA_PROTOCOL_VILDEHAYE_BEACON_RESPONSE  2
#define DATA_PROTOCOL_VILDEHAYE_SESSION          1

#if 0
const uint32_t radioData[] = {
	//RPE(DATA_PROTOCOL_ATLAS,            MODULATION_TYPE_FSK, 10,1,32,DATA_ENCODING_PLAIN  ), 0x333C3C33, 915000000, 380859,  999756, 2200000,
	RPE(DATA_PROTOCOL_VILDEHAYE_BEACON, MODULATION_TYPE_FSK, 10,1,32,DATA_ENCODING_CC1310_FASTLRM), 0x930B51DE, 915000000, 175000,  500000, 1410000,
  //RPE(DATA_PROTOCOL_VILDEHAYE_SESSION,MODULATION_TYPE_GFSK,10,1,32,DATA_ENCODING_PLAIN  ), 0x333C3C33, 915000000, 253906,  499878, 1100000,
};
//const int radioDataLength = 1*6 * sizeof(uint32_t);
static const int radioDataLength = RADIO_SETUP_BUFFER_SIZE(1);
#endif

/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;

//#define UNKNOWN 99
//#define TX_OK   0
//#define TX_FAIL 1
//#define RX_OK   2
//#define RX_FAIL 3

uint8_t countRx, countTxOk, countTxTooLate, countTxErr, state;
uint32_t timestampRx, timestampTx, timestampPostTx;

Mailbox_Handle basestationTxMailbox;

uint8_t radioSetupInUse = 0xFF; // radio not actually in use

/*
	Error handling function – replaces the default while(1) in RF driver.
	This is necessary for certain versions of TI-RTOS, not all; see
	http://processors.wiki.ti.com/index.php/CC1310_rev_B_PCN_information
*/static void errorCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e) {
    if ((int32_t)ch == RF_ERROR_CMDFS_SYNTH_PROG) {
        // Handle error
        // If CMD_FS is executed standalone, re-start CMD_FS in this function if there is time. If not, handle as in previous example
        // When CMD_FS is chained with RX/TX commands, the RX/TX is executed in parallel with this callback by the radio and will subsequently fail. Error can be handled as in previous example.
    }
}



static void basestationRxTaskFunction(UArg arg0, UArg arg1) {
#if 0
  System_printf("Setting up radio\n");

  RF_Params rfParams;
  RF_Params_init(&rfParams);
  rfParams.nInactivityTimeout = 100; // in usecs
  rfParams.pErrCb = errorCallback; // only necessary in certain versions of TI-RTOS

  /* Request access to the radio */
  rfHandle = RF_open(&rfObject, &(radio_mode[radioSetupInUse]), (RF_RadioSetup*)&(radio_cmd_prop_div_setup[radioSetupInUse]), &rfParams);

  /* Set the frequency */
  RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_fs[radioSetupInUse]), RF_PriorityNormal, NULL, 0);

  /* Modify CMD_PROP_RX command for application needs */
  (radio_cmd_prop_rx[radioSetupInUse]).pQueue = &dataQueue;           /* Set the Data Entity queue for received data */
  (radio_cmd_prop_rx[radioSetupInUse]).rxConf.bAutoFlushIgnored = 1;  /* Discard ignored packets from Rx queue */
  (radio_cmd_prop_rx[radioSetupInUse]).rxConf.bAutoFlushCrcErr = 1;   /* Discard packets with CRC error from Rx queue */
  (radio_cmd_prop_rx[radioSetupInUse]).maxPktLen = BUFFER_SIZE-NUM_APPENDED_BYTES-1;
  (radio_cmd_prop_rx[radioSetupInUse]).pktConf.bRepeatOk  = 0; // continue when a packet is received?
  (radio_cmd_prop_rx[radioSetupInUse]).pktConf.bRepeatNok = 1;

  /* Enter RX mode and stay forever in RX */
  //RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radio_setup]), RF_PriorityNormal, &callback, IRQ_RX_ENTRY_DONE);
  //RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radio_setup]), RF_PriorityNormal, &receiveCallback, IRQ_RX_ENTRY_DONE);

  System_printf("receive task done setting up, waiting for packets\n");
#endif
  while(1) {
#if 0
  	System_printf("posting rx\n");
  	radio_cmd_prop_rx[radioSetupInUse].status = IDLE;
    RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radioSetupInUse]), RF_PriorityNormal, &receiveCallback, IRQ_RX_ENTRY_DONE);

    //uint8_t outcome;
#endif
  	System_printf("waiting for a rx packet ready buffer is %d\n",readyBuffer.id);
  	Semaphore_pend(packetReceivedSemaphore, BIOS_WAIT_FOREVER);

  	// previous ready buffer is now used in queue, get another
  	//Mailbox_pend(freeMailbox, &readyBuffer,  BIOS_WAIT_FOREVER);

  	receiveGetBuffer();

  	uint8_t* rxpacket = buffers[ incomingBuffer.id ];
  	//uint8_t elementLength = rxpacket[0];
  	uint8_t payloadLength = rxpacket[1];
  	//uint8_t rssi          = rxpacket[1+1+payloadLength];
  	//uint8_t tsp = 1+1+payloadLength+1;
  	//uint32_t ts =  rxpacket[tsp] | (rxpacket[tsp+1]<<8) | (rxpacket[tsp+2]<<16) | (rxpacket[tsp+3]<<24);
  	//uint8_t  status       = rxpacket[1+1+payloadLength+1+4];

  	rxpacket[1+1+payloadLength+1+4] = radioSetupInUse; // replace the status by the radio setup index

#if 0
  	//System_printf("RX buffer %d\n",readyBuffer.id);
  	Mailbox_post(uartTxMailbox, &incomingBuffer, BIOS_WAIT_FOREVER);
  	//Mailbox_post(freeMailbox, &incomingBuffer, BIOS_WAIT_FOREVER);
#endif
  	rxpacket[1+1+payloadLength+1+4] = radioSetupInUse + 16;
  	countRx++;
  	Mailbox_post(basestationTxMailbox, &incomingBuffer, BIOS_WAIT_FOREVER);


  	//System_printf("rx! incoming buffer %d len %d sending to uart\n", incomingBuffer.id, incomingBuffer.length);
  	//System_printf("packet received! elen=%u plen=%u rssi=%u ts=%lu status=%02x\n",
  	//		          elementLength, payloadLength, rssi, ts, status);

  	//System_printf("rx! ts=%lu %lu %.6f\n",ts,(uint32_t) (ts + (3 + 0.001)*4000000),((uint32_t) (ts + (3 + 0.001)*4000000)-ts)/4e6);
  }
}

void basestation_gotoSetup(uint8_t c) {
	System_printf("goto setup %d\n",c);
	radioSetupInUse = 0;
}


/*
 * This task opens the radio and posts a receive command.
 * Responses from the receive command are not handled in this task but in the rx task.
 * Once the receive command is posted, the task repeatedly reads a packet from the tx mailbox.
 *
 * If the packet is marked with setup 0xFF (UART communication) or setup which is 16 or higher
 * (meaning it was received by the radio), the packet is send to the uart mailbox. It will be
 * sent to the host from there. In this case,
 * the radio receive command is posted again. If the setup is 16 or higher, it is adjusted down
 * before sending to the uart mailbox.
 *
 * If the packet has setup between 0 and 15, it is transmitted by the radio. To do that, we cancel
 * the receive command, post a transmit command with the time stamp given in the packet, then
 * release the buffer, and post the receive command again.
 */

static uint8_t configured = 0; // do we already have a radio setup configuration?

static void basestationTxTaskFunction(UArg arg0, UArg arg1) {

	uint8_t radioOpen = 0;
	RF_CmdHandle rxcmd;

  System_printf("Setting up radio\n");

  RF_Params rfParams;
  RF_Params_init(&rfParams);
  rfParams.nInactivityTimeout = 100; // in usecs


	while (1) {
		buffer_descriptor d;
		System_printf("waiting for tx request\n");
		state = 0; // waiting for mailbox, rx posted
		Mailbox_pend(basestationTxMailbox, &d, BIOS_WAIT_FOREVER);

  	uint8_t* rxpacket = buffers[ d.id ];
  	//uint8_t elementLength = rxpacket[0];
  	uint8_t payloadLength = rxpacket[1];
  	//uint8_t rssi          = rxpacket[1+1+payloadLength];
  	uint8_t tsp = 1+1+payloadLength+1;
  	//uint32_t ts =  rxpacket[tsp] | (rxpacket[tsp+1]<<8) | (rxpacket[tsp+2]<<16) | (rxpacket[tsp+3]<<24);
  	uint32_t ts;
  	ts  = rxpacket[tsp+3];
  	ts <<= 8;
  	ts |= rxpacket[tsp+2];
  	ts <<= 8;
  	ts |= rxpacket[tsp+1];
  	ts <<= 8;
  	ts |= rxpacket[tsp  ];
  	//uint8_t  status       = rxpacket[1+1+payloadLength+1+4];

  	uint8_t setup = rxpacket[1+1+payloadLength+1+4];

  	System_printf("got packet setup %d ts %d\n",setup,ts);

		//RF_runDirectCmd(rfHandle, CMD_ABORT);

  	if (setup == 0xFF) {
  		// a packet for this basestation
			System_printf("packet to bs!!!\n");

			if (radioOpen) { // meaning we are in rx
	  		RF_cancelCmd(rfHandle,rxcmd,0 /* 0=abrupt; 1=gracefull */);
	  		RF_close(rfHandle);
	  		radioOpen = 0;
			}

			leds_off(LEDS_TX);
			configured = 1;
			radioSetupInUse = 0;

			vildehayeHandlePacketNaked(rxpacket+2,rxpacket[1]);
			Mailbox_post(freeMailbox, &d, BIOS_WAIT_FOREVER);

			if (radioSetupInUse != 0xFF) {
				/* Request access to the radio */
				radioOpen = 1;
				rfHandle = RF_open(&rfObject, &(radio_mode[radioSetupInUse]), (RF_RadioSetup*)&(radio_cmd_prop_div_setup[radioSetupInUse]), &rfParams);

				/* Set the frequency */
				RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_fs[radioSetupInUse]), RF_PriorityNormal, NULL, 0);

				/* Modify CMD_PROP_RX command for application needs */
				(radio_cmd_prop_rx[radioSetupInUse]).status = IDLE;
				(radio_cmd_prop_rx[radioSetupInUse]).pQueue = &dataQueue;           /* Set the Data Entity queue for received data */
				(radio_cmd_prop_rx[radioSetupInUse]).rxConf.bAutoFlushIgnored = 1;  /* Discard ignored packets from Rx queue */
				(radio_cmd_prop_rx[radioSetupInUse]).rxConf.bAutoFlushCrcErr = 1;   /* Discard packets with CRC error from Rx queue */
				(radio_cmd_prop_rx[radioSetupInUse]).maxPktLen = BUFFER_SIZE-NUM_APPENDED_BYTES-1;
				(radio_cmd_prop_rx[radioSetupInUse]).pktConf.bRepeatOk  = 0; // End operation after receiving a packet correctly
				(radio_cmd_prop_rx[radioSetupInUse]).pktConf.bRepeatNok = 1; // Go back to sync search after receiving a packet with CRC error

				/* Enter RX mode and stay forever in RX */
				//RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radio_setup]), RF_PriorityNormal, &callback, IRQ_RX_ENTRY_DONE);
				//RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radio_setup]), RF_PriorityNormal, &receiveCallback, IRQ_RX_ENTRY_DONE);

				System_printf("receive task done setting up, waiting for packets\n");

				System_printf("posting rx\n");
				radio_cmd_prop_rx[radioSetupInUse].status = IDLE;
				rxcmd = RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radioSetupInUse]), RF_PriorityNormal, &receiveCallback, IRQ_RX_ENTRY_DONE);
			}
  	}
  	if (setup >= 16 && setup != 0xFF) {
  		rxpacket[1+1+payloadLength+1+4] -= 16; // adjust back
  		timestampRx = ts;
  		state=1; // sending to uart
  		Mailbox_post(uartTxMailbox, &d, BIOS_WAIT_FOREVER);
  		// post rx again
  		state=0;
  		radio_cmd_prop_rx[radioSetupInUse].status = IDLE;
  		rxcmd = RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radioSetupInUse]), RF_PriorityNormal, &receiveCallback, IRQ_RX_ENTRY_DONE);
    	leds_blink(LEDS_RX, 1); // received
  	}
  	if (setup < 16) { // transmit a packet on this setup
  		//RF_runDirectCmd(rfHandle, CMD_ABORT);
  		state=3; // canceling rx
  		RF_cancelCmd(rfHandle,rxcmd,0 /* 0=abrupt; 1=gracefull */);


  	  uint32_t rat_now = RF_getCurrentTime();
  	  uint32_t wait_us = ratDiffToUs(rat_now,ts);

  	  /* Make sure no RAT wrap around occurs while we wait for RAT; it does not work */
  	  if (rat_now > ts) { // wrap around
  	  	if (wait_us > 10000) // more than 10ms? if so, sleep until 10ms before deadline
  	  		Task_sleep((wait_us-10000) / Clock_tickPeriod);
  	  }

	  	(radio_cmd_prop_tx[radioSetupInUse]).status = IDLE;
	  	(radio_cmd_prop_tx[radioSetupInUse]).pktLen = payloadLength;
	  	(radio_cmd_prop_tx[radioSetupInUse]).pPkt   = rxpacket+2;
	  	(radio_cmd_prop_tx[radioSetupInUse]).startTrigger.triggerType = TRIG_ABSTIME;
	  	(radio_cmd_prop_tx[radioSetupInUse]).startTrigger.pastTrig = 0;
	  	(radio_cmd_prop_tx[radioSetupInUse]).startTime = ts;

	  	//System_printf("1\n");
	  	timestampTx = ts;
	  	timestampPostTx = RF_getCurrentTime();
	  	state=4; // posting tx, will wait
	  	RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_tx[radioSetupInUse]), RF_PriorityNormal, NULL, 0);
	  	switch ((radio_cmd_prop_tx[radioSetupInUse]).status) {
	  	case PROP_DONE_OK:
	  		countTxOk++;
	  		//rat_next = rat_time + 4000*tagPeriodMs;
	  		//outcome = TX_OK;
		  	leds_blink(LEDS_TX, 1);
	  		break;
	  	case ERROR_PAST_START:
	  		countTxTooLate++;
	  		//if (rat_slack < rat_slack_max) rat_slack += rat_slack_step;
	  		//rat_next = RF_getCurrentTime() + 4000*tagPeriodMs + rat_slack;
	  		//System_printf("+100us\n");
	  		//outcome = TX_FAIL;
		  	leds_blink(LEDS_TX, 3);
	  		break;
	  	default:
	  		countTxErr++;
	  		//if (rat_slack < rat_slack_max) rat_slack += rat_slack_step;
	  		//rat_next = RF_getCurrentTime() + 4000*tagPeriodMs + rat_slack;
	  		//System_printf("CMD_TX> %04x\n",(radio_cmd_prop_tx[slotRadioSetup]).status); // ADV
	  		//outcome = TX_FAIL;
		  	leds_blink(LEDS_TX, 2);
	  		break;
	  	}

	  	state=5;
	  	Mailbox_post(freeMailbox, &d, BIOS_WAIT_FOREVER);

  		// now repost the rx command
	  	state=0;
  		radio_cmd_prop_rx[radioSetupInUse].status = IDLE;
  		rxcmd = RF_postCmd(rfHandle, (RF_Op*)&(radio_cmd_prop_rx[radioSetupInUse]), RF_PriorityNormal, &receiveCallback, IRQ_RX_ENTRY_DONE);
  	}
	}
}


static vildehaye_packet_t vildehayePacket;
//static uint8_t vildehayePayloadPlusMeta[256]; // TODO mark it as in use or not, or switch to buffers

static void basestationUartTaskFunction(UArg arg0, UArg arg1) {

	//watchdog_init(20*1000, 20*1000);

	buffer_descriptor d;
	while (1) {
		System_printf("waiting for uart packet\n");
		//Mailbox_pend(uartRxMailbox, &d, BIOS_WAIT_FOREVER);
		if (Mailbox_pend(uartRxMailbox, &d, 10*1000000 / Clock_tickPeriod)) { // 4s
			System_printf("bs uart got buffer %d len %d\n",d.id, d.length);
			//watchdog_pacify();

			//uint8_t elementLength = buffers[d.id][0];
			uint8_t payloadLength = buffers[d.id][1];
			//uint8_t rssi          = buffers[d.id][1+1+payloadLength];
			//uint8_t tsp = 1+1+payloadLength+1;
			//uint32_t ts =  buffers[d.id][tsp] | (buffers[d.id][tsp+1]<<8) | (buffers[d.id][tsp+2]<<16) | (buffers[d.id][tsp+3]<<24);
			uint8_t  setup        = buffers[d.id][1+1+payloadLength+1+4];

			if (setup == 0xFF) { // this packet is for the basestation
				System_printf("packet to bs setup 0x%02x\n",setup);
				//vildehayeHandlePacketNaked(buffers[ d.id ]+2,buffers[ d.id ][1]);
				//Mailbox_post(freeMailbox, &d, BIOS_WAIT_FOREVER);
				Mailbox_post(basestationTxMailbox, &d, BIOS_WAIT_FOREVER);
			} else {
				System_printf("packet to radio setup 0x%02x\n",setup);
				Mailbox_post(basestationTxMailbox, &d, BIOS_WAIT_FOREVER);
			}
		} else {
			System_printf("timeout reading uartRxMailbox, sending state to PC\n");
			watchdog_pacify();

			Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);

			vildehayeInitPacket(&vildehayePacket, (buffers[d.id]) + 2 /* skip elen, plen */, 0, 256-8);

			if (!configured) {
				vildehayeAddTagState(&vildehayePacket,
						VH_TAGSTATE_NEEDS_CONFIGURATION | VH_TAGSTATE_NON_SESSION_CMD_OK,
						256-8,
						0); // tag state
			} else {
				vildehayeAddTagState(&vildehayePacket,
						                                  VH_TAGSTATE_NON_SESSION_CMD_OK,
						256-8,
						0); // tag state
			}

			vildehayeAddHeader(&vildehayePacket, VH_BASESTATION_STATE, 1+1+4);
			vildehayeAddUInt8 (&vildehayePacket, 1); // single bytes, unsigned
			vildehayeAddUInt8 (&vildehayePacket, state);
			vildehayeAddUInt8 (&vildehayePacket, countRx);
			vildehayeAddUInt8 (&vildehayePacket, countTxOk);
			vildehayeAddUInt8 (&vildehayePacket, countTxTooLate);
			vildehayeAddUInt8 (&vildehayePacket, countTxErr);

			//if (state==4) {
				vildehayeAddHeader (&vildehayePacket, VH_TEST_UNSIGNED, 4);
				vildehayeAddUInt32 (&vildehayePacket, timestampRx);
				vildehayeAddHeader (&vildehayePacket, VH_TEST_UNSIGNED, 4);
				vildehayeAddUInt32 (&vildehayePacket, timestampTx);
				vildehayeAddHeader (&vildehayePacket, VH_TEST_UNSIGNED, 4);
				vildehayeAddUInt32 (&vildehayePacket, timestampPostTx);
			//}

			System_printf("local clock %d\n",Seconds_get());
			vildehayeAddHeader (&vildehayePacket, VH_LOCAL_CLOCK, 4);
			vildehayeAddUInt32 (&vildehayePacket, Seconds_get());

			//vildehayeAddHeader (&vildehayePacket, VH_LOCAL_CLOCK, 4); // local clock
			//vildehayeAddUInt32 (&vildehayePacket, 1447890000);

			(buffers[d.id])[0] = 7 + vildehayePacket.length;
			(buffers[d.id])[1] =     vildehayePacket.length;
			(buffers[d.id])[2+vildehayePacket.length  ] = 0; // rssi
			(buffers[d.id])[2+vildehayePacket.length+1] = 0; // timestamp
			(buffers[d.id])[2+vildehayePacket.length+2] = 0; // timestamp
			(buffers[d.id])[2+vildehayePacket.length+3] = 0; // timestamp
			(buffers[d.id])[2+vildehayePacket.length+5] = 0xFF; // setup

			d.length = 7 + vildehayePacket.length + 1;

			if (!Mailbox_post(uartTxMailbox, &d, BIOS_NO_WAIT)) {
				System_printf("uart tx queue full\n");
				//vildehayeHandlePacket(buffers[d.id], d.length);
				Mailbox_post(freeMailbox, &d, BIOS_WAIT_FOREVER);
			}
		}
	}
}

#define BASESTATION_TASK_STACK_SIZE 1024

static Task_Params basestationTaskParams;
Task_Struct basestationTask;    /* not static so you can see in ROV */
static uint8_t basestationTaskStack[BASESTATION_TASK_STACK_SIZE];

static Task_Params basestationTxTaskParams;
static Task_Struct basestationTxTask;
static uint8_t basestationTxTaskStack[BASESTATION_TASK_STACK_SIZE];

static Task_Params basestationUartTaskParams;
static Task_Struct basestationUartTask;
static uint8_t basestationUartTaskStack[BASESTATION_TASK_STACK_SIZE];

/*
 * The initialization function of the basestation only starts
 * the UART task. The UART task will send requests for configuration,
 * and the response from the host will invoke basestationRadioSetup
 * which will perform the rest of the initialization.
 */

void basestationTask_init() {

	uint8_t* cdata = (uint8_t*) 0x0001e000; // configuration page
	uint16_t length = *((uint16_t*) cdata);
	if (length == 0xFFFF) {
		//System_printf("configuration data missing, halting\n");
		leds_on(LEDS_TX);
		System_printf("configuration data missing, not an issue for a basestation\n");
	} else {
		System_printf("configuration length %d\n",length);
		vildehayeHandlePacketNaked(cdata+2, length);
	}

	System_printf("basestation (uart) task init\n");
#if 0
	radioSetup_configureFromBuffer(radioData, radioDataLength);

	Task_Params_init(&basestationTaskParams);
	basestationTaskParams.stackSize = BASESTATION_TASK_STACK_SIZE;
	basestationTaskParams.priority = BASESTATION_TASK_PRIORITY;
	basestationTaskParams.stack = &basestationTaskStack;
	basestationTaskParams.arg0 = (UInt)1000000;

	Task_construct(&basestationTask, basestationRxTaskFunction, &basestationTaskParams, NULL);
#endif

	Task_Params_init(&basestationUartTaskParams);
	basestationUartTaskParams.stackSize = BASESTATION_TASK_STACK_SIZE;
	basestationUartTaskParams.priority = BASESTATION_UART_TASK_PRIORITY;
	basestationUartTaskParams.stack = &basestationUartTaskStack;
	basestationUartTaskParams.arg0 = (UInt)1000000;

	Task_construct(&basestationUartTask, basestationUartTaskFunction, &basestationUartTaskParams, NULL);

	System_printf("basestation radio setup (starting radio tasks)\n");

	basestationTxMailbox = Mailbox_create(sizeof(buffer_descriptor),BUFFER_COUNT, NULL,NULL);

	Task_Params_init(&basestationTaskParams);
	basestationTaskParams.stackSize = BASESTATION_TASK_STACK_SIZE;
	basestationTaskParams.priority = BASESTATION_TASK_PRIORITY;
	basestationTaskParams.stack = &basestationTaskStack;
	basestationTaskParams.arg0 = (UInt)1000000;

	Task_construct(&basestationTask, basestationRxTaskFunction, &basestationTaskParams, NULL);

	Task_Params_init(&basestationTxTaskParams);
	basestationTxTaskParams.stackSize = BASESTATION_TASK_STACK_SIZE;
	basestationTxTaskParams.priority = BASESTATION_TX_TASK_PRIORITY;
	basestationTxTaskParams.stack = &basestationTxTaskStack;
	basestationTxTaskParams.arg0 = (UInt)1000000;

	Task_construct(&basestationTxTask, basestationTxTaskFunction, &basestationTxTaskParams, NULL);

}

/*
 * This function is called by the vildehaye packed data handler.
 * It first configures the radio and then
 *  - creates a tx mailbox
 *  - creates the basestation rx task
 *  - creates the basestation tx task
 */


//void basestationRadioSetup(uint8_t* radioData, uint16_t radioDataLength) {
//	//if (configured) return;
//	leds_off(LEDS_TX);
//
//	configured = 1;
//
//	radioSetup_configureFromBuffer(radioData, radioDataLength);

	/*
	System_printf("basestation radio setup (starting radio tasks)\n");

	basestationTxMailbox = Mailbox_create(sizeof(buffer_descriptor),BUFFER_COUNT, NULL,NULL);

	Task_Params_init(&basestationTaskParams);
	basestationTaskParams.stackSize = BASESTATION_TASK_STACK_SIZE;
	basestationTaskParams.priority = BASESTATION_TASK_PRIORITY;
	basestationTaskParams.stack = &basestationTaskStack;
	basestationTaskParams.arg0 = (UInt)1000000;

	Task_construct(&basestationTask, basestationRxTaskFunction, &basestationTaskParams, NULL);

	Task_Params_init(&basestationTxTaskParams);
	basestationTxTaskParams.stackSize = BASESTATION_TASK_STACK_SIZE;
	basestationTxTaskParams.priority = BASESTATION_TX_TASK_PRIORITY;
	basestationTxTaskParams.stack = &basestationTxTaskStack;
	basestationTxTaskParams.arg0 = (UInt)1000000;

	Task_construct(&basestationTxTask, basestationTxTaskFunction, &basestationTxTaskParams, NULL);
	*/
//}

#endif // base station firmware
