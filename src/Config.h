// ----- Stepper Pins -----
//Arduino
//#define dirPin 4   //Update to 4 from 2
//#define stepPin 5  //Update to 5 from 3
//Teensy 3.2 HARDWARE encoder pins
    #define dirPin 9
    #define stepPin 10

// ----- Spindle Encoder Pins -----
    //#define chAPin 2
    //#define chBPin 3
    //Teensy
    #define chAPin 2
    #define chBPin 3

// ----- Control Encoder Pins -----
    #define clkPin 16
    #define dtPin 17

// ----- Button Pins -----
    #define modePin 6

//Stepper constants
    #define stepsPerRev 400
    #define maxStepperRPM 1200
    #define encoderPulsesPerRev 400

//Mechanical Constants
    //Pitch in mm on leadscrew
    #define leadscrewPitch 1.5

    //Ratio between stepper pulley and leadscrew. Ex Stepper pulley 20T, Leadscrew 40T: .5
    #define leadscrewRatio 0.5

//Buttons
    #define debounceDelay 50