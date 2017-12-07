################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../tags-cc1310.cfg 


# Each subdirectory must supply rules for building sources it contributes
configPkg/linker.cmd: ../tags-cc1310.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"/xs" --xdcpath= xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC1310F128 -r release -c --compileOptions " " "$<"
	@echo 'Finished building: $<'
	@echo ' '


