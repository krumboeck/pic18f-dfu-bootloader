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

#ifdef _DEBUG
	#include <stdio.h>

  #define debug(x) printf(x)
  #define debug2(x,y) printf(x,y)

  #ifdef _DEBUG_USB
    #define debug_usb(x) printf(x)
    #define debug2_usb(x,y) printf(x,y)
  #else
    #define debug_usb(x)
    #define debug2_usb(x,y)
  #endif

#else
  #define debug(x)
  #define debug2(x,y)
  #define debug_usb(x)
  #define debug2_usb(x,y)
#endif

