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

# The C compiler
CC=xc32-gcc

# The hexfile creator
HX=xc32-bin2hex

# The object dumper
OBJDMP=xc32-objdump

# The PIC device to be programmed
PROCESSOR = 32MX250F128B

# The utility for writing a .hex file to the PIC.
WRITE=nsprog

# The output target $(TARGET).hex
TARGET=out

# The name of the static library lib$(LIB).a
LIB=wcpic32

# Location of source files for static library.
LIBDIR=../wcpic32lib

# Additional linker flags
LINKFLAGS=-Map=$(TARGET).map

# if we have specified a linker script, add it
ifdef LINKSCRIPT
	LINKFLAGS:=$(LINKFLAGS)
endif

# List of object files needed to produce target.
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

HDRS := $(wildcard *.h)

# As of XC32 v3.00, we need -fgnu89-inline
# See https://ww1.microchip.com/downloads/en/DeviceDoc/xc32-v3.00-full-install-release-notes.html#Migration
CFLAGS = -g -O1 -x c -Wall -Wno-unused-value -fgnu89-inline

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
# Use Northern Software's nsprog to program the chip
write: $(TARGET).hex $(TARGET).dis
	$(WRITE) p -d PIC$(PROCESSOR) -i $(TARGETDIR)$(TARGET).hex 
