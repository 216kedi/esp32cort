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

## Ön koşullar

- **PlatformIO** (önerilen): VS Code + **PlatformIO IDE** eklentisi *veya* komut satırı —
  `python3 -m pip install platformio` (Python 3 gerekir).
- **USB-seri sürücüsü:** kartınızın çipine göre **CP2102** (Silicon Labs) veya **CH340**
  sürücüsünü kurun (Windows/macOS). Linux'ta genelde hazırdır.
- **Repoyu alın:**
  ```bash
  git clone -b claude/brave-wright-krcifb <repo-url>
  cd esp32cort
  ```

## Derleme ve yükleme (PlatformIO)

```bash
pip install platformio          # bir kez (VS Code eklentisi kullanıyorsanız gerekmez)
# Giriş katı:
pio run -e ground_floor
pio run -e ground_floor -t upload
# Üst kat:
pio run -e upper_floor -t upload
pio device monitor              # seri log (115200)
```

## Yükleme sorun giderme

- **Port:** `pio device list` ile portu görün; gerekirse `platformio.ini`'ye
  `upload_port = /dev/ttyUSB0` (veya `COMx`) ekleyin.
- **"Failed to connect to ESP32":** yükleme başlarken kartın **BOOT** tuşunu basılı tutun
  (bazı kartlar otomatik indirme moduna geçemez), bitince bırakın.
- **Linux izni:** `sudo usermod -aG dialout $USER` (sonra yeniden oturum açın).
- **Kart farklıysa:** `board = esp32dev` çoğu WROOM-32 için uygundur; değilse kartınıza göre
  değiştirin.

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

   **LED segment index'lerini ölçme:** Şeridi takıp ilk LED'i `0` kabul edin. Her dikdörtgen
   halkasının kaç LED olduğunu sayın; `RECT_SEGMENTS[]` her halka için `{kümülatif_başlangıç,
   uzunluk}` tutar. Örn. dış halka 30, sonraki 22, sonraki 16, en iç 12 LED ise:
   `{0,30},{30,22},{52,16},{68,12}`. Koridor = kalan aralık, örn. `{80, 40}`.

   **Aktif-seviyeyi belirleme:** Modül algıladığında çıkışını HIGH'a mı LOW'a mı çekiyor?
   Datasheet'e bakın veya multimetreyle ölçün; `*_ACTIVE_LEVEL` değerini buna göre ayarlayın
   (HC-SR501 PIR genelde HIGH; opto-izoleli röle kartları ve aşağıdaki optocoupler kurulumu
   genelde **LOW**).

## Gaz sensörü ↔ konvansiyonel yangın paneli

Teknik olarak bağlanabilir, ama duman/yangın zone'u ile **aynı bölgeye karıştırmayın**:
- Gaz dedektörünün **kuru kontak (röle) çıkışı** panelde **ayrı bir zone'a** uygun hat sonu
  direnciyle (EOL) bağlanır ve "GAZ" olarak etiketlenir.
- Yangın ve gaz müdahalesi farklıdır (gaz → vanayı kapat + havalandır). Çoğu standart yanıcı
  gaz algılamasının **ayrı sertifikalı sistem** olmasını ister.
- ESP32 zaten duman (18) ve gazı (19) **ayrı** okuduğu için, gaz dedektörü panele hiç
  bağlanmasa bile ESP32 sireni (röle 23) tetikleyip WhatsApp'ı **bağımsız** gönderir.

## Gerekli malzemeler (BOM)

| Parça | Adet (giriş katı) | Not |
|---|---|---|
| ESP32 geliştirme kartı (WROOM-32) | 1 / kat | `board = esp32dev` |
| WS2812B adreslenebilir LED şerit (5 V) | toplam `NUM_LEDS` | dikdörtgen + koridor |
| HC-SR501 PIR sensör | 3 (üst katta 2) | dijital çıkış |
| Opto-izoleli röle modülü (5 V) veya kontaktör | 1 | siren/güç anahtarlama |
| Gaz dedektör modülü (dijital DO çıkışlı) | 1 | — |
| Duman dedektörü / yangın paneli (kuru kontak rölesi) | 1 | mevcut panel |
| PC817 optocoupler + dirençler | panel/gaz arabirimi | aşağıdaki devre |
| 5 V güç kaynağı | 1 | akıma göre boyutlandır (aşağıda) |
| ~330 Ω direnç, ~1000 µF kondansatör | 1'er | LED veri/besleme koruması |
| Bağlantı kablosu, klemens | — | — |

## Güç beslemesi ve boyutlandırma

- WS2812B her LED tam beyazda **~60 mA** çeker → tepe akım ≈ `NUM_LEDS × 60 mA`.
  Örn. 120 LED ≈ **7.2 A @ 5 V**. Güç kaynağını tepe akıma göre (pay bırakarak) seçin;
  uygulamada renk/parlaklık daha düşükse ortalama akım daha azdır.
- **Ayrı bir 5 V kaynak** kullanın; şerit GND'si ile ESP32 GND'si **ortak** olmalı.
- ESP32'yi USB'den ya da 5 V kaynaktan **VIN** pinine vererek besleyin (3.3 V'a **değil**).
- Yangın/elektrik kesintisinde bildirim isteniyorsa ESP32 + WiFi için küçük UPS/akü
  düşünülebilir (zorunlu değil).

## Kablolama (pin-pin)

| Bileşen | Bileşen pini | Bağlanır |
|---|---|---|
| WS2812B şerit | +5 V | Güç kaynağı +5 V (girişe ~1000 µF) |
| | GND | Ortak GND (kaynak + ESP32) |
| | DIN | ESP32 **GPIO 4** (araya ~330 Ω seri direnç) |
| HC-SR501 PIR ×N | VCC | 5 V |
| | GND | Ortak GND |
| | OUT | ESP32 **GPIO 25 / 26 / 27** (çıkış 3.3 V) |
| Röle modülü | VCC / GND | 5 V / ortak GND |
| | IN | ESP32 **GPIO 23** |
| | COM/NO | siren veya kontaktör bobini (şebeke yükü → kontaktör) |
| Gaz modülü | VCC / GND | 5 V / ortak GND |
| | DO (dijital) | ESP32 **GPIO 19** (DO 5 V ise optocoupler/bölücü ile 3.3 V'a indirin) |
| Duman/panel rölesi | kuru kontak | **GPIO 18** — optocoupler üzerinden (aşağıda) |

> ⚠️ **12/24 V panel sinyalini doğrudan GPIO'ya vermeyin.** ESP32 girişleri 3.3 V'dur.

### Optocoupler arabirimi (12/24 V panel → 3.3 V GPIO 18)

PC817 ile galvanik izolasyon:
- **Giriş tarafı:** panel sinyali (+) → seri direnç → PC817 LED anot; panel (−) → PC817 LED katot.
  Seri direnç ≈ **1 kΩ (12 V)** / **2.2 kΩ (24 V)** (LED akımı ~10 mA).
- **Çıkış tarafı:** PC817 kollektör → **GPIO 18** ve aynı düğümden 3.3 V'a **10 kΩ pull-up**;
  emitör → GND.
- Bu kurulumda panel aktifken GPIO **LOW** olur → `SMOKE_ACTIVE_LEVEL = LOW` ayarlayın.
- **Voltajsız kuru kontak** ise: kontağı GPIO 18 ile GND arasına bağlayın ve 3.3 V'a **harici
  10 kΩ pull-up** ekleyin (firmware dahili pull-up açmaz, `INPUT` kullanır) → yine aktif-LOW.
  Gaz modülü kuru kontaksa GPIO 19 için de aynısı geçerlidir.

### Güvenlik / izolasyon hatırlatmaları
- Sertifikalı yangın paneli + sireni **hayati sistemdir**; ESP32 yalnızca **ek** bildirim katmanı.
- Röle kartı çoğunlukla **active-LOW** → `RELAY_ACTIVE_LEVEL = LOW`. Şebeke yükü için kontaktör.
- Tüm modüller **ortak GND** paylaşmalı; izolasyon gereken yerde optocoupler kullanın.

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
