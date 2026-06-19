// secrets.example.h
// -----------------------------------------------------------------------------
// Bu dosyayi include/secrets.h olarak KOPYALAYIN ve gercek degerlerle doldurun.
//     cp include/secrets.example.h include/secrets.h
// secrets.h .gitignore'dadir; ASLA commit edilmez.
//
// Eger secrets.h yoksa firmware bu ornek dosyayi (placeholder degerlerle) kullanir,
// boylece donanim olmadan da derlenebilir; ama WiFi/WhatsApp calismaz.
// -----------------------------------------------------------------------------
#pragma once

// ---- WiFi ----
#define WIFI_SSID      "DEGISTIR_WIFI_ADI"
#define WIFI_PASSWORD  "DEGISTIR_WIFI_SIFRESI"

// ---- WhatsApp (CallMeBot) ----
// Her sakin bir kez CallMeBot numarasina mesaj atip KENDI apikey'ini almalidir.
// Adimlar README.md icinde. Asagiya her sakin icin {telefon, apikey} ekleyin.
struct WaRecipient {
  const char* phone;    // uluslararasi format, or. "+905551112233"
  const char* apikey;   // o numaraya ozel CallMeBot anahtari
};

static const WaRecipient WA_RECIPIENTS[] = {
  { "+905551112233", "123456" },   // Daire 1
  { "+905554445566", "234567" },   // Daire 2
  // ... diger sakinler
};
static const size_t WA_RECIPIENT_COUNT =
    sizeof(WA_RECIPIENTS) / sizeof(WA_RECIPIENTS[0]);
