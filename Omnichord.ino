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
const byte SAMPLE_SILENCE = 0;

const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

typedef long SampleIndex;
typedef long Microseconds;
typedef float Frequency;

float samplesPerTick(float freq) {
  return freq/SAMPLE_FREQ * SAMPLES_PER_TICK;
}

typedef struct {
  float samplesPerTick;
} Note;

#import "notes"

typedef struct {
  Note *note;
  Microseconds triggered;
} Channel;

Channel channels[] = {
  { C_3, 1*TIMER_RESOLUTION },
  { E_3, 1.5*TIMER_RESOLUTION },
  { G_3, 2*TIMER_RESOLUTION },
  { Bb3, 2.5*TIMER_RESOLUTION },
};

int numberOfChannels = sizeof(channels)/sizeof(Channel);

void setup() 
{
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);
}

char sampled(Microseconds time, int channelIndex) {
  Channel channel = channels[channelIndex];

  Microseconds currentSampleTime = time - channel.triggered;
  SampleIndex sampleIndex = currentSampleTime * channel.note->samplesPerTick;

  if (sampleIndex >= 0 && sampleIndex < SAMPLE_SIZE) {
    return pgm_read_byte_near(SAMPLE + sampleIndex);
  } else {
    return SAMPLE_SILENCE;
  }
}

void loop() 
{
  Microseconds t = micros();
  int sum = 0;
 
  for (int i=0; i<numberOfChannels; i++) {
    sum += sampled(t, i);
  }
 
  int output = PWM_SILENCE + sum;
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


