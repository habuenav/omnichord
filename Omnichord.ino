#include "TimerOne.h"

int position;
int width = 1023;
int step_max = 20;
int step_min = -step_max;

const int WIDTH = 1023;
const int HALF_WIDTH = WIDTH / 2;
const unsigned long TIMER_RESOLUTION = 1000000;

const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

void setup() 
{
  Timer1.initialize();
  position = random(width);

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);
}


void monitor(int i) {
  Serial.println(i);
  delay(100);
}

void loop() 
{
  float pitch = 440;
  float t = micros() * pitch * 2 * PI / TIMER_RESOLUTION;
  double v = sin(t);
  int i = (int) (v * HALF_WIDTH) + HALF_WIDTH;
  Timer1.pwm(9, i, 1);

//  monitor(i);

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


