# ELS_TEENSY
This project is for using a Teensy 3.2 or Teensy 4.0. 
Its purpose is to allow for asynchronous stepper movements that are either coupled to spindle rotation, or independent of lathe spindle rotation. 
In its current form, it uses a Serial Wombat to handle the user input data over I2C. A HD44780 LCD is also on the I2C bus. 
The mode button is hooked up to the button function of the user input encoder. Stepper movements are controlled via the rotatino of the input encoder. 

The serial wombat is used to offload some of the work that the Teensy has to do. Eventually the serial wombat will also be in control of the LCD. 

The Teensy has hardware encoder availailabilty via the QuadDecode classes: https://forum.pjrc.com/threads/26803-Hardware-Quadrature-Code-for-Teensy-3-x
The Teensy 3.2 has HW quadrature decoding on pins 2/3 for encoder A and pins 25/32 for encoder B.
Currently, hardware decoding for the Teensy 4.0 would require usage of another library found here: https://forum.pjrc.com/threads/58478-Teensy-4-x-H-W-Quadrature-Encoder-Library
The Teensy 4.0 has HW quadrature decoding on pins 0-5, 7, 30, 31 and 33. 

More information on the Serial Wombat can be found here: https://www.serialwombat.com/
