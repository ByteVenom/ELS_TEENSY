#include "Main.h"

Encoder controlEncoder(clkPin, dtPin);
ezButton modeButton(clkPin);
QuadDecode<1> spindleEncoder;

// Encoder Setup
// Create a new instance of the AccelSteppe
RotateControl stepperController;
StepControl positionController;
Stepper stepper = Stepper(stepPin, dirPin);

void setup() {
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed((maxStepperRPM / 60) * stepsPerRev);
  stepperController.rotateAsync(stepper);
  stepperController.overrideSpeed(0);
  //Stepper pin modes
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  initHardware();
  currentMode = feed_rate;
  //attachInterrupt(digitalPinToInterrupt(chBPin), checkPosition, CHANGE);
  //Buttons pin modes
  pinMode(modePin, INPUT_PULLUP);
  modeButton.setDebounceTime(debounceDelay);

  spindleEncoder.setup();
  spindleEncoder.start();
  
    

  //Position
  spindlePosition = 0;
  lastSpindlePosition = 0;
  currentMode = feed_rate;
}

void loop() {
    spindlePosition = spindleEncoder.calcPosn();
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
  u_int32_t newPos = controlEncoder.read();
  u_int16_t deltaPos = (newPos - controlEncPos);
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
        stepperController.overrideSpeed(feedRateControl);
    }
    updateRPM(chARPM, currentMode, spindlePosition, feedRateControl);

  }
  

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
        setupFeedRate();
        
        stepperController.rotateAsync(stepper);
      break;
      case feed_rate:
      currentMode = feed_ratio;
      
      //Stopping motor in order to have position control take over
      stepperController.stopAsync();
      stepper.setPosition(0);
      break;
      //Currently in feed ratio, going to positioning
      case feed_ratio:
      currentMode = positioning;
      
      //Stop stepper
      stepperController.overrideSpeed(0);
      break;
      //Currently in positioning, going to feed
     
      default:

        break;
    }
  }
}

