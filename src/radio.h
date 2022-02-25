#include <RH_RF95.h>
#include "./generated/serde.h"

#ifndef radio_h
#define radio_h

class Radio {
  public:
    static void init();
    static bool receivePacket(long waitTime, Packet *target);
    static void sendPacket(Packet &data);
};

#endif