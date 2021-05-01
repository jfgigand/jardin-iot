/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <RH_NRF24.h>
 
RH_NRF24 nrf24(2, 4);

#ifndef STASSID
#define STASSID "acacia-relay"
#define STAPSK  "15233051"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "192.168.1.24";
const uint16_t port = 5858;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}


void loop() {

  
  Serial.println("Sending to gateway");
  char data[]     = "Coucou";
  
  Serial.print("INFO: ");
  Serial.print(data);
  Serial.println();
 
  nrf24.send((uint8_t*) data, sizeof(*data));
  nrf24.waitPacketSent();
  
  // Now wait for a reply
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  if (nrf24.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (nrf24.recv(buf, &len))
    {
     
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
     
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply.");
    
  }

  delay(5000);
}
