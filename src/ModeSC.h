#ifndef mode_h
#define mode_h
#include "Arduino.h"
#include "Bounce2.h"

#define DEBOUNCE_INTERVAL 10

class mode{
    public:
    uint8_t modeID;
    bool hasDis;
    mode(uint8_t id, bool distanceMode);
    mode();
    private:
};




#endif