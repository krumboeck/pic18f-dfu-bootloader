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

/*
 * Application start address
 */
#define ENTRY 0x4000

/*
 * Comment out the next two lines if there is no led
 */
#define LED_OUTPUT LATBbits.LATB2
#define LED_TRIS TRISBbits.TRISB2

/*
 * Comment out the next three lines if there is no button/jumper
 */
#define BUTTON_TRIS TRISBbits.TRISB4
#define BUTTON_PORT PORTB
#define BUTTON_PIN 4

/*
 * Internal config
 */
#define FLASH_END 0x7FFF
#define EP0_BUFFER_SIZE 32
#define DATA_BUFFER_SIZE 32
#define ERASE_PAGE_SIZE 64
#define MASS_ERASE_TIME 0x04FF
#define WRITE_TIME 0x0004
