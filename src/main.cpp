// main.cpp
// Elis Evleri - ESP32 koridor aydinlatma + guvenlik firmware.
// Kat profili (FLOOR_GROUND / FLOOR_UPPER) platformio.ini build_flags ile secilir.
#include <Arduino.h>
#include "config.h"
#include "led_effects.h"
#include "sensors.h"
#ifdef ENABLE_SAFETY
  #include "notifier.h"
  #include "safety.h"
#endif

LedController leds;
Sensors       sensors;
#ifdef ENABLE_SAFETY
  Notifier         notifier;
  SafetyController safety;
#endif

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("=== Elis Evleri ESP32 [" FLOOR_NAME "] ===");

  leds.begin();
  sensors.begin();
#ifdef ENABLE_SAFETY
  notifier.begin();
  safety.begin(&leds, &notifier, &sensors);
#endif
}

void loop() {
  uint32_t now = millis();

  sensors.update(now);          // PIR + (varsa) duman/gaz debounce
#ifdef ENABLE_SAFETY
  safety.update(now);           // tehlike -> role + bildir + LED alarm (ag'dan bagimsiz)
#endif

  // PIR hareketini LED bolgelerine esle (kat profiline gore)
#if defined(FLOOR_GROUND)
  if (sensors.motionActive(0)) leds.motionRect(now);                 // PIR1 -> dikdortgen
  if (sensors.motionActive(1) || sensors.motionActive(2))            // PIR2/PIR3 -> koridor
    leds.motionCorridor(now);
#else // FLOOR_UPPER
  if (sensors.motionActive(0) || sensors.motionActive(1))            // 2 PIR -> koridor
    leds.motionCorridor(now);
#endif

  leds.update(now);             // animasyon karesini ilerlet (engellemesiz)
#ifdef ENABLE_SAFETY
  notifier.update(now);         // WiFi yeniden baglanma + WhatsApp kuyrugu
#endif
}
