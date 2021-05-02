/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/


#include <RH_NRF24.h>

RH_NRF24 nrf24(2, 4);


void setup() {
  // Serial.begin(115200);

  Serial.println("Hello!");

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
    if (nrf24.recv(buf, &len)) {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    } else {
      Serial.println("recv failed");
    }
  } else {
    Serial.println("No reply.");
  }

  delay(3000);
}
