#include <Arduino.h>
#include <stdint.h>
#include "ModeSC.h"
#include "Config.h"
#include "SerialTransfer.h"
#include "QuadEncoder.h"
#include "Wire.h"

// MODE DEFINITIONS
#define FEED 0x20
#define FEED_DISTANCE 0x21
#define THREAD 0x22
#define THREAD_DISTANCE 0x23
#define POSITION 0x24
#define NUM_MODES 5

// Transmission commands
#define RPM_TX 0x30
#define MODE_CHANGE 0x31
#define REVERSE_STEPPER 0X32
#define STOP_STEPPER 0X33
#define ENGAGE_MOVEMENT 0x34
#define MOVEMENT_COMPLETE 0X35
#define STEPPER_INMOTION 0x36
#define CHANGE_FR 0x33
#define CHANGE_TP 0x38
#define CHANGE_DIST 0x39
#define SPINDLE_POSITION_TX 0x40

// Control Encoder
uint16_t controlEncPos;
// Spindle Encoder TimeKeepers
long rpmLastRead = 0;
// Spindle RPMs
short RPMUpdateFreq = 10;
volatile double RPM;
double lastRPM;
// Used for stepper calcs
int32_t lastSpindlePosition = 0;
int32_t deltaSpindlePosition;
int32_t spindlePosition;
bool printOUT = false;
// LCD Setup
double feedRate = 0;
double threadPitch = 0;
double travelDistance = 0;
int pitchCounter = 0;
unsigned long rpmUpdateCounter = 0;
unsigned long rpmUpdateDelay = (1.0 / (RPMCalcRateHz)) * 1000000.0;
// unsigned long rpmUpdateDelay = 5000000;
// Lathe Functions
// Spindle Ratio is the gear ratio between stepper and the lead screw.
// Ratio control

mode modes[NUM_MODES];
char currentMode;
struct __attribute__((packed)) STRUCT_3
{
  uint8_t command;
  double data;
} dataTransmission;

QuadEncoder spindleEncoder(1, SPINDLE_ENC_A, SPINDLE_ENC_B, 0);
IntervalTimer posTXTimer;
IntervalTimer rpmCalcTimer;
IntervalTimer stepTimer;
SerialTransfer latheBus;

volatile bool calcRPMReady = false;
volatile bool txSpindleReady = false;
uint16_t rpmCalc_spindeLastPos;
uint16_t stepCount = 0;
volatile bool stepPinState = false;
uint32_t stepTarget = 0;
int pulseWidth = 10;
u_int32_t distanceStepTarget = 0;
uint32_t distanceStepCount = 0;
bool clearToMove = false;
bool stepperIsRunning = false;
bool reverse = false;

void calcRPM();
void writeStep();
void writeStepRPM();
void stopStepper();
void receiveControlPanelData();
mode findModeOffID(uint8_t modeID);
void setupModes();
void transmitDataToControlPanel();
void receiveControlPanelCmd();
void transmitControlPanelCmd(uint8_t cmd, double data);
void transmitSpindlePos();

void setup()
{
  // Set the maximum speed and acceleration:
  // Stepper pin modes
  pinMode(directionPin, OUTPUT);
  pinMode(stepPulsePin, OUTPUT);
  pinMode(6, OUTPUT);

  Serial1.begin(57600);
  configST latheBusConfig;
  latheBusConfig.debug = false;
  latheBus.begin(Serial1);
  digitalWrite(6, HIGH);

  rpmCalcTimer.begin(calcRPM, rpmUpdateDelay);
  rpmCalcTimer.priority(200);
  currentMode = FEED;
  digitalWriteFast(directionPin, HIGH);
  spindleEncoder.setInitConfig();
  spindleEncoder.init();

  // Position
  spindlePosition = 0;
  lastSpindlePosition = 0;
}

void loop()
{
  spindlePosition = spindleEncoder.read();
  deltaSpindlePosition = ((spindlePosition) - (lastSpindlePosition));
  lastSpindlePosition = spindlePosition;
  if (abs(deltaSpindlePosition) > 0)
  {

    if (currentMode == THREAD || (currentMode == THREAD_DISTANCE && clearToMove))
    {
      // lead screw moves 1.5mm per rotation
      if (deltaSpindlePosition < 0)
      {
        digitalWriteFast(directionPin, HIGH);
      }
      else if (deltaSpindlePosition > 0)
      {
        digitalWriteFast(directionPin, LOW);
      }
      stepTarget = abs((threadPitch * (double)stepsPerRev * ((deltaSpindlePosition / (double)encoderPulsesPerRev)) / ((double)leadscrewRatio * (double)leadscrewPitch)));
      stepCount += stepTarget;

      stepTimer.begin(writeStep, pulseWidth);
      rpmCalcTimer.end();
    }
  }
  if (calcRPMReady)
  {
    // controlPanelData.RPM = RPM;
    transmitControlPanelCmd(RPM_TX, RPM);
    calcRPMReady = false;
  }
  if (txSpindleReady && currentMode == POSITION)
  {
    double angularPos = (spindlePosition / encoderPulsesPerRev) * 360.00;
    transmitControlPanelCmd(SPINDLE_POSITION_TX, angularPos);
    txSpindleReady = false;
  }
  // receiveControlPanelData();
  receiveControlPanelCmd();
}
void calcRPM()
{
  int32_t rpmCalc_currentPosition = spindleEncoder.read();
  int16_t rpmCalc_deltaPos = rpmCalc_currentPosition - rpmCalc_spindeLastPos;
  float percRot = ((rpmCalc_deltaPos) / (float)encoderPulsesPerRev);
  RPM = abs(percRot * RPMCalcRateHz * 60.00);
  rpmCalc_spindeLastPos = rpmCalc_currentPosition;
  calcRPMReady = true;
}
void transmitSpindlePos()
{
  txSpindleReady = true;
}

void writeStep()
{
  if (stepCount <= 0 || (distanceStepCount >= distanceStepTarget && (currentMode == THREAD_DISTANCE || currentMode == FEED_DISTANCE)))
  {
    stepTimer.end();
    stopStepper();
    // controlPanelData.movementCompleted = true;
    rpmCalcTimer.begin(calcRPM, rpmUpdateDelay);
    return;
  }
  if (stepPinState == false)
  {
    digitalWriteFast(stepPulsePin, HIGH);
    stepPinState = true;
  }
  else
  {
    digitalWriteFast(stepPulsePin, LOW);
    stepPinState = false;
    stepCount--;
    distanceStepCount++;
  }
}
void writeStepRPM()
{
  if (currentMode == FEED || (currentMode == FEED_DISTANCE && distanceStepTarget <= distanceStepCount))
  {
    if (stepPinState == false)
    {
      digitalWriteFast(stepPulsePin, HIGH);
      stepPinState = true;
      distanceStepCount++;
    }
    else
    {
      digitalWriteFast(stepPulsePin, LOW);
      stepPinState = false;
    }
  }
  else
  {
    stopStepper();
    distanceStepCount = 0;
  }
}
void stopStepper()
{
  stepCount = 0;
  distanceStepCount = 0;
  stepTimer.end();
  clearToMove = false;
  stepperIsRunning = false;
  // controlPanelData.inMovement = false;
  digitalWriteFast(stepPulsePin, LOW);
  transmitControlPanelCmd(MOVEMENT_COMPLETE, 0x00);
}

void receiveControlPanelCmd()
{
  if (latheBus.available())
  {
    latheBus.rxObj(dataTransmission);
    switch (dataTransmission.command)

    {
    case MODE_CHANGE:

      currentMode = dataTransmission.data;

      if (currentMode == POSITION)
      {
        posTXTimer.begin(transmitSpindlePos, rpmUpdateDelay);
        posTXTimer.priority(200);
      }
      else
      {
        posTXTimer.end();
      }
      // Serial.printf("Current mode is now %i", currentMode);
      break;
    case REVERSE_STEPPER:
      if (stepperIsRunning)
      {
        stopStepper();
        
      }
      digitalWriteFast(directionPin, !reverse);
      reverse = !reverse;
      // Serial.println("Stepper will stop and will be set to reverse");

      break;
    /* case STOP_STEPPER:
      stopStepper();
      break; */
    case ENGAGE_MOVEMENT:
      if (RPM > 0 && (currentMode == FEED_DISTANCE || currentMode == THREAD_DISTANCE))
      {
        distanceStepTarget = dataTransmission.data * (1 / leadscrewPitch) * (1 / leadscrewRatio) * stepsPerRev;
        distanceStepCount = 0;
        transmitControlPanelCmd(STEPPER_INMOTION, 0x00);
        clearToMove = true;
        // Serial.printf("Stepper will now move %d mm \n", dataTransmission.data);
      }
      break;
    case CHANGE_FR:
      if (dataTransmission.data != 0 && dataTransmission.data != feedRate && (currentMode == FEED || currentMode == FEED_DISTANCE))
      {
        feedRate = dataTransmission.data;
        if (feedRate < 0)
        {
          feedRate = 0;
          break;
        }
        uint32_t timeBetweenSteps = 1000000 / ((dataTransmission.data / leadscrewPitch) * (1 / leadscrewRatio) * (double)stepsPerRev);
        if (timeBetweenSteps < minPulseWidth)
        {
          timeBetweenSteps = minPulseWidth;
        }
        Serial.printf("Stepper is now running at: %lf \n", dataTransmission.data );
        if (currentMode == FEED)
        {
          stepTimer.begin(writeStepRPM, timeBetweenSteps);
          stepperIsRunning = true;
          transmitControlPanelCmd(STEPPER_INMOTION, 0x00);
        }
          
         

        // TRANSMIT OUT THAT STEPPER IS IN MOVEMENT
      }
      break;
    case CHANGE_TP:
      if (dataTransmission.data != threadPitch && (currentMode == THREAD || currentMode == THREAD_DISTANCE))
      {
        threadPitch = dataTransmission.data;
         Serial.printf("Stepper is set to thread: %lf \n", threadPitch );
      }
      break;
    case CHANGE_DIST:
      if ((currentMode == THREAD_DISTANCE || currentMode == FEED_DISTANCE) && travelDistance == 0)
      {
        travelDistance = dataTransmission.data;
        // Serial.printf("Stepper is set to move: %lf \n", travelDistance );
      }
      break;
    default:
      break;
    }
  }
}
// Send out completed acks and then negate them
/* void transmitDataToControlPanel(){
  latheBus.sendDatum(controlPanelData);

  if(controlPanelData.movementCompleted){
    controlPanelData.movementCompleted = false;
  }
} */
void transmitControlPanelCmd(uint8_t cmd, double data)
{
  dataTransmission.command = cmd;
  dataTransmission.data = data;
  latheBus.sendDatum(dataTransmission);
}
void setupModes()
{
  modes[0] = mode(FEED, false);
  modes[1] = mode(FEED_DISTANCE, true);
  modes[2] = mode(THREAD, false);
  modes[3] = mode(THREAD_DISTANCE, true);
  modes[4] = mode(POSITION, false);
}
