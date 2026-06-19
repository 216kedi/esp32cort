// notifier.h
// WiFi baglantisi + CallMeBot uzerinden cok-aliciya WhatsApp bildirimi.
// Engellemesiz kuyruk: her tetiklemede mesaj kuyruga alinir, update() her
// dongude bir aliciya gonderir (aralarinda SEND_SPACING_MS bekleyerek).
#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_SAFETY

class Notifier {
public:
  void begin();
  void update(uint32_t now);
  bool enqueue(const String& msg);   // mesaji TUM alicilara gondermek uzere kuyruga alir
  bool wifiConnected() const;

private:
  static const uint8_t QCAP = 6;
  String   queue_[QCAP];
  uint8_t  qHead_  = 0;
  uint8_t  qCount_ = 0;
  uint8_t  recipientIdx_ = 0;        // on mesaj icin siradaki alici
  uint32_t lastSendMs_   = 0;
  uint32_t lastWifiTryMs_= 0;

  void   ensureWifi(uint32_t now);
  bool   sendOne(const char* phone, const char* apikey, const String& msg);
  String urlEncode(const String& s);
};

#endif // ENABLE_SAFETY
