// =============================================================================
//  ELİS EVLERİ - Giriş Kat ESP32 Aydınlatma & Güvenlik Kontrolcüsü
// -----------------------------------------------------------------------------
//  Davranis ozeti:
//   * PIR1 (GPIO25): Hareket -> giris tavanindaki ic ice dikdortgenler SIRAYLA yanar,
//                    hareket bitince ters sirada soner.
//   * PIR2 (GPIO26): Koridor -> fade-in ile acilir, hareket bitince fade-out ile soner.
//   * PIR3 (GPIO27): 3. bolge (ikinci koridor/merdiven) -> fade-in / fade-out.
//   * Duman (GPIO18) veya Gaz (GPIO19) alarmi -> guvenlik rolesi (siren/GPIO23) aktif,
//                    LED seridi kirmizi yanip soner ve tum sakinlere WhatsApp gonderilir.
//   * Yangin/gaz alarmi her zaman aydinlatma animasyonlarinin ONUNDEDIR (oncelikli).
//
//  TUM ayarlar config.h dosyasindadir. Bu dosyayi normalde duzenlemeniz gerekmez.
// =============================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include "config.h"

// -----------------------------------------------------------------------------
//  Global durum
// -----------------------------------------------------------------------------
CRGB leds[NUM_LEDS];

// Bolge parlakliklari (0-255), her karede hedefe dogru yumusakca yaklasir
uint8_t  rectBri[NUM_RECTS];        // her dikdortgenin anlik parlakligi
int      rectsCommanded   = 0;       // su an "acik" komutu verilmis dikdortgen sayisi
uint32_t lastRectStepMs   = 0;

uint8_t  corridorBri = 0;            // koridor (PIR2) anlik parlaklik
uint8_t  zone3Bri    = 0;            // 3. bolge (PIR3) anlik parlaklik

// Hareket "tutma" zaman damgalari
uint32_t lastMotion1 = 0, lastMotion2 = 0, lastMotion3 = 0;

// Alarm durum/edge takibi
bool firePrev = false;
bool gasPrev  = false;

uint32_t lastFrameMs = 0;

// Duman/gaz sinyali icin debounce yapisi
struct Debounced { bool stable; bool lastRaw; uint32_t tChange; };
Debounced smokeDb = { false, false, 0 };
Debounced gasDb   = { false, false, 0 };

// -----------------------------------------------------------------------------
//  Yardimci fonksiyonlar
// -----------------------------------------------------------------------------

// Bir pini "aktif mi?" diye okur (aktif seviye config.h'ten gelir)
static inline bool readActive(uint8_t pin, int activeState) {
    return digitalRead(pin) == activeState;
}

// Debounce: sinyal SENSOR_DEBOUNCE_MS boyunca sabit kalirsa kararli kabul edilir
static bool debounceRead(Debounced &d, bool raw, uint32_t now) {
    if (raw != d.lastRaw) { d.lastRaw = raw; d.tChange = now; }
    if (now - d.tChange >= SENSOR_DEBOUNCE_MS) d.stable = raw;
    return d.stable;
}

// Parlakligi hedefe dogru "step" kadar yaklastirir (yumusak gecis)
static uint8_t approach(uint8_t cur, uint8_t target, uint8_t step) {
    if (cur < target) { int v = cur + step; return v > target ? target : (uint8_t)v; }
    if (cur > target) { int v = cur - step; return v < target ? target : (uint8_t)v; }
    return cur;
}

// Bir bolgeyi tek renk + parlaklikla doldurur
static void fillZone(const Zone &z, const CRGB &color, uint8_t bri) {
    CRGB c = color;
    c.nscale8_video(bri);
    for (uint16_t i = 0; i < z.count; i++) {
        uint16_t idx = z.start + i;
        if (idx < NUM_LEDS) leds[idx] = c;
    }
}

// URL kodlama (WhatsApp mesaj metni icin)
static String urlEncode(const String &s) {
    String out;
    const char *hex = "0123456789ABCDEF";
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += c;
        } else {
            out += '%';
            out += hex[(c >> 4) & 0xF];
            out += hex[c & 0xF];
        }
    }
    return out;
}

// -----------------------------------------------------------------------------
//  WiFi
// -----------------------------------------------------------------------------
static void wifiConnect() {
    if (WiFi.status() == WL_CONNECTED) return;
    Serial.printf("[WiFi] Baglaniliyor: %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
        delay(250);
        Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED)
        Serial.printf("[WiFi] Baglandi, IP: %s\n", WiFi.localIP().toString().c_str());
    else
        Serial.println("[WiFi] Baglanti basarisiz (alarm aninda tekrar denenecek).");
}

// Tek bir alICIya WhatsApp mesaji gonderir (CallMeBot)
static bool sendWhatsApp(const char *phone, const char *apikey, const String &msg) {
    if (WiFi.status() != WL_CONNECTED) { wifiConnect(); }
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClientSecure client;
    client.setInsecure();  // CallMeBot icin sertifika dogrulamasi yapilmaz
    HTTPClient https;

    String url = String("https://api.callmebot.com/whatsapp.php?phone=") + phone +
                 "&apikey=" + apikey + "&text=" + urlEncode(msg);

    if (!https.begin(client, url)) return false;
    int code = https.GET();
    https.end();

    Serial.printf("[WhatsApp] %s -> HTTP %d\n", phone, code);
    return code > 0 && code < 400;
}

// Tum sakinlere mesaj gonderir
static void notifyAllResidents(const String &msg) {
    Serial.printf("[Alarm] WhatsApp gonderiliyor: %s\n", msg.c_str());
    for (int i = 0; i < RESIDENT_COUNT; i++) {
        sendWhatsApp(RESIDENT_PHONES[i], RESIDENT_APIKEYS[i], msg);
        delay(150);  // alicilar arasi kisa bekleme
    }
}

// -----------------------------------------------------------------------------
//  Render: aydinlatma (alarm yokken)
// -----------------------------------------------------------------------------
static void renderLighting() {
    FastLED.clear();

    // Dikdortgenler (PIR1) - her biri kendi parlakligiyla cizilir
    for (int i = 0; i < NUM_RECTS; i++) {
        fillZone(RECTS[i], COLOR_NORMAL, rectBri[i]);
    }
    // Koridor (PIR2)
    fillZone(CORRIDOR_ZONE, COLOR_NORMAL, corridorBri);
    // 3. bolge (PIR3)
    fillZone(ZONE3, COLOR_NORMAL, zone3Bri);
}

// -----------------------------------------------------------------------------
//  Render: alarm (tum serit kirmizi yanip soner)
// -----------------------------------------------------------------------------
static void renderAlarm() {
    bool on = (millis() / ALARM_BLINK_MS) % 2 == 0;
    if (on) fill_solid(leds, NUM_LEDS, COLOR_ALARM);
    else    FastLED.clear();
}

// -----------------------------------------------------------------------------
//  setup()
// -----------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== ELIS EVLERI - Giris Kat Kontrolcusu ===");

    // Pin modlari
    pinMode(PIN_PIR1, PIR_INPUT_MODE);
    pinMode(PIN_PIR2, PIR_INPUT_MODE);
    pinMode(PIN_PIR3, PIR_INPUT_MODE);
    pinMode(PIN_SMOKE, SMOKE_INPUT_MODE);
    pinMode(PIN_GAS,   GAS_INPUT_MODE);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, !RELAY_ACTIVE_STATE);  // baslangicta role pasif

    // LED seridi
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(MAX_BRIGHTNESS);
    FastLED.clear(true);

    for (int i = 0; i < NUM_RECTS; i++) rectBri[i] = 0;

    wifiConnect();
    Serial.println("[Sistem] Hazir.");
}

// -----------------------------------------------------------------------------
//  loop()
// -----------------------------------------------------------------------------
void loop() {
    uint32_t now = millis();

    // --- 1) Guvenlik sensorleri (oncelikli) -------------------------------
    bool fireAlarm = debounceRead(smokeDb, readActive(PIN_SMOKE, SMOKE_ACTIVE_STATE), now);
    bool gasAlarm  = debounceRead(gasDb,   readActive(PIN_GAS,   GAS_ACTIVE_STATE),   now);
    bool alarmActive = fireAlarm || gasAlarm;

    // Alarm baslangic kenarinda (rising edge) WhatsApp gonder
    if (fireAlarm && !firePrev) {
        notifyAllResidents(String("🔥 ") + BUILDING_NAME + " - " + FLOOR_NAME +
                           ": DUMAN/YANGIN ALARMI algilandi! Lutfen binayi kontrol edin.");
    }
    if (gasAlarm && !gasPrev) {
        notifyAllResidents(String("⚠️ ") + BUILDING_NAME + " - " + FLOOR_NAME +
                           ": GAZ ALARMI algilandi! Gaz vanasini kapatip ortami havalandirin.");
    }
    firePrev = fireAlarm;
    gasPrev  = gasAlarm;

    // Guvenlik rolesi (siren/guc): alarm suresince aktif
    digitalWrite(PIN_RELAY, alarmActive ? RELAY_ACTIVE_STATE : !RELAY_ACTIVE_STATE);

    // --- 2) Hareket sensorleri --------------------------------------------
    if (readActive(PIN_PIR1, PIR_ACTIVE_STATE)) lastMotion1 = now;
    if (readActive(PIN_PIR2, PIR_ACTIVE_STATE)) lastMotion2 = now;
    if (readActive(PIN_PIR3, PIR_ACTIVE_STATE)) lastMotion3 = now;

    bool lit1 = (lastMotion1 != 0) && (now - lastMotion1 < MOTION_HOLD_MS);
    bool lit2 = (lastMotion2 != 0) && (now - lastMotion2 < MOTION_HOLD_MS);
    bool lit3 = (lastMotion3 != 0) && (now - lastMotion3 < MOTION_HOLD_MS);

    // --- 3) ~60 FPS animasyon + render ------------------------------------
    if (now - lastFrameMs >= FRAME_MS) {
        lastFrameMs = now;

        // PIR1: dikdortgenleri sirayla ac/kapat
        int rectTarget = lit1 ? NUM_RECTS : 0;
        if (now - lastRectStepMs >= RECT_STEP_MS) {
            lastRectStepMs = now;
            if (rectsCommanded < rectTarget) rectsCommanded++;       // disardan ice ac
            else if (rectsCommanded > rectTarget) rectsCommanded--;  // icerden disa kapat
        }
        for (int i = 0; i < NUM_RECTS; i++) {
            uint8_t target = (i < rectsCommanded) ? RECT_ON_BRI : 0;
            rectBri[i] = approach(rectBri[i], target, RECT_FADE_STEP);
        }

        // PIR2 / PIR3: fade-in / fade-out
        corridorBri = approach(corridorBri, lit2 ? CORRIDOR_ON_BRI : 0, CORRIDOR_FADE_STEP);
        zone3Bri    = approach(zone3Bri,    lit3 ? ZONE3_ON_BRI    : 0, CORRIDOR_FADE_STEP);

        // Render (alarm her seyin onunde)
        if (alarmActive) renderAlarm();
        else             renderLighting();

        FastLED.show();
    }
}
