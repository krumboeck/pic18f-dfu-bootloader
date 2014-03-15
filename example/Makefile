
OUTPUT=example
MCU=18f2550

CC=sdcc
AS=gpasm
LD=gplink
SC=srec_cat

CP=cp
MV=mv

ENTRY=0x4000
END=0x7FFF

DFUPY=../dfu/dfu.py

CFLAGS=-S -mpic16 -p$(MCU) -Wall -I/usr/share/sdcc/include/pic16 -I. --ivt-loc=$(ENTRY)
ASFLAGS=
LDFLAGS=-I/usr/share/sdcc/lib/pic16 -w -r -m -s $(MCU).lkr

CSRCS=main.c

ASMSRCS = $(CSRCS:.c=.asm)
OBJS = $(ASMSRCS:.asm=.o)

all: $(OUTPUT).dfu

$(OUTPUT).dfu: $(OUTPUT).bin
	$(DFUPY) -b $(ENTRY):$(OUTPUT).bin $(OUTPUT).dfu

$(OUTPUT).bin: $(OUTPUT).hex
	$(SC) $(OUTPUT).hex -intel -crop $(ENTRY) $(END) -offset -$(ENTRY) -o $(OUTPUT).bin -binary

$(OUTPUT).hex: $(ASMSRCS) $(OBJS) crt018.o
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(OBJS) crt018.o pic$(MCU).lib libsdcc.lib libio$(MCU).lib libc18f.lib

clean:
	rm -f *.o
	rm -f *.asm
	rm -f *.lst
	rm -f *.cod
	rm -f *.hex
	rm -f *.map
	rm -f *.bin
	rm -f *.dfu

%.asm : %.c
	$(CC) $(CFLAGS) $< -o $@

%.o : %.asm
	$(AS) $(ASFLAGS) -o $@ -c $<

crt018.o:
	$(AS) $(ASFLAGS) -p$(MCU) -o $@ -c crt018.s

