#include <RH_RF95.h>
#include "./src/generated/serde.h"

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
  Serial.print("ID: ");
  printId(randomIdPrefix, randomIdSuffix);
  Serial.println("Waiting for packets...");

  long waitTime = random(WAIT_MS_FLOOR, WAIT_MS_CEIL);
  if(rf95.waitAvailableTimeout(waitTime)) {
    Serial.println("Received packet");
    if(rf95.recv(buf, &len)) {
      if(isPacket(buf, len)) {
        Packet packet = deserialize(buf);
        Serial.print("Received ID: ");
        printId(packet.id_prefix, packet.id_suffix);
      }
      else {
        Serial.println("Packet was unparseable.");
      }

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
  Packet packet = buildPacket(PacketArgs {
    .id_prefix = randomIdPrefix,
    .id_suffix = randomIdSuffix,
  });
  serialize(buf, packet);
  Serial.println("Sending packet...");
  rf95.send(buf, sizeof(packet));

  // Wait until packet has sent maybe??
  rf95.waitPacketSent();
}

void printId(long prefix, long suffix) {
  Serial.print(prefix);
  Serial.print("-");
  Serial.println(suffix);
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