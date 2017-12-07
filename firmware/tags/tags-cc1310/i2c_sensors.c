#include <stdlib.h>
#include <stdint.h>
//#include <boolean.h>
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

#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/PIN.h>

#include "Board.h"
#include "config.h"
#include "buffers.h"
#include "uart.h"

static PIN_Handle hFlashPin = NULL;
static PIN_State pinState;

static PIN_Config BoardFlashPinTable[] = {
    Board_SPI_FLASH_CS | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* Ext. flash chip select */
    PIN_TERMINATE
};

static void flashSelect(void) {
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_ON);
}

static void flashDeselect(void) {
    PIN_setOutputValue(hFlashPin,Board_SPI_FLASH_CS,Board_FLASH_CS_OFF);
}

I2C_Handle      i2cHandle;
SPI_Handle      spiHandle;
void i2cSensorsInit() {
	I2C_Params      params;
	I2C_Params_init(&params);
	params.transferMode  = I2C_MODE_BLOCKING;
	i2cHandle = I2C_open(0 /* peripheral index */, &params);
	if (!i2cHandle) {
	    System_printf("I2C did not open");
	}

	hFlashPin = PIN_open(&pinState, BoardFlashPinTable);


    SPI_Params      spiparams;
	SPI_Params_init(&spiparams);
	spiparams.bitRate  = 40000;
    spiparams.mode = SPI_MASTER;
	spiparams.transferMode = SPI_MODE_BLOCKING;
	//spiparams.frameFormat = SPI_POL0_PHA0; // default
	spiparams.frameFormat = SPI_POL1_PHA1; // default
	spiparams.dataSize = 8; // bits, default
	spiHandle = SPI_open(Board_SPI0 , &spiparams);
	if (!spiHandle) {
	    System_printf("SPI did not open");
	}

}


static void testTaskFunction(UArg arg0, UArg arg1) {
	bool ret;
	int i;
	while (1) {
		buffer_descriptor d;
		Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);
		uint8_t* string = buffers[ d.id ];

		Task_sleep(300 / Clock_tickPeriod); // must wait 250us after POR

		SPI_Transaction spiTransaction;
		uint8_t spiTxBuffer[4];
		uint8_t spiRxBuffer[4];
		spiTransaction.txBuf = spiTxBuffer;
		spiTransaction.rxBuf = spiRxBuffer;

		/*
		spiTxBuffer[0] = 0xFF; // reset command
		spiTransaction.count = 1;

		flashSelect(); //test
		ret = SPI_transfer(spiHandle, &spiTransaction);
		flashDeselect();
*/
		Task_sleep(2500 / Clock_tickPeriod); // must wait 2ms after reset

		spiTxBuffer[0] = 0x9A; // read ID command
		spiTxBuffer[1] = 0x0;  // dummy
		spiTxBuffer[2] = 0x0;  // dummy
		spiTxBuffer[3] = 0x0;  // dummy

		spiTransaction.count = 4;

		spiTxBuffer[0] = 0x9F; // read ID command
		spiTransaction.count = 3;

		flashSelect(); //test
		ret = SPI_transfer(spiHandle, &spiTransaction);
		flashDeselect();

		sprintf(string+strlen(string), "spi ret=%d read-id=%02x %02x\r\n",ret, spiRxBuffer[2], spiRxBuffer[1]);
		System_printf("spi ret=%d read-id=%02x %02x\r\n",ret, spiRxBuffer[2], spiRxBuffer[1]);

#if 1
		I2C_Transaction i2cTransaction;
		i2cTransaction.slaveAddress = (0x76 | 0x00); // BME280 with address pin tied to VDD

		uint8_t value;
		uint8_t writeBuffer[2];

		writeBuffer[0] = 0xD0; // ID
		i2cTransaction.writeBuf = writeBuffer; // register id
		i2cTransaction.writeCount = 1;
		i2cTransaction.readBuf = &value;
		i2cTransaction.readCount = 1;
		ret = I2C_transfer(i2cHandle, &i2cTransaction);
		sprintf(string+strlen(string), "ret=%d id=0x%02x\r\n",ret, value);

		writeBuffer[0] = 0xF2; // CTRL_HUM
		writeBuffer[1] = 0x01; // measure humidity, no oversampling
		i2cTransaction.writeBuf = writeBuffer; // register id
		i2cTransaction.writeCount = 2;
		i2cTransaction.readBuf = &value;
		i2cTransaction.readCount = 0;
		ret = I2C_transfer(i2cHandle, &i2cTransaction);

		writeBuffer[0] = 0xF4; // CTRL_MEAS
		writeBuffer[1] = (1 << 5) // measure temp, no oversampling
				       | (1 << 2) // measure pressure, no oversampling
					   | 3;       // formced (one-shot) mode
		i2cTransaction.writeBuf = writeBuffer; // register id
		i2cTransaction.writeCount = 2;
		i2cTransaction.readBuf = &value;
		i2cTransaction.readCount = 0;
		ret = I2C_transfer(i2cHandle, &i2cTransaction);

		do {
			writeBuffer[0] = 0xF3; // status
			i2cTransaction.writeBuf = writeBuffer; // register id
			i2cTransaction.writeCount = 1;
			i2cTransaction.readBuf = &value;
			i2cTransaction.readCount = 1;
			ret = I2C_transfer(i2cHandle, &i2cTransaction);
		} while ((value & 0x04) != 0); // while measuring, continue to poll

		uint8_t data[8];
		writeBuffer[0] = 0xF7; // all measurements
		i2cTransaction.writeBuf = writeBuffer; // register id
		i2cTransaction.writeCount = 1;
		i2cTransaction.readBuf = data;
		i2cTransaction.readCount = 8;
		ret = I2C_transfer(i2cHandle, &i2cTransaction);

		for (i=0; i<8; i++) {
			sprintf(string + strlen(string), "%02x ",data[i]);
		}
#endif

#if 1
		// BMI160
//		I2C_Transaction i2cTransaction;
		i2cTransaction.slaveAddress = (0x68 | 0x00); // BMI160 with address pin tied to GND

//		uint8_t value;
//		uint8_t writeBuffer[2];

		writeBuffer[0] = 0x00; // ID register, should be 11010001=
		i2cTransaction.writeBuf = writeBuffer; // register id
		i2cTransaction.writeCount = 1;
		i2cTransaction.readBuf = &value;
		i2cTransaction.readCount = 1;
		ret = I2C_transfer(i2cHandle, &i2cTransaction);
		sprintf(string+strlen(string), "BMI160 ret=%d id=0x%02x\r\n",ret, value);

#endif
		sprintf(string+strlen(string), "\r\n");

		d.length = strlen(string);
		Mailbox_post(uartTxMailbox, &d, BIOS_WAIT_FOREVER);


		Task_sleep(4000000 / Clock_tickPeriod);
	}
}

#define TAG_TASK_STACK_SIZE 1024

static Task_Params testTaskParams;
Task_Struct testTask;    /* not static so you can see in ROV */
static uint8_t testTaskStack[TAG_TASK_STACK_SIZE];

void testTask_init() {

	Task_Params_init(&testTaskParams);
	testTaskParams.stackSize = TAG_TASK_STACK_SIZE;
	testTaskParams.priority  = TAG_TASK_PRIORITY;
	testTaskParams.stack     = &testTaskStack;
	testTaskParams.arg0      = (UInt)1000000;

	Task_construct(&testTask, testTaskFunction, &testTaskParams, NULL);
}




/* value should be  0110 1000 = 0x68 */
uint8_t mpu9150WhoAmI(uint8_t* value, uint8_t address0) {
	I2C_Transaction i2cTransaction;
	uint8_t regAddress[] = { 0x75 };
	i2cTransaction.writeBuf = regAddress;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = value;
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = (0x68 | address0);
	bool ret = I2C_transfer(i2cHandle, &i2cTransaction);
	if (!ret) return 0;
	else      return 1;
}

