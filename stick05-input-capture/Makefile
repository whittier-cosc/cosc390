############################################################
#
# Makefile for PIC32 projects
#
# Jeff Lutgen
#
# Inspired by the Makefile from Northwestern's NU32 project
#
#############################################################

#This file is used to:
#	1. compile .c files into .o files
#	2. link the .o files into a .elf binary
#	3. convert the .elf into a .hex
#	4. write the .hex file to the PIC
# The following may need to be edited by you so that
# make knows where to find the compiler tools (xc32).
#
# XC32PATH is the path to the xc32 bin directory. This directory contains
# tools such as the compiler (xc32-gcc) and hex converter 
# (xc32-bin2hex)
# Windows example (note the use of slashes not backslashes):
# XC32PATH=C:/Program Files (x86)/Microchip/xc32/v1.40/bin
# Mac example:
# XC32PATH =/Applications/microchip/xc32/v1.40/bin
#
# PORT is the COM port used by the PIC32
# Windows Example:
# PORT=\\.\COM22
# Mac Example:
# PORT=/dev/tty.usbserial=00001024A
#
# TERMEMU is your terminal emulator
# Windows example:
# TERMEMU=C:/Program Files (x86)/PuTTY/putty.exe
# Mac example (screen is pre-installed and already on your execution path 
# so you can safely omit the full path)
# TERMEMU=screen

#TOOL=PKOB   # microstickII pickit on board
TOOL=PK4     # pickit 4

# Compiler flags for additional include directories
INCLUDES = -Iperipherals

# How to call MPLab's ipecmd
ifdef OS     # Windows
	IPECMD=ipecmd.exe
else         # MacOS
	IPECMD=/Applications/microchip/mplabx/v5.15/mplab_platform/mplab_ipe/bin/ipecmd.sh
endif

# the C compiler
CC=xc32-gcc

# the hexfile creator
HX=xc32-bin2hex

# the object dumper
OBJDMP=xc32-objdump

# the download utility
WRITE=$(IPECMD) -P32MX250F128B -TP$(TOOL) -M -OL

# the output target $(TARGET).hex
TARGET=out

ifdef OS        # Windows
	TARGETDIR=
else            # MacOS
	TARGETDIR=${CURDIR}/
endif

# additional linker flags
LINKFLAGS=-Map=$(TARGET).map

# if we have specified a linker script, add it
ifdef LINKSCRIPT
	LINKFLAGS:=$(LINKFLAGS)
endif

OBJS := $(patsubst %.c, %.o, $(wildcard *.c)) \
        $(patsubst %.c, %.o, $(wildcard peripherals/*.c))
HDRS := $(wildcard *.h) \
        $(wildcard peripherals/*.h)
PROCESSOR = 32MX250F128B
CFLAGS = -g -O1 -x c -Wall

# if on Windows (but not MinGW), use a different RM
ifdef OS
    ifndef MINGW_PREFIX
	RM = del /Q
    endif
endif

# What to do for "make all"
.PHONY: all
all : $(TARGET).hex $(TARGET).dis
# Turn the elf file into a hex file.
$(TARGET).hex: $(TARGET).elf
	@echo Creating hex file
	$(HX) $(TARGET).elf

# Perform disassembly.
$(TARGET).dis: $(TARGET).elf
	$(OBJDMP) -S $< > $@

# Link all the object files into an elf file.
$(TARGET).elf: $(OBJS)
	@echo Linking elf file
	$(CC) -mprocessor=$(PROCESSOR) -o $(TARGET).elf $(OBJS) -Wl,$(LINKFLAGS)

# Create an object file for each C file. Force recompile if *any* header has changed.
%.o: %.c $(HDRS)
	@echo Creating object file $@
	$(CC) $(CFLAGS) $(INCLUDES) -c -mprocessor=$(PROCESSOR) -o $@ $<

.PHONY: clean
# Delete all hex, map, object, and elf files, and other assorted crud
clean:
	$(RM) *.hex *.map *.o *.elf *.dep *.dis log.* *.xml* *~
	cd peripherals && $(RM) *.hex *.map *.o *.elf *.dep *.dis log.* *.xml* *~

.PHONY: write
# Use MPLAB's ipecmd to program the chip
write: $(TARGET).hex $(TARGET).dis
	$(WRITE) -F$(TARGETDIR)$(TARGET).hex 

.PHONY: putty
putty:
	"$(TERMEMU)" -serial -sercfg 230400,R "$(PORT)"

.PHONY: screen
screen:
	"$(TERMEMU)" "$(PORT)" 230400,crtscts
