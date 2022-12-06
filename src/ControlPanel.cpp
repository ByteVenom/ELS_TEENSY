#include "ControlPanel.h"


hd44780_I2Cexp lcd(0x27);


void ControlPanel::updateRPM(int RPM, char currentMode, int32_t spindlePosition, int32_t feedRateControl){
    //add in angle readout only in positioning mod

    lcd.setCursor(5, 1);
    
    lcd.print(RPM);
    lcd.print("   ");
   if(currentMode == 'p'){
    lcd.setCursor(6,2);
      double encoderAngle =  ((spindlePosition % encoderPulsesPerRev)/ (double) encoderPulsesPerRev) * 360;
      lcd.print( (double) encoderAngle);
   }else if(currentMode == 'f'){
    double leadScrewRPMRatio = ((feedRateControl * maxStepperRPM)/2)/RPM;
    lcd.setCursor(0,3);
    lcd.print( (double) leadScrewRPMRatio);
   }
    
      
     
    //Calculate current feed ratio based off rpm and commanded stepper RPM
   
    
    
    
      
    
  
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
  lcd.print("mm/s: 0.00    ");
}

void ControlPanel::setupFeedRatio(int feedRatio){
  lcd.home();
  lcd.print("Feed Ratio ");
  lcd.setCursor(0, 2);
  lcd.print("mm/rev: 0          ");
  lcd.setCursor(8, 2);
  lcd.print(feedRatio);
}

void ControlPanel::setupPositioning(int32_t spindlePosition){
  lcd.home();
  lcd.print("Positioning  ");
  lcd.setCursor(0, 2);
  lcd.print("Angle:");
  double spindlePositionIntegral;
  double spindlePositionFractional = modf( (double) spindlePosition/encoderPulsesPerRev, &spindlePositionIntegral );
  double spindleAngle =  spindlePositionFractional * 360;
  lcd.print( (double) spindleAngle, 5);
  lcd.setCursor(0,4);
  lcd.print("     ");

}

void ControlPanel::updateFeedRatio(int pitchCounter ){
  lcd.setCursor(8, 2);

  lcd.print(pitches[pitchCounter]);
  
  
}

void ControlPanel::updateFeedRate(float feedRate){
  
  //double displayFeedRate = ((feedRate / (double) feedRateRes) * maxStepperRPM * leadscrewPitch * leadscrewRatio) / 60.00;
  //mm/s takes 1mm pitch lead screw, which means 1 rpm = 1mm.  
  //1rpm = 1/60 mm/s
  //Stepper to leadscrew ratio = .5
  //displayFeedRate = map(displayFeedRate,0, 100, 0, 600);
  lcd.setCursor(6, 2);
  lcd.print((double)feedRate);
  lcd.print(" ");
  
}

