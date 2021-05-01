#include <RH_NRF24.h>
 
RH_NRF24 nrf24(2, 4);

void setup() {
  Serial.begin(115200);
  if (!nrf24.init()) {
    Serial.println("init failed");
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1)) {
    Serial.println("setChannel failed");
  }
  if (!nrf24.setRF(RH_NRF24::DataRate250kbps, RH_NRF24::TransmitPower0dBm)) {
    Serial.println("setRF failed");
  }
  Serial.println("Transmitter started");
}

void loop() {
  //Serial.println("Sending to gateway");
  char data[] = "Patate";

#define RECV
#ifdef EMIT
  Serial.print("SEND: ");
  Serial.print(data);
  Serial.println();
 
  nrf24.send((uint8_t*) data, sizeof(data));
  nrf24.waitPacketSent();
# ifndef RECV
  delay(1000);
# endif
#endif
#ifdef RECV
  // Now wait for a reply
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  if (nrf24.waitAvailableTimeout(3000)) {
    // Should be a reply message for us now
    if (nrf24.recv(buf, &len)) {
      Serial.print("REPLY: ");
      Serial.println((char*)buf);
    }
    else {
      Serial.println("recv failed");
    }
  }
  else {
    Serial.println("REPLY: ");
  }
#endif
}
