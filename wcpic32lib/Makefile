############################################################
#
# Makefile for Whittier College PIC32 library
#
# Jeff Lutgen
#
#############################################################

ifdef WSL_DISTRO_NAME        # Windows Subsystem for Linux
    XC32PATH=/mnt/c/Program\ Files\ \(x86\)/Microchip/xc32/v1.40/bin
	CC=$(XC32PATH)/xc32-gcc.exe
	AR=$(XC32PATH)/xc32-ar.exe
else
	CC=xc32-gcc
	AR=xc32-ar
endif

# The name of the static library lib$(LIB).a
LIB=wcpic32

ifdef OS        # Windows
	TARGETDIR=
else            # MacOS
	TARGETDIR=${CURDIR}/
endif

# List of object files needed to produce target.
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

HDRS := $(wildcard *.h)

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
all : lib$(LIB).a

# Create static library.
lib$(LIB).a: $(OBJS)
	@echo Creating static library $@
	$(AR) -rcs lib$(LIB).a $(OBJS)

# Create an object file for each C file. Force recompile if *any* header has changed.
%.o: %.c $(HDRS)
	@echo Creating object file $@
	$(CC) $(CFLAGS) -c -mprocessor=$(PROCESSOR) -o $@ $<

.PHONY: clean
# Delete all hex, map, object, library, and elf files, and other assorted crud
clean:
	$(RM) *.o *.a *.dep log.* *.xml* *~
