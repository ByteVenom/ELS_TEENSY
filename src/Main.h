#include <Arduino.h>
#include <TeensyStep.h>
#include <ezButton.h>

#include "Encoder.h"
#include "QuadDecode_def.h"

// Include the LiquidCrystal_I2C
#ifndef _MAIN_H
#define _MAIN_H



#include "Config.h"
#include "ControlPanel.h"


#include <stdint.h>

    
    
    
    //Spindle Encoder
    
    //Control encoder
    
    long controlEncPos = 0;
    bool controlEncNewPos = false;
    //True for clockwise, false for ccw
    bool controlEncDirection = true;


    //Hall Effect Sensor Settings
    bool hall = false; //Are we using hall effect (True) or quadrature encoder (false)
    int prevHallState = 0;
    int currentHallState = 0;

    //Spindle Encoder TimeKeepers
    long chALastRead;
    long rpmLastRead = 0;
    //Spindle RPMs
    double chARPM;
    double lastRPM;
    //Used for stepper calcs
    unsigned long lastSpindlePosition;
    unsigned long spindlePosition;

    bool printOUT = false;
    //LCD Setup
    FeedMode currentMode;
    int feedRatePercentage = 0;
    int feedRatio = 0;
    int pitchCounter = 0;


    unsigned long rpmUpdateCounter = 0;
    unsigned long rpmUpdateDelay = 700;
    //Lathe Functions
    //Spindle Ratio is the gear ratio between stepper and the lead screw.
    //Ratio control
    double feedRatioControl = 0;
    double feedRateControl = 0;

    //METHODS 
    void calcRPM();

        void modeControl();

        void loop();

        void setup();

#endif