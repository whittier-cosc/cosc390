############################################################
#
# Makefile for Whittier College PIC32 projects
#
# Jeff Lutgen
#
# Inspired by the Makefile from Northwestern's NU32 project
#
#############################################################

# This file contains rules to do the following:
#	1. compile .c files into .o files
#	2. link the .o files in this directory (and any library code they use) into 
#		a .elf binary
#	3. convert the .elf into a .hex
#	4. write the .hex file to the PIC
#
# The following may need to be edited by you so that
# make knows where to find the compiler tools (xc32-*).
#
# XC32PATH is the path to the xc32 bin directory. This directory contains
# tools such as the compiler (xc32-gcc) and hex converter 
# (xc32-bin2hex)
# Windows example (note the use of slashes not backslashes):
# XC32PATH=C:/Program Files (x86)/Microchip/xc32/v1.40/bin
# Mac example:
# XC32PATH =/Applications/microchip/xc32/v1.40/bin

# IPEPATH is the path to `ipecmd', the command-line tool for programming
# Microchip devices.

ifdef WSL_DISTRO_NAME        # Windows Subsystem for Linux
    XC32PATH=/mnt/c/Program\ Files/Microchip/xc32/v3.01/bin
    IPEPATH=/mnt/c/Program\ Files/Microchip/MPLABX/v5.50/mplab_platform/mplab_ipe
else
    XC32PATH=
    IPEPATH=   
endif



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

# How to call MPLab's ipecmd
ifdef OS     # Windows
    IPECMD=ipecmd
else ifdef WSL_DISTRO_NAME # Windows Subsystem for Linux
    IPECMD=$(IPEPATH)/ipecmd.exe
else         # MacOS
    IPECMD=/Applications/microchip/mplabx/v5.15/mplab_platform/mplab_ipe/bin/ipecmd.sh
endif

ifdef WSL_DISTRO_NAME
    # The C compiler
    CC=$(XC32PATH)/xc32-gcc.exe
    
    # The hexfile creator
    HX=$(XC32PATH)/xc32-bin2hex.exe
    
    # The object dumper
    OBJDMP=$(XC32PATH)/xc32-objdump.exe
else
    # The C compiler
    CC=xc32-gcc
    
    # The hexfile creator
    HX=xc32-bin2hex
    
    # The object dumper
    OBJDMP=xc32-objdump
endif

# The utility for writing a .hex file to the PIC.
WRITE=$(IPECMD) -P32MX250F128B -TP$(TOOL) -M -OL

# The output target $(TARGET).hex
TARGET=out

# The name of the static library lib$(LIB).a
LIB=wcpic32

# Location of source files for static library.
LIBDIR=../wcpic32lib

ifdef OS        # Windows
	TARGETDIR=
else ifdef WSL_DISTRO_NAME
	TARGETDIR=
else            # MacOS
	TARGETDIR=${CURDIR}/
endif

# Additional linker flags
LINKFLAGS=-Map=$(TARGET).map

# if we have specified a linker script, add it
ifdef LINKSCRIPT
	LINKFLAGS:=$(LINKFLAGS)
endif

# List of object files needed to produce target.
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

HDRS := $(wildcard *.h)

PROCESSOR = 32MX250F128B
# As of XC32 v3.00, we need -fgnu89-inline
# See https://ww1.microchip.com/downloads/en/DeviceDoc/xc32-v3.00-full-install-release-notes.html#Migration
CFLAGS = -g -O1 -x c -Wall -Wno-unused-value -fgnu89-inline

# if on Windows (but not MinGW), use a different RM
#ifdef OS
#    ifndef MINGW_PREFIX
#	RM = del /Q
#    endif
#endif

# What to do for "make all"
.PHONY: all
all : $(TARGET).hex $(TARGET).dis
# Turn the elf file into a hex file.
$(TARGET).hex: $(TARGET).elf
	@echo Creating hex file $@
	$(HX) $(TARGET).elf

# Generate disassembly file.
$(TARGET).dis: $(TARGET).elf
	@echo Creating disassembly file $@
	$(OBJDMP) -S $< > $@

# Link all the object files and any local library code used by them into an elf file.
$(TARGET).elf: $(OBJS) $(LIBDIR)/lib$(LIB).a
	@echo Linking elf file $@
	$(CC) -mprocessor=$(PROCESSOR) -o $(TARGET).elf $(OBJS) -Wl,$(LINKFLAGS) \
	-L$(LIBDIR) -l$(LIB)

# Create an object file for each C file. Force recompile if *any* header has changed.
%.o: %.c $(HDRS)
	@echo Creating object file $@
	$(CC) $(CFLAGS) -I$(LIBDIR) -c -mprocessor=$(PROCESSOR) -o $@ $<

# How to build the static library
$(LIBDIR)/lib$(LIB).a:
	make -C $(LIBDIR)

.PHONY: clean
# Delete all hex, map, object, and elf files, and other assorted crud
clean:
	$(RM) *.hex *.map *.o *.a *.elf *.dep *.dis log.* *.xml* *~

.PHONY: write
# Use MPLAB's ipecmd to program the chip
write: $(TARGET).hex $(TARGET).dis
	$(WRITE) -F$(TARGETDIR)$(TARGET).hex 

.PHONY: batch-write
# Use MPLAB's ipecmd in batch mode (faster after the first download)
# to program the chip
batch-write: $(TARGET).hex $(TARGET).dis
	$(WRITE) -F$(TARGETDIR)$(TARGET).hex -OB

.PHONY: termite
termite:
	"/c/Program Files (x86)/Termite/Termite.exe" &

.PHONY: putty
putty:
	"$(TERMEMU)" -serial -sercfg 230400,R "$(PORT)"

.PHONY: screen
screen:
	"$(TERMEMU)" "$(PORT)" 230400,crtscts
