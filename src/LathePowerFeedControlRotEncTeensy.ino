#include <TeensyStep.h>
#include <ezButton.h>
//#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include <ezButton.h>

// Include the LiquidCrystal_I2C
#include <Wire.h>
#include <hd44780.h>  // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include "Config.h"

//Control encoder
Encoder controlEncoder(clkPin, dtPin);
Encoder spindleEncoder(chAPin, chBPin);
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
//Button declarations
ezButton modeButton(modePin);


// ----- MODE DECLERATIONS -----

enum FeedMode {
  feed_rate = 0,
  feed_ratio = 1,
  positioning = 2
};
//LCD Setup
FeedMode currentMode = feed_rate;
int feedRatePercentage = 0;
int feedRatio = 0;
int pitchCounter = 0;
double pitches[11] = { .4, .45, .5, .7, .8, 1.0, 1.25, 1.5, 1.75, 2, 2.5 };
hd44780_I2Cexp lcd(0x27);
unsigned long rpmUpdateCounter = 0;
unsigned long rpmUpdateDelay = 700;
// Encoder Setup
// Create a new instance of the AccelStepper class:
RotateControl stepperController;
StepControl positionController;
Stepper stepper = Stepper(stepPin, dirPin);
//Lathe Functions
//Spindle Ratio is the gear ratio between stepper and the lead screw.
//Ratio control
double spindleRatio = 2;
double feedRatioControl = 0;
double feedRateControl = 0;

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed((maxStepperRPM / 60) * stepsPerRev);
  stepperController.rotateAsync(stepper);
  stepperController.overrideSpeed(0);
  //Stepper pin modes
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);

  //attachInterrupt(digitalPinToInterrupt(chBPin), checkPosition, CHANGE);
  //Buttons pin modes
  pinMode(modePin, INPUT_PULLUP);
  modeButton.setDebounceTime(debounceDelay);
  
  //LCD
  lcd.begin(20, 4);
  //Serial.begin(115200);
  //Serial.println("Hi");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("RPM: ");
  lcd.print((int)chARPM);
  lcd.home();
  lcd.print("Feed  ");
  lcd.setCursor(0, 2);
  lcd.print("mm/s: ");
  lcd.setCursor(6, 2);
  lcd.print("0");

  //Position
  spindlePosition = 0;
  lastSpindlePosition = 0;
  currentMode = feed_rate;
}

void loop() {
    spindlePosition = spindleEncoder.read();
    int deltaSpindlePosition = ( spindlePosition - lastSpindlePosition);
    if (deltaSpindlePosition > 0) {
      calcRPM();
      lastSpindlePosition = spindlePosition;
      if(currentMode == feed_ratio){
        //lead screw moves 1.588mm per rotation
        int target = feedRatioControl * 2 * ((deltaSpindlePosition/encoderPulsesPerRev) * stepsPerRev);
          stepper.setTargetRel(target);
          positionController.move(stepper);
      }
    }
 // }

  //Handle buttons
  modeControl();
  //Check control encoder direction
  long newPos = controlEncoder.read();
  int deltaPos = (newPos - controlEncPos);
  controlEncNewPos = newPos != controlEncPos;
  controlEncPos = newPos;

  //MODE BUTTON

  if (controlEncNewPos) {
         //Feed ratio
    if (currentMode == feed_ratio){
      //Serial.print("P");
        //Max ratio will be set to 2x
        //Stepper RPM will top out at 1200 RPM, so max spindle RPM for that would be 300 (because of 2/1 reduction)
        if (feedRatio + deltaPos > 200) {
          feedRatio = 200;
        } else if (feedRatio + deltaPos < 0) {
          feedRatio = 0;
        } else {
          feedRatio += deltaPos;
        }
        lcd.setCursor(6, 2);
        feedRatioControl = feedRatio / 100.0;
        lcd.print((double)feedRatioControl);
        lcd.print("  mm/rev ");
        //LEAD SCREW IS 16 TPI OR 1.588mm
        //Only necessary for stepper speed control, not for positional mode
//        speedRatio = chARPM / feedRatio.
        //Now we have a ratio of 1200 rpm, but we need 
        //ratio of 1200 rpm = 1200 mm/m, but 20 mm/s, so 1:1 ratio. 
        //To include rpm we need to limit stepper rpm to 1200. 
        //Calculate a ratio between RPM and stepper RPM to achieve ratio
        // if ratio > 1, set to 1200 display
    }else if (currentMode == feed_rate){
      //Feed rate
        if (feedRatePercentage + deltaPos > 1600) {
          feedRatePercentage = 1600;
        } else if (feedRatePercentage + deltaPos < 0) {
          feedRatePercentage = 0;
        } else {
          feedRatePercentage += deltaPos;
        }
        lcd.setCursor(6, 2);
        feedRateControl = feedRatePercentage / 1600.0;
        //mm/s takes 1mm pitch lead screw, which means 1 rpm = 1mm. 
        //1rpm = 1/60 mm/s
        //Stepper to leadscrew ratio = .5
        double displayFeedRate = feedRateControl*maxStepperRPM/120;
        //displayFeedRate = map(displayFeedRate,0, 100, 0, 600);
        lcd.print((double)displayFeedRate);
        lcd.print(" ");
        
        stepperController.overrideSpeed(feedRateControl);
    }
 
      
      //Positioning 
     
    }
  
  displayProcess();

}
void calcRPM() {
  unsigned long currentTime = micros();
  unsigned long deltaTime = currentTime - rpmLastRead;
  int deltaPos = abs(spindlePosition - lastSpindlePosition);//
  rpmLastRead = currentTime;
  double invDeltaTime = (double)1 / deltaTime; //degrees per microsecond
  invDeltaTime = invDeltaTime * (double) 60 * 1000 * 1000; //Degrees per minute
  invDeltaTime = invDeltaTime / (double) encoderPulsesPerRev; //Revolutions per Minute (RPM) 
  chARPM = invDeltaTime * deltaPos;
}

int avgArray(double avgArray[], int maxCount) {
  int sum = 0;
  for (int i = 0; i < maxCount; i++) {
    sum += avgArray[i];
  }
  return sum / maxCount;
}

void displayProcess() {
  
}

void modeControl(){
    modeButton.loop();
    
      if (modeButton.isPressed()) {
        Serial.println(currentMode);
    // 0 == feed rate (mm/s), 1 == feed ratio (mm/rev), 2 == positioning, 3 == thread
    switch(currentMode){
      //THREAD CURRENTLY NOT SUPPORTED
      //Currently in feed, going to feed ratio
       case positioning:
        currentMode = feed_rate;
        lcd.home();
        lcd.print("Feed       ");
        lcd.setCursor(0, 2);
        lcd.print("mm/s: 0       ");
        stepperController.rotateAsync(stepper);
      break;
      case feed_rate:
      currentMode = feed_ratio;
      lcd.home();
      lcd.print("Feed Ratio ");
      lcd.setCursor(0, 2);
      lcd.print("Ratio:            ");
      //Stopping motor in order to have position control take over
      stepperController.stopAsync();
      stepper.setPosition(0);
      break;
      //Currently in feed ratio, going to positioning
      case feed_ratio:
      currentMode = positioning;
      lcd.home();
      lcd.print("Positioning  ");
      lcd.setCursor(0, 2);
      lcd.print("Angle:");
      double encoderRatio =  (spindlePosition % encoderPulsesPerRev);
      lcd.print( (double) encoderRatio);
      lcd.setCursor(0,4);
      lcd.print("     ");
      //Stop stepper
      stepperController.overrideSpeed(0);
      break;
      //Currently in positioning, going to feed
     
      default:

        break;
    }
  }
}
