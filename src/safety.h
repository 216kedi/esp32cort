// safety.h
// Duman/gaz tehlikesini degerlendirir: role (siren) surer, LED alarmini tetikler,
// WhatsApp bildirimini BIR KEZ kuyruga alir; kosul temizlenip re-arm suresi
// dolunca sifirlanir. Mandallama burada yapilir (titreme/spam engellenir).
#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_SAFETY

#include "led_effects.h"
#include "sensors.h"
#include "notifier.h"

class SafetyController {
public:
  void begin(LedController* leds, Notifier* notifier, Sensors* sensors);
  void update(uint32_t now);

private:
  LedController* leds_     = nullptr;
  Notifier*      notifier_ = nullptr;
  Sensors*       sensors_  = nullptr;

  bool     engaged_     = false;     // alarm devrede mi (mandalli)
  bool     clearing_    = false;     // kosul temizlendi, re-arm sayimi
  uint32_t clearSinceMs_= 0;

  void setRelay(bool on);
};

#endif // ENABLE_SAFETY
