include ../common/Makefile.common

BINS = bootrom.bin osimage.bin proxmark3_recovery.bin

all: $(BINS)

bootrom.bin: ../bootrom/obj/bootrom.elf
	$(OBJCOPY) --gap-fill=0xff --pad-to 0x00102000 -O binary $^ $@

osimage.bin: ../armsrc/obj/osimage.elf
	$(OBJCOPY) --gap-fill=0xff -O binary $^ $@

proxmark3_recovery.bin: bootrom.bin osimage.bin
	cat bootrom.bin osimage.bin > $@

clean: 
	rm -f $(BINS)

