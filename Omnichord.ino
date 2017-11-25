#include "limits.h"
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
const int SOFT_POT_MAX = 1024;
const int SOFT_POT_MIN = 64;
const int SOFT_POT_RESOLUTION = SOFT_POT_MAX - SOFT_POT_MIN;

typedef long SampleIndex;
typedef long Microseconds;
typedef float Frequency;

float samplesPerTick(float freq) {
  return freq/SAMPLE_FREQ * SAMPLES_PER_TICK;
}

typedef struct {
  float samplesPerTick;
} Note;

#include "notes"

typedef struct {
  Note *note;
  Microseconds triggered;
  int softPotMin;
  int softPotMax;
} Channel;

Channel channels[] = {
  { C_3, LONG_MAX },
  { E_3, LONG_MAX },
  { G_3, LONG_MAX },
  { A_3, LONG_MAX },
  { C_4, LONG_MAX },
  { E_4, LONG_MAX },
  { G_4, LONG_MAX },
  { A_4, LONG_MAX },
};

int numberOfChannels = sizeof(channels)/sizeof(Channel);

void setup() 
{
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);

  float softPotStepsPerChannel = (float) SOFT_POT_RESOLUTION/numberOfChannels;
  for(int i=0; i<numberOfChannels; i++) {
    channels[i].softPotMin = SOFT_POT_MIN + i * softPotStepsPerChannel;
    channels[i].softPotMax = channels[i].softPotMin + softPotStepsPerChannel;
  }
}

char getSample(Channel *channel, Microseconds time) {
  Microseconds currentSampleTime = time - channel->triggered;
  SampleIndex sampleIndex = currentSampleTime * channel->note->samplesPerTick;

  if (sampleIndex >= 0 && sampleIndex < SAMPLE_SIZE) {
    return pgm_read_byte_near(SAMPLE + sampleIndex);
  } else {
    return SAMPLE_SILENCE;
  }
}

void loop() 
{
  int softPot = analogRead(SOFT_POT_PIN);

  Microseconds t = micros();
  int sum = 0;

  for (int i=0; i<numberOfChannels; i++) {
    Channel *channel = &channels[i];

    if (softPot > channel->softPotMin && softPot < channel->softPotMax) {
      channel->triggered = t;
    }

    sum += getSample(channel, t);
  }
 
  int output = PWM_SILENCE + sum;
  Timer1.pwm(PWM_PIN, output, PWM_PERIOD);
}

/*
Connections:
Pin 9 through potentiometer to speaker +
Speaker GND to GDN
*/

