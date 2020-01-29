################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F3xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Core/MIDI/subdir.mk
-include Core/Display/subdir.mk
-include Core/Console/subdir.mk
-include Core/Audio/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
experiment.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
experiment.list \

OBJCOPY_BIN += \
experiment.bin \


# All Target
all: experiment.elf secondary-outputs

# Tool invocations
experiment.elf: $(OBJS) $(USER_OBJS) /Users/C/Documents/Source/STM32CubeWorkspace/experiment/STM32F303ZETX_FLASH.ld
	arm-none-eabi-gcc -o "experiment.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/Users/C/Documents/Source/STM32CubeWorkspace/experiment/STM32F303ZETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="experiment.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

experiment.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "experiment.list"
	@echo 'Finished building: $@'
	@echo ' '

experiment.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "experiment.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
