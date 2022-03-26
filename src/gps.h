#include <Adafruit_GPS.h>

#ifndef gps_h
#define gps_h

class GPS {
  public:
    GPS(Uart *serialPin);

    void init();
    void update();

    float latDegrees();
    float lonDegrees();
    bool fix();
    uint8_t satellites();

  private:
    Adafruit_GPS _gps;
    Uart *_serialPin;
    float _latDegrees;
    float _lonDegrees;
};

#endif