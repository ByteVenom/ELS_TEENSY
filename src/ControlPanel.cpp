#include "ControlPanel.h"
#include "LathePowerFeedControlRotEncTeensy.ino"

void :: updateData(void){
    //add in angle readout only in positioning mode
  if (millis()%displayUpdateDelay == 0) {
    lcd.setCursor(5, 1);
    int dispRPM = chARPM;
    //1 digit
    lcd.print(dispRPM);
    lcd.print("   ");
    switch(currentMode){
      case positioning:
      lcd.setCursor(6,2);
      double encoderAngle =  (spindlePosition % encoderPulsesPerRev);
      lcd.print( (double) encoderAngle);
    break;
    case feed_rate:
    //Calculate current feed ratio based off rpm and commanded stepper RPM
    double leadScrewRPMRatio = ((feedRateControl * maxStepperRPM)/2)/chARPM;
    lcd.setCursor(0,3);
    lcd.print( (double) leadScrewRPMRatio);
    break;
    
      
    }
  }



}