#include <Adafruit_GPS.h>
#include "./src/generated/serde.h"
#include "./src/radio.h"
#include "./src/led.h"
#include "./src/gps.h"

// How long to spin just collecting GPS data
#define GPS_SPIN_MS 3900
// How long to wait for packets. You should always wait longer than the spin, since the other party
// may still be spin looping collecting GPS data if you don't wait long enough.
#define WAIT_MS_FLOOR 4000
#define WAIT_MS_CEIL 5000

// 64 bits of random ID
long randomIdPrefix, randomIdSuffix;

GPS gps(&Serial1);
uint32_t timer = millis();

struct DistanceArgs {
  float myLatDegrees;
  float myLonDegrees;
  float theirLatDegrees;
  float theirLonDegrees;
};

void setup() {
  // Setup random seed based on analog read from pin 0, which should be random electrical noise
  randomSeed(analogRead(0));

  // Lots of GPS examples set this crazy high, so that you can echo all GPS data. But we're not
  // going to echo all GPS data so just set it to a normal value.
  Serial.begin(9600);

  // Generate IDs
  randomIdPrefix = random(LONG_MAX);
  randomIdSuffix = random(LONG_MAX);

  gps.init();
  LED::BuiltIn::init();
  Radio::init();

  // If we've gotten this far, everything has initialized successfully. Turn the LED to be
  // statically on to indicate success.
  LED::BuiltIn::on();
}

void loop() {
  gps.update();

  // If we've spun enough collecting GPS data, try to send/receive packets
  if(millis() - timer > GPS_SPIN_MS) {
    Serial.println("Collected GPS data.");
    Serial.print("ID: ");
    printId(randomIdPrefix, randomIdSuffix);

    Serial.print("Fix: "); Serial.println(gps.fix());
    if (gps.fix()) {
      Serial.print("Latitude (degrees):"); Serial.println(gps.latDegrees());
      Serial.print("Longitude (degrees):"); Serial.println(gps.lonDegrees());
      Serial.print("Satellites: "); Serial.println(gps.satellites());
    }

    // Send before and after to solve races: whoever is the first to get a message successfully will
    // immediately break their receive() block and then instantly send another message back,
    // catching the other party during their receive() block in most cases.
    sendPacket();
    receivePacket();
    sendPacket();

    // Reset the timer AFTER sending/receiving, or else it'll never spin and collect GPS data:
    // receive blocks for a long time, and send blocks for a bit too. You want to spin starting from
    // the end of send/receive/send.
    Serial.println("Spinning to collect GPS data...");
    timer = millis();
  }
}

float gpsDistance(DistanceArgs args) {
  float earthRadius = 6371000; // in meters

  float myLatRadians = args.myLatDegrees * PI / 180.0;
  float theirLatRadians = args.theirLatDegrees * PI / 180.0;

  float deltaLat = (args.theirLatDegrees - args.myLatDegrees) * PI / 180.0;
  float deltaLon = (args.theirLonDegrees - args.myLonDegrees) * PI / 180.0;

  // Haversine distance formula
  float a = sin(deltaLat / 2.0) * sin(deltaLat / 2.0) +
            cos(myLatRadians) * cos(theirLatRadians) *
            sin(deltaLon / 2) * sin(deltaLon / 2.0);
  float c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return earthRadius * c;
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

    if(packet.fix) {
      Serial.print("Received latitude (degrees): ");
      Serial.println(packet.lat_degrees);
      Serial.print("Received longitude (degrees): ");
      Serial.println(packet.lon_degrees);
      if(gps.fix()) {
        Serial.print("Distance (meters):");
        Serial.println(gpsDistance(DistanceArgs {
          .myLatDegrees = gps.latDegrees(),
          .myLonDegrees = gps.lonDegrees(),
          .theirLatDegrees = packet.lat_degrees,
          .theirLonDegrees = packet.lon_degrees,
        }));
      }
      else {
        Serial.println("We don't have a fix!");
      }
    }
    else {
      Serial.println("Packet sender doesn't yet have a fix.");
    }
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
    .lat_degrees = gps.latDegrees(),
    .lon_degrees = gps.lonDegrees(),
    .fix = gps.fix(),
  });

  Serial.println("Sending packet...");
  Radio::sendPacket(packet);
}

void printId(long prefix, long suffix) {
  Serial.print(prefix);
  Serial.print("-");
  Serial.println(suffix);
}