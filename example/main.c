/*
 * PIC18F DFU Bootloader
 *
 * Author: Bernd Krumböck <krumboeck@universalnet.at>
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

#pragma stack 0x200 255

#define LED LATBbits.LATB0

void delay_ms(unsigned int duration) {
	unsigned int i;
	unsigned int j;
	for (i = duration; i != 0; i--) {
		for (j = 0; j <= 1000; j++) {
			__asm
				nop
				nop
				nop
			__endasm;
		}
		__asm
			nop
			nop
		__endasm;
	}
}

void main(void) {
	TRISBbits.TRISB0 = 0;

	while (1) {
		LED = 1;
		delay_ms(500);
		LED = 0;
		delay_ms(500);
	}

}
