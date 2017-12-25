################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS

GEN_OPTS__FLAG := --cmd_file="configPkg/compiler.opt"
GEN_CMDS__FLAG := -l"configPkg/linker.cmd"

ORDERED_OBJS += \
"./CC1310_LAUNCHXL.obj" \
"./basestation.obj" \
"./buffers.obj" \
"./ccfg.obj" \
"./i2c_sensors.obj" \
"./leds.obj" \
"./main.obj" \
"./radio_setup.obj" \
"./random.obj" \
"./receive.obj" \
"./rf_queue_pointer.obj" \
"./scif.obj" \
"./scif_framework.obj" \
"./scif_osal_tirtos.obj" \
"./sensor_controller.obj" \
"./sensors_batmon.obj" \
"./tag.obj" \
"./uart.obj" \
"./vildehaye.obj" \
"./watchdog.obj" \
"../cc13x0f128.cmd" \
$(GEN_CMDS__FLAG) \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/grlib/lib/grlib.aem3" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/display/lib/display.aem3" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/mw/lcd/lib/lcd.aem3" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x0.aem3" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/devices/cc13x0/driverlib/bin/ccs/driverlib.lib" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/drivers/rf/lib/rf_multiMode_cc13x0.aem3" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/drivers/lib/drivers_cc13x0.aem3" \
-l"C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source/ti/drivers/pdm/lib/pdm_cc13x0.aem3" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
tag-cc1310-v3.out \

EXE_OUTPUTS__QUOTED += \
"tag-cc1310-v3.out" \

BIN_OUTPUTS += \
tag-cc1310-v3.hex \

BIN_OUTPUTS__QUOTED += \
"tag-cc1310-v3.hex" \


# All Target
all: tag-cc1310-v3.out secondary-outputs

# Tool invocations
tag-cc1310-v3.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/bin/armcl" -mv7M3 --code_state=16 -me -O2 --define=DeviceFamily_CC13X0 --define=DEBUG --define=CC1310_V3 --define=VH_TAG_FIRMWARE --define=ccs -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi -z -m"tags-cc1310.map" --heap_size=0 --stack_size=1024 -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/lib" -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="tags-cc1310_linkInfo.xml" --rom_model -o "tag-cc1310-v3.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

tag-cc1310-v3.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.6.LTS/bin/armhex" --memwidth=8 --romwidth=8 --intel -o "tag-cc1310-v3.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_CMDS__QUOTED)$(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)$(GEN_OPTS__QUOTED)
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-$(RM) "CC1310_LAUNCHXL.obj" "basestation.obj" "buffers.obj" "ccfg.obj" "i2c_sensors.obj" "leds.obj" "main.obj" "radio_setup.obj" "random.obj" "receive.obj" "rf_queue_pointer.obj" "scif.obj" "scif_framework.obj" "scif_osal_tirtos.obj" "sensor_controller.obj" "sensors_batmon.obj" "tag.obj" "uart.obj" "vildehaye.obj" "watchdog.obj" 
	-$(RM) "CC1310_LAUNCHXL.d" "basestation.d" "buffers.d" "ccfg.d" "i2c_sensors.d" "leds.d" "main.d" "radio_setup.d" "random.d" "receive.d" "rf_queue_pointer.d" "scif.d" "scif_framework.d" "scif_osal_tirtos.d" "sensor_controller.d" "sensors_batmon.d" "tag.d" "uart.d" "vildehaye.d" "watchdog.d" 
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

