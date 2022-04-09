#include "ControlPanel.h"


hd44780_I2Cexp lcd(0x27);


void ControlPanel::updateRPM(int RPM, char currentMode, int32_t spindlePosition, int32_t feedRateControl){
    //add in angle readout only in positioning mode
  if (millis()%displayUpdateDelay == 0) {
    lcd.setCursor(5, 1);
    int dispRPM = RPM;
    //1 digit
    lcd.print(dispRPM);
    lcd.print("   ");
    switch(currentMode){
      case 'p':
      lcd.setCursor(6,2);
      double encoderAngle =  (spindlePosition % encoderPulsesPerRev);
      lcd.print( (double) encoderAngle);
    break;
    case 'f':
    //Calculate current feed ratio based off rpm and commanded stepper RPM
    double leadScrewRPMRatio = ((feedRateControl * maxStepperRPM)/2)/RPM;
    lcd.setCursor(0,3);
    lcd.print( (double) leadScrewRPMRatio);
    break;
    
      
    }
  }
}

void ControlPanel::initHardware(){
  
   //LCD
  lcd.begin(LCD_cols, LCD_rows);
  
  //Serial.begin(115200);
  //Serial.println("Hi");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("RPM: ");
  lcd.print("0");
  lcd.home();
  lcd.print("Feed  ");
  lcd.setCursor(0, 2);
  lcd.print("mm/s: ");
  lcd.setCursor(6, 2);
  lcd.print("0");
 

}

void ControlPanel::setupFeedRate(){
  lcd.home();
  lcd.print("Feed       ");
  lcd.setCursor(0, 2);
  lcd.print("mm/s: 0       ");
}

void ControlPanel::setupFeedRatio(){
  lcd.home();
  lcd.print("Feed Ratio ");
  lcd.setCursor(0, 2);
  lcd.print("Ratio:            ");
}

void ControlPanel::setupPositioning(int32_t spindlePosition){
  lcd.home();
  lcd.print("Positioning  ");
  lcd.setCursor(0, 2);
  lcd.print("Angle:");
  double spindleAngle =  (spindlePosition % encoderPulsesPerRev);
  lcd.print( (double) spindleAngle);
  lcd.setCursor(0,4);
  lcd.print("     ");

}

void ControlPanel::updateFeedRatio(int32_t spindlePosition, int feedRatio, int feedRatioControl){
  lcd.setCursor(6, 2);
  feedRatioControl = feedRatio / 100.0;
  lcd.print((double)feedRatioControl);
  lcd.print("  mm/rev ");
  
}

void ControlPanel::updateFeedRate(int feedRatePercentage){
  
  double feedRateControl = feedRatePercentage / 1600.0;
  //mm/s takes 1mm pitch lead screw, which means 1 rpm = 1mm. 
  //1rpm = 1/60 mm/s
  //Stepper to leadscrew ratio = .5
  double displayFeedRate = feedRateControl*maxStepperRPM/120;
  //displayFeedRate = map(displayFeedRate,0, 100, 0, 600);
  lcd.setCursor(6, 2);
  lcd.print((double)displayFeedRate);
  lcd.print(" ");
  
}

void ControlPanel::ModeControl(char currentMode, bool buttonPressed){
    
    
      if (buttonPressed) {
        Serial.println(currentMode);
    // 0 == feed rate (mm/s), 1 == feed ratio (mm/rev), 2 == positioning, 3 == thread
      switch(currentMode){
      //THREAD CURRENTLY NOT SUPPORTED      
      //Currently in positioning, going to feed
      case 'p':
        currentMode = 'f';
        ControlPanel::setupFeedRate();        
       // stepperController.rotateAsync(stepper);
      break;
      //Currently in feed, going to feed ratio
      case 'f':
      currentMode = 't';
      
      //Stopping motor in order to have position control take over
      //stepperController.stopAsync();
      //stepper.setPosition(0);
      break;
      //Currently in feed ratio, going to positioning
      case 't':
      currentMode = 'p';
      
      //Stop stepper
      //stepperController.overrideSpeed(0);
      break;
      
     
      default:

        break;
    }
  }

}