#include <stdlib.h>
#include <time.h>

#include <xdc/std.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

#include "config.h"

#include "buffers.h"
#include "receive.h"

#include "Board.h"
#include "uart.h"
//#include "uart_printf.h"

#include "radio_setup.h"
#include "tag.h"
#include "basestation.h"
//#include "controller.h"
#include "leds.h"
#include "watchdog.h"
#include "sensor_controller.h"
#include <i2c_sensors.h>

/*
 *  ======== main ========
 */

//void UartPrintf_init(UART_Handle handle);

int main(void) {

	Board_initGeneral();

#ifdef TAG_FIRMWARE
	// first thing, to initialize GPIOs
#ifdef HALL_SENSOR
	sensorcontroller_init();
#endif
#endif

	Board_initUART();

	leds_init();

	System_printf("***BUILD TIME DATE "__TIME__" "__DATE__" ***\n");

	buffers_init();
//	receive_init();

	System_printf("going to radio setup\n");

//	radioSetup_init();

//#if defined(CC1310_V3) || ( defined(CC1310_LAUNCHPAD) && defined(TAG_FIRMWARE) )
#if defined(CC1310_V3)
	uartTasks_init(Board_UART);
	SPI_init();
	i2cSensorsInit();
	testTask_init();
#else

#ifdef TAG_FIRMWARE
  //flash_displayAddresses();
	//watchdog_init( 2*tagPeriodMs, 8*tagPeriodMs );

	//sensorcontroller_startTasks();

	tagTask_init();
#endif

#ifdef BASESTATION_FIRMWARE
	System_printf("goint to uart tasks\n");

	uartTasks_init(Board_UART);
	System_printf("goint to bs tasks\n");
	basestationTask_init();
#endif
#endif

	BIOS_start();

	return (0);
}
