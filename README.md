# ESP32-S3 Kontrollü Buck Converter

Bu proje, 12 V DC giriş gerilimini yaklaşık 5 V seviyesine düşüren senkron buck converter devresinin ESP32-S3 ile kontrol edilmesini amaçlamaktadır.

ESP32-S3 tarafından üretilen 20 kHz PWM sinyali IR2184 gate driver üzerinden MOSFET’lere uygulanmaktadır. Çıkış gerilimi kart üzerindeki gerilim ölçüm katından alınarak ESP32 ADC girişinde okunmakta ve yerel web arayüzünde canlı olarak gösterilmektedir.

## Özellikler

* ESP32-S3 ile 20 kHz PWM üretimi
* Web arayüzünden manuel duty cycle ayarı
* PWM açma, kapatma ve acil durdurma
* Canlı VOUT ve VSENSE_OUT ölçümü
* ADC kalibrasyonu
* Yazılımsal aşırı gerilim koruması
* Wi-Fi Access Point üzerinden yerel kontrol
* Mobil ve masaüstü tarayıcı desteği

## Temel Parametreler

| Parametre             |   Değer |
| --------------------- | ------: |
| Giriş gerilimi        | 12 V DC |
| Gate driver beslemesi | 11.78 V |
| PWM frekansı          |  20 kHz |
| PWM çözünürlüğü       |  10 bit |
| Test duty değeri      |     %42 |
| Çıkış gerilimi        | 4.987 V |
| VSENSE_OUT            | 1.557 V |
| Kalibrasyon katsayısı |   3.175 |
| Test yükü             |   100 Ω |

## ESP32-S3 Bağlantıları

| ESP32-S3 | Buck kartı                   |
| -------- | ---------------------------- |
| GPIO15   | IN PWM girişi                |
| GPIO4    | VSENSE_OUT                   |
| 3V3      | Gerilim ölçüm katı beslemesi |
| GND      | Ortak GND                    |

GPIO15 ve GPIO4 hatlarında 1 kΩ seri direnç kullanılmıştır.

## Tespit Edilen Sorunlar

### IR2184 VCC bağlantısı

IR2184 gate driver VCC hattının karta düzgün şekilde ulaşmadığı tespit edilmiştir. Gate driver VCC pinine jumper bağlantısı yapılarak 11.78 V besleme uygulanmıştır.

### Eksik gerilim ölçüm dirençleri

Kart üzerinde R3, R4 ve R6 elemanlarının eksik olduğu görülmüştür. Gerilim ölçüm katı aşağıdaki değerlerle tamamlanmıştır:

```text
R3 = 1 MΩ
R4 = 453 kΩ
R5 = Boş
R6 = 0 Ω / lehim köprüsü
```

Bu değişiklik sonrasında VSENSE_OUT sinyali 5 V çıkış seviyesinde doğrusal ve kararlı şekilde ölçülebilmiştir.

### Op-amp çalışma aralığı

İlk gerilim bölücü oranında op-amp çıkışı yaklaşık 1.88 V seviyesinde sınırlanmıştır. R3 direnci 1 MΩ olarak değiştirilerek op-amp giriş seviyesi düşürülmüş ve sorun giderilmiştir.

### Akım ölçüm katı

ISENSE_OUT hattında yaklaşık 5.9 V ölçülmüştür. Bu değer ESP32 ADC giriş seviyesi için güvenli değildir. Akım ölçüm katı doğrulanana kadar ISENSE_OUT ESP32’ye bağlanmamalıdır.

## Web Arayüzü

ESP32-S3 kendi Wi-Fi ağını oluşturur.

```text
Wi-Fi adı: Buck_Control
Parola: buck12345
Adres: http://192.168.4.1
```

Web paneli üzerinden duty cycle ayarlanabilir, PWM kontrol edilebilir ve çıkış gerilimi canlı olarak izlenebilir.

## Arduino IDE Ayarları

```text
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
Serial Monitor: 115200 baud
```

Projede Arduino-ESP32 2.x LEDC API kullanılmıştır:

```cpp
ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
ledcAttachPin(PIN_PWM, PWM_CHANNEL);
```

## Güvenlik

* İlk testlerde akım limitli laboratuvar güç kaynağı kullanılmalıdır.
* ESP32 GPIO pinlerine 3.3 V üzerinde gerilim uygulanmamalıdır.
* ISENSE_OUT mevcut durumda ESP32’ye bağlanmamalıdır.
* IR2184 VCC hattı ESP32 3.3 V çıkışından beslenmemelidir.
* Duty cycle düşük değerden başlanarak kademeli artırılmalıdır.
* Yazılımsal korumalar donanımsal korumaların yerine geçmez.

## Sonuç

Proje kapsamında buck converter güç katı başarıyla çalıştırılmış, kart üzerindeki gerilim ölçüm problemleri giderilmiş, ESP32 ADC sistemi kalibre edilmiş ve yerel web arayüzü üzerinden manuel kontrol sağlanmıştır.

%42 duty cycle seviyesinde yaklaşık 12 V girişten 4.987 V çıkış elde edilmiştir.

## Gelecek Geliştirmeler

* Akım ölçüm katının düzeltilmesi
* Donanımsal aşırı akım koruması
* Kapalı çevrim PI kontrol
* PLECS modeli ile karşılaştırma
* Verim ve sıcaklık testleri
* Yeni PCB revizyonu
* Web arayüzüne canlı grafik ve veri kaydı

## Akademik Bilgi

**Akademik danışman:** Doç. Dr. Mehmet Dal
**Kurum:** Kocaeli Üniversitesi

## Lisans

Bu proje için açık kaynak lisansı henüz belirlenmemiştir.
