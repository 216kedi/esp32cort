// sensors.cpp
#include "sensors.h"

void Sensors::begin() {
  for (uint8_t i = 0; i < PIR_COUNT; i++) {
    pinMode(PIR_PINS[i], INPUT);     // HC-SR501 aktif surulur; modulun farkliysa INPUT_PULLDOWN/UP
  }
#ifdef ENABLE_SAFETY
  pinMode(SMOKE_PIN, INPUT);
  pinMode(GAS_PIN,   INPUT);
#endif
}

void Sensors::update(uint32_t now) {
  for (uint8_t i = 0; i < PIR_COUNT; i++) {
    bool raw = (digitalRead(PIR_PINS[i]) == PIR_ACTIVE_LEVEL);
    if (raw != pirs_[i].lastRaw) {
      pirs_[i].lastRaw      = raw;
      pirs_[i].lastChangeMs = now;
    }
    if ((now - pirs_[i].lastChangeMs) >= PIR_DEBOUNCE_MS) {
      pirs_[i].stable = raw;         // kararli seviye
    }
  }

#ifdef ENABLE_SAFETY
  updateHazard(now, SMOKE_PIN, SMOKE_ACTIVE_LEVEL, smokeStable_, smokeLastRaw_, smokeChangeMs_);
  updateHazard(now, GAS_PIN,   GAS_ACTIVE_LEVEL,   gasStable_,   gasLastRaw_,   gasChangeMs_);
#endif
}

bool Sensors::motionActive(uint8_t i) const {
  return (i < PIR_COUNT) ? pirs_[i].stable : false;
}

#ifdef ENABLE_SAFETY
void Sensors::updateHazard(uint32_t now, uint8_t pin, uint8_t activeLevel,
                           bool& stable, bool& lastRaw, uint32_t& changeMs) {
  bool raw = (digitalRead(pin) == activeLevel);
  if (raw != lastRaw) {
    lastRaw  = raw;
    changeMs = now;
  }
  if ((now - changeMs) >= HAZARD_DEBOUNCE_MS) {
    stable = raw;
  }
}
#endif
