#include <RH_RF95.h>
#include "./generated/serde.h"

#ifndef radio_h
#define radio_h

class Radio {
  public:
    static void init();
    static bool receiveBytes(uint8_t buf[], uint8_t *receivedLen, long waitTime);
    static void sendBytes(uint8_t buf[], uint8_t len);
};

#endif