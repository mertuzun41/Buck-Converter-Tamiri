#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ===================== PINLER =====================
#define PIN_PWM 15
#define PIN_VSENSE_ADC 4

// ===================== PWM AYARLARI =====================
#define PWM_CHANNEL 0
#define PWM_FREQ    20000
#define PWM_RES     10

const int PWM_MAX = (1 << PWM_RES) - 1;

// ===================== KALİBRASYON =====================
// Son ölçümüne göre:
// VOUT = 4.978 V civarı
// VSENSE_OUT ≈ 1.568 V civarı
// VSENSE_GAIN ≈ 3.175
const float VSENSE_GAIN = 3.175;

// ===================== GÜVENLİK =====================
const float DUTY_MAX = 45.0;
const float VOUT_TRIP = 5.60;

// Başlangıç duty değeri
float dutyPercent = 42.0;

// Başlangıçta PWM kapalı başlasın
bool outputEnabled = false;
bool faultLatched = false;

WebServer server(80);

// ===================== WIFI AP AYARLARI =====================
const char* AP_SSID = "Buck_Control";
const char* AP_PASS = "buck12345";   // en az 8 karakter olmalı

// ===================== PWM FONKSİYONLARI =====================
void applyPWM()
{
  float duty = dutyPercent;

  if (duty < 0) duty = 0;
  if (duty > DUTY_MAX) duty = DUTY_MAX;

  int pwmValue = 0;

  if (outputEnabled && !faultLatched)
  {
    pwmValue = (int)(PWM_MAX * duty / 100.0);
  }
  else
  {
    pwmValue = 0;
  }

  ledcWrite(PWM_CHANNEL, pwmValue);
}

void setDutyPercent(float duty)
{
  if (duty < 0) duty = 0;
  if (duty > DUTY_MAX) duty = DUTY_MAX;

  dutyPercent = duty;
  applyPWM();
}

// ===================== ADC OKUMA =====================
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

float readVout()
{
  return readVsenseVoltage() * VSENSE_GAIN;
}

// ===================== HTML ARAYÜZ =====================
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Buck Converter Kontrol Paneli</title>
  <style>
    :root {
      --bg: #07111f;
      --card: rgba(255,255,255,0.08);
      --card2: rgba(255,255,255,0.12);
      --text: #eef6ff;
      --muted: #92a4b8;
      --accent: #4da3ff;
      --good: #30d158;
      --warn: #ffcc00;
      --danger: #ff453a;
      --border: rgba(255,255,255,0.16);
    }

    * {
      box-sizing: border-box;
      font-family: Inter, Segoe UI, Arial, sans-serif;
    }

    body {
      margin: 0;
      min-height: 100vh;
      background:
        radial-gradient(circle at top left, rgba(77,163,255,0.28), transparent 35%),
        radial-gradient(circle at bottom right, rgba(48,209,88,0.18), transparent 35%),
        var(--bg);
      color: var(--text);
      padding: 22px;
    }

    .container {
      max-width: 1080px;
      margin: 0 auto;
    }

    .header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      margin-bottom: 22px;
    }

    .title h1 {
      margin: 0;
      font-size: 28px;
      letter-spacing: -0.5px;
    }

    .title p {
      margin: 6px 0 0;
      color: var(--muted);
      font-size: 14px;
    }

    .badge {
      padding: 10px 14px;
      border: 1px solid var(--border);
      background: var(--card);
      border-radius: 999px;
      font-weight: 700;
      font-size: 13px;
      white-space: nowrap;
      backdrop-filter: blur(14px);
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 16px;
    }

    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 24px;
      padding: 20px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.25);
      backdrop-filter: blur(18px);
    }

    .card h2 {
      margin: 0 0 14px;
      color: var(--muted);
      font-size: 13px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1px;
    }

    .value {
      font-size: 38px;
      font-weight: 800;
      letter-spacing: -1px;
      margin: 4px 0;
    }

    .unit {
      color: var(--muted);
      font-size: 18px;
      margin-left: 4px;
    }

    .sub {
      color: var(--muted);
      margin-top: 8px;
      font-size: 13px;
    }

    .wide {
      grid-column: span 2;
    }

    .full {
      grid-column: 1 / -1;
    }

    .slider-row {
      display: flex;
      align-items: center;
      gap: 16px;
      margin-top: 14px;
    }

    input[type="range"] {
      width: 100%;
      accent-color: var(--accent);
    }

    .duty-box {
      width: 92px;
      padding: 12px;
      border-radius: 14px;
      border: 1px solid var(--border);
      background: rgba(0,0,0,0.22);
      color: var(--text);
      font-size: 18px;
      font-weight: 800;
      text-align: center;
    }

    .buttons {
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      margin-top: 18px;
    }

    button {
      border: 0;
      border-radius: 16px;
      padding: 13px 18px;
      color: white;
      font-weight: 800;
      cursor: pointer;
      transition: 0.15s ease;
      font-size: 14px;
    }

    button:hover {
      transform: translateY(-1px);
      filter: brightness(1.08);
    }

    .btn-on {
      background: linear-gradient(135deg, #1dd75f, #0ea447);
    }

    .btn-off {
      background: linear-gradient(135deg, #ff9f0a, #ff6b00);
    }

    .btn-danger {
      background: linear-gradient(135deg, #ff453a, #b00020);
    }

    .btn-set {
      background: linear-gradient(135deg, #4da3ff, #0057d9);
    }

    .status {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 10px 12px;
      border-radius: 999px;
      background: var(--card2);
      border: 1px solid var(--border);
      color: var(--muted);
      font-weight: 700;
      font-size: 13px;
    }

    .dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--warn);
      box-shadow: 0 0 18px var(--warn);
    }

    .dot.on {
      background: var(--good);
      box-shadow: 0 0 18px var(--good);
    }

    .dot.fault {
      background: var(--danger);
      box-shadow: 0 0 18px var(--danger);
    }

    .bar {
      height: 12px;
      width: 100%;
      border-radius: 999px;
      background: rgba(255,255,255,0.12);
      overflow: hidden;
      margin-top: 12px;
    }

    .bar-fill {
      height: 100%;
      width: 0%;
      border-radius: 999px;
      background: linear-gradient(90deg, #30d158, #4da3ff);
      transition: width 0.2s ease;
    }

    .footer {
      color: var(--muted);
      font-size: 12px;
      margin-top: 16px;
      text-align: center;
    }

    @media (max-width: 850px) {
      .grid {
        grid-template-columns: 1fr;
      }

      .wide, .full {
        grid-column: auto;
      }

      .header {
        flex-direction: column;
        align-items: flex-start;
      }

      .value {
        font-size: 34px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="title">
        <h1>Buck Converter Kontrol Paneli</h1>
        <p>ESP32-S3 PWM kontrol ve canlı VOUT izleme arayüzü</p>
      </div>
      <div class="badge">Local Web UI • 192.168.4.1</div>
    </div>

    <div class="grid">
      <div class="card">
        <h2>Çıkış Gerilimi</h2>
        <div class="value"><span id="vout">--</span><span class="unit">V</span></div>
        <div class="bar"><div id="voutBar" class="bar-fill"></div></div>
        <div class="sub">Hedef güvenli bölge: yaklaşık 5.00 V</div>
      </div>

      <div class="card">
        <h2>VSENSE_OUT</h2>
        <div class="value"><span id="vsense">--</span><span class="unit">V</span></div>
        <div class="sub">ADC girişindeki ölçülen voltaj</div>
      </div>

      <div class="card">
        <h2>Durum</h2>
        <div class="status">
          <span id="statusDot" class="dot"></span>
          <span id="statusText">Bağlanıyor...</span>
        </div>
        <div class="sub" style="margin-top:16px;">Aşırı gerilim koruması: 5.60 V</div>
      </div>

      <div class="card wide">
        <h2>PWM Duty Ayarı</h2>
        <div class="value"><span id="dutyText">42.00</span><span class="unit">%</span></div>
        <div class="slider-row">
          <input id="dutySlider" type="range" min="0" max="45" value="42" step="0.1">
          <input id="dutyInput" class="duty-box" type="number" min="0" max="45" step="0.1" value="42">
        </div>
        <div class="buttons">
          <button class="btn-set" onclick="sendDuty()">Duty Uygula</button>
          <button class="btn-on" onclick="enableOutput()">PWM Aç</button>
          <button class="btn-off" onclick="disableOutput()">PWM Kapat</button>
          <button class="btn-danger" onclick="emergencyStop()">Acil Durdur</button>
        </div>
      </div>

      <div class="card">
        <h2>PWM Bilgisi</h2>
        <div class="value"><span id="pwmValue">--</span></div>
        <div class="sub">10-bit PWM değeri, maksimum 1023</div>
      </div>

      <div class="card full">
        <h2>Not</h2>
        <div class="sub">
          Bu panel manuel sabit PWM kontrolü içindir. Duty otomatik değişmez. ISENSE_OUT ve SD hatları şu an kullanılmamalıdır.
        </div>
      </div>
    </div>

    <div class="footer">Buck Converter Web Control • ESP32-S3</div>
  </div>

<script>
  const slider = document.getElementById("dutySlider");
  const input = document.getElementById("dutyInput");
  const dutyText = document.getElementById("dutyText");

  slider.addEventListener("input", () => {
    input.value = slider.value;
    dutyText.textContent = Number(slider.value).toFixed(1);
  });

  input.addEventListener("input", () => {
    let v = Number(input.value);
    if (v < 0) v = 0;
    if (v > 45) v = 45;
    slider.value = v;
    dutyText.textContent = v.toFixed(1);
  });

  async function sendDuty() {
    const duty = Number(input.value);
    await fetch(`/set?duty=${duty}`);
    await updateData();
  }

  async function enableOutput() {
    await sendDuty();
    await fetch("/enable?state=1");
    await updateData();
  }

  async function disableOutput() {
    await fetch("/enable?state=0");
    await updateData();
  }

  async function emergencyStop() {
    await fetch("/stop");
    await updateData();
  }

  function setStatus(enabled, fault) {
    const dot = document.getElementById("statusDot");
    const text = document.getElementById("statusText");

    dot.className = "dot";

    if (fault) {
      dot.classList.add("fault");
      text.textContent = "HATA / PWM KAPALI";
    } else if (enabled) {
      dot.classList.add("on");
      text.textContent = "PWM AKTİF";
    } else {
      text.textContent = "PWM KAPALI";
    }
  }

  async function updateData() {
    try {
      const res = await fetch("/api");
      const data = await res.json();

      document.getElementById("vout").textContent = data.vout.toFixed(3);
      document.getElementById("vsense").textContent = data.vsense.toFixed(3);
      document.getElementById("dutyText").textContent = data.duty.toFixed(2);
      document.getElementById("pwmValue").textContent = data.pwm;

      slider.value = data.duty;
      input.value = data.duty.toFixed(1);

      let percent = Math.min(Math.max((data.vout / 5.6) * 100, 0), 100);
      document.getElementById("voutBar").style.width = percent + "%";

      setStatus(data.enabled, data.fault);
    } catch (e) {
      document.getElementById("statusText").textContent = "Bağlantı yok";
    }
  }

  setInterval(updateData, 500);
  updateData();
</script>
</body>
</html>
)rawliteral";

// ===================== WEB HANDLERLAR =====================
void handleRoot()
{
  server.send_P(200, "text/html", MAIN_page);
}

void handleApi()
{
  float vsense = readVsenseVoltage();
  float vout = vsense * VSENSE_GAIN;

  if (outputEnabled && !faultLatched && vout > VOUT_TRIP)
  {
    faultLatched = true;
    outputEnabled = false;
    applyPWM();
  }

  int pwmValue = 0;
  if (outputEnabled && !faultLatched)
  {
    pwmValue = (int)(PWM_MAX * dutyPercent / 100.0);
  }

  String json = "{";
  json += "\"duty\":" + String(dutyPercent, 2) + ",";
  json += "\"vsense\":" + String(vsense, 4) + ",";
  json += "\"vout\":" + String(vout, 4) + ",";
  json += "\"pwm\":" + String(pwmValue) + ",";
  json += "\"enabled\":" + String(outputEnabled ? "true" : "false") + ",";
  json += "\"fault\":" + String(faultLatched ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void handleSetDuty()
{
  if (server.hasArg("duty"))
  {
    float duty = server.arg("duty").toFloat();
    setDutyPercent(duty);
    server.send(200, "text/plain", "OK");
  }
  else
  {
    server.send(400, "text/plain", "Missing duty");
  }
}

void handleEnable()
{
  if (server.hasArg("state"))
  {
    int state = server.arg("state").toInt();

    if (state == 1)
    {
      faultLatched = false;
      outputEnabled = true;
    }
    else
    {
      outputEnabled = false;
    }

    applyPWM();
    server.send(200, "text/plain", "OK");
  }
  else
  {
    server.send(400, "text/plain", "Missing state");
  }
}

void handleStop()
{
  outputEnabled = false;
  faultLatched = false;
  setDutyPercent(0);
  applyPWM();

  server.send(200, "text/plain", "STOPPED");
}

// ===================== SETUP =====================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_VSENSE_ADC, ADC_11db);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_PWM, PWM_CHANNEL);

  ledcWrite(PWM_CHANNEL, 0);

  Serial.println();
  Serial.println("Buck Converter Web Arayuz Basliyor...");
  Serial.println("PWM baslangicta kapali.");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("WiFi agi: ");
  Serial.println(AP_SSID);
  Serial.print("Sifre: ");
  Serial.println(AP_PASS);
  Serial.print("Web arayuz IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/api", handleApi);
  server.on("/set", handleSetDuty);
  server.on("/enable", handleEnable);
  server.on("/stop", handleStop);

  server.begin();

  Serial.println("Web server basladi.");
  Serial.println("Tarayici adresi: http://192.168.4.1");
}

// ===================== LOOP =====================
void loop()
{
  server.handleClient();

  static unsigned long lastPrint = 0;
  static unsigned long lastSafety = 0;

  if (millis() - lastSafety > 250)
  {
    lastSafety = millis();

    float vout = readVout();

    if (outputEnabled && !faultLatched && vout > VOUT_TRIP)
    {
      faultLatched = true;
      outputEnabled = false;
      applyPWM();

      Serial.println("HATA: VOUT fazla yuksek! PWM kapatildi.");
    }
  }

  if (millis() - lastPrint > 1000)
  {
    lastPrint = millis();

    float vsense = readVsenseVoltage();
    float vout = vsense * VSENSE_GAIN;

    Serial.print("Duty: ");
    Serial.print(dutyPercent, 2);
    Serial.print(" % | PWM: ");
    Serial.print(outputEnabled ? "ACIK" : "KAPALI");
    Serial.print(" | VSENSE_OUT: ");
    Serial.print(vsense, 3);
    Serial.print(" V | VOUT: ");
    Serial.print(vout, 3);
    Serial.println(" V");
  }
}