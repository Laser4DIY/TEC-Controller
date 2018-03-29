//---- Ardunino ADC and temp conversion routines
float volt2temp(float x);
double ReadPotT(int pin);
int temp2volt(float x);

//--------------oversample internal 10bit ADC-------------------------

//average, oversampling
double ReadAnalogAverage(int pin, int average) {
  double inputsum = 0;
  for (int inputcount = 1; inputcount <= average; inputcount++) {
    inputsum += analogRead(pin);
  }
  return inputsum / average;
}

//read pot and allow for stability only discrete steps of 0.2C
double ReadPotT(int pin) {
  double pot = 0;
  if (pot < lowertemplimit) {
    return lowertemplimit;
  }
  else if (pot > uppertemplimit) {
    return uppertemplimit;
  }
  else {
    return (double)round(5 * pot) / 5;
  }
}


//volt2temp interpolating function from Mathematica
float volt2temp(float x) {
  return 63.76 - 0.02716 * x + 0.00000573677 * x * x - 0.0000000008381 * x * x * x;
}

//temp2volt interpolating function from Mathematica
int temp2volt(float x) {
  return round(3799.7 - 68.282 * x - 0.21658 * x * x + 0.005728 * x * x * x);
}


//OLD CURRENT CONVERSION, NEEDS TO BE UPDATED!
float readPWMcurrent() {
  float x;
  float convfactor = 5.46875;
  x = 2800 - convfactor * ReadAnalogAverage(CHANNEL1_CURRENT_PIN, 8) -  currentzero;
  return x;
}

