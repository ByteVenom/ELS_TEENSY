#ifndef CONFIG_T
#define CONFIG_T

// ----- Stepper Pins -----
//Arduino
//#define directionPin 4   //Update to 4 from 2
//#define stepPulsePin 5  //Update to 5 from 3
// Stepper  pins
    #define directionPin 9
    #define stepPulsePin 10

// ----- Spindle Encoder Pins -----    
    //Teensy 3.2 TFM1 pins are 3,4.

// ----- Control Encoder Pins OF SERIAL WOMBAT -----
    #define clkPin 1
    #define dtPin 2

// ----- Button Pins OF SERIAL WOMBAT -----
    #define modePin 3

//Stepper constants
    #define stepsPerRev 400
    #define maxStepperRPM 1200

//Encoder and RPM constants
    #define encoderPulsesPerRev 80
    #define RPMCalcRateHz 2

//Mechanical Constants
    //Pitch in mm on leadscrew
    #define leadscrewPitch 1.5

    //Ratio between stepper pulley and leadscrew. Ex Stepper pulley 20T, Leadscrew 40T: .5
    #define leadscrewRatio 0.5
//Thread pithces
    #define pitches[11] = { .4, .45, .5, .7, .8, 1.0, 1.25, 1.5, 1.75, 2, 2.5 };

//LCD
    #define LCD_address 0x12
    #define LCD_rows 4
    #define LCD_cols 20

//Buttons
    #define debounceDelay 50

#endif