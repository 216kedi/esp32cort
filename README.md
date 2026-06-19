# ELİS EVLERİ — Giriş Kat ESP32 Aydınlatma & Güvenlik Kontrolcüsü

ESP32 tabanlı; adreslenebilir LED ile **hareketle tetiklenen dekoratif aydınlatma** ve
**duman/gaz alarmında WhatsApp bildirimi** yapan kontrol yazılımı.

> ⚠️ **Önemli — can güvenliği uyarısı:** Bu sistem mevcut **sertifikalı yangın/gaz alarm
> sistemine EK (akıllı aydınlatma + bilgilendirme) olarak** tasarlanmıştır. Sertifikalı
> konvansiyonel yangın paneli ve gaz dedeksiyon sisteminin **yerine geçmez**. Yangın/gaz
> dedeksiyonu yerel mevzuata (TR: *Binaların Yangından Korunması Hakkında Yönetmelik* ve
> ilgili gaz dedeksiyon zorunlulukları) ve sertifikalı ekipmana uygun olmalıdır.

---

## 1. Pin Haritası

| İşlev | Yön | GPIO |
|---|---|---|
| Adreslenebilir LED (Durum Göstergesi) | Çıkış | **GPIO 4** |
| Güvenlik Rölesi (Güç/Siren Kontrolü) | Çıkış | **GPIO 23** |
| 1. PIR Sensörü (Hareket) | Giriş | **GPIO 25** |
| 2. PIR Sensörü (Hareket) | Giriş | **GPIO 26** |
| 3. PIR Sensörü (Hareket) | Giriş | **GPIO 27** |
| Gaz Sensörü (Lojik Kart Sinyali) | Giriş | **GPIO 19** |
| Duman Dedektörü (Yangın Röle Sinyali) | Giriş | **GPIO 18** |

> Ortak GND şart: PIR'ler, gaz/duman röle kontakları, röle modülü ve LED besleme GND'si
> ESP32 GND'si ile **ortak** olmalıdır. LED şeridi ayrı 5V güçten beslenmeli; sadece veri
> hattı (GPIO4) ve GND ESP32'ye gelmelidir.

---

## 2. Çalışma Mantığı

- **PIR 1 → Giriş tavanı (iç içe dikdörtgenler):** Hareket algılanınca iç içe geçmiş
  dikdörtgenler **sırayla (dıştan içe) yanar**; hareket bitince `MOTION_HOLD_MS` süresi
  sonunda ters sırada söner.
- **PIR 2 → Koridor:** Hareketle **fade-in** açılır, hareket bitince **fade-out** söner.
- **PIR 3 → 3. bölge** (ikinci koridor / merdiven sahanlığı): PIR2 gibi fade-in/fade-out.
- **Duman (GPIO18) veya Gaz (GPIO19) alarmı:**
  1. Güvenlik rölesi (GPIO23) aktif olur (siren / güç kontrolü),
  2. Tüm LED şeridi **kırmızı yanıp söner**,
  3. Tüm sakinlere **WhatsApp mesajı** gönderilir.
  Alarm, aydınlatma animasyonlarının **önündedir** (öncelikli).

Tek bir adreslenebilir veri hattı (GPIO4) üzerindeki LED'ler `config.h` içinde mantıksal
bölgelere ayrılır: `RECTS[]` (dikdörtgenler), `CORRIDOR_ZONE`, `ZONE3`.

---

## 3. "Gaz sensörünü de konvansiyonel panele bağlayabilir miyiz?" — Kısa cevap: **Önermiyoruz**

**Neden:** Konvansiyonel yangın paneli; duman/ısı dedektörü ve yangın butonu gibi
**yangın başlatma cihazları** için (EOL sonlandırma dirençleriyle, denetimli bölge
mantığıyla) tasarlanmıştır. Yanıcı gaz / LPG / doğalgaz dedektörü farklı bir kategoridir:

- Gaz dedektörünün çıkışı panelin beklediği dedektör imzasına/denetimine uymaz; **arıza
  (fault)** veya hatalı alarma yol açabilir.
- Yangın ile gazın **doğru tepkileri farklıdır:** yangında siren + tahliye; yanıcı gazda
  ise **gaz vanasını kapatmak + havalandırmak** ve kıvılcım riski olan yükleri tetiklememek
  gerekir. İkisini aynı bölgede birleştirmek yanlış müdahaleye yol açabilir.
- Mevzuat çoğu durumda gaz dedeksiyonunu **ayrı, kendine ait bir sistem** (gaz dedeksiyon
  paneli + selenoid/küresel vana) olarak ister.

**Önerilen çözüm (bu projeyle uyumlu):**

1. **Duman dedektörü** mevcut konvansiyonel yangın panelinde kalsın. Panelin **yangın röle
   çıkışı** ESP32 **GPIO18**'e (kuru kontak) gelsin — ESP32 yangını "görüp" WhatsApp atsın.
2. **Gaz sensörü** ayrı kalsın; lojik/röle çıkışı ESP32 **GPIO19**'a gelsin (tablodaki
   "Lojik Kart Sinyali" zaten bu). ESP32 gaz alarmında WhatsApp atar, sireni çalar ve
   istenirse **gaz selenoid vanasını** kapatabilir (GPIO23 rölesi veya ikinci bir röle).
3. **Mutlaka panele sinyal vermek gerekiyorsa:** doğrudan dedeksiyon bölgesine değil, panel
   üreticisinin onayladığı **arabirim/bölge izleme (input/monitor) modülü** üzerinden,
   **"teknik alarm / yangın dışı"** giriş olarak tanımlanmalı ve yerel mevzuata göre
   doğrulanmalıdır.

Özet: **Gaz dedektörünü yangın panelinin dedeksiyon loop'una koymayın.** Bu firmware
zaten her iki sinyali ayrı ayrı toplayıp tek noktadan (WhatsApp + siren) bildirir.

---

## 4. WhatsApp Bildirimi (CallMeBot)

Bu firmware varsayılan olarak ücretsiz **CallMeBot** servisini kullanır.

Her sakin için kurulum (bir kez):
1. CallMeBot WhatsApp numarasını rehbere ekleyin (servisin sayfasındaki güncel numara).
2. O numaraya `I allow callmebot to send me messages` yazın.
3. Dönen **apikey**'i `src/config.h` içindeki `RESIDENT_PHONES[]` ve `RESIDENT_APIKEYS[]`
   dizilerine ekleyin (sıralar eşleşmeli).

```cpp
const char* RESIDENT_PHONES[]  = { "+9053xxxxxxxx", "+9054xxxxxxxx" };
const char* RESIDENT_APIKEYS[] = { "111111",        "222222"        };
```

> **Çok sayıda sakin** varsa CallMeBot pratik olmayabilir. Üretim için **Twilio WhatsApp
> API** veya **WhatsApp Business API** önerilir; ya da tek bir "bina yöneticisi" numarasına
> gönderip yönetici sakinlere iletir. Gönderim, `main.cpp` içindeki `sendWhatsApp()`
> fonksiyonuyla soyutlanmıştır — sağlayıcı değişimi tek fonksiyonda yapılır.

---

## 5. Kurulum & Yükleme

### PlatformIO (önerilen)
```bash
pio run                 # derle
pio run -t upload       # ESP32'ye yükle
pio device monitor      # seri monitör (115200)
```

### Arduino IDE
1. **ESP32** kart paketini ve **FastLED** kütüphanesini kurun.
2. `src/main.cpp` → `elis_giris_kat.ino` olarak kopyalayın, `config.h`'yi yanına koyun.
3. Kart: *ESP32 Dev Module* seçip yükleyin.

---

## 6. Sahaya Uyarlama (sık ayarlar — hepsi `src/config.h`)

| Ayar | Açıklama |
|---|---|
| `NUM_LEDS` | Şeritteki toplam LED sayısı |
| `RECTS[]` | Her dikdörtgenin `{başlangıç, adet}` aralığı (sıra = yanma sırası) |
| `CORRIDOR_ZONE`, `ZONE3` | Koridor ve 3. bölge LED aralıkları |
| `MOTION_HOLD_MS` | Hareket sonrası ışıkların açık kalma süresi |
| `RECT_STEP_MS` | Dikdörtgenler arası geçiş hızı |
| `CORRIDOR_FADE_STEP` | Fade-in/out yumuşaklığı (küçük = yavaş) |
| `COLOR_NORMAL` | Normal aydınlatma rengi (varsayılan sıcak beyaz) |
| `*_ACTIVE_STATE`, `*_INPUT_MODE` | Sensör/röle sinyal seviyeleri (kablajınıza göre) |

Sinyal seviyeleri sahadaki donanıma göre kontrol edilmeli: PIR'ler genelde **aktif-HIGH**;
röle kuru kontakları genelde **GND'ye çekme (aktif-LOW + pull-up)**. Röle modülünüz
**aktif-LOW** ise `RELAY_ACTIVE_STATE LOW` yapın.

---

## 7. Diğer Katlar (LED için 2 PIR)

Diğer katlarda dekoratif giriş aydınlatması yoktur; sadece **2 PIR ile koridor LED**
istenmektedir. Aynı firmware kullanılabilir: `config.h` içinde `RECTS[]`'i boş bırakıp
(`NUM_RECTS = 0`) PIR2 ve PIR3'ü iki koridor bölgesine atayın; gaz/duman bağlı değilse o
girişler pasif kalır. İstenirse ayrı, sadeleştirilmiş bir kat-firmware'i de eklenebilir.
