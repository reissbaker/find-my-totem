#include <RH_RF95.h>
#include "./generated/serde.h"
#include "./radio.h"
#include "./led.h"

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 915.0

#define FAILED_MS 100
#define SUCCESS_MS 1000

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void Radio::init() {
  pinMode(RFM95_RST, OUTPUT);
  // Magic initial write copied from Adafruit example code
  digitalWrite(RFM95_RST, HIGH);

  // Magic manual reset copied from Adafruit lmao
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // If radio init failed, blink really fast
  if(!rf95.init()) {
    Serial.println("Radio init failed");
    while(true) {
      LED::BuiltIn::blink(FAILED_MS);
    }
  }

  if(!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("Frequency setting failed");
    while(true) {
      LED::BuiltIn::blink(FAILED_MS);
    }
  }
  Serial.println(RF95_FREQ);

  // MORE POWER
  rf95.setTxPower(20);
}

bool Radio::receiveBytes(uint8_t buf[], uint8_t *receivedLen, long waitTime) {
  if(rf95.waitAvailableTimeout(waitTime)) {
    Serial.println("Received packet");
    if(rf95.recv(buf, receivedLen)) {
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      return true;
    }
    else {
      Serial.println("recv failed");
    }
  }
  else {
    Serial.println("No packets received within timeout");
  }
  return false;
}

void Radio::sendBytes(uint8_t buf[], uint8_t len) {
  rf95.send(buf, len);

  // Wait until packet has sent maybe??
  rf95.waitPacketSent();
}