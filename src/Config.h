#ifndef CONFIG_T
#define CONFIG_T

// ----- Stepper Pins -----
//Arduino
//#define directionPin 4   //Update to 4 from 2
//#define stepPulsePin 5  //Update to 5 from 3
// Stepper  pins
    #define directionPin 9
    const int stepPulsePin = 10;

// ----- Spindle Encoder Pins -----    
    //Teensy 3.2 TFM1 pins are 3,4.
    #define SPINDLE_ENC_A 2
    #define SPINDLE_ENC_B 3

// ----- Control Encoder Pins OF SERIAL WOMBAT -----
    const short clkPin =  1;
    #define dtPin 2

// ----- Button Pins OF SERIAL WOMBAT -----
    #define modePin 3

//Stepper constants
    #define stepsPerRev 10000
    #define maxStepperRPM 4000
    #define minPulseWidth 5
//Encoder and RPM constants
    #define encoderPulsesPerRev 400
    #define RPMCalcRateHz 4

//Mechanical Constants
    //Pitch in mm on leadscrew
    #define leadscrewPitch 3.175

    //Ratio between stepper pulley and leadscrew. Ex Stepper pulley 20T, Leadscrew 40T: .5
    #define leadscrewRatio 0.5

//LCD
    #define LCD_address 0x12
    #define LCD_rows 4
    #define LCD_cols 20

//Buttons
    #define debounceDelay 50
//Feed
    //Resolution of feed rate. In mm
    #define feedRateRes 0.1 
    #define maxFeedRate 6




#endif