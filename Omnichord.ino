#include "limits.h"
#include "TimerOne.h"
#include "sample"

typedef int Pin;

const signed long TIMER_RESOLUTION = 1000000;
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

const int BUTTON1_PIN = 2;
const int BUTTON2_PIN = 3;

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
  { F_3, A_3, C_4, Eb4, F_4, A_4, C_5, Eb5 },
  { G_3, B_3, C_4, F_4, G_4, B_4, D_5, F_5 },
};

volatile Chord *activeChord = &chords[0];

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
  { BUTTON1_PIN },
  { BUTTON2_PIN },
};

const int numberOfButtons = sizeof(buttons)/sizeof(Button);

void initPressable(Pressable *pressable) {
  pressable->lastDebounceTime = 0;
  pressable->state = RELEASED;
  pressable->previousState = RELEASED;
}

volatile Microseconds time;

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

void updateButton(Button *button) {
  TriggerState state = digitalRead(button->pin);

  if (state != button->pressable.state) {
    if (state == PRESSED) {
      button->pressable.state = PRESSED;
    } else {
      button->pressable.state = RELEASED;
    }
  }
}

void updateChords() {
  if (buttons[0].pressable.state == PRESSED) {
    activeChord = &chords[1];
  } else {
    if (buttons[1].pressable.state == PRESSED) {
      activeChord = &chords[2];
    } else {
      activeChord = &chords[0];
    }
  }
}

void button1ChangeHandler() {
  updateButton(&buttons[0]);
  updateChords();
}

void button2ChangeHandler() {
  updateButton(&buttons[1]);
  updateChords();
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

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), button1ChangeHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), button2ChangeHandler, CHANGE);
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

void updateTrigger(Channel *channel, Stringgg *string, TriggerState state, Microseconds time) {
  if (handlePressable(&channel->pressable, state, time)) {
    string->triggered = time;
    string->isRinging = true;
  }
}

void updateStrings(Microseconds time) {
  int softPot = analogRead(SOFT_POT_PIN);

  for (int i=0; i<numberOfChannels; i++) {
    Channel *channel = &channels[i];
    Stringgg *string = (*activeChord)[i];

    if (softPot >= channel->softPotMin && softPot < channel->softPotMax) {
      updateTrigger(channel, string, PRESSED, time);
    } else {
      updateTrigger(channel, string, RELEASED, time);
    }
  }
}

unsigned long loopCounter = 0;
Microseconds timer = 0;
Microseconds previousTime = 0;

int previousStringIndex = -1;

void loop() 
{
  time = micros();

  if (timer >= TIMER_RESOLUTION) {
    Serial.println(loopCounter);
    loopCounter = 0;
    timer = 0;
  } else {
    loopCounter += 1;
    timer += (time - previousTime);
    previousTime = time; 
  }

//  updateStrings(time);

  int stringIndex = (time / (TIMER_RESOLUTION/3)) % numberOfChannels;
  if (stringIndex != previousStringIndex) {
    Stringgg *string = (*activeChord)[stringIndex];
    string->triggered = time;
    string->isRinging = true;
    previousStringIndex = stringIndex;
  }

  int sum = 0;
  for (int i=0; i<numberOfStrings; i++) {
    Stringgg *string = &strings[i];

    if (string->isRinging) {
      sum += getSample(string, time);
    }
  }
 
  int output = PWM_SILENCE + sum;
  Timer1.pwm(PWM_PIN, output, PWM_PERIOD);
}
