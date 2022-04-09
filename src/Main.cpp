#include <Arduino.h>
#include <TeensyStep.h>
#include <ezButton.h>
#include <TeensyTimerTool.h>
#include <stdint.h>


#include "Encoder.h"
#include "QuadDecode_def.h"
#include "Config.h"
#include "ControlPanel.h"

#include <SerialWombat.h>

//Control Encoder
uint16_t controlEncPos;
//Hall Effect Sensor Settings
bool hall = false; //Are we using hall effect (True) or quadrature encoder (false)
int prevHallState = 0;
int currentHallState = 0;
//Spindle Encoder TimeKeepers
long chALastRead;
long rpmLastRead = 0;
//Spindle RPMs
short RPMUpdateFreq = 10;
double RPM;
double lastRPM;
//Used for stepper calcs
unsigned long lastSpindlePosition;
unsigned long spindlePosition;
bool printOUT = false;
//LCD Setup
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

char currentMode;

QuadDecode<1> spindleEncoder;
TeensyTimerTool::PeriodicTimer rpmCalcTimer;
ControlPanel controlPanel;
// Encoder Setup
// Create a new instance of the AccelSteppe
RotateControl stepperController;
StepControl positionController;
Stepper stepper = Stepper(stepPulsePin, directionPin);

SerialWombat controlPanelIOExpander;
SerialWombatDebouncedInput modeButton(controlPanelIOExpander);
SerialWombatQuadEnc controlEncoder(controlPanelIOExpander);


void calcRPM();

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed((maxStepperRPM / 60) * stepsPerRev);
  stepperController.rotateAsync(stepper);
  stepperController.overrideSpeed(0);
  //Stepper pin modes
  pinMode(directionPin, OUTPUT);
  pinMode(stepPulsePin, OUTPUT);
  controlPanel.initHardware();
  rpmCalcTimer.begin(calcRPM, 250'000);
  currentMode = 'f';
  //attachInterrupt(digitalPinToInterrupt(chBPin), checkPosition, CHANGE);
  //Buttons pin modes
  pinMode(modePin, INPUT_PULLUP);
  
  spindleEncoder.setup();
  spindleEncoder.start();
  //Position
  spindlePosition = 0;
  lastSpindlePosition = 0;

   
  Wire.begin();
   controlPanelIOExpander.begin(0x6C);
  controlEncoder.begin(clkPin, dtPin,0,false, QE_ONHIGH_POLL);
  modeButton.begin(modePin);
}

void loop() {
    spindlePosition = spindleEncoder.calcPosn();
    u_int16_t deltaSpindlePosition = ( spindlePosition - lastSpindlePosition);
    if (deltaSpindlePosition > 0) {
      lastSpindlePosition = spindlePosition;
      if(currentMode == 't'){
        //lead screw moves 1.588mm per rotation
        u_int16_t target = feedRatioControl * 2 * ((deltaSpindlePosition/encoderPulsesPerRev) * stepsPerRev);
        stepper.setTargetRel(target);
        positionController.move(stepper);
      }
    }
 // }

  //Handle buttons
  controlPanel.ModeControl(currentMode, modeButton.digitalRead());
  //Check control encoder direction
  u_int32_t newPos = controlEncoder.read();
  u_int32_t deltaPos = (newPos - controlEncPos);
  uint32_t controlEncNewPos = newPos != controlEncPos;
  controlEncPos = newPos;

  //MODE BUTTON

  if (controlEncNewPos) {
         //Feed ratio
    if (currentMode == 't'){
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
    }else if (currentMode == 'f'){
      //Feed rate
        if (feedRatePercentage + deltaPos > 1600) {
          feedRatePercentage = 1600;
        } else if (feedRatePercentage + deltaPos < 0) {
          feedRatePercentage = 0;
        } else {
          feedRatePercentage += deltaPos;
        }                
        stepperController.overrideSpeed(feedRateControl);
    }
    controlPanel.updateRPM(RPM, currentMode, spindlePosition, feedRateControl);

  }
  

}
void calcRPM() {
  int deltaPos = abs(spindlePosition - lastSpindlePosition);//
  RPM =  deltaPos;
}

int avgArray(double avgArray[], int maxCount) {
  int sum = 0;
  for (int i = 0; i < maxCount; i++) {
    sum += avgArray[i];
  }
  return sum / maxCount;
}




