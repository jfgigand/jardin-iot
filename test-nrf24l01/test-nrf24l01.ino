#include <RH_NRF24.h>
 
RH_NRF24 nrf24(2, 4);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ; // wait for serial port to connect. Needed for Leonardo only
  if (!nrf24.init()) 
  {
    Serial.println("init failed");
 
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(3)) 
  {
    Serial.println("setChannel failed");
  }
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    Serial.println("setRF failed");
  }
  Serial.println("Transmitter started");

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

  delay(2000);
}
