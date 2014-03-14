
all:
	make -C bootloader all
	make -C example all
	
clean:
	make -C bootloader clean
	make -C example clean
