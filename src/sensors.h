// sensors.h
// PIR hareket sensorleri (debounce'lu seviye) ve - giris katinda -
// duman/gaz tehlike girisleri (debounce'lu seviye).
// Mandal (latch) / re-arm politikasi SafetyController'da tutulur.
#pragma once
#include <Arduino.h>
#include "config.h"

class Sensors {
public:
  void begin();
  void update(uint32_t now);

  uint8_t pirCount() const { return PIR_COUNT; }
  bool motionActive(uint8_t i) const;   // debounce'lu, seviye tabanli

#ifdef ENABLE_SAFETY
  bool smokeActive() const { return smokeStable_; }
  bool gasActive()   const { return gasStable_; }
#endif

private:
  struct PirCh {
    bool     stable      = false;
    bool     lastRaw     = false;
    uint32_t lastChangeMs= 0;
  };
  PirCh pirs_[PIR_COUNT];

#ifdef ENABLE_SAFETY
  bool     smokeStable_ = false, smokeLastRaw_ = false;
  uint32_t smokeChangeMs_ = 0;
  bool     gasStable_   = false, gasLastRaw_   = false;
  uint32_t gasChangeMs_   = 0;
  void updateHazard(uint32_t now, uint8_t pin, uint8_t activeLevel,
                    bool& stable, bool& lastRaw, uint32_t& changeMs);
#endif
};
