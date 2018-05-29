#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

#include "config.h"
#include "uart.h"
#include "buffers.h"
#include "leds.h"
/*
#define QUEUE_ELEMENT_COUNT 8

// This structure can be added to a Queue because the first field is a Queue_Elem.
typedef struct uartQueueElement_st {
 Queue_Elem elem;
 uint16_t length;
 uint8_t  data[256];
} uartQueueElement_t;

uartQueueElement_t uartQueueElement[QUEUE_ELEMENT_COUNT];

Queue_Handle uartTxQueue;
Queue_Handle uartRxQueue;
Queue_Handle freeQueue;

*/

static UART_Handle uartHandle = NULL;

Mailbox_Handle uartTxMailbox;
Mailbox_Handle uartRxMailbox;

/*
 * There seems to be a bug in BLOCKING mode that
 * causes errors when UART_read is called from one
 * task and UART_read from another.
 */
#define UART_USE_CALLBACK_MODE
#ifdef UART_USE_CALLBACK_MODE
static Semaphore_Handle uartRxSemaphore;
static Semaphore_Handle uartTxSemaphore;
static Semaphore_Handle uartAccessSemaphore;

static void uartLocalRead(UART_Handle h, uint8_t* buffer, size_t size) {
	Semaphore_pend(uartAccessSemaphore, BIOS_WAIT_FOREVER);
	UART_read(h, buffer, size);
	Semaphore_post(uartAccessSemaphore);
	Semaphore_pend(uartRxSemaphore, BIOS_WAIT_FOREVER);
}
static void uartLocalWrite(UART_Handle h, const uint8_t* buffer, size_t size) {
	Semaphore_pend(uartAccessSemaphore, BIOS_WAIT_FOREVER);
	UART_write(h, buffer, size);
	Semaphore_post(uartAccessSemaphore);
	Semaphore_pend(uartTxSemaphore, BIOS_WAIT_FOREVER);
}
static void uartTxCallback(UART_Handle h, void* buffer, size_t size) {
	Semaphore_post(uartTxSemaphore);
}
static void uartRxCallback(UART_Handle h, void* buffer, size_t size) {
	Semaphore_post(uartRxSemaphore);
}
#define uart_read(h,b,s) uartLocalRead(h,b,s)
#define uart_write(h,b,s) uartLocalWrite(h,b,s)
#else
#define uart_read(h,b,s) UART_read(h,b,s)
#define uart_write(h,b,s) UART_write(h,b,s)
#endif

//static uint8_t uartBuffer[256];

// END is 0xC0 in SLIP
#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

/*

static uint8_t slipTxBuffer[2*BUFFER_SIZE+1];
static uint32_t slipEncode(uint8_t* p, uint32_t length) {
	uint32_t i, j;

	j=0;
	for (i=0; i<length; i++) {
		uint8_t b = *p;
		p++;
		switch (b) {
		case SLIP_END:
			slipTxBuffer[ j++ ] = SLIP_ESC;
			slipTxBuffer[ j++ ] = SLIP_ESC_END;
			break;
		case SLIP_ESC:
			slipTxBuffer[ j++ ] = SLIP_ESC;
			slipTxBuffer[ j++ ] = SLIP_ESC_ESC;
			break;
		default:
			slipTxBuffer[ j++ ] = b;
			break;
		}
	}
	slipTxBuffer[ j++ ] = SLIP_END;

	return j;
}

static uint8_t slipRxBuffer[2*BUFFER_SIZE+1];
static uint32_t slipDecode(uint8_t* outp, uint32_t length) {
	uint32_t i, j, k;

	j=0;
	k=0;
	for (i=0; i<length; i++) {
		uint8_t b = slipRxBuffer[ k++ ];
		switch (b) {
		case SLIP_END:
			return j; // we are done; normally the end need not be returned at all
		case SLIP_ESC:
			b = slipRxBuffer[ k++ ];
			if (b == SLIP_ESC_END) outp[ j++ ] = SLIP_END;
			if (b == SLIP_ESC_ESC) outp[ j++ ] = SLIP_ESC;
			break;
		default:
			outp[ j++ ] = b;
			break;
		}
	}

	return j;
}

*/

static const uint8_t slip_end[] = { SLIP_END };
//static uint8_t slip_esc[]     = { SLIP_ESC };
static const uint8_t slip_esc_end[] = { SLIP_ESC, SLIP_ESC_END };
static const uint8_t slip_esc_esc[] = { SLIP_ESC, SLIP_ESC_ESC };
static uint8_t txbyte;

static void uartTxTaskFunction(UArg arg0, UArg arg1) {
	uint16_t i;
	buffer_descriptor d;

	//System_printf("in uart tx task\n");

	while (1) {
		Mailbox_pend(uartTxMailbox, &d, BIOS_WAIT_FOREVER);
		//System_printf("uart write %d bytes\n",d.length);
		uart_write(uartHandle, slip_end, 1); // just an escape character
		//System_printf("00\n");
		for (i=0; i<d.length; i++) {
			txbyte = buffers[ d.id ][ i ];
			switch (txbyte) {
			case SLIP_END:
				uart_write(uartHandle, slip_esc_end, 2);
				break;
			case SLIP_ESC:
				uart_write(uartHandle, slip_esc_esc, 2);
				break;
			default:
				uart_write(uartHandle, &txbyte, 1);
				break;
			}
			//System_printf("> %d\n",i);
		}
		uart_write(uartHandle, slip_end, 1); // just an escape character
		//System_printf("uart write %d bytes done\n",d.length);

		//Task_sleep(1000000);
		//System_printf("uart task got buffer %d ");
		//int i;
		//for (i=0; i<d.length; i++) System_printf("%c",buffers[d.id][i]);

		//uint32_t n = slipEncode(buffers[d.id], d.length);
		//UART_write(uartHandle, slipTxBuffer, n);

		Mailbox_post(freeMailbox, &d, BIOS_WAIT_FOREVER);

		//uartQueueElement_t* e = Queue_get(uartTxQueue);


		//Queue_put(freeQueue, &(e->elem));
	}
}

#define UART_TX_TASK_STACK_SIZE 512

static Task_Params uartTxTaskParams;
static Task_Struct uartTxTask; /* not static so you can see in ROV */
static uint8_t uartTxTaskStack[UART_TX_TASK_STACK_SIZE];

//#include <ti/drivers/Power.h>
//#include <ti/sysbios/family/arm/cc26xx/PowerCC2650.h>

/*
static int16_t offset;
static int16_t readToNewline() {
	offset=-1;
	do {
		offset++;
		UART_read(uartHandle, slipRxBuffer+offset, 1);
	} while (slipRxBuffer[offset] != '\n' && offset<sizeof(slipRxBuffer)-1);
	return offset+1;
}
*/

static void uartRxTaskFunction(UArg arg0, UArg arg1) {
	uint8_t b;
	uint8_t rxbyte;

	//System_printf("in uart rx task skip to END\n");

	do {
		uart_read(uartHandle, &rxbyte, 1);
	}while (rxbyte != SLIP_END);
	while (1) {
		buffer_descriptor d;
		Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);

		uint16_t i = 0;
		uint8_t done = 0;
		while (!done) {
			uart_read(uartHandle, &rxbyte, 1);
			switch (rxbyte) {
			case SLIP_END:
				done = 1;
				break;
			case SLIP_ESC:
				uart_read(uartHandle, &b, 1);
				if (b==SLIP_ESC_ESC) buffers[ d.id ][ i++ ] = SLIP_ESC;
				if (b==SLIP_ESC_END) buffers[ d.id ][ i++ ] = SLIP_END;
				break;
			default:
				buffers[ d.id ][ i++ ] = rxbyte;
				break;
			}
			if (i == BUFFER_SIZE) i--; // don't overrun the buffer
		}

		if (i==0 || i==BUFFER_SIZE-1) { // empty packet or buffer overrun
			Mailbox_post(freeMailbox, &d, BIOS_WAIT_FOREVER);
			continue;
		}

		//System_printf("in uart rx read %d bytes\n",i);
		d.length = i;
		Mailbox_post(uartRxMailbox, &d, BIOS_WAIT_FOREVER);
	}
}
/*
static void XXXuartRxTaskFunction(UArg arg0, UArg arg1) {
	System_printf("uart rx task 1\n");

	//Power_setConstraint(Power_IDLE_PD_DISALLOW);
	//Power_setConstraint(Power_SB_DISALLOW);

	//UART_read(uartHandle, slipRxBuffer, sizeof(slipRxBuffer)); // ignore first packet (may be incomplete)
	readToNewline();
	System_printf("uart rx task 2\n");

	while (1) {
		buffer_descriptor d;
		System_printf("uart rx task 3\n");

		//uint32_t n = UART_read(uartHandle, slipRxBuffer, sizeof(slipRxBuffer));
		uint16_t n = readToNewline();
		System_printf("uart rx task 4\n");
		if (n==sizeof(slipRxBuffer)) {
			System_printf("uart rx skip a full buffer\n",n);
			continue; // got to end of buffer, not END byte
		}

		System_printf("uart rx read %d bytes 1st 0x%02x\n",n,slipRxBuffer[0]);


		Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);

		System_printf("uart rx read %d bytes 1st 0x%02x\n",n,slipRxBuffer[0]);
		d.length = (uint8_t) slipDecode(buffers[d.id], n);

		//leds_blink(LEDS_RED, 3);

		System_printf("uart rx task posting buffer %d len %d 1st 0x%02x\n",d.id, d.length, buffers[d.id][0]);
		Mailbox_post(uartRxMailbox, &d, BIOS_WAIT_FOREVER);
	}
}
*/
#define UART_RX_TASK_STACK_SIZE 512

static Task_Params uartRxTaskParams;
static Task_Struct uartRxTask; /* not static so you can see in ROV */
static uint8_t uartRxTaskStack[UART_RX_TASK_STACK_SIZE];

extern void uartTasks_init(uint32_t uart_index) {
	//Mailbox_Params mboxParams;
	//Mailbox_Params_init(&mboxParams);
//System_printf("1 uart init\n");
	uartTxMailbox = Mailbox_create(sizeof(buffer_descriptor),BUFFER_COUNT, NULL,NULL);
//System_printf("2 uart init\n");
	uartRxMailbox = Mailbox_create(sizeof(buffer_descriptor),BUFFER_COUNT, NULL,NULL);
//System_printf("3 uart init\n");

	UART_Params uartParams;
  UART_Params_init(&uartParams);
  uartParams.writeDataMode = UART_DATA_BINARY;
  //uartParams.writeReturnMode = UART_RETURN_FULL; // don't add a new line, we add it in slipEncode
  uartParams.writeTimeout = UART_WAIT_FOREVER;

  uartParams.readDataMode = UART_DATA_BINARY;
  uartParams.readReturnMode = UART_RETURN_FULL;
  uartParams.readEcho = UART_ECHO_OFF;
  uartParams.readTimeout = UART_WAIT_FOREVER;

  //uartParams.baudRate = 9600;
  uartParams.baudRate = 115200;

#ifdef UART_USE_CALLBACK_MODE
  uartParams.readCallback = uartRxCallback;
  uartParams.writeCallback = uartTxCallback;
  uartParams.writeMode = UART_MODE_CALLBACK;
  uartParams.readMode = UART_MODE_CALLBACK;

	Semaphore_Params semParams;
  Semaphore_Params_init(&semParams);
  semParams.mode = Semaphore_Mode_BINARY;
  uartRxSemaphore = Semaphore_create(0, &semParams, NULL);

  Semaphore_Params_init(&semParams);
  semParams.mode = Semaphore_Mode_BINARY;
  uartTxSemaphore = Semaphore_create(0, &semParams, NULL);

  Semaphore_Params_init(&semParams);
  semParams.mode = Semaphore_Mode_BINARY;
  uartAccessSemaphore = Semaphore_create(1, &semParams, NULL);


#else
  uartParams.writeMode = UART_MODE_BLOCKING;
  uartParams.readMode = UART_MODE_BLOCKING;
#endif
  uartHandle = UART_open(uart_index, &uartParams);

	Task_Params_init(&uartTxTaskParams);
  uartTxTaskParams.stackSize = UART_TX_TASK_STACK_SIZE;
  uartTxTaskParams.priority = UART_TX_TASK_PRIORITY;
  uartTxTaskParams.stack = &uartTxTaskStack;
  uartTxTaskParams.arg0 = (UInt)1000000;

	System_printf("uart tx task init\n");
  Task_construct(&uartTxTask, uartTxTaskFunction, &uartTxTaskParams, NULL);

	Task_Params_init(&uartRxTaskParams);
  uartRxTaskParams.stackSize = UART_RX_TASK_STACK_SIZE;
  uartRxTaskParams.priority = UART_RX_TASK_PRIORITY;
  uartRxTaskParams.stack = &uartRxTaskStack;
  uartRxTaskParams.arg0 = (UInt)1000000;

	System_printf("uart rx task init\n");
  Task_construct(&uartRxTask, uartRxTaskFunction, &uartRxTaskParams, NULL);
}

