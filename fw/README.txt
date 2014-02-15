Firmware for IRE-HID

Compile with SDCC:
make -f Makefile.sdcc

Compile with Keil:
make -f Makefile.keil

Firmware can be downloaded to the MCU:
make -f Makefile.sdcc download
or
make -f Makefile.keil download

This downloads ire_hid.hex to the MCU over C interface.

On Linux it executes the ec2writeflash utility (http://ec2drv.sourceforge.net/quickstart.html).
On Windows FlasUtilCl.exe commandline utility is used, downloadable from Silicon Labs web site (http://www.silabs.com/products/mcu/Pages/8-bit-microcontroller-software.aspx)

Bear in mind, all Keil and Silicon Labs utilities must be in your path.
