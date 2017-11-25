#include "TimerOne.h"
#include "sample"

const unsigned long TIMER_RESOLUTION = 1000000;
const int PWM_PIN = 9;
const int PWM_PERIOD = 8;
const int PWM_MAX = 1023;

const int SAMPLE_FREQ = 8000; // Hz
const float SAMPLES_PER_TICK = (float) SAMPLE_FREQ / TIMER_RESOLUTION;
const int SAMPLE_MAX = 255;
const int SAMPLE_MULTIPLIER = (PWM_MAX+1) / (SAMPLE_MAX+1);
const byte SAMPLE_SILENCE = 127;

const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

typedef long SampleIndex;
typedef long Microseconds;
typedef float Frequency;

typedef struct {
  Frequency freq;
  Microseconds triggered;
} Note;

Note notes[] = {
  { 261.63, 1*TIMER_RESOLUTION }
};

void setup() 
{
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);
}

byte sampled(Microseconds time, int noteIndex) {
  Note note = notes[noteIndex];
  Microseconds currentSampleTime = time - note.triggered;
  SampleIndex sampleIndex = currentSampleTime * SAMPLES_PER_TICK;
  if (sampleIndex >= 0 && sampleIndex < SAMPLE_SIZE) {
    return pgm_read_byte_near(SAMPLE + sampleIndex);
  } else {
    return SAMPLE_SILENCE;
  }
}

void loop() 
{
 Microseconds t = micros();
 byte b = sampled(t, 0);
 int i = (int) b * SAMPLE_MULTIPLIER;
 Timer1.pwm(PWM_PIN, i, PWM_PERIOD);

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


