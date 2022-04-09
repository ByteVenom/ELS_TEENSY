#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <Wire.h>
#include <hd44780.h>  // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "Config.h"
#include <ezButton.h>
#include <TeensyStep.h>

//Feedmode is a char
// 'f' feed 
// 't' ratio
// 'p' positioning

#define displayUpdateDelay 500


class ControlPanel{
    
public:

    void initHardware();

    void updateRPM(int RPM, char currentMode, int32_t spindlePosition, int32_t feedRateControl);

    void setupFeedRate(void);

    void updateFeedRatio(int32_t spindlePosition, int feedRatio, int feedRatioControl);

    void updateFeedRate(int feedRatePercentage);

    void setupFeedRatio(void);

    void setupPositioning(int32_t spindlePosition);

    void ModeControl(char currentMode, bool buttonPressed);
};

    



#endif