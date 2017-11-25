#include "TimerOne.h"
#include "sample"

const unsigned long TIMER_RESOLUTION = 1000000;
const int PWM_PIN = 9;
const int PWM_PERIOD = 8;
const int PWM_MAX = 1023;
const int PWM_SILENCE = 511;

const int SAMPLING_FREQ = 8000; // Hz
const float SAMPLE_FREQ = 130.81; // Hz (C-3)
const float SAMPLES_PER_TICK = (float) SAMPLING_FREQ / TIMER_RESOLUTION;
const int SAMPLE_MULTIPLIER = 4;
const byte SAMPLE_SILENCE = 0;

const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

typedef long SampleIndex;
typedef long Microseconds;
typedef float Frequency;

typedef struct {
  Frequency freq;
  Microseconds triggered;
} Channel;

Channel channels[] = {
  { 130.81, 1*TIMER_RESOLUTION },
  { 164.81, 1.5*TIMER_RESOLUTION },
  { 261.62, 2*TIMER_RESOLUTION },
};

void setup() 
{
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);
}

char sampled(Microseconds time, int channelIndex) {
  Channel channel = channels[channelIndex];
  float samplesPerTick = channel.freq/SAMPLE_FREQ * SAMPLES_PER_TICK;
  Microseconds currentSampleTime = time - channel.triggered;
  SampleIndex sampleIndex = currentSampleTime * samplesPerTick;
  if (sampleIndex >= 0 && sampleIndex < SAMPLE_SIZE) {
    return pgm_read_byte_near(SAMPLE + sampleIndex);
  } else {
    return SAMPLE_SILENCE;
  }
}

void loop() 
{
 Microseconds t = micros();
 int sum = sampled(t, 0) + sampled(t, 2);
 int output = PWM_SILENCE + sum * SAMPLE_MULTIPLIER/2;
 Timer1.pwm(PWM_PIN, output, PWM_PERIOD);

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


