// led_effects.h
// Engellemesiz (millis tabanli) LED animasyon denetleyicisi.
//  - Dikdortgen bolge (sadece giris kati): sirayla yanma (sequential fill)
//  - Koridor bolgesi: fade in / hold / fade out
//  - Alarm: tum serit kirmizi flas (hareketi ezer)
#pragma once
#include <FastLED.h>
#include "config.h"

class LedController {
public:
  void begin();
  void update(uint32_t now);          // her dongude cagrilir; kareyi ilerletir

  // Hareket tetikleyicileri (PIR seviyesi aktifken her dongude cagrilabilir):
#if defined(FLOOR_GROUND)
  void motionRect(uint32_t now);      // PIR1 -> dikdortgen sirayla yanma
#endif
  void motionCorridor(uint32_t now);  // koridor PIR(ler) -> fade in

  // Guvenlik denetleyicisi tarafindan cagrilir:
  void alarmFlash();                  // kirmizi flas moduna gec (mandalli)
  void clearAlarm();                  // alarmdan cik, sondur

private:
  CRGB leds_[NUM_LEDS];
  uint32_t lastFrameMs_ = 0;

  // Alarm durumu (her seyi ezer)
  bool     alarmActive_ = false;
  bool     flashOn_     = false;
  uint32_t lastFlashMs_ = 0;

  // Koridor bolgesi durum makinesi
  enum CorState : uint8_t { COR_OFF, COR_FADE_IN, COR_HOLD, COR_FADE_OUT };
  CorState corState_     = COR_OFF;
  uint8_t  corBri_       = 0;
  uint32_t corHoldUntil_ = 0;

#if defined(FLOOR_GROUND)
  // Dikdortgen bolge durum makinesi
  enum RectState : uint8_t { RECT_OFF, RECT_SEQ_FILL, RECT_HOLD, RECT_FADE_OUT };
  RectState rectState_    = RECT_OFF;
  uint8_t   rectBri_      = 0;
  uint8_t   rectCursor_   = 0;     // kac halka acildi (sequential fill)
  uint32_t  rectStepMs_   = 0;
  uint32_t  rectHoldUntil_= 0;
#endif

  void renderNormal();
  void renderAlarm(uint32_t now);
  void fillSegment(const LedSegment& seg, const CRGB& color, uint8_t bri);
};
