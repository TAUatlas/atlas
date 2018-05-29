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
#include "spi_flash.h"
#include "bmi160_support.h"

I2C_Handle      i2c;
I2C_Params      i2cParams;
I2C_Transaction i2cTransaction;
uint8_t txBuffer[10];
uint8_t rxBuffer[30];
float factor = 1;
uint8_t gRange = 2;

#define TAG_TASK_STACK_SIZE 1024

static Task_Params testTaskParams;
Task_Struct testTask;    /* not static so you can see in ROV */
static uint8_t testTaskStack[TAG_TASK_STACK_SIZE];
Task_Struct sensorsUartTask;    /* not static so you can see in ROV */
static void bmi160Function(UArg arg0, UArg arg1);
void readDataFunction();

void i2cSensorsInit() {
    I2C_init();
	I2C_Params      params;
	I2C_Params_init(&params);
	params.transferMode  = I2C_MODE_BLOCKING;
	i2c = I2C_open(0 /* peripheral index */, &params);
	if (!i2c) {
	    System_printf("I2C did not open");
	}

	Task_Params_init(&testTaskParams);
    testTaskParams.stackSize = TAG_TASK_STACK_SIZE;
    testTaskParams.priority  = 5;
    testTaskParams.stack     = &testTaskStack;
    testTaskParams.arg0       = (UInt)1000000;

    Task_construct(&testTask, bmi160Function, &testTaskParams, NULL);
}

void bmi160Setup_TicksFactor(const uint32_t* sensorData) {
    factor=*sensorData;
}

void bmi160Setup_GRange(const uint8_t* sensorData) {
    gRange = *sensorData;
}
static void bmi160Function(UArg arg0, UArg arg1) {
	bmi160_initialize_sensor(gRange);
	struct bmi160_accel_t accelxyz;
    BMI160_RETURN_FUNCTION_TYPE com_rslt;
    int i=0,pageCounter=0;
    uint8_t byteCounter = 9;
    uint8_t page[PAGE_SIZE];
    uint8_t check[PAGE_SIZE];
    u8 v_data_rdy_u8;
    buffer_descriptor d;

    bool b;
    Types_FreqHz freq;
    Timestamp_getFreq(&freq);

    // check commands - long wait for flash boot and enough time for user command
    if (Mailbox_pend(uartRxMailbox, &d, 10000000)){
        if (buffers[d.id][0] == 0xa){
            readDataFunction();
            return;
        }
    }

    // read configuration
    spiFlashReadPage(pageCounter, page);

    uint32_t ticks = factor * 1000000 / Clock_tickPeriod;//*((uint32_t*)(page+2));
    // write mode
    spiFlashEraseChip(); // can't start writing without clearing the flash
    uint32_t start = Timestamp_get32();
	while (true){
        Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);
        uint8_t* string = buffers[ d.id ];
        if (bmi160_get_accel_data_rdy(&v_data_rdy_u8) == 0){
            if (bmi160_read_accel_xyz(&accelxyz) != 0){
                sprintf(string, "BMI160 ret=%d error reading\r\n",com_rslt);
            }else{
                //BMI160_RETURN_FUNCTION_TYPE com_rslt = bmi160_read_gyro_xyz(&gyro);
                sprintf(string, "BMI160 x=%d y=%d z=%d count=%d page=%d time=%d\r\n", (uint16_t)accelxyz.x, (uint16_t)accelxyz.y, (uint16_t)accelxyz.z, byteCounter, pageCounter, Timestamp_get32()/freq.lo);
                if (PAGE_SIZE - byteCounter > 6){
                    *((struct bmi160_accel_t*)(page + byteCounter)) = accelxyz;
                    byteCounter+=6;
                }else{
                    uint32_t time = Timestamp_get32()/freq.lo;
                    sprintf(string, "BMI160 freq.lo=%d now=%d factor=%d G=%d\r\n", freq.lo, time, factor, gRange);
                    page[0] = BMI_160_PAGE;

                    memcpy(page+1, &time, 4);
                    memcpy(page+5, &factor, 4);
                    spiFlashWritePage(pageCounter, page);

                    // check
                    spiFlashReadPage(pageCounter, check);
                    b = true;
                    for (i=0;i<256;i++){
                        if (check[i] != page[i]){
                            b = false;
                            break;
                        }
                    }

                    if (b){
                        sprintf(string+strlen(string), "CHECK page=%d\r\n", pageCounter);
                        pageCounter++;
                    }

                    byteCounter = 9;
                    start = Timestamp_get32();
                }
            }
        }else{
            sprintf(string, "BMI160 ret=%d d=%d not ready\r\n",com_rslt,v_data_rdy_u8);
        }

#if 0
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

		d.length = strlen(string);
		Mailbox_post(uartTxMailbox, &d, BIOS_WAIT_FOREVER);

		Task_sleep(ticks);
	}
}

void readDataFunction(){
    uint32_t i=0;
    uint8_t page[PAGE_SIZE];
    spiFlashReadPage(i, page);
    while (page[0] != EMPTY_PAGE){
        i++;
        buffer_descriptor d;
        Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);
        uint8_t* string = buffers[ d.id ];
        memcpy(string, page, PAGE_SIZE);
        d.length = PAGE_SIZE;
        Mailbox_post(uartTxMailbox, &d, BIOS_WAIT_FOREVER);
        spiFlashReadPage(i, page);
    }

    // end packet
    buffer_descriptor d;
    Mailbox_pend(freeMailbox, &d, BIOS_WAIT_FOREVER);
    buffers[ d.id ][0] = DONE_READ_PAGE;
    d.length = 1;
    Mailbox_post(uartTxMailbox, &d, BIOS_WAIT_FOREVER);
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
	bool ret = I2C_transfer(i2c, &i2cTransaction);
	if (!ret) return 0;
	else      return 1;
}

