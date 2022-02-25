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

// Singleton data buffer used by the radio driver
// Space/time tradeoff: by storing this here we avoid allocating this on every radio interaction,
// but pay a static cost in RAM to constantly maintain this buffer.
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

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

bool Radio::receivePacket(long waitTime, Packet *target) {
  uint8_t receivedLen;

  if(!rf95.waitAvailableTimeout(waitTime)) {
    Serial.println("No packets received within timeout");
    return false;
  }

  Serial.print("Received packet with RSSI: ");
  Serial.println(rf95.lastRssi(), DEC);

  if(!rf95.recv(buf, &receivedLen)) {
    Serial.println("recv failed");
    return false;
  }

  if(!Packet::isPacket(buf, receivedLen)) {
    Serial.println("Can't deserialize packet");
    return false;
  }

  Packet::deserialize(buf, target);
  return true;
}

void Radio::sendPacket(Packet &data) {
  data.serialize(buf);
  rf95.send(buf, data.size());

  // Wait until packet has sent maybe??
  rf95.waitPacketSent();
}