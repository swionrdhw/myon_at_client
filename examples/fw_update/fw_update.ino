/*
  Example for uploading a firmware update file to the m.YON mioty module over UART.
  The firmware file is part of the program loaded on the arduino, so it needs ~130kB
  flash space available for that.

  Setup:
  Connect m.YON UART RX and TX to Arduino TX and RX. The m.YON uses 3.3V logic.
  Change the pin definition below to your setup.

  An additional UART is used to send debugging logs to a connected PC.
  It can be removed if it is not needed or the target device does not have two UART peripherals.

  The path to the firmware file must be adjusted to match the desired file on the system.
*/

#include "HardwareSerial.h"

#include "miotyAtClient.h"

#include "XModem.h"
#include "incbin.h"

// change pins to your setup
#define MYON_TX_PIN PA9
#define MYON_RX_PIN PA10

// UART to PC for debugging logs
#define PC_TX_PIN PA2
#define PC_RX_PIN PA3

// adjust to your firmware file path, relative to this sketch
#define FW_FILE_PATH "0544930_FW_m.YON_bidi_1.3.0.gbl"

void crc_16_chksum(byte *data, size_t dataSize, byte *chksum);


HardwareSerial SerialMyon(MYON_RX_PIN, MYON_TX_PIN);
HardwareSerial SerialPC(PC_RX_PIN, PC_TX_PIN);

XModem xmodem;

// include the FW update file in the arduino program
INCBIN(FwFile, FW_FILE_PATH);

void setup() {
  SerialPC.begin(460800);
  SerialMyon.begin(9600);

  // send an empty command, if the device is already in bootloader, this will exit it.
  SerialMyon.write('\r');
  delay(500);

  // start bootloader
  miotyAtClient_startBootloader();
  SerialPC.println("Starting bootloader");
  SerialMyon.end();

  // set baudrate to 115200 for the bootloader.
  SerialMyon.begin(115200);
  delay(500);

  xmodem.begin(SerialMyon, XModem::ProtocolType::CRC_XMODEM);
  // The predefined CRC calculation in the XModem library does not work on little endian systems, so we provide our own.
  xmodem.setChksumHandler(&crc_16_chksum);

  // upload firmware file
  SerialPC.println("Starting FW upload");
  if (xmodem.send((unsigned char *)gFwFileData, gFwFileSize)) {
    SerialPC.println("FW upload success");
  } else {
    SerialPC.println("FW upload failed");
  }
  SerialMyon.end();

  // set baudrate to 9600 again
  SerialMyon.begin(9600);
  delay(500);
  // read info
  uint8_t info_buffer[100] = { 0 };
  size_t info_len = sizeof(info_buffer);
  if (miotyAtClient_getEpInfo(info_buffer, &info_len) == MIOTYATCLIENT_RETURN_CODE_OK) {
    SerialPC.write(info_buffer, info_len);
    SerialPC.println("");
  } else {
    SerialPC.println("Could not read information");
  }
}

void loop() {
}

// needed UART implementations for miotyAtClient
void miotyAtClientWrite(const uint8_t *data, size_t len) {
  SerialMyon.write(data, len);
}

bool miotyAtClientRead(uint8_t *data, size_t *len_out) {
  int i = 0;
  while (SerialMyon.available() > 0) {
    data[i++] = SerialMyon.read();
  }
  *len_out = i;
  return true;
}

// XMODEM CRC16 implementation
void crc_16_chksum(byte *data, size_t dataSize, byte *chksum) {
  const uint16_t crc_prime = 0x1021;
  uint16_t crc = 0;

  for (size_t i = 0; i < dataSize; ++i) {
    crc ^= (((uint16_t)data[i]) << 8);
    for (byte j = 0; j < 8; ++j) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ crc_prime;
      } else {
        crc <<= 1;
      }
    }
  }
  chksum[0] = (crc >> 8) & 0xFF;
  chksum[1] = crc & 0xFF;
}
