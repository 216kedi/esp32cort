// led_effects.cpp
#include "led_effects.h"

static const CRGB kNormalColor = CRGB(NORMAL_R, NORMAL_G, NORMAL_B);

void LedController::begin() {
  FastLED.addLeds<LED_CHIPSET, LED_PIN, LED_COLOR_ORDER>(leds_, NUM_LEDS);
  FastLED.setBrightness(255);        // parlaklik bolge bazinda nscale8 ile uygulanir
  fill_solid(leds_, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void LedController::fillSegment(const LedSegment& seg, const CRGB& color, uint8_t bri) {
  CRGB c = color;
  c.nscale8_video(bri);
  for (uint16_t i = 0; i < seg.length; i++) {
    uint16_t idx = seg.start + i;
    if (idx < NUM_LEDS) leds_[idx] = c;
  }
}

#if defined(FLOOR_GROUND)
void LedController::motionRect(uint32_t now) {
  if (alarmActive_) return;
  rectHoldUntil_ = now + HOLD_MS;
  if (rectState_ == RECT_OFF) {
    rectState_  = RECT_SEQ_FILL;     // bastan sirayla yanma
    rectCursor_ = 0;
    rectBri_    = NORMAL_BRIGHTNESS;
    rectStepMs_ = now;
  } else if (rectState_ == RECT_FADE_OUT) {
    rectState_ = RECT_HOLD;          // halkalar zaten acilmisti, tekrar tut
    rectBri_   = NORMAL_BRIGHTNESS;
  }
}
#endif

void LedController::motionCorridor(uint32_t now) {
  if (alarmActive_) return;
  corHoldUntil_ = now + HOLD_MS;
  if (corState_ == COR_OFF || corState_ == COR_FADE_OUT) {
    corState_ = COR_FADE_IN;         // mevcut parlaklik korunur, yukari rampa
  }
}

void LedController::alarmFlash() {
  alarmActive_ = true;
  flashOn_     = true;
  lastFlashMs_ = 0;                  // ilk karede hemen flas
}

void LedController::clearAlarm() {
  alarmActive_  = false;
  corState_     = COR_OFF;
  corBri_       = 0;
#if defined(FLOOR_GROUND)
  rectState_    = RECT_OFF;
  rectBri_      = 0;
  rectCursor_   = 0;
#endif
  fill_solid(leds_, NUM_LEDS, CRGB::Black);
}

void LedController::renderAlarm(uint32_t now) {
  if (now - lastFlashMs_ >= ALARM_FLASH_MS) {
    lastFlashMs_ = now;
    flashOn_ = !flashOn_;
  }
  fill_solid(leds_, NUM_LEDS, flashOn_ ? CRGB::Red : CRGB::Black);
}

void LedController::renderNormal() {
  fill_solid(leds_, NUM_LEDS, CRGB::Black);

#if defined(FLOOR_GROUND)
  // Acilmis halka sayisi: SEQ_FILL'de imlec kadar, HOLD/FADE_OUT'ta hepsi.
  uint8_t revealed = 0;
  if (rectState_ == RECT_SEQ_FILL)        revealed = rectCursor_;
  else if (rectState_ != RECT_OFF)        revealed = RECT_SEGMENT_COUNT;
  for (uint8_t step = 0; step < revealed; step++) {
    uint8_t idx = RECT_FILL_OUTER_TO_INNER ? step : (RECT_SEGMENT_COUNT - 1 - step);
    fillSegment(RECT_SEGMENTS[idx], kNormalColor, rectBri_);
  }
#endif

  if (corState_ != COR_OFF) {
    fillSegment(CORRIDOR_SEGMENT, kNormalColor, corBri_);
  }
}

void LedController::update(uint32_t now) {
  if (now - lastFrameMs_ < FRAME_INTERVAL_MS) return;   // kare hizina sinirla
  lastFrameMs_ = now;

  if (alarmActive_) {
    renderAlarm(now);
    FastLED.show();
    return;
  }

#if defined(FLOOR_GROUND)
  switch (rectState_) {
    case RECT_SEQ_FILL:
      if (now - rectStepMs_ >= SEQ_STEP_MS) {
        rectStepMs_ = now;
        rectCursor_++;
        if (rectCursor_ >= RECT_SEGMENT_COUNT) {
          rectState_     = RECT_HOLD;
          rectHoldUntil_ = now + HOLD_MS;
        }
      }
      break;
    case RECT_HOLD:
      if ((int32_t)(now - rectHoldUntil_) >= 0) rectState_ = RECT_FADE_OUT;
      break;
    case RECT_FADE_OUT:
      if (rectBri_ <= FADE_STEP) { rectBri_ = 0; rectState_ = RECT_OFF; }
      else                       { rectBri_ -= FADE_STEP; }
      break;
    default: break;
  }
#endif

  switch (corState_) {
    case COR_FADE_IN:
      if (corBri_ + FADE_STEP >= NORMAL_BRIGHTNESS) { corBri_ = NORMAL_BRIGHTNESS; corState_ = COR_HOLD; }
      else                                          { corBri_ += FADE_STEP; }
      break;
    case COR_HOLD:
      if ((int32_t)(now - corHoldUntil_) >= 0) corState_ = COR_FADE_OUT;
      break;
    case COR_FADE_OUT:
      if (corBri_ <= FADE_STEP) { corBri_ = 0; corState_ = COR_OFF; }
      else                      { corBri_ -= FADE_STEP; }
      break;
    default: break;
  }

  renderNormal();
  FastLED.show();
}
