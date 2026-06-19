// notifier.cpp
#include "notifier.h"

#ifdef ENABLE_SAFETY

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ctype.h>

// secrets.h varsa onu, yoksa ornek dosyayi kullan (donanimsiz derleme icin).
#if defined(__has_include)
  #if __has_include("secrets.h")
    #include "secrets.h"
  #else
    #include "secrets.example.h"
    #warning "include/secrets.h bulunamadi - secrets.example.h (placeholder) kullaniliyor. WiFi/WhatsApp calismaz."
  #endif
#else
  #include "secrets.h"
#endif

void Notifier::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiTryMs_ = millis();
  // Not: setInsecure() kullanildigi icin TLS sertifika dogrulamasi (ve saat) gerekmez.
  // Pinli root CA'ya gecilirse burada configTime(...) ile SNTP saati ayarlanmalidir.

  if (String(WIFI_SSID).startsWith("DEGISTIR")) {
    Serial.println("[NOTIFIER] UYARI: secrets.h yapilandirilmamis (placeholder SSID).");
  }
  Serial.printf("[NOTIFIER] WiFi baglaniyor: %s (alici sayisi: %u)\n",
                WIFI_SSID, (unsigned)WA_RECIPIENT_COUNT);
}

bool Notifier::wifiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

void Notifier::ensureWifi(uint32_t now) {
  if (WiFi.status() == WL_CONNECTED) return;
  if ((now - lastWifiTryMs_) < WIFI_RETRY_MS) return;
  lastWifiTryMs_ = now;
  Serial.println("[NOTIFIER] WiFi yeniden baglanma deneniyor...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool Notifier::enqueue(const String& msg) {
  if (qCount_ >= QCAP) {
    Serial.println("[NOTIFIER] Kuyruk dolu, mesaj dusuruldu.");
    return false;
  }
  uint8_t tail = (qHead_ + qCount_) % QCAP;
  queue_[tail] = msg;
  qCount_++;
  Serial.printf("[NOTIFIER] Kuyruga alindi (%u): %s\n", qCount_, msg.c_str());
  return true;
}

void Notifier::update(uint32_t now) {
  ensureWifi(now);
  if (qCount_ == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if ((now - lastSendMs_) < SEND_SPACING_MS) return;

  if (WA_RECIPIENT_COUNT == 0) {     // alici yoksa mesaji dusur
    qHead_ = (qHead_ + 1) % QCAP;
    qCount_--;
    return;
  }

  const WaRecipient& r = WA_RECIPIENTS[recipientIdx_];
  sendOne(r.phone, r.apikey, queue_[qHead_]);
  lastSendMs_ = now;

  recipientIdx_++;
  if (recipientIdx_ >= WA_RECIPIENT_COUNT) {
    recipientIdx_ = 0;
    qHead_ = (qHead_ + 1) % QCAP;    // mesaj tum alicilara gitti, kuyruktan cikar
    qCount_--;
  }
}

bool Notifier::sendOne(const char* phone, const char* apikey, const String& msg) {
  WiFiClientSecure client;
  client.setInsecure();              // CallMeBot icin: sertifika dogrulamasi atlanir
  client.setTimeout(8);              // saniye

  HTTPClient https;
  https.setConnectTimeout(8000);     // ms

  String url = "https://api.callmebot.com/whatsapp.php?phone=" + urlEncode(phone) +
               "&text=" + urlEncode(msg) +
               "&apikey=" + urlEncode(apikey);

  if (!https.begin(client, url)) {
    Serial.println("[NOTIFIER] https.begin basarisiz");
    return false;
  }
  int code = https.GET();
  https.end();

  Serial.printf("[NOTIFIER] -> %s : HTTP %d\n", phone, code);
  return (code > 0 && code < 400);
}

String Notifier::urlEncode(const String& s) {
  String out;
  const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += c;
    } else {
      out += '%';
      out += hex[(c >> 4) & 0x0F];
      out += hex[c & 0x0F];
    }
  }
  return out;
}

#endif // ENABLE_SAFETY
