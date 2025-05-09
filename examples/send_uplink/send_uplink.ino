/*
  Simple example to send a mioty uplink with the Swissphone m.YON mioty module.
  It initializes the m.YON by resetting it and reading the end-point information
  and attaches it locally if not already done to make it ready to send mioty messages.
  It uses the pre-configured EUI, short address and netwoek key.
  These can be changed when the device is detached.

  Setup:
  Connect m.YON UART RX and TX to Arduino TX and RX. The m.YON uses 3.3V logic.
  Change the pin definition below to your setup.

  An additional UART is used to send debugging logs to a connected PC.
  It can be removed if it is not needed or the target device does not have two UART peripherals.
*/

#include "HardwareSerial.h"

#include "miotyAtClient.h"

// change pins to your setup
#define MYON_TX_PIN PA9
#define MYON_RX_PIN PA10

// UART to PC for debugging logs
#define PC_TX_PIN PA2
#define PC_RX_PIN PA3


HardwareSerial SerialMyon(MYON_RX_PIN, MYON_TX_PIN);
HardwareSerial SerialPC(PC_RX_PIN, PC_TX_PIN);


void setup() {
  SerialPC.begin(460800);
  SerialMyon.begin(9600);

  // reset mYON
  miotyAtClient_reset();
  // wait for it to start up again
  delay(500);

  // read info
  uint8_t info_buffer[100] = {0};
  size_t info_len = sizeof(info_buffer);
  if (miotyAtClient_getEpInfo(info_buffer, &info_len) == MIOTYATCLIENT_RETURN_CODE_OK) {
    SerialPC.write(info_buffer, info_len);
    SerialPC.println("");
  }
  else {
    SerialPC.println("Reading EP info failed");
  }
  if (miotyAtClient_getCoreLibInfo(info_buffer, &info_len) == MIOTYATCLIENT_RETURN_CODE_OK) {
    SerialPC.write(info_buffer, info_len);
    SerialPC.println("");
  }
  else {
    SerialPC.println("Reading CoreLib info failed");
  }

  // read EUI
  uint8_t eui_a[8];
  if (miotyAtClient_getOrSetEui(eui_a, false) == MIOTYATCLIENT_RETURN_CODE_OK) {
    SerialPC.printf("EUI: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
                    eui_a[0], eui_a[1], eui_a[2], eui_a[3], eui_a[4], eui_a[5], eui_a[6], eui_a[7]);
  }
  else {
    SerialPC.println("Reading EUI failed");
  }

  uint8_t short_addr[2];
  if (miotyAtClient_getOrSetShortAddress(short_addr, false) == MIOTYATCLIENT_RETURN_CODE_OK) {
    SerialPC.printf("Short address: %02X%02X\n", short_addr[0], short_addr[1]);
  }
  else {
    SerialPC.println("Reading short address failed");
  }

  // if EUI, short address or network key need to be changed, detach and change it here.
  // check first if the wanted value is already set to avoid unnecessary detaches, as this will reset
  // the mioty mac package counter which could lead to problems in the mioty backend.


  // check attachment and attach if not attached
  bool attached = false;
  miotyAtClient_getAttachment(&attached);
  SerialPC.printf("Attached: %u\n", attached);
  if (!attached)
  {
    uint8_t mac_state = 0;
    miotyAtClient_macAttachLocal(&mac_state);
    SerialPC.printf("MAC: %u\n", mac_state);
  }
}

void loop() {
  // dummy data
  uint8_t uplink_a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint32_t pcnt;

  SerialPC.println("Sending msg");
  miotyAtClient_returnCode ret = miotyAtClient_sendMessageUni(uplink_a, sizeof(uplink_a), &pcnt);
  SerialPC.printf("Ret: %u, PCNT: %u\n", ret, pcnt);
  // wait a minute before sending again
  delay(60 * 1000);
}

// needed UART implementations for miotyAtClient
void miotyAtClientWrite(const uint8_t *data, size_t len) {
  SerialMyon.write(data, len);
}

bool miotyAtClientRead (uint8_t *data, size_t *len_out) {
  int i = 0;
  while (SerialMyon.available() > 0) {
    data[i++] = SerialMyon.read();
  }
  *len_out = i;
  return true;
}
