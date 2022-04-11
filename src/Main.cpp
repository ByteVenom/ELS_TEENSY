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
volatile double RPM;
double lastRPM;
//Used for stepper calcs
unsigned long lastSpindlePosition;
u_int16_t deltaSpindlePosition;
unsigned long spindlePosition;
bool printOUT = false;
//LCD Setup
int feedRatePercentage = 0;
int feedRatio = 0;
int pitchCounter = 0;
unsigned long rpmUpdateCounter = 0;
unsigned long rpmUpdateDelay = (1.0/(RPMCalcRateHz))* 1000000;
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

volatile bool calcRPMReady = false;
uint16_t rpmCalc_spindeLastPos;

void calcRPM();

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed((maxStepperRPM / 60) * stepsPerRev);
  stepperController.rotateAsync(stepper);
  stepperController.overrideSpeed(0);
  //Stepper pin modes
  pinMode(directionPin, OUTPUT);
  pinMode(stepPulsePin, OUTPUT);
   Wire.begin();
   controlPanelIOExpander.begin(0x6C);
  modeButton.begin(modePin, 400, true, true);
  controlEncoder.begin(clkPin, dtPin,5,true, QE_ONHIGH_POLL);
  
  controlPanel.initHardware();
  rpmCalcTimer.begin(calcRPM, rpmUpdateDelay);
  currentMode = 'f';

  
  
  spindleEncoder.setup();
  spindleEncoder.start();
  //Position
  spindlePosition = 0;
  lastSpindlePosition = 0;

   
 
}

void loop() {
    spindlePosition = spindleEncoder.calcPosn();
     deltaSpindlePosition = ( spindlePosition - lastSpindlePosition);
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
  if(modeButton.digitalRead() && modeButton.readDurationInTrueState_mS() <= 5){
    controlPanel.ModeControl(currentMode);
  }
  
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
        if (feedRatio + deltaPos > 100) {
          feedRatio = 100;
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
        controlPanel.updateFeedRate(feedRatePercentage);
    }
    

  }
  if(calcRPMReady){
    controlPanel.updateRPM(RPM, currentMode, spindlePosition, feedRateControl);
    
    calcRPMReady = false;
  }
  

}
void calcRPM() {
  int32_t rpmCalc_currentPosition = spindleEncoder.calcPosn();
  int16_t rpmCalc_deltaPos = rpmCalc_currentPosition - rpmCalc_spindeLastPos;
  float percRot = ((rpmCalc_deltaPos) /(float) encoderPulsesPerRev);
  RPM =   percRot * RPMCalcRateHz * 60.00;
  
  rpmCalc_spindeLastPos = rpmCalc_currentPosition;
  calcRPMReady = true;
  
  
}

int avgArray(double avgArray[], int maxCount) {
  int sum = 0;
  for (int i = 0; i < maxCount; i++) {
    sum += avgArray[i];
  }
  return sum / maxCount;
}




