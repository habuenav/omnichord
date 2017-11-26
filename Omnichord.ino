#include "limits.h"
#include "TimerOne.h"
#include "sample"

typedef int Pin;

const unsigned long TIMER_RESOLUTION = 1000000;
const Pin PWM_PIN = 9;
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
  Microseconds triggered;
  boolean isRinging;
} Stringgg;

#include "strings"

typedef Stringgg *Chord[8];

Chord chords[] = {
  { C_3, E_3, G_3, A_3, C_4, E_4, G_4, A_4 },
  { F_3, A_3, C_3, Eb3, F_4, A_4, C_4, Eb4 },
};

Chord *activeChord = &chords[0];

typedef struct {
  Microseconds lastDebounceTime;
  TriggerState state;
  TriggerState previousState;
} Pressable;

typedef struct {
  int softPotMin;
  int softPotMax;
  Pressable pressable;
} Channel;

const int numberOfChannels = 8;
Channel channels[numberOfChannels];

typedef struct {
  Pin pin;
  Pressable pressable;
} Button;

Button buttons[] = {
  { 2 },
};

const int numberOfButtons = sizeof(buttons)/sizeof(Button);

void initPressable(Pressable *pressable) {
  pressable->lastDebounceTime = 0;
  pressable->state = RELEASED;
  pressable->previousState = RELEASED;
}

void setup() {
  Timer1.initialize();

  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);

  for(int i=0; i<numberOfButtons; i++) {
    initPressable(&(buttons[i].pressable));
  }

  float softPotStepsPerChannel = (float) SOFT_POT_RESOLUTION/numberOfChannels;
  float triggerMargin = 1.0/6;
  for(int i=0; i<numberOfChannels; i++) {
    initPressable(&(channels[i].pressable));

    channels[i].softPotMin = 1 + SOFT_POT_MIN + i * softPotStepsPerChannel + triggerMargin * softPotStepsPerChannel;
    channels[i].softPotMax = SOFT_POT_MIN + (i+1) * softPotStepsPerChannel - triggerMargin * softPotStepsPerChannel;
  }
}

char getSample(Stringgg *string, Microseconds time) {
  Microseconds currentSampleTime = time - string->triggered;
  SampleIndex sampleIndex = currentSampleTime * string->samplesPerTick;

  if (sampleIndex < SAMPLE_SIZE) {
    return pgm_read_byte_near(SAMPLE + sampleIndex);
  } else {
    string->isRinging = false;
    return SAMPLE_SILENCE;
  }
}

boolean handlePressable(Pressable *pressable, TriggerState state, Microseconds time) {
  TriggerState wasJustPressed = false;

  if (state != pressable->previousState) {
    pressable->lastDebounceTime = time;
  }

  if ((time - pressable->lastDebounceTime) > DEBOUNCE_DELAY) {
    if (state != pressable->state) {
      if ((state == PRESSED) && (pressable->state == RELEASED)) {
        wasJustPressed = true;
      }
      pressable->state = state;
    }
  }

  pressable->previousState = state;
  return wasJustPressed;
}

void updateTrigger(Channel *channel, Stringgg *string, TriggerState state, Microseconds time) {
  if (handlePressable(&channel->pressable, state, time)) {
    string->triggered = time;
    string->isRinging = true;
  }
}

void updateButton(Button *button, Microseconds time) {
  TriggerState state = digitalRead(button->pin);

  handlePressable(&(button->pressable), state, time);
}

void loop() 
{
  int softPot = analogRead(SOFT_POT_PIN);

  Microseconds t = micros();
  int sum = 0;

  for (int i=0; i<numberOfButtons; i++) {
    updateButton(&buttons[i], t);
  }

  if (buttons[0].pressable.state == PRESSED) {
    activeChord = &chords[1];
  } else {
    activeChord = &chords[0];
  }

  for (int i=0; i<numberOfChannels; i++) {
    Channel *channel = &channels[i];
    Stringgg *string = (*activeChord)[i];

    if (softPot >= channel->softPotMin && softPot < channel->softPotMax) {
      updateTrigger(channel, string, PRESSED, t);
    } else {
      updateTrigger(channel, string, RELEASED, t);
    }
  }

  for (int i=0; i<numberOfStrings; i++) {
    Stringgg *string = &strings[i];

    if (string->isRinging) {
      sum += getSample(string, t);
    }
  }
 
  int output = PWM_SILENCE + sum;
  Timer1.pwm(PWM_PIN, output, PWM_PERIOD);
}
