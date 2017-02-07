

#TOOL_PREFIX = /opt/ARM/arm-linaro-eabi-4.6
TOOL_PREFIX = /root/ARM/gcc-arm-none-eabi-4_9-2015q2

PRG_PREFIX = $(TOOL_PREFIX)/bin/arm-none-eabi-
LIB_PREFIX = $(TOOL_PREFIX)/arm-none-eabi/lib/thumb/cortex-m4/float-abi-hard/fpuv4-sp-d16/

# LIB_PREFIX is not used

CC      = $(PRG_PREFIX)gcc
LD      = $(PRG_PREFIX)gcc

rsynth-portDIR	= ./rsynth-2.0-port
ttsd= ./english2phoneme

# Object files
OBJECTS = 	startup_stm32f4xx.o system_stm32f4xx.o main.o codec.o i2s.o \
			stm32f4xx_tim.o adc.o stm32f4xx_pwr.o\
			stm32f4xx_gpio.o stm32f4xx_i2c.o stm32f4xx_rcc.o \
			stm32f4xx_spi.o stm32f4xx_dma.o stm32f4xx_adc.o misc.o \
			syscalls.o audio.o LPC/lpc.o effect.o biquad.o vocode.o\
			sp0256.o simpleklatt.o parwave.o sam.o render.o tms5200x.o tube.o channelv.o svf.o \
			wvocoder.o wfilterbank.o digitalker.o nvp.o  braidworm.o voicform.o scformant.o \
			ntube.o worming.o lpc4.o wavetable.o newvotrax.o sp0256bend.o\
			$(ttsd)/parse.o $(ttsd)/saynum.o $(ttsd)/newenglish.o $(ttsd)/phoneme.o $(ttsd)/spellwor.o \
#			lpcanalysis.o
#			ownvocoder.o

OBJECTS += \
 $(rsynth-portDIR)/holmes.o					\
 $(rsynth-portDIR)/elements.o				\
 $(rsynth-portDIR)/nsynth.o					\
 $(rsynth-portDIR)/say.o					\
 $(rsynth-portDIR)/darray.o					\
 $(rsynth-portDIR)/phtoelm.o				\
 $(rsynth-portDIR)/def_pars.o				\
 $(rsynth-portDIR)/trie.o					


# Linker script
#LDSCRIPT = stm32f407.ld
LDSCRIPT = stm32_flash.ld

#CFLAGS = -g -ggdb -O2 -mlittle-endian -mthumb -std=gnu99
#CFLAGS +=  -I. -DARM_MATH_CM4 -D'__FPU_PRESENT=1'-DTEST_PUSHPULL #-DTEST_STRAIGHT #-DPCSIM #-DLACH #-DSUSP #-DTENE #-DTEST_EEG  -DTEST_EFFECTS
#CFLAGS += -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mthumb -mfloat-abi=hard -Wall

# LACH is -DTEST option!

CFLAGS  =  -I. -IInclude -I/~/ARM/CMSIS_5 -Iinc -I./rsynth-2.0-port -I./LPC -I./vocoder -std=gnu99 \
	-DARM_MATH_CM4 -D'__FPU_PRESENT=1' \
	-mthumb \
	-mfpu=fpv4-sp-d16 \
	-c -lm -lc -fno-common -O2 \
	-mcpu=cortex-m4 \
	-mfloat-abi=hard \
	-fsingle-precision-constant -fsigned-char -Wdouble-promotion


AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mthumb -mfloat-abi=hard
#LFLAGS  = -Map main.map -nostartfiles -T $(LDSCRIPT)
#LFLAGS  = -T $(LDSCRIPT) -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -L/root/CMSIS/FFTEX
#LFLAGS_END = -larm_cortexM4lf_math -Wl,--gc-sections,-Map=myfile.map -lm -lc

#LFLAGS  = -T $(LDSCRIPT) -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16  -L$(LIB_PREFIX) 
#LFLAGS_END = -lm -lc -Wl,--gc-sections,-Map=myfile.map

LFLAGS  = -T $(LDSCRIPT) -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -L/root/ARM/CMSIS_5/CMSIS/Lib -L/root/ARM/CMSIS_5/CMSIS/Lib/GCC
#LFLAGS_END = -larm_cortexM4lf_math -ldsplib_lm4f -Wl,--gc-sections,-Map=myfile.map -lm -lc
LFLAGS_END = -larm_cortexM4lf_math -Wl,--gc-sections,-Map=myfile.map -lm -lc

# Executables
ARCH = arm-none-eabi
#CC = $(ARCH)-gcc
#LD = $(ARCH)-ld -v

OBJCPY      = $(PRG_PREFIX)objcopy
OBJDMP      = $(PRG_PREFIX)objdump

AS = $(PRG_PREFIX)as
#OBJCPY = $(ARCH)-objcopy
#OBJDMP = $(ARCH)-objdump
GDB = $(PRG_PREFIX)-gdb

#CPFLAGS = --output-target=binary -j .text -j .data
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

FLASH = st-flash

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) *.lst *.elf *.bin *.map *.dmp

flash: gdb_flash

stlink_flash: main.bin
	$(FLASH) write main.bin 0x08000000

gdb_flash: main.elf
	$(GDB) -x flash_cmd.gdb -batch

disassemble: main.elf
	$(OBJDMP) -dS main.elf > main.dis

dist:
	tar -c *.h *.c *.s Makefile *.cmd *.cfg openocd_doflash | gzip > darkint.zip

main.ihex: main.elf
	$(OBJCPY) --output-target=ihex main.elf main.ihex

main.bin: main.elf 
	$(OBJCPY) $(CPFLAGS) main.elf main.bin
	$(OBJDMP) $(ODFLAGS) main.elf > main.dmp
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(LD) $(LFLAGS) -o main.elf $(OBJECTS) $(LFLAGS_END)

startup_stm32f4xx.o: startup_stm32f4xx.s
	$(AS) $(AFLAGS) startup_stm32f4xx.s -o startup_stm32f4xx.o > startup_stm32f4xx.lst

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

