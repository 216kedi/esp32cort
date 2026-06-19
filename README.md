# Elis Evleri — ESP32 Koridor Aydınlatma + Güvenlik Firmware

Apartman ("Elis Evleri") için **her kata ayrı bir ESP32**. Her ünite adreslenebilir
WS2812 LED şeridini hareket sensörleriyle (PIR) sürer. **Giriş katı** ünitesi ayrıca
duman/gaz dedektörlerini okur, sireni (röle) tetikler ve sakinlere **WhatsApp** bildirimi
gönderir.

> ⚠️ **Güvenlik notu:** Bu firmware **sertifikalı yangın panelinin yerine geçmez.** Panel +
> sireni hayati güvenlik sistemidir. ESP32 yalnızca panelin röle sinyalini okuyup üstüne *ek*
> WhatsApp bildirimi ve isteğe bağlı kendi rölesini ekler. WiFi/yazılım arızası sertifikalı
> sistemi asla engellememelidir.

## Davranış

### Giriş katı (`ground_floor` profili — 3 PIR)
| Sensör | Pin | Davranış |
|---|---|---|
| 1. PIR | GPIO 25 | **Dikdörtgen bölge** — iç içe halkalar **sırayla yanar** (sequential fill), sonra bekler, sonra söner |
| 2. PIR | GPIO 26 | **Koridor (1. baş)** — koridor bölgesi **fade-in** |
| 3. PIR | GPIO 27 | **Koridor (2. baş)** — koridor bölgesi **fade-in** |

Koridor: 2. veya 3. PIR'den biri hareket görürse fade-in; ikisi de boşalıp `HOLD_MS`
dolunca fade-out.

Duman (GPIO 18) **veya** gaz (GPIO 19) algılanınca → röle (GPIO 23) ON + tüm şerit kırmızı
flaş + tüm sakinlere WhatsApp.

### Üst katlar (`upper_floor` profili — 2 PIR)
Dikdörtgen bölge **yok**; tüm şerit tek koridor bölgesidir. 2 PIR (GPIO 25, 26) → fade-in /
hold / fade-out. Duman/gaz varsayılan olarak **kapalı** (o katta dedektör varsa
`platformio.ini`'de `-D ENABLE_SAFETY` satırını açın).

## Pin haritası (giriş katı)

| İşlev | Yön | GPIO |
|---|---|---|
| Adreslenebilir LED (WS2812) | Çıkış | 4 |
| Güvenlik rölesi (güç/siren) | Çıkış | 23 |
| 1. PIR (dikdörtgen) | Giriş | 25 |
| 2. PIR (koridor 1. baş) | Giriş | 26 |
| 3. PIR (koridor 2. baş) | Giriş | 27 |
| Gaz sensörü | Giriş | 19 |
| Duman dedektörü (panel rölesi) | Giriş | 18 |

## Derleme ve yükleme (PlatformIO)

```bash
pip install platformio          # bir kez
# Giriş katı:
pio run -e ground_floor
pio run -e ground_floor -t upload
# Üst kat:
pio run -e upper_floor -t upload
pio device monitor              # seri log (115200)
```

## Kurulum / ayarlar

1. **secrets dosyası:**
   ```bash
   cp include/secrets.example.h include/secrets.h
   ```
   `include/secrets.h` içine WiFi bilgilerini ve WhatsApp alıcılarını yazın. Bu dosya
   `.gitignore`'dadır, commit edilmez. (Dosya yoksa firmware örnek dosyayı placeholder
   değerlerle kullanır; WiFi/WhatsApp çalışmaz ama derlenir.)

2. **CallMeBot opt-in (her sakin için bir kez):** CallMeBot ücretsiz API yalnızca **opt-in
   yapmış** numaraya mesaj gönderir. Her sakin:
   - WhatsApp'tan **+34 644 51 95 23** numarasına `I allow callmebot to send me messages`
     yazar,
   - dönen mesajdaki **apikey**'i alır,
   - `secrets.h` içindeki `WA_RECIPIENTS` listesine `{ "+90...", "apikey" }` olarak eklenir.

   > CallMeBot kendi şartlarında "kritik/acil kullanım için değildir" der. Bu yüzden hayati
   > güvenlik = panel + siren; WhatsApp en-iyi-çaba **ek** bildirimdir. Daha yüksek güvenilirlik
   > gerekirse `notifier` modülü Twilio/Meta Cloud API'ye çevrilebilir (arayüz soyut tutuldu).

3. **`src/config.h` içindeki `// AYAR` değerleri** (şerit takılınca ölçülüp güncellenir):
   - `NUM_LEDS` — şeritteki toplam LED sayısı
   - `RECT_SEGMENTS[]` — dikdörtgen halkalarının `{başlangıç, uzunluk}` index aralıkları
   - `CORRIDOR_SEGMENT` — koridor LED aralığı
   - `RECT_FILL_OUTER_TO_INNER` — sıralı yanma yönü
   - `PIR_ACTIVE_LEVEL`, `SMOKE_ACTIVE_LEVEL`, `GAS_ACTIVE_LEVEL`, `RELAY_ACTIVE_LEVEL`
     — sensör/röle modüllerinizin mantık seviyesine göre **doğrulayın**
   - `HOLD_MS`, `FADE_STEP`, `SEQ_STEP_MS`, `NORMAL_BRIGHTNESS`, renk

## Gaz sensörü ↔ konvansiyonel yangın paneli

Teknik olarak bağlanabilir, ama duman/yangın zone'u ile **aynı bölgeye karıştırmayın**:
- Gaz dedektörünün **kuru kontak (röle) çıkışı** panelde **ayrı bir zone'a** uygun hat sonu
  direnciyle (EOL) bağlanır ve "GAZ" olarak etiketlenir.
- Yangın ve gaz müdahalesi farklıdır (gaz → vanayı kapat + havalandır). Çoğu standart yanıcı
  gaz algılamasının **ayrı sertifikalı sistem** olmasını ister.
- ESP32 zaten duman (18) ve gazı (19) **ayrı** okuduğu için, gaz dedektörü panele hiç
  bağlanmasa bile ESP32 sireni (röle 23) tetikleyip WhatsApp'ı **bağımsız** gönderir.

## Donanım / kablolama uyarıları

1. **Seviye/izolasyon:** Panel ve gaz modülü çıkışları ESP32'ye **3.3 V-güvenli** verilmeli
   (optocoupler veya kuru kontak + pull direnci). **12/24 V doğrudan GPIO'ya verilmez.**
2. **Röle kartı (GPIO 23):** Aktif seviyesini doğrulayın (çoğu kart active-LOW →
   `RELAY_ACTIVE_LEVEL = LOW`). Gerçek yük/siren için kontaktör kullanın.
3. **LED beslemesi:** WS2812 şeridi ayrı 5 V güç ister; GND ESP32 ile ortak olmalı. Veri
   hattına ~330 Ω seri direnç ve besleme girişine ~1000 µF kondansatör önerilir.
4. **Güç sürekliliği:** Yangın/elektrik kesintisinde bildirim için ESP32 + WiFi'ya küçük bir
   UPS/akü düşünülebilir (zorunlu değil).

## Mimari

Engellemesiz (`millis()` tabanlı, `loop()` içinde `delay()` yok) modüler tasarım:

| Modül | Sorumluluk |
|---|---|
| `src/config.h` | Tüm ayarlar (pin, LED, bölge, zaman, seviye, kat profili) |
| `src/led_effects.*` | `LedController` — sıralı yanma, fade in/out, alarm flaş (FSM) |
| `src/sensors.*` | PIR + duman/gaz debounce'lu seviye okuma |
| `src/safety.*` | `SafetyController` — röle + bildir-bir-kez + re-arm (yalnız `ENABLE_SAFETY`) |
| `src/notifier.*` | WiFi + CallMeBot çok-alıcı kuyruğu (yalnız `ENABLE_SAFETY`) |
| `src/main.cpp` | Modülleri kurar ve `update(now)` döngüsü |

```
loop(): sensors.update -> safety.update -> (PIR->bolge esleme) -> leds.update -> notifier.update
```

## Test (tam donanım olmadan)

- **Seri log:** her durum geçişi/bildirim `Serial @115200`'e yazılır.
- **Jumper/buton:** PIR/duman/gaz pinlerini 3.3 V/GND'ye butonla bağlayıp olay enjekte edin.
- **Küçük şerit:** 16–30 LED ile küçültülmüş `RECT_SEGMENTS[]` ile animasyonu görsel doğrulayın.
- **Wokwi:** ESP32 + WS2812 + butonlarla mantık donanımdan önce simüle edilebilir.
