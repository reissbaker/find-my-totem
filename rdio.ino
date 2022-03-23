#include <Adafruit_GPS.h>
#include "./src/generated/serde.h"
#include "./src/radio.h"
#include "./src/led.h"

#define WAIT_MS_FLOOR 1500
#define WAIT_MS_CEIL 3000

#define GPSSerial Serial1

// 64 bits of random ID
long randomIdPrefix, randomIdSuffix;

Adafruit_GPS GPS(&GPSSerial);
uint32_t timer = millis();

void setup() {
  // Setup random seed based on analog read from pin 0, which should be random electrical noise
  randomSeed(analogRead(0));

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  Serial.begin(115200);

  // 9600 NMEA is the default baud rate for Adafruit GPS modules
  GPS.begin(9600);

  // Turn on RMC (recommended minimum) and GGA (fix data) including altitude
  // TODO: investigate if we need GGA?
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Adafruit recommends going no higher than 1Hz update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);
  // Magic delay from Adafruit example GPS code, woo
  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

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
  // We have to read from the GPS character-by-character, which seems crazy but that's the way the
  // cookie crumbles. We apparently don't actually need the output; this just causes us to pull from
  // the buffer, and the GPS lib will buffer it once more in memory.
  // We need to check for new NMEA sentences at least every ten calls, or else risk missing short
  // NMEA sentences. This API is extremely silly.
  for(int i = 0; i < 10; i++) {
    GPS.read();
  }
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // Print debug info every 2s or so
  if(millis() - timer > 2000) {
    timer = millis();

    Serial.print("ID: ");
    printId(randomIdPrefix, randomIdSuffix);

    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);

    Serial.print("Fix: "); Serial.print(GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Latitude (degrees):"); Serial.println(dmsToDegrees(GPS.latitude, GPS.lat));
      Serial.print("Longitude (degrees):"); Serial.println(dmsToDegrees(GPS.longitude, GPS.lon));
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }

  // TODO: it seems like we actually want to spin and collect GPS data for a bit, and only send
  // and receive packets once we've got some data collected. Do stuff with timers for that.
  //receivePacket();
  //sendPacket();
}

float dmsToDegrees(nmea_float_t dms, char direction) {
  float dmsDegrees = floor(dms / 100.0);
  float dmsMinutes = floor(dms - (dmsDegrees * 100.0));
  float dmsSeconds = 60.0 * (dms - (dmsDegrees * 100.0) - dmsMinutes);
  float converted = dmsDegrees + (dmsMinutes / 60.0) + (dmsSeconds / 3600.0);
  if(direction == 'N') return converted;
  if(direction == 'E') return converted;
  return -1 * converted;
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
    .lat_degrees = dmsToDegrees(GPS.latitude, GPS.lat),
    .lon_degrees = dmsToDegrees(GPS.longitude, GPS.lon),
    .fix = GPS.fix,
  });

  Serial.println("Sending packet...");
  Radio::sendPacket(packet);
}

void printId(long prefix, long suffix) {
  Serial.print(prefix);
  Serial.print("-");
  Serial.println(suffix);
}