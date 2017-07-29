################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/memoria/subdir.mk
-include src/kernel/subdir.mk
-include src/filesystem/subdir.mk
-include src/cpu/subdir.mk
-include src/consola/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: tp-2017-1c-Codeando-por-un-sueldo

# Tool invocations
tp-2017-1c-Codeando-por-un-sueldo: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "tp-2017-1c-Codeando-por-un-sueldo" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) tp-2017-1c-Codeando-por-un-sueldo
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
