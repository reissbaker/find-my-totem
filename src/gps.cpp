#include <Adafruit_GPS.h>
#include "./gps.h"

float dmsToDegrees(nmea_float_t dms, char direction) {
  float dmsDegrees = floor(dms / 100.0);
  float dmsMinutes = floor(dms - (dmsDegrees * 100.0));
  float dmsSeconds = 60.0 * (dms - (dmsDegrees * 100.0) - dmsMinutes);
  float converted = dmsDegrees + (dmsMinutes / 60.0) + (dmsSeconds / 3600.0);
  if(direction == 'N') return converted;
  if(direction == 'E') return converted;
  return -1 * converted;
}

GPS::GPS(Uart *serialPin) {
  this->_gps = Adafruit_GPS(serialPin);
  this->_serialPin = serialPin;
}

void GPS::init() {
  // 9600 NMEA is the default baud rate for Adafruit GPS modules
  this->_gps.begin(9600);

  // Turn on RMC (recommended minimum) and GGA (fix data) including altitude
  this->_gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Adafruit recommends going no higher than 1Hz update rate
  this->_gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  // Request updates on antenna status, comment out to keep quiet
  this->_gps.sendCommand(PGCMD_ANTENNA);

  // Magic delay from Adafruit example GPS code, woo
  delay(1000);

  // Send firmware version
  this->_serialPin->println(PMTK_Q_RELEASE);
}

void GPS::update() {
  // We have to read from the GPS character-by-character, which seems crazy but that's the way the
  // cookie crumbles. We apparently don't actually need the output; this just causes us to pull from
  // the buffer, and the GPS lib will buffer it once more in memory.
  // Note that the .read() call is non-blocking, so it's safe to just loop a whole bunch and keep
  // reading. 100 is a total guess as to how much is reasonable, and can be tuned as necessary.
  // It's a ring buffer so theoretically failing to call update for a while and then catching up
  // should be "fine" for some defn of fine (you won't be toooooooo stale).
  for(int charactersRead = 0; charactersRead < 100; charactersRead++) {
    // We need to check for new NMEA sentences at least every ten calls, or else risk missing short
    // NMEA sentences. This API is extremely silly.
    for(int i = 0; i < 10; i++) {
      this->_gps.read();
    }

    if (this->_gps.newNMEAreceived()) {
      // this also sets the newNMEAreceived() flag to false
      if (!this->_gps.parse(this->_gps.lastNMEA()))
        // It's possible for parsing to fail, in which case we should just keep going
        continue;
    }
  }

  // If we've gotten this far, we've parsed new NMEA sentences. Update the parsed lat/lon.
  if(this->_gps.fix) {
    this->_latDegrees = dmsToDegrees(this->_gps.latitude, this->_gps.lat);
    this->_lonDegrees = dmsToDegrees(this->_gps.longitude, this->_gps.lon);
  }
}

float GPS::latDegrees() {
  return this->_latDegrees;
}

float GPS::lonDegrees() {
  return this->_lonDegrees;
}

bool GPS::fix() {
  return this->_gps.fix;
}

uint8_t GPS::satellites() {
  return this->_gps.satellites;
}