#ifdef VH_BASESTATION_FIRMWARE
#define BASESTATION_FIRMWARE
#endif

//#define LINEAR_REGULATOR

//#define TAG_FIRMWARE
#ifdef VH_TAG_FIRMWARE
#define TAG_FIRMWARE
#endif

#if defined(CC1310_V3)
//#define EXT_REG_1_8V
#endif

#if defined(CC1310_V2_9)
#define HALL_SENSOR
#endif

#ifdef EXT_REG_1_8V
#define SET_CCFG_MODE_CONF_DCDC_RECHARGE             0x1        // Do not use the DC/DC during recharge in powerdown
#define SET_CCFG_MODE_CONF_DCDC_ACTIVE               0x1        // Do not use the DC/DC during active mode
#endif

#ifdef CC1310_LAUNCHPAD
#define CC1310
#define LEDS_TX 1
#define LEDS_RX 2
#define LEDS_ANY 1
#define LEDS_INITIAL_BLINK_COUNTER 0xFF
#define SET_CCFG_BL_CONFIG_BL_PIN_NUMBER             13       // DIO number for boot loader backdoor
#endif

#if defined(CC1310_V2_2) || defined(CC1310_V2_9) || defined(CC1310_V3)
#define CC1310
#define LEDS_TX 1
#define LEDS_RX 1
#define LEDS_ANY 1
#define LEDS_INITIAL_BLINK_COUNTER 0
#define SET_CCFG_BL_CONFIG_BL_PIN_NUMBER             6       // DIO number for boot loader backdoor
#endif

#ifdef CC2650_LAUNCHPAD
#define CC2650
#define LEDS_TX 1
#define LEDS_RX 2
#define LEDS_ANY 1
#define LEDS_INITIAL_BLINK_COUNTER 0xFF
#define SET_CCFG_BL_CONFIG_BL_PIN_NUMBER             15       // DIO number for boot loader backdoor
#endif


//#define RADIO_SETUPS_COUNT 3

//#define RADIO_SETUP_TOA    0
//#define RADIO_SETUP_ID     1
//#define RADIO_SETUP_COMM   2

//#define radio_setup RADIO_SETUP_COMM

//#define SET_CCFG_SIZE_AND_DIS_FLAGS_DIS_ALT_DCDC_SETTING 0x1    // Alternative DC/DC setting disabled
#ifdef LINEAR_REGULATOR
#define SET_CCFG_MODE_CONF_DCDC_RECHARGE             0x1        // Do not use the DC/DC during recharge in powerdown
#define SET_CCFG_MODE_CONF_DCDC_ACTIVE               0x1        // Do not use the DC/DC during active mode
#endif

//#define SET_CCFG_BL_CONFIG_BOOTLOADER_ENABLE         0xC5       // Enable ROM boot loader
//#define SET_CCFG_BL_CONFIG_BL_LEVEL                  0x1        // Active high to open boot loader backdoor
//#define SET_CCFG_BL_CONFIG_BL_PIN_NUMBER             15       // DIO number for boot loader backdoor
//#define SET_CCFG_BL_CONFIG_BL_ENABLE                 0xC5       // Enabled boot loader backdoor

#define SET_CCFG_BL_CONFIG_BOOTLOADER_ENABLE         0xC5       // Enable ROM boot loader
#define SET_CCFG_BL_CONFIG_BL_LEVEL                  0x0        // Active low, has an internal pullup
#define SET_CCFG_BL_CONFIG_BL_ENABLE                 0xC5       // Enabled boot loader backdoor
//#define SET_CCFG_BL_CONFIG_BL_PIN_NUMBER             15       // DIO number for boot loader backdoor; per platform

#define CCFG_FORCE_VDDR_HH 0x0        // Use default VDDR trim
//#define CCFG_FORCE_VDDR_HH 0x1 // Use default VDDR trim

#define BUFFER_COUNT 12
#define BUFFER_SIZE  256

#define LEDS_GRN_TASK_PRIORITY         1
#define LEDS_RED_TASK_PRIORITY         2

#define UART_RX_TASK_PRIORITY          3
#define UART_TX_TASK_PRIORITY          4

#define BASESTATION_UART_TASK_PRIORITY 5
#define BASESTATION_TX_TASK_PRIORITY   6

#define TAG_TASK_PRIORITY              7
#define BASESTATION_TASK_PRIORITY      7

#define WATCHDOG_TASK_PRIORITY         8




