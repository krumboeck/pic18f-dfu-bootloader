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
* Edit Makefiles
* Type 'make' on project root directory

Install bootloader with PICkit2
-------------------------------
pk2cmd -PPIC18F2550 -M -Fbootloader.hex -R

Flash application with dfu-utils
--------------------------------
dfu-util -D example.dfu

What works
----------
* Download application

What does not
-------------
* Upload

Should work, but not tested
---------------------------
* Interrupts

ToDo
----
* Implement Upload
* Fix bugs and check boundaries
* Make it more configurable
