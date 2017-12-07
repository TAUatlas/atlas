#include <stdlib.h>
#include <time.h>

#include <xdc/std.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/hal/Timer.h>

#include <ti/drivers/PIN.h>

#include "config.h"
#include "Board.h"

#include "leds.h"

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State  ledPinState;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config pinTable[] = {
  	Board_RLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    //Board_GLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
   PIN_TERMINATE
};

#define LEDS_TASK_STACK_SIZE 512

static Task_Params ledsTaskParams;
Task_Struct ledsRedTask;    /* not static so you can see in ROV */
Task_Struct ledsGrnTask;    /* not static so you can see in ROV */
static uint8_t redTaskStack[LEDS_TASK_STACK_SIZE];
static uint8_t grnTaskStack[LEDS_TASK_STACK_SIZE];

Semaphore_Handle ledsSemaphores[3];

static PIN_Id leds_get(uint8_t led) {
	if (led == 0      ) return PIN_UNASSIGNED;
	if (led == LEDS_TX) return Board_RLED;
	if (led == LEDS_RX) return Board_GLED;
	return 0xFF; // error
}

void leds_on(uint8_t led) {
	PIN_setOutputValue(ledPinHandle, leds_get(led), Board_LED_ON);
}

void leds_off(uint8_t led) {
	PIN_setOutputValue(ledPinHandle, leds_get(led), Board_LED_OFF);
}

void leds_toggle(uint8_t led) {
	PIN_setOutputValue(ledPinHandle, leds_get(led),!PIN_getOutputValue(leds_get(led)));
}

#ifndef LEDS_INITIAL_BLINK_COUNTER
#define LEDS_INITIAL_BLINK_COUNTER 0
#endif

static uint8_t leds_counter = LEDS_INITIAL_BLINK_COUNTER;
void leds_setBlinkCounter(uint8_t c) {
	leds_counter = c;
	System_printf("LEDS setting blink counter to %d\n",leds_counter);
}

void leds_incrementBlinkCounter() {
	if (leds_counter!=0xFF) leds_counter++;
	System_printf("LEDS incrementing blink counter to %d\n",leds_counter);
}

static void ledsTaskFunction(UArg arg0, UArg arg1) {
	uint8_t symbolic = (uint8_t) arg0;
	PIN_Id physical;

	switch (symbolic) {
	case 1: // usually tx
		physical = Board_RLED;
		break;
	case 2: //usually rx
		physical = Board_GLED;
		break;
	case 0:
	default:
		physical = PIN_UNASSIGNED;
		return; // we don't really need the task to do nothing
	}

	uint32_t onPeriod =   25000/Clock_tickPeriod; // 25ms in us divided by clock period in us
	uint32_t offPeriod = 300000/Clock_tickPeriod; // 200ms
	System_printf("LEDS %d on %d off %d\n",symbolic, onPeriod,offPeriod);

	while (1) {
		Semaphore_pend(ledsSemaphores[symbolic], BIOS_WAIT_FOREVER);
		//System_printf("LEDS %d blinking\n",symbolic);
		PIN_setOutputValue(ledPinHandle, physical, Board_LED_ON);
		Task_sleep(onPeriod);
		PIN_setOutputValue(ledPinHandle, physical, Board_LED_OFF);
		Task_sleep(offPeriod); // 100ms
	}
}

void leds_blink(uint8_t led, uint8_t howmanytimes) {
	//System_printf("LEDS blink %d %d %d\n",led, howmanytimes, leds_counter);
	if (led==0) return; // a request for a LED not on this board
	if (leds_counter==0  ) return;
	if (leds_counter!=0xFF) leds_counter--; // not forever
	//PIN_Id id = leds_get(led);

	uint32_t count = Semaphore_getCount(ledsSemaphores[led]);
	if (count > 0) {
		//System_printf("LEDS already blinking (count=%d)\n",count);
		return; // if it's already blinking, skip
	}

	for (; howmanytimes>0; howmanytimes--) {
		Semaphore_post(ledsSemaphores[led]);
		//leds_on(led);
		//Task_sleep(25000 / Clock_tickPeriod);
		//leds_off(led);
		//if (howmanytimes > 1) Task_sleep(2*50000 / Clock_tickPeriod);
	}
}

void leds_init() {
	/* Open LED pins */
	ledPinHandle = PIN_open(&ledPinState, pinTable);
	if(!ledPinHandle) {
		System_abort("Error initializing board LED pins\n");
	}

	Semaphore_Params semParams;
	semParams.mode = Semaphore_Mode_COUNTING;
  Semaphore_Params_init(&semParams);
  ledsSemaphores[LEDS_TX] = Semaphore_create(0, &semParams, NULL);
  ledsSemaphores[LEDS_RX] = Semaphore_create(0, &semParams, NULL);

  System_printf("LEDS constructed semaphores for leds %d, %d\n",LEDS_TX, LEDS_RX);

  Task_Params_init(&ledsTaskParams);
	ledsTaskParams.stackSize = LEDS_TASK_STACK_SIZE;
	ledsTaskParams.priority  = LEDS_RED_TASK_PRIORITY;
	ledsTaskParams.stack     = &redTaskStack;
	ledsTaskParams.arg0      = (UInt) LEDS_TX;
	Task_construct(&ledsRedTask, ledsTaskFunction, &ledsTaskParams, NULL);

	ledsTaskParams.stackSize = LEDS_TASK_STACK_SIZE;
	ledsTaskParams.priority  = LEDS_GRN_TASK_PRIORITY;
	ledsTaskParams.stack     = &grnTaskStack;
	ledsTaskParams.arg0      = (UInt) LEDS_RX;
	Task_construct(&ledsGrnTask, ledsTaskFunction, &ledsTaskParams, NULL);

	System_printf("LEDS initial blink counter = %d\n",leds_counter);
}

