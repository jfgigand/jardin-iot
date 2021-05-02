/*
 *
 * echo bonjour >/dev/udp/195.154.46.26/8888
*/

// #define JARDIN_USE_RF433
#define JARDIN_NRF24_PIN_CE 2
#define JARDIN_NRF24_PIN_CSN 4

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#ifdef JARDIN_USE_RF433
# include <RH_ASK.h>
#else
# include <RH_NRF24.h>
#endif
#include "jardin.h"


#ifndef STASSID
//#define STASSID "acacia-relay"
#define STASSID "acacia-jardin"
#define STAPSK  "15233051"
//#define STASSID "acacia-dlink"
//#define STAPSK  "47082351"
#endif

// Chip enable
// Slave select


const char* ssid     = STASSID;
const char* password = STAPSK;

IPAddress udpDestIP(195, 154, 46, 26);
unsigned int udpDestPort = 8888;
WiFiUDP udp;

#ifdef JARDIN_USE_RF433
RH_ASK radio(2000, 4);
#else
RH_NRF24 radio(JARDIN_NRF24_PIN_CE, JARDIN_NRF24_PIN_CSN);
#endif

void sendUdp(jardin_data_abstract_t *data, size_t size)
{
  udp.beginPacket(udpDestIP, udpDestPort);
  udp.write((uint8_t *) data, size);
  udp.endPacket();
}

void initialize_data_entry(jardin_data_abstract_t *data, jardin_event_type_t type)
{
  data->jardin_id = JARDIN_ID;
  data->box_type = BOX_TYPE_RELAIS;
  data->box_id = JARDIN_BOX_RELAIS_ESP_ID;
  data->event_type = type;
}
void send_uptime()
{
  static unsigned long last_millis = 0;
  static unsigned short millis_cycle = 0;
  unsigned long milliseconds = millis();
  jardin_data_uptime_t data_uptime;

  initialize_data_entry((jardin_data_abstract_t *) &data_uptime, EVENT_TYPE_UPTIME);
  if (milliseconds < last_millis) {
    millis_cycle++;
    last_millis = milliseconds;
  }
  // a cycle is 4294967.296 seconds (so we will have 0.296s drifting every 49 days)
  data_uptime.uptime = (millis_cycle * 4294967) + (milliseconds / 1000);
  Serial.println(F("Sending event uptime"));
  Serial.println(data_uptime.uptime);
  sendUdp((jardin_data_abstract_t *)&data_uptime, sizeof(data_uptime));
}

void send_wifi_signal() {
  jardin_data_signal_strength_t data_wifi;

  initialize_data_entry((jardin_data_abstract_t *) &data_wifi, EVENT_TYPE_WIFI_SIGNAL);
  data_wifi.strength = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI): "));
  Serial.println(data_wifi.strength);
  sendUdp((jardin_data_abstract_t *)&data_wifi, sizeof(data_wifi));
}

void setup_radio()
{
#ifdef JARDIN_USE_RF433
  if (radio.init()) {
    Serial.println("RF433 init successfully");
    radio.setModeRx();
  } else {
    Serial.println("RF433 init failed");
  }
#else
  Serial.println("NRF433...");
  if (radio.init()) {
    Serial.println("NRF433 init successfully");
    if (!radio.setChannel(JARDIN_NRF24_CHANNEL)) {
      Serial.println("NRF433 setChannel failed");
    }
    if (!radio.setRF(RH_NRF24::DataRate250kbps, RH_NRF24::TransmitPower0dBm)) {
      Serial.println("NRF433 setRF failed");
    }
    Serial.println("NRF433 started");

  } else {
    Serial.println("NRF433 init failed");
  }
#endif
}

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setup_radio();
}

void loop() {
#ifdef JARDIN_USE_RF433
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
#else
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
#endif
  uint8_t buflen = sizeof(buf);

  if (radio.waitAvailableTimeout(1000)) {
    if (radio.recv(buf, &buflen)) {
      int i;

      // Message with a good checksum received, dump it.
      radio.printBuffer("Got:", buf, buflen);

      udp.beginPacket(udpDestIP, udpDestPort);
      udp.write(buf, buflen);
      udp.endPacket();

    } else {
      Serial.println("radio_ask no msg");
    }
  } else {
    Serial.println("silence!");
  }
  send_uptime();
  send_wifi_signal();
}
