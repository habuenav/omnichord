#include "TimerOne.h"


int position;
int width = 1023;
int step_max = 20;
int step_min = -step_max;

const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

void setup() 
{
  Timer1.initialize();
  position = random(width);

//  Serial.begin(9600);
//  pinMode(SOFT_POT_PIN, INPUT);
}

void loop() 
{
  Timer1.pwm(9, position, 8);

  position = position + random(step_min, step_max);
  if (position > width) {
    position = 2*width - position;
  } else {
    if (position < 0) {
      position = -position;
    }
  };



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
