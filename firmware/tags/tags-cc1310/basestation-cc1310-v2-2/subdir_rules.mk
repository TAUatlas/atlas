################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
CC1310_LAUNCHXL.obj: ../CC1310_LAUNCHXL.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="CC1310_LAUNCHXL.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

basestation.obj: ../basestation.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="basestation.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

buffers.obj: ../buffers.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="buffers.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

ccfg.obj: ../ccfg.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="ccfg.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

i2c_sensors.obj: ../i2c_sensors.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="i2c_sensors.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

leds.obj: ../leds.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="leds.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="main.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

radio_setup.obj: ../radio_setup.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="radio_setup.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

random.obj: ../random.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="random.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

receive.obj: ../receive.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="receive.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

rf_queue_pointer.obj: ../rf_queue_pointer.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="rf_queue_pointer.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

scif.obj: ../scif.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="scif.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

scif_framework.obj: ../scif_framework.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="scif_framework.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

scif_osal_tirtos.obj: ../scif_osal_tirtos.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="scif_osal_tirtos.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sensor_controller.obj: ../sensor_controller.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="sensor_controller.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sensors_batmon.obj: ../sensors_batmon.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="sensors_batmon.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

tag.obj: ../tag.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="tag.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

build-890640431:
	@$(MAKE) -Onone -f subdir_rules.mk build-890640431-inproc

build-890640431-inproc: ../tags-cc1310.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_50_03_33_core/xs" --xdcpath="C:/ti/simplelink_cc13x0_sdk_1_50_00_08/source;C:/ti/simplelink_cc13x0_sdk_1_50_00_08/kernel/tirtos/packages;C:/ti/ccsv7/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS" --compileOptions "-mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path=\"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include\" --include_path=\"C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310\" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-890640431 ../tags-cc1310.cfg
configPkg/compiler.opt: build-890640431
configPkg/: build-890640431

uart.obj: ../uart.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="uart.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

vildehaye.obj: ../vildehaye.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="vildehaye.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

watchdog.obj: ../watchdog.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_15.12.7.LTS/include" --include_path="C:/Users/Nir Zaidman/github/atlas/firmware/tags/tags-cc1310" -g --define=DEBUG --define=DeviceFamily_CC13X0 --define=CC1310_V2_2 --define=VH_BASESTATION_FIRMWARE --define=ccs --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="watchdog.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


