//========================= main loop ========================

void loop ()
{
  //randomizing the read time avoids commensurability oscill.

  now = millis();
  randomtime = (unsigned) random(3 * SampleTime / 4  ); //!!!!!!
  timeChange = (now - lastTime);
  if (timeChange >= randomtime) {

    //----------------------------- main ADC reads------------
    current = readPWMcurrent();
    Input = volt2temp(4 * ReadAnalogAverage(CHANNEL1_ADC1_PIN, 256));
    int setpoint_adc = ReadAnalogAverage(SETPOINT_PIN, 256);
    TSetpoint = 10.00 + setpoint_adc / 50.0;
    //diagnosis output over serial every second (Input, Setpoint, Output)
    if (millis() - lastMessageTime > 1000) {
      Serial.print("I: "); Serial.print(Input); Serial.print(", S:"); Serial.print(TSetpoint); Serial.print(", O: "); Serial.println(Output);
      lastMessageTime = millis();
    }
    lastTime = now;
  }

  //do main PID computation
  CHANNEL1_PID.Compute();

  if  (isfault) {
    Serial.println("FAULT");
    digitalWrite(CHANNEL1_PWM_PIN, HIGH);
    digitalWrite(CHANNEL2_PWM_PIN, HIGH);
  } else {
    analogWrite(CHANNEL1_PWM_PIN, Output);
    analogWrite(CHANNEL2_PWM_PIN, Output);
  }

  //------ periodic error capture---------------------

  if (millis() > periodicTimeCheck)
  {
    //check if thermistor temp is outside rangePID
    Tprotect = 4 * ReadAnalogAverage(protecttemppin, 2);
    int tempprotect = volt2temp(100);

    //do checks and shutdown if needed
    if (Tprotect < catchuppertemplimit || Tprotect > catchlowertemplimit   //thermistor open or shorted
        ||  abs(current) > 1.5 * currentlimit
       ) {
      Serial.print(Tprotect);
#if  drv8872                              //shutdown timer 1
      TCCR1B = 0x00;
      digitalWrite(CHANNEL1_PWM_PIN, LOW);
      digitalWrite(CHANNEL2_PWM_PIN, LOW);
#endif
      isfault = true;
      digitalWrite(LED_PIN, HIGH); //turn ARDUINO LED on in case of error
    } else {
      if (isfault) {
        digitalWrite(CHANNEL1_DIR_PIN, HIGH);
        digitalWrite(CHANNEL2_DIR_PIN, HIGH);
#if  drv8872
        setupPWM16() ;      //restart PWM
#endif
        isfault = false;
        digitalWrite(LED_PIN, LOW); //turn ARDUINO LED on in case of no error
      }
    }
    periodicTimeCheck += 250;
  }
  //end periodic serial IO and error capture, do it every 0.25sec


} //end main loop


