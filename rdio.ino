#include <RH_RF95.h>
#include "./generated/serde.h"

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define RF95_FREQ 915.0

#define FAILED_MS 100
#define SUCCESS_MS 1000

#define WAIT_MS_FLOOR 1500
#define WAIT_MS_CEIL 3000

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// packet counter, we increment per xmission
int16_t packetnum = 0;

// 64 bits of random ID
long randomIdPrefix, randomIdSuffix;

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  // Magic initial write copied from Adafruit example code
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(8400);

  // Setup random seed based on analog read from pin 0, which should be random electrical noise
  randomSeed(analogRead(0));

  // Generate IDs
  randomIdPrefix = random(LONG_MAX);
  randomIdSuffix = random(LONG_MAX);

  // Setup LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Magic manual reset copied from Adafruit lmao
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // If radio init failed, blink really fast
  if(!rf95.init()) {
    Serial.println("Radio init failed");
    while(true) {
      blink(FAILED_MS);
    }
  }

  if(!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("Frequency setting failed");
    while(true) {
      blink(FAILED_MS);
    }
  }
  Serial.println(RF95_FREQ);

  // MORE POWER
  rf95.setTxPower(20);
}

void loop() {
  ledOn();

  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  /**
   * Attempt to receive a packet within a random time
   * -----------------------------------------------------------------------------------------------
   */
  Serial.print("ID ");
  Serial.print(randomIdPrefix);
  Serial.print("-");
  Serial.println(randomIdSuffix);
  Serial.println("Waiting for packets...");

  long waitTime = random(WAIT_MS_FLOOR, WAIT_MS_CEIL);
  if(rf95.waitAvailableTimeout(waitTime)) {
    Serial.println("Received packets");
    if(rf95.recv(buf, &len)) {
      Serial.print("Received byte prefix: ");
      Serial.println(buf[0]);

      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    }
    else {
      Serial.println("recv failed");
    }
  }
  else {
    Serial.println("No packets received within timeout");
  }

  /**
   * Send a packet
   * -----------------------------------------------------------------------------------------------
   */
  Packet packet = Packet {
    .id_prefix = randomIdPrefix,
    .id_suffix = randomIdSuffix,
  };
  serialize(buf, packet);
  rf95.send(buf, sizeof(packet));

  // Wait until packet has sent maybe??
  rf95.waitPacketSent();
}

void blink(int millis) {
  ledOn();
  delay(millis);
  ledOff();
  delay(millis);
}

void ledOn() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void ledOff() {
  digitalWrite(LED_BUILTIN, LOW);
}

uint8_t byteFromLong(long num, int bytePosition) {
  long mask = 1;
  for(int i = 0; i < 8; i++) {
    mask |= (mask << 1) | 1;
  }
  mask = mask << (bytePosition * 8);

  long masked = num & mask;
  return masked >> (bytePosition * 8);
}
long longFromBytes(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
  long num = 0;
  num = (num | byte3) << 3;
  num = (num | byte2) << 2;
  num = (num | byte1) << 1;
  return num | byte0;
}