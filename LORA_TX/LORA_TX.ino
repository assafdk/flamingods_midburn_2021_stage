#include <SPI.h>
#include "LoRa.h"
#include <Wire.h>
#define OLED_RESET 9
int counter = 0;

void setup() {
  //   rtc.begin();
  // rtc.adjust(DateTime(2020, 11, 5, 11, 14, 50));

  delay(100);

  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  // send packet
  //return;
  for (int i = 0; i < 500; i++) {
    delay(1000);
    Serial.println("Sending data");
    uint8_t data[8] = {'0', '1', '0', '0', '1', '0', '0', '\0'};
    LoRa.beginPacket();
    LoRa.write(data, 8);
    LoRa.endPacket(true);
  }


}
