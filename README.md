DFU bootloader for PIC18F
=========================

Note
----
THIS SOFTWARE COMES WITHOUT ANY WARRANTY!

License
-------
LGPL v2.1 (DFU part use MIT)

Requirements
------------
* PIC182550 (it may work on other MCU with little code changes)
* PICkit2 or any other Programmer
* SDCC
* GNU PIC Utilities (gputils)
* SRecord
* dfu-utils or any other DFU Software

How to build
------------
* Enter bootloader directory
* Edit Makefile
* Type 'make'

Install bootloader with PICkit2
-------------------------------
pk2cmd -PPIC18F2550 -M -Foutput.hex -R

Flash application with dfu-utils
--------------------------------
dfu-util -D output.dfu
