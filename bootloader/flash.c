/*
 * PIC18F DFU Bootloader
 *
 * Author: Bernd Krumböck <krumboeck@universalnet.at>
 *         Based on André Gentric's and
 *         Régis Blanchot's <rblanchot@gmail.com> USB Pinguino Bootloader,
 *         Based on Albert Faber's JAL bootloader
 *         and Alexander Enzmann's USB Framework
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#include <pic18fregs.h>
#include "typedef.h"
#include "flash.h"

void eraseFlash(u32 address) {

    PIR2bits.EEIF = 0;

	TBLPTRL = (address) & 0xFF;
	TBLPTRH = (address >> 8) & 0xFF;
	TBLPTRU = (address >> 16) & 0xFF;

    /*
     * bit 7, EEPGD = 1, memory is flash (unimplemented on J PIC)
     * bit 6, CFGS  = 0, enable acces to flash (unimplemented on J PIC)
     * bit 5, WPROG = 1, enable single word write (unimplemented on non-J PIC)
     * bit 4, FREE  = 0, enable write operation (1 if erase operation)
     * bit 3, WRERR = 0,
     * bit 2, WREN  = 1, enable write to memory
     * bit 1, WR    = 0,
     * bit 0, RD    = 0, (unimplemented on J PIC)
     */

    EECON1 = 0xA4; // 0b10100100

    EECON1bits.FREE = 1;// perform erase operation
    EECON2 = 0x55;      // unlock sequence
    EECON2 = 0xAA;      // unlock sequence
    EECON1bits.WR = 1;  // start write or erase operation
    EECON1bits.FREE = 0;// back to write operation

    while (!PIR2bits.EEIF);
    PIR2bits.EEIF = 0;
    EECON1bits.WREN = 0;
}

void readFlash(u32 address, u8 *buffer, u8 length) {

	int counter;

	TBLPTRL = (address) & 0xFF;
	TBLPTRH = (address >> 8) & 0xFF;
	TBLPTRU = (address >> 16) & 0xFF;


	/*
    EECON1 = 0xA4; // 0b10100100
    EECON2 = 0x55;
    EECON2 = 0xAA;
	EECON1bits.RD = 1;
	*/

	EECON1bits.EEPGD = 1;
	EECON1bits.CFGS = 0;
	EECON1bits.RD = 1;

    __asm
    	NOP
    	NOP
    	NOP
    __endasm;

	for (counter = 0; counter < length; counter++) {
        // TBLPTR is incremented after the read
        __asm
        	TBLRD*+
        __endasm;
        *(buffer + counter) = TABLAT;
    }

}

void writeFlash(u32 address, u8 *buffer, u8 length) {

	int counter;

    PIR2bits.EEIF = 0;

	TBLPTRL = (address) & 0xFF;
	TBLPTRH = (address >> 8) & 0xFF;
	TBLPTRU = (address >> 16) & 0xFF;

    /*
     * bit 7, EEPGD = 1, memory is flash (unimplemented on J PIC)
     * bit 6, CFGS  = 0, enable acces to flash (unimplemented on J PIC)
     * bit 5, WPROG = 1, enable single word write (unimplemented on non-J PIC)
     * bit 4, FREE  = 0, enable write operation (1 if erase operation)
     * bit 3, WRERR = 0,
     * bit 2, WREN  = 1, enable write to memory
     * bit 1, WR    = 0,
     * bit 0, RD    = 0, (unimplemented on J PIC)
     */

    EECON1 = 0xA4; // 0b10100100

    INTCONbits.GIE = 0;

    /// The programming block is 32 bytes for all chips except x5k50
    /// The programming block is 64 bytes for x5k50.

    // Load max. 32 holding registers
    for (counter = 0; counter < length; counter++) {
        TABLAT = buffer[counter]; // present data to table latch
        // write data in TBLWT holding register
        // TBLPTR is incremented after the read/write
        __asm
        	TBLWT*+
        __endasm;
    }

    // start block write
    // one step back to be inside the 32 bytes range
    __asm
    	TBLRD*-
    __endasm;


    EECON2 = 0x55;
    EECON2 = 0xAA;

    EECON1bits.WR = 1;      // WR = 1; start write or erase operation
                            // WR cannot be cleared, only set, in software.
                            // It is cleared in hardware at the completion
                            // of the write or erase operation.
                            // CPU stall here for 2ms

    INTCONbits.GIE = 1;

    while (!PIR2bits.EEIF);
    PIR2bits.EEIF = 0;
    EECON1bits.WREN = 0;

}
