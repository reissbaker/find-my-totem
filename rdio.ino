#include "./src/generated/serde.h"
#include "./src/radio.h"
#include "./src/led.h"

#define WAIT_MS_FLOOR 1500
#define WAIT_MS_CEIL 3000

// 64 bits of random ID
long randomIdPrefix, randomIdSuffix;

void setup() {
  // Setup random seed based on analog read from pin 0, which should be random electrical noise
  randomSeed(analogRead(0));

  Serial.begin(8400);

  // Generate IDs
  randomIdPrefix = random(LONG_MAX);
  randomIdSuffix = random(LONG_MAX);

  LED::BuiltIn::init();
  Radio::init();

  // If we've gotten this far, everything has initialized successfully. Turn the LED to be
  // statically on to indicate success.
  LED::BuiltIn::on();
}

void loop() {
  Serial.print("ID: ");
  printId(randomIdPrefix, randomIdSuffix);

  receivePacket();
  sendPacket();
}

/**
 * Attempt to receive a packet
 * -------------------------------------------------------------------------------------------------
 */
void receivePacket() {
  Packet packet;

  Serial.println("Waiting for packets...");
  long waitTime = random(WAIT_MS_FLOOR, WAIT_MS_CEIL);
  if(Radio::receivePacket(waitTime, &packet)) {
    Serial.print("Received ID: ");
    printId(packet.id_prefix, packet.id_suffix);
  }
}

/**
 * Send a packet
 * -------------------------------------------------------------------------------------------------
 */
void sendPacket() {
  Packet packet = Packet(PacketArgs {
    .id_prefix = randomIdPrefix,
    .id_suffix = randomIdSuffix,
  });

  Serial.println("Sending packet...");
  Radio::sendPacket(packet);
}

void printId(long prefix, long suffix) {
  Serial.print(prefix);
  Serial.print("-");
  Serial.println(suffix);
}