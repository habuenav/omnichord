#include "TimerOne.h"
#include "sample"

const unsigned long TIMER_RESOLUTION = 1000000;
const float PI2 = PI*2;
const float PI2TIMER_RESOLUTION = PI*2/TIMER_RESOLUTION;
const int PWM_PIN = 9;
const int PWM_PERIOD = 8;
const int WIDTH = 1023;
const int MAX_AMPLITUDE = WIDTH / 2;

const SAMPLE_FRQ = 8000; // Hz

const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

void setup() 
{
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);
}


void monitor(int i) {
  Serial.println(i);
  delay(100);
}

float pitchC = 261.63;
float multiplierC = pitchC * PI2TIMER_RESOLUTION;

float pitchE = 329.63;
float multiplierE = pitchE * PI2TIMER_RESOLUTION;

float pitchG = 392.00;
float multiplierG = pitchG * PI2TIMER_RESOLUTION;

float pitchBb = 466.16;
float multiplierBb = pitchBb * PI2TIMER_RESOLUTION;

float signal(long micros, float multiplier) {
  return sin(micros * multiplier);
}

void loop() 
{
  byte x = sample[0];
  long t = micros();
  float v = 
    signal(t, multiplierBb);

  v = v/4;

  int i = MAX_AMPLITUDE + (int) (v * MAX_AMPLITUDE);
  Timer1.pwm(PWM_PIN, i, PWM_PERIOD);

  monitor(i);

//  int softPotADC = analogRead(SOFT_POT_PIN);
//  int softPotPosition = map(softPotADC, 0, 1023, 0, GRAPH_LENGTH);
//
//  Serial.print("<"); // Starting end
//  for (int i=0; i<GRAPH_LENGTH; i++) {
//    if (i == softPotPosition) Serial.print("|");
//    else Serial.print("-");
//  }
//  Serial.println("> (" + String(softPotADC) + ")");
//
//  delay(25);
}

/*
Connections:
Pin 9 through potentiometer to speaker +
Speaker GND to GDN
*/


