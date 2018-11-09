/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//===================== setup ==========================================


void setup ()
{
  Serial.begin(115200);   // serial I/O
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("SETUP START");
  eep.begin_spi(EEPROM_CSPIN);
  
  //setup registers and pins for PWM
  pinMode(CHANNEL1_PWM_PIN, OUTPUT);
  pinMode(CHANNEL2_PWM_PIN, OUTPUT);
  pinMode(CHANNEL1_DITHER_PIN, OUTPUT);
  analogWriteResolution(12); //increase to 12 bit PWM resolution
  //ATTENTION! pwm+timer frequency has to be changed to 20kHz at "C:\Users\Work\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\variants\arduino_due_x\variant.h"

  //prepare digital pins
  pinMode(CHANNEL1_DIR_PIN, OUTPUT);
  pinMode(CHANNEL2_DIR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(CHANNEL1_DIR_PIN, LOW); 
  digitalWrite(CHANNEL2_DIR_PIN, LOW); 
  analogWrite(CHANNEL1_PWM_PIN, midval);  //set to zero PWM drive
  analogWrite(CHANNEL2_PWM_PIN, midval);  //set to zero PWM drive
  Serial.println("TEST2");
  digitalWrite(CHANNEL1_DIR_PIN, HIGH);
  digitalWrite(CHANNEL2_DIR_PIN, HIGH);
  pinMode(SETPOINT_PIN, INPUT);
 //----------------------------------------------------------------------------
  //check if there are settings stored in EEprom, if not or zero, replace by defaults
  
  //if not use default values
  if ((int)settings.ee_kp < 10) {
    kp = def_kp; ki = def_ki; kd = def_kd; 
  } else {
    kp = settings.ee_kp; ki = settings.ee_ki; kd = settings.ee_kd;
  }

  if ((int)settings.ee_currentzero< 1) {
    currentzero=def_currentzero;
  } else {
    currentzero= settings.ee_currentzero;
  }

    if ((int)settings.ee_currentlimit< 1) {
    currentlimit=def_currentlimit;
  } else {
    currentlimit= settings.ee_currentlimit;
  }
  
    //set drive for max current
  if ((int)settings.ee_maxval < 1) {
    maxval = def_maxval;
  } else {
    maxval = settings.ee_maxval;
  }
   minval = framesize - maxval;

//----------------------------------------------------------------------------
 //setup PID

  CHANNEL1_PID.SetOutputLimits(0, 4095);             //limits for 12bit PWM  
  CHANNEL1_PID.SetMode(AUTOMATIC); //turn the PID on
  lastTime = millis() - SampleTime; //shift to force PID immedially after read

//----------------------------------------------------------------------------


   Input = TSetpoint;
   Serial.println("SETUP DONE");
}

