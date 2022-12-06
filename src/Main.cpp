#include <Arduino.h>
#include <stdint.h>

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
int32_t lastSpindlePosition = 0;
int32_t deltaSpindlePosition;
int32_t spindlePosition;
bool printOUT = false;
//LCD Setup
double feedRate = 0;
int pitchCounter = 0;
unsigned long rpmUpdateCounter = 0;
unsigned long rpmUpdateDelay = (1.0/(RPMCalcRateHz)) * 1000000.0;
//unsigned long rpmUpdateDelay = 5000000;
//Lathe Functions
//Spindle Ratio is the gear ratio between stepper and the lead screw.
//Ratio control
double feedRateControl = 0;

char currentMode;


QuadDecode<1> spindleEncoder;
IntervalTimer rpmCalcTimer;
IntervalTimer stepTimer;
ControlPanel controlPanel;
// Encoder Setup
// Create a new instance of the AccelSteppe

SerialWombat controlPanelIOExpander;
SerialWombatDebouncedInput modeButton(controlPanelIOExpander);
SerialWombatQuadEnc controlEncoder(controlPanelIOExpander);

volatile bool calcRPMReady = false;
uint16_t rpmCalc_spindeLastPos;
uint16_t stepCount = 0;
volatile bool stepPinState = false;
uint32_t stepTarget = 0;
int pulseWidth =10;

void calcRPM();
void writeStep();
void writeStepRPM();
void stopStepper();


void setup() {
  // Set the maximum speed and acceleration:
  //Stepper pin modes
  pinMode(directionPin, OUTPUT);
  pinMode(stepPulsePin, OUTPUT);
  pinMode(6, OUTPUT);

   Wire.begin();
   controlPanelIOExpander.begin(0x6C);
  modeButton.begin(modePin, 400, true, true);
  controlEncoder.begin(clkPin, dtPin,10,true, QE_ONHIGH_POLL);
      digitalWrite(6, HIGH);
  controlPanel.initHardware();
  rpmCalcTimer.begin(calcRPM, rpmUpdateDelay);
  rpmCalcTimer.priority(200);
  currentMode = 'f';
  digitalWriteFast(directionPin, HIGH);
  spindleEncoder.setup();
  spindleEncoder.start();

  //Position
  spindlePosition = 0;
  lastSpindlePosition = 0;

   
 controlEncoder.write(0);
}

void loop() {
    spindlePosition = spindleEncoder.calcPosn();
     deltaSpindlePosition = ( (spindlePosition) - (lastSpindlePosition));
     lastSpindlePosition = spindlePosition;
    if (abs(deltaSpindlePosition) > 0) {
      
      if(currentMode == 't'){
        //lead screw moves 1.5mm per rotation
        if(deltaSpindlePosition < 0){
          digitalWriteFast(directionPin, HIGH);
        }else if(deltaSpindlePosition > 0){
          digitalWriteFast(directionPin, LOW);
        }
        stepTarget = abs((pitches[pitchCounter] * (double) stepsPerRev  *  ((deltaSpindlePosition/(double)encoderPulsesPerRev))/(  (double) leadscrewRatio *(double)leadscrewPitch)) );
        stepCount +=stepTarget;
        //stepTarget = 1600;
        //stepper.setTargetRel(deltaSpindlePosition * stepsPerRev);
        //positionController.move(stepper);
        //stepper.setPosition(0);
        //stepper.setTargetRel(target * -1.00);
        //positionController.move(stepper);
        //Serial.println(stepTarget);
        
        stepTimer.begin(writeStep, pulseWidth);
        rpmCalcTimer.end(); 
        

      }
    }
 // }

  //Handle buttons
  if(modeButton.digitalRead() && modeButton.readDurationInTrueState_mS() <= 15){
       //Serial.println(currentMode);
    // 0 == feed rate (mm/s), 1 == feed ratio (mm/rev), 2 == positioning, 3 == thread
      if(currentMode == 'f'){
        currentMode = 't';
         //stepperController.stopAsync();
        stopStepper();
        controlPanel.setupFeedRatio(pitches[pitchCounter]);
        
      }else if(currentMode == 't'){
        currentMode = 'p';
        controlPanel.setupPositioning(0);
      }
      else if(currentMode == 'p'){
        currentMode = 'f';
        //End threading step timer in case its still running
        stepTimer.end();
        digitalWriteFast(directionPin, HIGH);
        feedRate = 0;
        controlPanel.setupFeedRate();
      }
  }
  
  //Check control encoder direction
  u_int32_t newPos = 0;
  int_fast16_t deltaPos = 0;
  uint32_t controlEncNewPos = 0;
  //Only poll encoder if threading stepper is not currently stepping;
  if(stepCount <= 0){
    newPos = controlEncoder.read();
    deltaPos = (newPos - controlEncPos);
    controlEncNewPos = newPos != controlEncPos;
    controlEncPos = newPos;
  }
  
  

  //Control encoder motion

  if (controlEncNewPos) {
         //Feed ratio
         //Serial.println(newPos);
    if(deltaPos >= 400){
          deltaPos = 0;
    }      

    if (currentMode == 't'){
      //Serial.print("P");
        //Max ratio will be set to 2x
        //Stepper RPM will top out at 1200 RPM, so max spindle RPM for that would be 300 (because of 2/1 reduction)
        if (pitchCounter + deltaPos > 11) {
          pitchCounter = 11;
        } else if (pitchCounter + deltaPos < 0) {
          pitchCounter = 0;
        } else {
          pitchCounter += deltaPos;
        }
        controlPanel.updateFeedRatio(pitchCounter);
        
        

    }else if (currentMode == 'f'){
      //Feed rate
      //Feed rate percentage is a 0-100 number for speed factor control. 

      //Since the serial wombat overflows to 65535 when we try to go below 0, our delta pos would be much greater than anything we'd normally see. 
      //Thus to prevent the speed from going to max, we set it to 0. 
        
        //A delta pos >4 means that the user is spinning the knob quite quickly. This aids in some acceleration;
        float deltaPosDec = ((double) deltaPos) * feedRateRes;
        if(abs(deltaPosDec) > .3){
          deltaPosDec = deltaPosDec * 2;
        }
        if (feedRate + deltaPosDec > maxFeedRate) {
          feedRate = 0;
          stopStepper();
        } else if (feedRate + deltaPosDec <= 0) {
          feedRate = 0;
          stopStepper();
        } else {
          feedRate += deltaPosDec;
          //Ex target RPM for 1mm/s = 80 ==> 0.000075 S = 75 us. 13khz
          int_fast16_t feedRateInterruptTime = 1000000.000000 * ((leadscrewPitch* (double) leadscrewRatio)/(feedRate*2*(double) stepsPerRev));
          stepTimer.begin(writeStepRPM, feedRateInterruptTime);
          stepPinState = false;
          
        }  
        controlPanel.updateFeedRate(feedRate);              
        
    }
  }
  if(calcRPMReady){
    controlPanel.updateRPM(RPM, currentMode, spindlePosition, feedRateControl);
    //Serial.println("update)");
    calcRPMReady = false;
  }
  

}
void calcRPM() {
  int32_t rpmCalc_currentPosition = spindleEncoder.calcPosn();
  int16_t rpmCalc_deltaPos = rpmCalc_currentPosition - rpmCalc_spindeLastPos;
  float percRot = ((rpmCalc_deltaPos) /(float) encoderPulsesPerRev);
  RPM =   abs(percRot * RPMCalcRateHz * 60.00);
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
void writeStep(){
  if(stepCount == 0){
    stepTimer.end();
    rpmCalcTimer.begin(calcRPM, rpmUpdateDelay);
    return;
  }
  if(stepPinState == false){
    digitalWriteFast(stepPulsePin, HIGH);
    stepPinState = true;
    
  }else{
    digitalWriteFast(stepPulsePin, LOW);
    stepPinState = false;
    stepCount--;
  }
  
}
void writeStepRPM(){
  if(currentMode == 'f'){ 
    if(stepPinState == false){
      digitalWriteFast(stepPulsePin, HIGH);
      
      stepPinState = true;
    
    }else{
      digitalWriteFast(stepPulsePin, LOW);
      stepPinState = false;
    }
  }else{
    stopStepper();
  }
}
void stopStepper(){
  stepTimer.end();
  digitalWriteFast(stepPulsePin, LOW);
}


