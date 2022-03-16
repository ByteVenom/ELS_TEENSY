#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <Wire.h>
#include <hd44780.h>  // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "Config.h"

#define displayUpdateDelay 500
enum FeedMode {
// ----- MODE DEC
feed_rate = 0,
feed_ratio = 1,
positioning = 2
};


    void initHardware();

    void updateRPM(int chARPM, FeedMode currentMode, int32_t spindlePosition, int32_t feedRateControl);

    void setupFeedRate(void);

    void updateFeedRatio(int32_t spindlePosition, int feedRatio, int feedRatioControl);

    void updateFeedRate(int feedRatePercentage);

    void setupFeedRatio(void);

    void setupPositioning(int32_t spindlePosition);




#endif