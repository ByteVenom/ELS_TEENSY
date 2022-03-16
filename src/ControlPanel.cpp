#include "ControlPanel.h"


hd44780_I2Cexp lcd(0x27);

void updateRPM(int chARPM, FeedMode currentMode, int32_t spindlePosition, int32_t feedRateControl){
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

void initHardware(){
  
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

void setupFeedRate(){
  lcd.home();
  lcd.print("Feed       ");
  lcd.setCursor(0, 2);
  lcd.print("mm/s: 0       ");
}

void setupFeedRatio(){
  lcd.home();
  lcd.print("Feed Ratio ");
  lcd.setCursor(0, 2);
  lcd.print("Ratio:            ");
}

void setupPositioning(int32_t spindlePosition){
  lcd.home();
  lcd.print("Positioning  ");
  lcd.setCursor(0, 2);
  lcd.print("Angle:");
  double spindleAngle =  (spindlePosition % encoderPulsesPerRev);
  lcd.print( (double) spindleAngle);
  lcd.setCursor(0,4);
  lcd.print("     ");

}

void updateFeedRatio(int32_t spindlePosition, int feedRatio, int feedRatioControl){
  lcd.setCursor(6, 2);
  feedRatioControl = feedRatio / 100.0;
  lcd.print((double)feedRatioControl);
  lcd.print("  mm/rev ");
  
}

void updateFeedRate(int feedRatePercentage){
  
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