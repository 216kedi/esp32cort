// =============================================================================
//  ELİS EVLERİ - Giriş Kat ESP32 Kontrolcüsü  ->  KULLANICI AYARLARI
// =============================================================================
//  Sistemi sahaya uyarlamak icin SADECE bu dosyayi duzenlemeniz yeterlidir.
//  (LED sayilari, bolge sinirlari, pinler, zamanlamalar, WiFi, telefonlar)
// =============================================================================
#pragma once
#include <FastLED.h>

// -----------------------------------------------------------------------------
//  Bina / konum bilgisi (WhatsApp mesajlarinda kullanilir)
// -----------------------------------------------------------------------------
#define BUILDING_NAME "ELIS EVLERI"
#define FLOOR_NAME    "Giris Kat"

// -----------------------------------------------------------------------------
//  WiFi (WhatsApp bildirimi icin gereklidir)
// -----------------------------------------------------------------------------
#define WIFI_SSID     "WIFI_ADINIZ"
#define WIFI_PASS     "WIFI_SIFRENIZ"

// -----------------------------------------------------------------------------
//  WhatsApp alicilari  (CallMeBot - https://www.callmebot.com/blog/free-api-whatsapp-messages/)
//  Her sakin kendi telefonundan CallMeBot numarasina aktivasyon mesaji gonderip
//  kendi apikey'ini alir. Numara formati: +90XXXXXXXXXX
//  Dizilerdeki sira ESLESMELIDIR (1. telefon -> 1. apikey).
// -----------------------------------------------------------------------------
const char* RESIDENT_PHONES[] = {
    "+905xxxxxxxxx",   // 1. sakin / yonetici
    "+905yyyyyyyyy",   // 2. sakin
};
const char* RESIDENT_APIKEYS[] = {
    "111111",          // 1. sakin apikey
    "222222",          // 2. sakin apikey
};
const int RESIDENT_COUNT = sizeof(RESIDENT_PHONES) / sizeof(RESIDENT_PHONES[0]);

// -----------------------------------------------------------------------------
//  GPIO PIN HARITASI  (proje tablosundaki degerler)
// -----------------------------------------------------------------------------
#define LED_PIN     4    // Adreslenebilir LED veri hatti (Durum Gostergesi - Cikis)
#define PIN_RELAY   23   // Guvenlik Rolesi (Guc/Siren Kontrolu - Cikis)
#define PIN_PIR1    25   // 1. PIR  -> giris: ic ice dikdortgenler sirayla yanar
#define PIN_PIR2    26   // 2. PIR  -> koridor: fade-in / fade-out
#define PIN_PIR3    27   // 3. PIR  -> 3. bolge (varsayilan: ikinci koridor/merdiven)
#define PIN_GAS     19   // Gaz Sensoru   (Lojik Kart Sinyali - Giris)
#define PIN_SMOKE   18   // Duman Dedektoru (Yangin Role Sinyali - Giris)

// -----------------------------------------------------------------------------
//  Adreslenebilir LED seridi
// -----------------------------------------------------------------------------
#define LED_TYPE        WS2812B   // serit tipiniz farkliysa degistirin (WS2811/SK6812..)
#define COLOR_ORDER     GRB
#define NUM_LEDS        318       // seritteki TOPLAM LED sayisi
#define MAX_BRIGHTNESS  200       // 0-255 genel parlaklik (akim/guc siniri icin)

// -----------------------------------------------------------------------------
//  BOLGE HARITASI  (tek veri hatti uzerindeki LED'lerin mantiksal bolunmesi)
//  Zone = { baslangic_index, led_adedi }
// -----------------------------------------------------------------------------
struct Zone { uint16_t start; uint16_t count; };

// 1) Ic ice gecmis dikdortgenler (giris tavani) - DIStan ICE sirali.
//    PIR1 algilayinca sirayla yanar. Sirayi degistirmek icin dizinin
//    elemanlarinin yerini degistirin.
const Zone RECTS[] = {
    {   0, 60 },   // dikdortgen 1 (en dis)
    {  60, 48 },   // dikdortgen 2
    { 108, 36 },   // dikdortgen 3
    { 144, 24 },   // dikdortgen 4 (en ic)
};
const int NUM_RECTS = sizeof(RECTS) / sizeof(RECTS[0]);

// 2) Koridor seridi - PIR2 ile fade-in/fade-out
const Zone CORRIDOR_ZONE = { 168, 90 };

// 3) 3. bolge (ikinci koridor / merdiven sahanligi) - PIR3 ile fade-in/fade-out
const Zone ZONE3 = { 258, 60 };

// -----------------------------------------------------------------------------
//  Renkler
// -----------------------------------------------------------------------------
const CRGB COLOR_NORMAL = CRGB(255, 150, 70);  // sicak beyaz (normal aydinlatma)
const CRGB COLOR_ALARM  = CRGB(255,   0,  0);  // alarm: kirmizi

// -----------------------------------------------------------------------------
//  Parlaklik seviyeleri (0-255, bolge bazinda)
// -----------------------------------------------------------------------------
#define RECT_ON_BRI      255
#define CORRIDOR_ON_BRI  220
#define ZONE3_ON_BRI     220

// -----------------------------------------------------------------------------
//  Zamanlamalar
// -----------------------------------------------------------------------------
#define MOTION_HOLD_MS     30000  // hareket bittikten sonra isiklarin acik kalma suresi
#define RECT_STEP_MS         220  // dikdortgenler arasi gecis suresi (sirali yanma hizi)
#define RECT_FADE_STEP        12  // her karede dikdortgen parlaklik degisimi (yumusaklik)
#define CORRIDOR_FADE_STEP     6  // koridor fade hizi (kucuk=yavas/yumusak)
#define FRAME_MS              16  // ~60 FPS render araligi
#define ALARM_BLINK_MS       350  // alarm kirmizi yanip sonme periyodu
#define SENSOR_DEBOUNCE_MS   250  // duman/gaz sinyali kararlilik suresi (yanlis tetik onleme)

// -----------------------------------------------------------------------------
//  Giris/cikis sinyal seviyeleri  (sahadaki kablaja gore ayarlayin)
// -----------------------------------------------------------------------------
#define PIR_INPUT_MODE     INPUT        // PIR cikisi genelde aktif-HIGH surer
#define PIR_ACTIVE_STATE   HIGH

#define SMOKE_INPUT_MODE   INPUT_PULLUP // role kuru kontagi GND'ye cekiyorsa pull-up
#define SMOKE_ACTIVE_STATE LOW

#define GAS_INPUT_MODE     INPUT_PULLUP // gaz karti kontagi GND'ye cekiyorsa pull-up
#define GAS_ACTIVE_STATE   LOW

#define RELAY_ACTIVE_STATE HIGH         // role modulu aktif-LOW ise LOW yapin
