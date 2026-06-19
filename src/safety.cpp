// safety.cpp
#include "safety.h"

#ifdef ENABLE_SAFETY

void SafetyController::begin(LedController* leds, Notifier* notifier, Sensors* sensors) {
  leds_     = leds;
  notifier_ = notifier;
  sensors_  = sensors;
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);
}

void SafetyController::setRelay(bool on) {
  uint8_t level = on ? RELAY_ACTIVE_LEVEL
                     : (RELAY_ACTIVE_LEVEL == HIGH ? LOW : HIGH);
  digitalWrite(RELAY_PIN, level);
}

void SafetyController::update(uint32_t now) {
  bool smoke  = sensors_->smokeActive();
  bool gas    = sensors_->gasActive();
  bool hazard = smoke || gas;

  if (hazard) {
    clearing_ = false;
    if (!engaged_) {
      engaged_ = true;
      setRelay(true);                // ONCE role/siren (ag'dan bagimsiz)
      leds_->alarmFlash();           // sonra kirmizi flas
      String msg = String("Elis Evleri [") + FLOOR_NAME + "] ACIL: ";
      if (smoke && gas)      msg += "DUMAN ve GAZ algilandi!";
      else if (smoke)        msg += "DUMAN algilandi!";
      else                   msg += "GAZ algilandi!";
      msg += " Lutfen binayi kontrol edin.";
      notifier_->enqueue(msg);       // BIR KEZ (yukselen kenarda)
      Serial.printf("[SAFETY] ALARM: %s\n", msg.c_str());
    }
  } else {
    if (engaged_) {
      if (!clearing_) {
        clearing_     = true;
        clearSinceMs_ = now;
      } else if ((now - clearSinceMs_) >= ALARM_REARM_MS) {
        engaged_  = false;
        clearing_ = false;
        setRelay(false);
        leds_->clearAlarm();
        notifier_->enqueue(String("Elis Evleri [") + FLOOR_NAME + "]: Alarm normale dondu.");
        Serial.println("[SAFETY] Temizlendi / yeniden hazir.");
      }
    }
  }
}

#endif // ENABLE_SAFETY
