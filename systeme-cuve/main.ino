// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module
// Tested on Arduino Mega, Duemilanova, Uno, Due, Teensy, ESP-12

// HC-SR04 sensor (niveau d’eau) https://www.youtube.com/watch?v=6F1B_N6LuKw
// caparaison (à 7:32) https://www.youtube.com/watch?v=vGlKVRvnjws

// démo du capteur de pression https://www.youtube.com/watch?v=BqLeKm8-MmM

#define JARDIN_BOX_ID           0x0a01
#define PIN_RADIO_RX            4
#define PIN_RADIO_TX            5
#define PIN_RADIO_PTT           0

#define RADIO_SPEED             2000
#define RADIO_PTT_INVERTED      false

#include "../common/jardin.h"

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

RH_ASK radio_ask(RADIO_SPEED, PIN_RADIO_RX, PIN_RADIO_TX,
    PIN_RADIO_PTT, RADIO_PTT_INVERTED);

void setup()
{
  Serial.begin(9600);
  Serial.println(F("systeme-cuve"));
  if (radio_ask.init()) {
    radio_ask.setModeRx();
  } else {
    Serial.println(F("radio_ask init failed"));
  }
}

void loop()
{
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen;
    jardin_data_abstract_t *data = (jardin_data_abstract_t *) buf;

    if (radio_ask.waitAvailableTimeout(1000)) {
      buflen = sizeof(buf);
      if (radio_ask.recv(buf, &buflen)) {
	radio_ask.printBuffer("Got:", buf, buflen);

        if (data->jardin_id == JARDIN_ID) {
          Serial.print("radio_ask msg of type: ");
          Serial.println(data->event_type);
          // data->event_type
        } else {
          Serial.println("radio_ask msg bad signature: ignored");
        }

      } else {
        Serial.println("radio_ask no msg");
      }
    }
}
