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

static __code char __at(__CONFIG1H) conf1H = _OSC_HS__HS_PLL__USB_HS_1H & _FCMEN_OFF_1H & _IESO_ON_1H;
static __code char __at(__CONFIG1L) conf1L = _USBPLL_CLOCK_SRC_FROM_96MHZ_PLL_2_1L & _CPUDIV__OSC1_OSC2_SRC___1__96MHZ_PLL_SRC___2__1L &_PLLDIV_DIVIDE_BY_5__20MHZ_INPUT__1L;
static __code char __at(__CONFIG2H) conf2H = _WDT_DISABLED_CONTROLLED_2H;
static __code char __at(__CONFIG2L) conf2L = _BODEN_ON_2L & _BODENV_4_2V_2L;
static __code char __at(__CONFIG3H) conf3H = _PBADEN_PORTB_4_0__CONFIGURED_AS_DIGITAL_I_O_ON_RESET_3H;
static __code char __at(__CONFIG4L) conf4L = _LVP_OFF_4L & _ENHCPU_OFF_4L; // & _BACKBUG_OFF_4L & _STVR_OFF_4L;
