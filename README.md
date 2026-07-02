# ESP32-S3 Kontrollü Buck Converter

ESP32-S3 ile PWM kontrollü, çıkış gerilimi ölçülebilen ve yerel web arayüzü üzerinden yönetilebilen senkron buck converter projesidir.

Sistem, yaklaşık 12 V DC giriş gerilimini ayarlanabilir duty cycle kullanarak yaklaşık 0–5 V aralığına düşürmektedir. ESP32-S3 tarafından üretilen PWM sinyali IR2184 gate driver devresine uygulanmakta, çıkış gerilimi ise kart üzerindeki gerilim ölçüm devresi üzerinden ESP32 ADC girişine aktarılmaktadır.

Proje kapsamında yalnızca kontrol yazılımı geliştirilmemiş; kart üzerindeki eksik ve hatalı donanım noktaları da ölçüm yapılarak tespit edilmiş ve düzeltilmiştir.

---

## Projenin Amacı

Bu projenin temel amaçları şunlardır:

* ESP32-S3 ile sabit frekanslı PWM üretmek
* Senkron buck converter güç katını kontrol etmek
* Duty cycle değişiminin çıkış gerilimine etkisini incelemek
* Çıkış gerilimini ESP32 ADC birimi ile ölçmek
* Ölçüm sistemini multimetre sonuçlarına göre kalibre etmek
* Yerel Wi-Fi ağı üzerinden çalışan web kontrol paneli oluşturmak
* Kart üzerindeki donanımsal problemleri tespit etmek ve çözmek
* Güvenli çalışma için yazılımsal sınırlar oluşturmak

---

## Temel Sistem Özellikleri

| Özellik                   |           Değer |
| ------------------------- | --------------: |
| Mikrodenetleyici          |        ESP32-S3 |
| Giriş gerilimi            |         12 V DC |
| Hedef çıkış gerilimi      | Yaklaşık 5 V DC |
| Gate driver               |          IR2184 |
| PWM frekansı              |          20 kHz |
| PWM çözünürlüğü           |          10 bit |
| Maksimum yazılımsal duty  |             %45 |
| Test duty değeri          |             %42 |
| Ölçülen çıkış gerilimi    |         4.987 V |
| Ölçülen VSENSE_OUT        |         1.557 V |
| ADC kalibrasyon katsayısı |           3.175 |
| Gate driver VCC gerilimi  |         11.78 V |
| Test yükü                 |           100 Ω |

---

## Kullanılan Donanımlar

* ESP32-S3 geliştirme kartı
* Senkron buck converter PCB
* IR2184 gate driver
* İki adet güç MOSFET’i
* Çıkış endüktörü
* Giriş ve çıkış kapasitörleri
* LMV321 veya benzeri gerilim buffer op-amp katı
* INA181 akım algılama yükselteci
* 10 mΩ şönt direnç
* 1 MΩ gerilim bölücü direnci
* 453 kΩ gerilim bölücü direnci
* 0 Ω bağlantı direnci veya lehim köprüsü
* 100 Ω test yükü
* Akım limitli laboratuvar güç kaynağı
* Multimetre
* Jumper kablolar

---

## ESP32-S3 Bağlantıları

| ESP32-S3 pini | Buck kart bağlantısı | Açıklama                              |
| ------------- | -------------------- | ------------------------------------- |
| GPIO15        | IN                   | 1 kΩ seri direnç üzerinden PWM girişi |
| GPIO4         | VSENSE_OUT           | 1 kΩ seri direnç üzerinden ADC girişi |
| 3V3           | 3V3                  | Gerilim ölçüm devresinin beslemesi    |
| GND           | GND                  | Ortak toprak bağlantısı               |

Mevcut test sürümünde aşağıdaki hatlar ESP32-S3’e bağlanmamıştır:

* SD
* ISENSE_OUT
* VS

---

## Çalışma Prensibi

ESP32-S3, GPIO15 pini üzerinden 20 kHz frekansında PWM üretmektedir. PWM sinyali IR2184 gate driver girişine uygulanır.

IR2184, high-side ve low-side MOSFET’leri tamamlayıcı şekilde sürerek senkron buck converter güç katını çalıştırır.

Buck converter çıkışındaki gerilim, R3 ve R4 dirençlerinden oluşan gerilim bölücü üzerinden düşürülür. Bu gerilim U2 op-amp katı üzerinden buffer edilerek VSENSE_OUT hattına aktarılır.

VSENSE_OUT sinyali ESP32-S3 GPIO4 ADC girişinde okunur. Okunan değer kalibrasyon katsayısı ile çarpılarak gerçek çıkış gerilimi hesaplanır.

Kullanılan hesap:

```cpp
VOUT = VSENSE_OUT * VSENSE_GAIN;
```

Deneysel kalibrasyon sonucunda:

```cpp
const float VSENSE_GAIN = 3.175;
```

değeri kullanılmıştır.

---

## PWM Ayarları

PWM sistemi aşağıdaki ayarlarla çalışmaktadır:

```cpp
#define PIN_PWM 15
#define PWM_CHANNEL 0
#define PWM_FREQ 20000
#define PWM_RES 10
```

10 bit PWM çözünürlüğünde PWM sayısal aralığı:

```text
0–1023
```

Duty cycle değerinin sayısal PWM karşılığı:

```cpp
pwmValue = PWM_MAX * dutyPercent / 100.0;
```

Örneğin %42 duty için yaklaşık PWM değeri:

```text
429
```

---

## Deney Sonuçları

100 Ω yük ve yaklaşık 12 V giriş gerilimi altında elde edilen sonuçlar:

| Duty cycle | Çıkış gerilimi |
| ---------: | -------------: |
|        %30 |         3.50 V |
|        %35 |         4.00 V |
|        %40 |         4.70 V |
|        %42 |        4.987 V |

Son test noktasında:

```text
Duty cycle  = %42
VOUT        = 4.987 V
VSENSE_OUT  = 1.557 V
```

Kalibrasyon katsayısı teorik olarak:

```text
VSENSE_GAIN = VOUT / VSENSE_OUT
VSENSE_GAIN = 4.987 / 1.557
VSENSE_GAIN ≈ 3.20
```

ESP32 ADC ölçüm sonuçlarının multimetreyle daha iyi eşleşmesi için yazılımda aşağıdaki değer kullanılmıştır:

```cpp
const float VSENSE_GAIN = 3.175;
```

---

# Tespit Edilen Donanımsal Sorunlar

## 1. IR2184 VCC bağlantısının bulunmaması

İlk testlerde IR2184 gate driver entegresinin VCC bacağında gerilim ölçülememiştir.

Şematikte IR2184 için VCC hattı bulunmasına rağmen bu hat kontrol konnektörüne çıkarılmamış veya PCB üzerinde gate driver’a düzgün şekilde ulaştırılmamıştır.

Gate driver beslenmediği için ESP32-S3 tarafından PWM gönderilmesine rağmen MOSFET’ler anahtarlama yapmamıştır.

### Uygulanan çözüm

IR2184 VCC hattına geçici jumper bağlantısı yapılmış ve yaklaşık 11.78 V besleme uygulanmıştır.

Bu bağlantıdan sonra:

* Gate driver çalışmaya başlamıştır.
* MOSFET’ler sürülmüştür.
* Duty cycle değişimine göre çıkış gerilimi değişmiştir.
* %42 duty seviyesinde yaklaşık 5 V çıkış elde edilmiştir.

### Kalıcı çözüm önerisi

Yeni PCB revizyonunda IR2184 VCC hattı:

* Doğrudan PCB izi ile taşınmalı
* Ayrı bir besleme konnektörüne çıkarılmalı
* Gate driver yakınına uygun bypass kapasitörleri eklenmeli
* Geçici jumper bağlantısı kullanılmamalıdır

---

## 2. R3, R4 ve R6 elemanlarının eksik olması

Kart üzerindeki gerilim ölçüm devresi incelendiğinde R3, R4 ve R6 elemanlarının lehimlenmediği görülmüştür.

Eksik dirençler nedeniyle:

* U2 op-amp giriş sinyali tanımsız kalmıştır.
* VSENSE_OUT gerçek çıkış gerilimini takip etmemiştir.
* VOUT sıfır olmasına rağmen VSENSE_OUT hattında hatalı gerilim görülmüştür.
* ESP32 ADC sistemi yanlış çıkış gerilimi hesaplamıştır.

### Uygulanan çözüm

Gerilim ölçüm katına dirençler lehimlenmiştir.

İlk olarak kullanılan değerler:

```text
R3 = 536 kΩ
R4 = 453 kΩ
R5 = Boş
R6 = 0 Ω
```

R6 için 0 Ω direnç yerine lehim köprüsü uygulanmıştır.

---

## 3. Op-amp çıkışının yüksek gerilim bölgesinde sınırlanması

İlk gerilim bölücü değerleri kullanıldığında düşük çıkış gerilimlerinde VSENSE_OUT düzgün şekilde artmıştır.

Ancak çıkış gerilimi 4.7–5 V seviyelerine yaklaştığında VSENSE_OUT yaklaşık 1.85–1.88 V seviyesinde sınırlanmıştır.

Ölçülen bazı değerler:

|   VOUT | VSENSE_OUT |
| -----: | ---------: |
| 3.50 V |     1.60 V |
| 4.00 V |     1.76 V |
| 4.70 V |     1.85 V |
| 4.90 V |     1.88 V |

R3 ve R4 orta noktasında yaklaşık 2.20 V görülmesine rağmen op-amp çıkışı bu gerilimi takip edememiştir.

Kart üzerindeki U2 entegresinin üst işaretlemesinin A63A olduğu görülmüştür. Kullanılan komponentin şemada belirtilen rail-to-rail LMV321 yerine farklı veya uyumsuz bir op-amp olabileceği değerlendirilmiştir.

### Uygulanan çözüm

Op-amp giriş gerilimini azaltmak amacıyla R3 direnci değiştirilmiştir.

Son kullanılan direnç değerleri:

```text
R3 = 1 MΩ
R4 = 453 kΩ
R5 = Boş
R6 = 0 Ω veya lehim köprüsü
```

Bu değişiklikten sonra 5 V çıkış seviyesinde op-amp giriş ve çıkış gerilimi yaklaşık 1.56 V seviyesine düşürülmüştür.

Son ölçüm:

```text
VOUT       = 4.987 V
VSENSE_OUT = 1.557 V
```

Böylece op-amp doğrusal çalışma bölgesinde tutulmuş ve gerilim ölçüm sistemi kararlı hale getirilmiştir.

---

## 4. ISENSE_OUT akım ölçüm problemi

Akım ölçüm devresinde INA181 kullanıldığı belirtilmiştir. Sensörün besleme gerilimi 3.3 V ve REF bağlantısı GND olmasına rağmen U1 çıkışında yaklaşık 5.9 V ölçülmüştür.

Ölçülen değer:

```text
ISENSE_OUT ≈ 5.9 V
```

3.3 V ile beslenen doğru çalışan bir INA181 çıkışının 5.9 V üretmesi beklenen bir durum değildir.

Olası nedenler:

* Kartta farklı veya yanlış komponent kullanılması
* U1 pin diziliminin footprint ile uyumsuz olması
* Entegrenin ters lehimlenmesi
* PCB net bağlantısında hata bulunması
* Komponentin arızalı olması

### Mevcut çözüm durumu

ISENSE_OUT hattı ESP32’ye bağlanmamıştır.

Akım ölçüm devresi doğrulanmadan bu hattın ESP32 ADC girişine bağlanması güvenli değildir.

ESP32-S3 GPIO pinleri 3.3 V seviyesinde çalıştığından 5.9 V seviyesindeki bir sinyal mikrodenetleyiciye zarar verebilir.

---

# Yazılım

Yazılım aşağıdaki görevleri yerine getirir:

* 20 kHz PWM üretir
* Duty cycle değerini sınırlar
* VSENSE_OUT ADC değerini okur
* Çoklu ADC örneğinin ortalamasını alır
* Gerçek VOUT değerini hesaplar
* Seri monitöre ölçüm sonuçlarını gönderir
* Wi-Fi Access Point oluşturur
* HTTP web sunucusu çalıştırır
* Web arayüzünden duty ayarı kabul eder
* PWM açma ve kapatma işlemlerini gerçekleştirir
* Acil durdurma komutunu işler
* Aşırı gerilim durumunda PWM’i kapatır

---

## ADC Ölçümü

VSENSE_OUT değeri tek bir ADC ölçümü yerine çok sayıda örneğin ortalaması alınarak hesaplanmaktadır.

Örnek kullanım:

```cpp
float readVsenseVoltage()
{
  const int samples = 160;
  uint32_t sum_mV = 0;

  for (int i = 0; i < samples; i++)
  {
    sum_mV += analogReadMilliVolts(PIN_VSENSE_ADC);
    delayMicroseconds(250);
  }

  float avg_mV = (float)sum_mV / samples;
  return avg_mV / 1000.0;
}
```

Bu yöntem anahtarlama gürültüsünün ve ADC dalgalanmasının etkisini azaltır.

---

# Web Arayüzü

ESP32-S3, Access Point modunda kendi Wi-Fi ağını oluşturur.

Ağ bilgileri:

```text
Wi-Fi adı: Buck_Control
Parola: buck12345
IP adresi: 192.168.4.1
```

Kullanıcı telefon veya bilgisayar ile `Buck_Control` ağına bağlandıktan sonra tarayıcıya aşağıdaki adresi yazar:

```text
http://192.168.4.1
```

---

## Web Arayüzü Özellikleri

Web kontrol panelinde aşağıdaki özellikler bulunmaktadır:

* Canlı VOUT gösterimi
* Canlı VSENSE_OUT gösterimi
* Duty cycle slider kontrolü
* Sayısal duty girişi
* PWM açma butonu
* PWM kapatma butonu
* Acil durdurma butonu
* 10 bit PWM değerinin görüntülenmesi
* PWM aktif veya kapalı durum göstergesi
* Aşırı gerilim hata göstergesi
* Mobil ve masaüstü ekran uyumluluğu

Duty cycle web arayüzü üzerinden manuel olarak belirlenmektedir. Duty değeri otomatik olarak artırılmaz veya azaltılmaz.

---

## HTTP Uç Noktaları

| Adres             | Görevi                                                 |
| ----------------- | ------------------------------------------------------ |
| `/`               | Ana web kontrol panelini açar                          |
| `/api`            | Canlı ölçüm ve sistem bilgilerini JSON olarak gönderir |
| `/set?duty=42`    | Duty değerini ayarlar                                  |
| `/enable?state=1` | PWM çıkışını aktif eder                                |
| `/enable?state=0` | PWM çıkışını kapatır                                   |
| `/stop`           | Duty değerini sıfırlar ve PWM’i kapatır                |

Örnek API cevabı:

```json
{
  "duty": 42.00,
  "vsense": 1.568,
  "vout": 4.978,
  "pwm": 429,
  "enabled": true,
  "fault": false
}
```

---

# Arduino IDE Ayarları

Önerilen Arduino IDE ayarları:

```text
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
Serial Monitor: 115200 baud
```

Bu projede Arduino-ESP32 2.x LEDC API kullanılmıştır.

PWM başlatma kodu:

```cpp
ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
ledcAttachPin(PIN_PWM, PWM_CHANNEL);
```

Arduino-ESP32 3.x sürümlerinde LEDC fonksiyonlarının kullanımı farklı olabilir.

---

# Güvenli Çalıştırma Sırası

1. Tüm bağlantıları enerjisiz durumda kontrol edin.
2. VOUT ile GND arasına 100 Ω test yükü bağlayın.
3. ESP32-S3’ü USB üzerinden çalıştırın.
4. Gerilim ölçüm devresine 3.3 V besleme verin.
5. ESP32 ve buck kartı GND hatlarını ortak bağlayın.
6. VIN girişine akım limitli 12 V kaynak bağlayın.
7. IR2184 VCC girişine yaklaşık 10–12 V uygulayın.
8. ESP32 Wi-Fi ağına bağlanın.
9. Web arayüzünü açın.
10. PWM’i düşük duty seviyesinde başlatın.
11. Duty değerini kademeli olarak artırın.
12. VOUT değerini hem web arayüzünden hem multimetreden takip edin.

---

# Güvenlik Uyarıları

* İlk testlerde batarya yerine akım limitli laboratuvar güç kaynağı kullanın.
* ESP32 GPIO pinlerine 3.3 V üzerinde gerilim uygulamayın.
* ISENSE_OUT hattını mevcut durumda ESP32’ye bağlamayın.
* IR2184 VCC hattını ESP32 3.3 V çıkışından beslemeyin.
* VIN, VCC ve 3V3 hatlarının görevlerini karıştırmayın.
* High-side MOSFET ölçümünde osiloskop GND klipsini switch node hattına bağlamayın.
* İlk denemelerde düşük duty kullanın.
* Test sırasında MOSFET, bobin ve kapasitör sıcaklıklarını kontrol edin.
* Geçici jumper bağlantılarını kalıcı PCB tasarımında kullanmayın.
* Web arayüzündeki yazılımsal korumaları tek güvenlik yöntemi olarak kabul etmeyin.
* Kritik korumalar için donanımsal aşırı akım ve aşırı gerilim devreleri ekleyin.

---

# Proje Sonuçları

Proje sonunda aşağıdaki çalışmalar başarıyla gerçekleştirilmiştir:

* ESP32-S3 ile 20 kHz PWM üretilmiştir.
* IR2184 gate driver üzerinden buck güç katı sürülmüştür.
* Duty cycle ile çıkış geriliminin kontrol edilebildiği doğrulanmıştır.
* %42 duty seviyesinde yaklaşık 5 V çıkış alınmıştır.
* Eksik gerilim ölçüm dirençleri tespit edilmiştir.
* Gerilim ölçüm devresi yeniden düzenlenmiştir.
* Op-amp çalışma aralığına uygun yeni gerilim bölücü tasarlanmıştır.
* ESP32 ADC ölçümü multimetreye göre kalibre edilmiştir.
* Yerel Wi-Fi üzerinden çalışan web kontrol paneli geliştirilmiştir.
* Manuel duty kontrolü sağlanmıştır.
* PWM açma, kapatma ve acil durdurma özellikleri eklenmiştir.
* Yazılımsal aşırı gerilim kapatması uygulanmıştır.
* Akım ölçüm katındaki komponent uyumsuzluğu tespit edilmiştir.

---

# Gelecek Geliştirmeler

* Doğru INA181 komponenti ile akım ölçüm sisteminin tamamlanması
* Donanımsal aşırı akım koruması eklenmesi
* Gerçek kapalı çevrim PI kontrol uygulanması
* Yük değişimlerinde geçici durum analizi yapılması
* Verim ölçümü yapılması
* MOSFET ve bobin sıcaklıklarının ölçülmesi
* PLECS modeli oluşturulması
* Simülasyon ve gerçek kart sonuçlarının karşılaştırılması
* PCB’nin yeni revizyonunun hazırlanması
* IR2184 VCC hattının PCB üzerinde düzeltilmesi
* SD hattının güvenli şekilde ESP32 tarafından kontrol edilmesi
* Web arayüzüne canlı grafik eklenmesi
* Ölçüm verilerinin CSV dosyası olarak kaydedilmesi
* Wi-Fi ağı için kullanıcı adı ve parola güvenliğinin artırılması
* OTA kablosuz yazılım güncelleme özelliği eklenmesi

---

# Akademik Çalışma

Bu proje; güç elektroniği, senkron DC-DC dönüştürücüler, mikrodenetleyici tabanlı PWM kontrolü, gate driver devreleri, ADC ölçümü, kalibrasyon ve gömülü web sistemlerini bir araya getiren deneysel bir çalışmadır.

**Akademik danışman:** Doç. Dr. Mehmet Dal
**Kurum:** Kocaeli Üniversitesi

---

# Katkıda Bulunma

Projeye katkı sağlamak için:

1. Repoyu fork edin.
2. Yeni bir geliştirme dalı oluşturun.
3. Değişikliklerinizi ekleyin.
4. Açıklayıcı bir commit mesajı yazın.
5. Pull request oluşturun.

Hata bildirimleri ve geliştirme önerileri GitHub Issues bölümünden paylaşılabilir.

---

# Lisans

Bu proje için lisans henüz belirlenmemiştir.

Açık kaynak olarak yayınlanması durumunda MIT License tercih edilebilir.
