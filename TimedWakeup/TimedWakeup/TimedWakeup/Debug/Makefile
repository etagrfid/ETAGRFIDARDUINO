################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../linker_scripts/ \
../linker_scripts/linker_scripts/ \
../linker_scripts/linker_scripts/gcc/


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../ArduinoLowPower.cpp \
../RTCZero.cpp \
../Sketch.cpp


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
ArduinoLowPower.o \
RTCZero.o \
Sketch.o

OBJS_AS_ARGS +=  \
ArduinoLowPower.o \
RTCZero.o \
Sketch.o

C_DEPS +=  \
ArduinoLowPower.d \
RTCZero.d \
Sketch.d

C_DEPS_AS_ARGS +=  \
ArduinoLowPower.d \
RTCZero.d \
Sketch.d

OUTPUT_FILE_PATH +=TimedWakeup.elf

OUTPUT_FILE_PATH_AS_ARGS +=TimedWakeup.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+=  \
C:/Users/mb235713/Documents/Atmel\ Studio/7.0/TimedWakeup/TimedWakeup/ArduinoCore/Debug/libArduinoCore.a

LINKER_SCRIPT_DEP+=  \
../linker_scripts/linker_scripts/gcc/flash_with_bootloader.ld


# AVR32/GNU C Compiler







./%.o: .././%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.2.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD21G18A__ -DDEBUG -DF_CPU=48000000L -DARDUINO=10805 -DARDUINO_SAM_ZERO -DARDUINO_ARCH_SAM -DUSB_VID=0x2a03 -DUSB_PID=0x804f -DUSB_PRODUCT="\"Arduino M0 Pro\"" -DUSB_MANUFACTURER="\"Arduino LLC\"" -DUSBCON -DSAMD_SERIES -DARDUINO_ARCH_SAMD  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\4.2.0\CMSIS\Include" -I"..\\..\ArduinoCore\include\core" -I"..\\..\ArduinoCore\include\variants" -I"..\\..\ArduinoCore\include\core\avr" -I"..\\..\ArduinoCore\include\core\USB" -I"..\\..\ArduinoCore\include\libraries\Arduino_Low_Power" -I"..\\..\ArduinoCore\include\libraries\RTCZero" -I"../../../../../../../AppData/Local/Arduino15/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Include" -I"..\..\..\..\..\..\..\AppData\Local\Arduino15\packages\arduino\tools\CMSIS-Atmel\1.1.0\CMSIS\Device\ATMEL"  -Os -fno-threadsafe-statics -fno-exceptions -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -g3 -w -mcpu=cortex-m0plus -c -std=gnu++11 --param max-inline-insns-single=500 -nostdlib -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Linker : 6.2.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -mthumb -Wl,-Map="TimedWakeup.map" --specs=nano.specs --specs=nosys.specs -Wl,--start-group -lm -lArduinoCore  -Wl,--end-group -L"..\linker_scripts\linker_scripts\gcc" -L"C:\Users\mb235713\Documents\Atmel Studio\7.0\TimedWakeup\TimedWakeup\ArduinoCore\Debug"  -Wl,--gc-sections -mcpu=cortex-m0plus -Tflash_without_bootloader.ld -Wl,--cref -Os -Wl,--check-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O binary "TimedWakeup.elf" "TimedWakeup.bin"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "TimedWakeup.elf" "TimedWakeup.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary "TimedWakeup.elf" "TimedWakeup.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objdump.exe" -h -S "TimedWakeup.elf" > "TimedWakeup.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "TimedWakeup.elf" "TimedWakeup.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-size.exe" "TimedWakeup.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "TimedWakeup.elf" "TimedWakeup.a" "TimedWakeup.hex" "TimedWakeup.bin" "TimedWakeup.lss" "TimedWakeup.eep" "TimedWakeup.map" "TimedWakeup.srec"
	