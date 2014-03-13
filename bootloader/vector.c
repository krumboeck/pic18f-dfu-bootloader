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

#include "config.h"

/*
 * Interrupt Vector Remapping
 * Only high vector should be needed
 */

void interrupt_at_high_vector(void) __naked __interrupt 1 {
    __asm
    	goto ENTRY + 0x0008
    __endasm;
}

void interrupt_at_low_vector(void) __naked __interrupt 2 {
    __asm
    	goto ENTRY + 0x0018
    __endasm;
}
