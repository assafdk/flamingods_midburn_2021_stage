#include <SPI.h>
#include "LoRa.h"
#include <Wire.h>

#define OLED_RESET 9
String LoRaData;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");

  while (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    Serial.println("Retrying in a few seconds...");
    delay(5000);
  }
  Serial.println("LoRa finally started!");  
  Serial.println("Setup Done");
}

void loop() {

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet: ");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    Serial.println("");



    // print RSSI of packet
     Serial.print("' with RSSI ");
       Serial.println(LoRa.packetRssi());
  }
}
