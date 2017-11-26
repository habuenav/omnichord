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
const int SOFT_POT_MIN = 0;
const int SOFT_POT_RESOLUTION = SOFT_POT_MAX - SOFT_POT_MIN;

typedef int TriggerState;
static const TriggerState PRESSED  = HIGH;
static const TriggerState RELEASED = LOW;

typedef long Microseconds;
static const Microseconds DEBOUNCE_DELAY = 3000;

typedef long SampleIndex;
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
  Microseconds lastDebounceTime;
  TriggerState state;
  TriggerState previousState;
  boolean isRinging;
} Channel;

Channel channels[] = {
  { C_3 },
  { E_3 },
  { G_3 },
  { A_3 },
  { C_4 },
  { E_4 },
  { G_4 },
  { A_4 },
};

int numberOfChannels = sizeof(channels)/sizeof(Channel);

void setup() 
{
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);

  float softPotStepsPerChannel = (float) SOFT_POT_RESOLUTION/numberOfChannels;
  float triggerMargin = 1.0/6;
  for(int i=0; i<numberOfChannels; i++) {
    channels[i].triggered = LONG_MAX;
    channels[i].lastDebounceTime = 0;
    channels[i].state = RELEASED;
    channels[i].previousState = RELEASED;
    channels[i].isRinging = false;

    channels[i].softPotMin = 1 + SOFT_POT_MIN + i * softPotStepsPerChannel + triggerMargin * softPotStepsPerChannel;
    channels[i].softPotMax = SOFT_POT_MIN + (i+1) * softPotStepsPerChannel - triggerMargin * softPotStepsPerChannel;
//    Serial.print(channels[i].softPotMin);
//    Serial.print(" ");
//    Serial.print(channels[i].softPotMax);
//    Serial.print(" ");
//    Serial.println(channels[i].softPotMax - channels[i].softPotMin);
  }
}

char getSample(Channel *channel, Microseconds time) {
  Microseconds currentSampleTime = time - channel->triggered;
  SampleIndex sampleIndex = currentSampleTime * channel->note->samplesPerTick;

  if (sampleIndex < SAMPLE_SIZE) {
    return pgm_read_byte_near(SAMPLE + sampleIndex);
  } else {
    channel->isRinging = false;
    return SAMPLE_SILENCE;
  }
}

void updateTrigger(Channel *channel, TriggerState state, Microseconds time) {
  if (state != channel->previousState) {
    channel->lastDebounceTime = time;
  }

  if ((time - channel->lastDebounceTime) > DEBOUNCE_DELAY) {
    if (state != channel->state) {
      if ((state == PRESSED) && (channel->state == RELEASED)) {
        channel->triggered = time;
        channel->isRinging = true;
      }
      channel->state = state;
    }
  }

  channel->previousState = state;
}

void loop() 
{
  int softPot = analogRead(SOFT_POT_PIN);

  Microseconds t = micros();
  int sum = 0;

  for (int i=0; i<numberOfChannels; i++) {
    Channel *channel = &channels[i];

    Serial.print(channel->isRinging);
    Serial.print(" ");

    if (softPot >= channel->softPotMin && softPot < channel->softPotMax) {
      updateTrigger(channel, PRESSED, t);
    } else {
      updateTrigger(channel, RELEASED, t);
    }

    if (channel->isRinging) {
      sum += getSample(channel, t);
    }
  }
  Serial.println();
 
  int output = PWM_SILENCE + sum;
  Timer1.pwm(PWM_PIN, output, PWM_PERIOD);
}

/*
Connections:
Pin 9 through potentiometer to speaker +
Speaker GND to GDN
*/

