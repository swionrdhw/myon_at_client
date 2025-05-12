# Example for m.YON mioty module firmware update over UART.

This example code reboots the modem into the bootloader and uploads the firmware file via XMODEM protocol.

In this example, the firmware update is done on every startup without checking which version is already installed on the m.YON module. In a real application, the version should be checked before and only update if a newer version is available.

The bootloader is started by an UART command in this example. If the RESET and SAFEBOOT pins are connected, these can also be used to start the bootloader by holding the SAFEBOOT pin low while toggeling the RESET pin.

These additional arduino libraries are needed to build this example:
  - [XModem](https://github.com/gilman88/xmodem-lib/tree/master) (v1.0.3)
  - [incbin](https://github.com/AlexIII/incbin-arduino) (v0.1.2)

The m.YON firmware is included in the flash memory of the arduino. Therefore a platform with a large enough flash memory (>130kB) is needed for this example.

The path to the firmware update file in the example has to be adjusted to match the file in the users system.
