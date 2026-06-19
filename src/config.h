// config.h
// -----------------------------------------------------------------------------
// Tum ayarlanabilir degerlerin tek kaynagi: pinler, LED sayisi/bolgeleri,
// zaman asimlari, aktif-seviye bayraklari ve kat profili secimi.
// "// AYAR" ile isaretli satirlar saha kurulumunda degistirilmelidir.
// -----------------------------------------------------------------------------
#pragma once
#include <Arduino.h>

// ============================ KAT PROFILI ====================================
// platformio.ini ortamlari -D FLOOR_GROUND veya -D FLOOR_UPPER tanimlar.
// Hicbiri tanimli degilse (or. Arduino IDE) giris kati varsayilir.
#if !defined(FLOOR_GROUND) && !defined(FLOOR_UPPER)
  #define FLOOR_GROUND
#endif

// Guvenlik alt sistemi (duman/gaz + role + WhatsApp) giris katinda her zaman aktif.
// Ust katta yalnizca o katta dedektor varsa build_flags ile -D ENABLE_SAFETY eklenir.
#if defined(FLOOR_GROUND) && !defined(ENABLE_SAFETY)
  #define ENABLE_SAFETY
#endif

#if defined(FLOOR_GROUND)
  #define FLOOR_NAME "Giris Kat"
#else
  #define FLOOR_NAME "Ust Kat"
#endif

// =============================== PINLER ======================================
constexpr uint8_t LED_PIN   = 4;   // Adreslenebilir LED (WS2812) veri hatti
constexpr uint8_t RELAY_PIN = 23;  // Guvenlik rolesi (siren/guc)
constexpr uint8_t PIR1_PIN  = 25;  // giris: dikdortgen bolge   | ust: koridor A
constexpr uint8_t PIR2_PIN  = 26;  // koridor 1. bas            | ust: koridor B
constexpr uint8_t PIR3_PIN  = 27;  // koridor 2. bas            | ust: kullanilmaz
constexpr uint8_t GAS_PIN   = 19;  // Gaz sensoru (lojik sinyal)
constexpr uint8_t SMOKE_PIN = 18;  // Duman / yangin paneli role sinyali

// ============================= LED SERIDI ====================================
#define LED_CHIPSET     WS2812B    // AYAR: serit cipseti (WS2812B / WS2811 ...)
#define LED_COLOR_ORDER GRB        // AYAR: renk sirasi (cogu WS2812B = GRB)

constexpr uint16_t NUM_LEDS = 120; // AYAR: seritteki TOPLAM LED sayisi

// Normal (alarm disi) aydinlatma rengi/parlakligi
constexpr uint8_t NORMAL_R = 255;          // AYAR: sicak beyaz
constexpr uint8_t NORMAL_G = 200;
constexpr uint8_t NORMAL_B = 120;
constexpr uint8_t NORMAL_BRIGHTNESS = 200; // AYAR: 0-255 azami parlaklik

struct LedSegment { uint16_t start; uint16_t length; };  // [start, start+length)

// --------------------------- Bolge yerlesimi ---------------------------------
// Serit tek veri hattinda ama mantiksal bolgelere ayrilir. Bolgeler index
// araliklariyla tanimlanir; degerler serit takilinca olculup guncellenir.
#if defined(FLOOR_GROUND)
  // Dikdortgen bolge: ic ice halkalar, DISTAN ICE sirayla (foto 2). AYAR.
  constexpr LedSegment RECT_SEGMENTS[] = { {0, 30}, {30, 22}, {52, 16}, {68, 12} };
  constexpr uint8_t    RECT_SEGMENT_COUNT = sizeof(RECT_SEGMENTS) / sizeof(RECT_SEGMENTS[0]);
  constexpr bool       RECT_FILL_OUTER_TO_INNER = true;   // AYAR: dis->ic / ic->dis
  // Koridor bolgesi: dikdortgenden sonraki kalan LED'ler. AYAR.
  constexpr LedSegment CORRIDOR_SEGMENT = { 80, 40 };
#else  // FLOOR_UPPER : dikdortgen yok, tum serit koridor
  constexpr LedSegment CORRIDOR_SEGMENT = { 0, NUM_LEDS };
#endif

// =========================== ZAMANLAMA / EFEKT ===============================
constexpr uint16_t FRAME_INTERVAL_MS = 16;     // ~60 FPS (LED kare araligi)
constexpr uint8_t  FADE_STEP         = 4;      // AYAR: fade hizi (parlaklik/kare)
constexpr uint16_t SEQ_STEP_MS       = 180;    // AYAR: dikdortgenler arasi bekleme
constexpr uint32_t HOLD_MS           = 30000;  // AYAR: hareket sonrasi acik kalma (ms)
constexpr uint16_t ALARM_FLASH_MS    = 250;    // alarm kirmizi flas periyodu

// ============================ PIR SENSORLERI =================================
constexpr uint8_t  PIR_ACTIVE_LEVEL = HIGH;    // AYAR: HC-SR501 genelde HIGH
constexpr uint16_t PIR_DEBOUNCE_MS  = 50;      // elektriksel gurultu filtresi

#if defined(FLOOR_GROUND)
  constexpr uint8_t PIR_COUNT = 3;
  constexpr uint8_t PIR_PINS[PIR_COUNT] = { PIR1_PIN, PIR2_PIN, PIR3_PIN };
#else
  constexpr uint8_t PIR_COUNT = 2;
  constexpr uint8_t PIR_PINS[PIR_COUNT] = { PIR1_PIN, PIR2_PIN };
#endif

// ====================== GUVENLIK (duman/gaz/role) ============================
// Bu sabitler her zaman tanimli (zararsiz); yalnizca ENABLE_SAFETY ile kullanilir.
constexpr uint8_t  SMOKE_ACTIVE_LEVEL = HIGH;   // AYAR: panel rolesi NO/NC'ye gore
constexpr uint8_t  GAS_ACTIVE_LEVEL   = HIGH;   // AYAR: gaz modulu cikis mantigi
constexpr uint16_t HAZARD_DEBOUNCE_MS = 200;    // tehlike girisi debounce
constexpr uint8_t  RELAY_ACTIVE_LEVEL = LOW;    // AYAR: cogu role karti active-LOW
constexpr uint32_t ALARM_REARM_MS     = 60000;  // kosul temizlendikten sonra re-arm (ms)

// =========================== AG / BILDIRIM ===================================
constexpr uint32_t WIFI_RETRY_MS    = 10000;    // WiFi yeniden baglanma araligi
constexpr uint32_t SEND_SPACING_MS  = 2500;     // alicilar arasi gonderim araligi (rate limit)
