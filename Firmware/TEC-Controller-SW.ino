/* Arduino based TEC Controller
by Tobias Gutmann 2018
based on http://hololaser.kwaoo.me/electronics/Arduino_TEC/Arduino-TEC.html

--- HARDEWARE CONFIGS: ---

    JUMPER SETTINGS (defines which is the top or bottom board):
    
    Board TOP: Short on the pads to Pins: 2,4,6,8,A0,A2,A4,A6
    Board BOTTOM: Short on the pads to Pins: 3,5,7,9, A1, A3, A5, A7

    Resistor assembly variant:  
    R1=R2=R4=10k, R5=2.7k, R3=empty: Range ~5-55 degrees
*/

#include <PID_v1.h> //Runs on Due also with severals instances
#include <SPIEEP_DUE.h> //used for SPI EEPROM, special DUE edition
#include "SPI.h" //used for SPI EEPROM

//PIN definitions

SPIEEP eep(16, 16, 2048); // Microchip 25A
#define EEPROM_CSPIN 52  // Chip Select attached to pin 52

#define CHANNEL1_PWM_PIN 4
#define CHANNEL2_PWM_PIN 8
#define CHANNEL3_PWM_PIN 9
#define CHANNEL4_PWM_PIN 5

#define CHANNEL1_DIR_PIN 2
#define CHANNEL2_DIR_PIN 6
#define CHANNEL3_DIR_PIN 7
#define CHANNEL4_DIR_PIN 3

#define CHANNEL1_ADC1_PIN A0
#define CHANNEL2_ADC1_PIN A4
#define CHANNEL3_ADC1_PIN A1
#define CHANNEL4_ADC1_PIN A5

#define CHANNEL1_ADC2_PIN A2
#define CHANNEL2_ADC2_PIN A6
#define CHANNEL3_ADC2_PIN A3
#define CHANNEL4_ADC2_PIN A7

#define CHANNEL1_CURRENT_PIN A8
#define CHANNEL2_CURRENT_PIN A9
#define CHANNEL3_CURRENT_PIN A10
#define CHANNEL4_CURRENT_PIN A11

#define CHANNEL1_DITHER_PIN 10
#define CHANNEL2_DITHER_PIN 12
#define CHANNEL3_DITHER_PIN 11
#define CHANNEL4_DITHER_PIN 13

#define LED_PIN 13
#define SETPOINT_PIN A11

//----------------------------------------

int def_currentzero = -3;  //for 0.1 Ohm + Opamp extra


//PWM hardware configs
int def_currentlimit = 2500; //mA TEC
int  midval = 2045;

//---------------------PWM defaults-------------------

//opamp configs
double TSetpoint = 20.000;
//restrict controls to non-saturated region
int lowertemplimit = 5;
int uppertemplimit = 55;
//for thermistor far-out-of-range (incl open/shorted) shutdown
// measured directly at protecttemppin
int catchlowertemplimit = 2900; //lim < 6C
int catchuppertemplimit = 800; //lim > 60C

//analog pins

int protecttemppin = A2;

//-----  PWM setup ------------------------

//resolution of PWM output, determines a lot of other vars
uint16_t framesize = 4095;  //12bit PWM on Arduino Due
int icrsize = 255;        //determines PWM frequ, 62 khz, basic 8 bit res

uint16_t midframe = (framesize - 1) / 2 + 1;
volatile int Nsteps = 2 * (midframe / (icrsize + 1));
uint16_t minval = midframe / 2; //PWM drive limt default min  ok
uint16_t  maxval = framesize - minval;
uint16_t  def_maxval = maxval;

volatile int icount = 0;
volatile int Nbaseval;
volatile int Nfraction;

float current = 0;
int therm;
int count = 0;
int inputtrack = 0;
int input10 = 0;
int inputcount = 0;
int currentzero;

//--------------- other variables ----------------------------
boolean serialverbose = false;
boolean iflabview = false;
boolean ttymode = true;
boolean isfault = false;

//TEC
boolean tuning = false;
boolean currentflip = false;
double  Tprotect;

//----------  timing --------------------------------
int SampleTime = 100; //msec, should match internal PID sampling time

long int timescale = 64000; //corr to 1sec!

unsigned long periodicTimeCheck;
unsigned long lastTime;
unsigned long now;
unsigned long timeChange;
unsigned long randomtime;
unsigned long lastMessageTime = 0;



//----------------------PID setup------------------------
//PID defaults
double def_kp = 300;
double def_ki = 17; 
double def_kd = 0;

double kp = def_kp;
double ki = def_ki;
double kd = def_kd;
int currentlimit = def_currentlimit;

//strucht for storing defaults in EEprom
struct settings_t
{
  double ee_kp; //Arduino Due: 8byte
  double ee_ki; //Arduino Due: 8byte
  double ee_kd; //Arduino Due: 8byte
  uint16_t ee_maxval; //Arduino Due: 2Byte
  int ee_currentzero; //Arduino Due: 4byte
  int ee_currentlimit; //Arduino Due: 4byte
} settings; //SUM for Arduino DUE: 34byte

//init
double Input = TSetpoint; //default approx room temp from thermistor
double Output = midval;  //zero drive to bridge

//Specify the links and initial PID tuning parameters   kp, ki, kd
PID CHANNEL1_PID(&Input, &Output, &TSetpoint, kp, ki, kd, REVERSE);
