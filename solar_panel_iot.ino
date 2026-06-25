/*
 * ============================================================
 *   IoT-ENABLED WATERLESS SOLAR PANEL CLEANING SYSTEM
 *   WITH DUAL MODE AND POWER EFFICIENCY MONITORING
 * ============================================================
 * Authors    : M. Thanuja Sri (44130246)
 *              Melvin Ancy. X (44130272)
 *              Nave Swety. M  (44130302)
 * College    : Sathyabama Institute of Science and Technology
 *              Department of Electronics and Communication Engineering
 * Guide      : Dr. G.D. Anbarasi Jebaselvi, M.E., Ph.D.
 * Course     : SCSBDPROJ - Design Thinking and Innovations
 * Duration   : November 2025 – April 2026
 * Board      : ESP32 WROOM
 *
 * Description:
 *   An IoT-based automated solar panel cleaning system that
 *   detects dust accumulation using an optical dust sensor and
 *   performs waterless cleaning using a servo-driven brush and
 *   air blower. Supports dual mode operation:
 *     AUTO  → cleans automatically when dust exceeds threshold
 *     MANUAL → user triggers cleaning via web dashboard
 *   Real-time data (dust level, temperature, humidity, power)
 *   is displayed on a responsive web interface hosted by ESP32
 *   via Wi-Fi, accessible from any phone or laptop.
 *
 * Hardware Components:
 *   - ESP32 WROOM (microcontroller + Wi-Fi)
 *   - Optical Dust Sensor (dust density detection)
 *   - DHT11 (temperature & humidity monitoring)
 *   - SG90 Servo Motor (brush sweep mechanism)
 *   - 5V DC Blower Fan (air-based dust removal)
 *   - Adafruit INA219 (power monitoring sensor)
 *   - Lithium-ion Battery + Boost Converter (5V power supply)
 *   - USB Type-C Charging Module
 *   - Push Button + LED (manual control & status indicator)
 *
 * Pin Configuration:
 *   GPIO 4  → DHT11 Data
 *   GPIO 34 → Dust Sensor Analog Output (ADC)
 *   GPIO 18 → Servo Motor PWM
 *   GPIO 25 → Blower Fan (via transistor)
 *   GPIO 14 → Dust Sensor LED Power
 *   GPIO 19 → System Power ON LED
 *   SDA/SCL → INA219 (I2C)
 *
 * Libraries Required:
 *   WiFi, WebServer, WiFiManager, ESP32Servo,
 *   Wire, Adafruit_INA219, DHT sensor library
 * ============================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <DHT.h>

// ── Wi-Fi Access Point Credentials (for initial setup) ──────
#define AP_SSID            "Solar-clean"
#define AP_PASSWORD        "setup1234"
#define PORTAL_TIMEOUT_SEC 180     // Config portal times out after 3 min

// ── Web Server on port 80 ────────────────────────────────────
WebServer server(80);

// ── Pin Definitions ──────────────────────────────────────────
#define DHT_PIN      4     // DHT11 data pin
#define DHT_TYPE     DHT11
#define SERVO_PIN    18    // Servo motor PWM pin
#define LED_POWER    14    // Dust sensor LED power control
#define MEASURE_PIN  34    // Dust sensor analog output (ADC)
#define MOTOR_PIN    25    // Blower fan control pin
#define POWERON_LED  19    // System ON status LED

// ── Object Declarations ──────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);
Servo brushServo;
Adafruit_INA219 ina219;

// ── Sensor Data Variables ────────────────────────────────────
float dustDensity = 0;
float temperature = 0;
float humidity    = 0;
float voltage     = 0;
float current     = 0;
float powerValue  = 0;

// ── System State Variables ───────────────────────────────────
float threshold        = 0.10;   // Dust level that triggers AUTO cleaning
bool  autoMode         = true;   // true = AUTO, false = MANUAL
bool  manualCleanRequest = false; // Set to true when user clicks Clean on dashboard

// ============================================================
// READ DUST SENSOR
// Uses optical method: LED pulse → ADC read → density formula
// dustDensity = 0.17 * voltage - 0.1
// ============================================================
void readDust() {
  digitalWrite(LED_POWER, LOW);      // Turn on sensor LED
  delayMicroseconds(280);            // Wait for LED to stabilize

  int voMeasured = analogRead(MEASURE_PIN);  // Read ADC value

  delayMicroseconds(40);
  digitalWrite(LED_POWER, HIGH);     // Turn off sensor LED
  delayMicroseconds(9680);           // Complete sampling cycle

  // Convert 12-bit ADC reading to voltage (ESP32 = 3.3V ref)
  float calcVoltage = voMeasured * (3.3 / 4095.0);

  // Apply dust sensor formula to get dust density (mg/m³ approx)
  dustDensity = 0.17 * calcVoltage - 0.1;
  if (dustDensity < 0) dustDensity = 0;  // Clamp negative noise
}

// ============================================================
// READ DHT11 — Temperature & Humidity
// ============================================================
void readTemp() {
  temperature = dht.readTemperature();
  humidity    = dht.readHumidity();
}

// ============================================================
// READ POWER (INA219)
// Note: INA219 calibration was not fully completed in prototype.
// Static values used for demonstration during testing.
// ============================================================
void readPower() {
  voltage    = 2.28;
  current    = 0.42;
  powerValue = voltage * current;
}

// ============================================================
// BRUSH SWEEP — Servo sweeps 0° to 180° and back
// ============================================================
void sweepBrush() {
  // Forward sweep
  for (int pos = 0; pos <= 180; pos += 5) {
    brushServo.write(pos);
    delay(20);
  }
  // Return sweep
  for (int pos = 180; pos >= 0; pos -= 5) {
    brushServo.write(pos);
    delay(20);
  }
}

// ============================================================
// GENERATE HTML DASHBOARD
// Responsive dark-themed web interface with live sensor data,
// mode indicator, toggle button, and manual clean button.
// Auto-refreshes every 2 seconds.
// ============================================================
String getHTML() {
  String html = R"rawliteral(
<!DOCTYPE html><html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="2">
<title>Solar Cleaner</title>
<style>
:root{
  --bg:#0f172a;
  --card:#1e293b;
  --accent:#38bdf8;
  --green:#22c55e;
  --red:#ef4444;
  --text:#e2e8f0;
}
body{
  margin:0;
  font-family:'Segoe UI',sans-serif;
  background:linear-gradient(135deg,#0f172a,#020617);
  color:var(--text);
  text-align:center;
}
.header{
  padding:20px;
  font-size:22px;
  font-weight:600;
  letter-spacing:1px;
  background:rgba(255,255,255,0.03);
  backdrop-filter:blur(6px);
  box-shadow:0 2px 10px rgba(0,0,0,0.3);
}
.container{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(140px,1fr));
  gap:12px;
  padding:15px;
}
.card{
  background:var(--card);
  padding:15px;
  border-radius:12px;
  box-shadow:0 4px 12px rgba(0,0,0,0.4);
  transition:0.3s;
}
.card:hover{
  transform:translateY(-3px);
  box-shadow:0 6px 18px rgba(0,0,0,0.6);
}
.label{ font-size:13px; opacity:0.7; }
.value{ font-size:20px; font-weight:bold; margin-top:5px; }
.mode{
  display:inline-block;
  padding:6px 12px;
  border-radius:20px;
  font-size:13px;
  font-weight:600;
}
.btn{
  padding:12px 18px;
  border:none;
  border-radius:10px;
  margin:8px;
  font-size:14px;
  cursor:pointer;
  transition:0.3s;
}
.btn-primary{ background:var(--accent); color:black; }
.btn-primary:hover{ background:#0ea5e9; }
.btn-success{ background:var(--green); color:black; }
.btn-success:hover{ background:#16a34a; }
.btn-disabled{ background:#334155; color:#94a3b8; cursor:not-allowed; }
.actions{ margin-top:10px; }
</style>
</head>
<body>
<div class="header">SMART SOLAR CLEANER</div>
<div class="container">
)rawliteral";

  // Mode card — green badge for AUTO, amber for MANUAL
  html += "<div class='card'><div class='label'>Mode</div><div class='value'>"
          "<span class='mode' style='background:"
          + String(autoMode ? "#22c55e" : "#f59e0b") + ";'>"
          + String(autoMode ? "AUTO" : "MANUAL")
          + "</span></div></div>";

  // Sensor data cards
  html += "<div class='card'><div class='label'>Dust</div><div class='value'>"
          + String(dustDensity, 2) + "</div></div>";

  html += "<div class='card'><div class='label'>Temperature</div><div class='value'>"
          + String(temperature, 1) + " &deg;C</div></div>";

  html += "<div class='card'><div class='label'>Humidity</div><div class='value'>"
          + String(humidity, 1) + " %</div></div>";

  html += "<div class='card'><div class='label'>Power</div><div class='value'>"
          + String(powerValue, 2) + " W</div></div>";

  // Control buttons
  html += "<div class='actions'>";
  html += "<button class='btn btn-primary' onclick=\"location.href='/toggle'\">Toggle Mode</button>";

  if (autoMode) {
    html += "<button class='btn btn-disabled'>Clean Disabled</button>";
  } else {
    html += "<button class='btn btn-success' onclick=\"location.href='/clean'\">Start Cleaning</button>";
  }

  html += "</div></body></html>";
  return html;
}

// ── Web Server Route Handlers ────────────────────────────────
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleToggle() {
  autoMode = !autoMode;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleClean() {
  if (!autoMode) {
    manualCleanRequest = true;  // Will be processed in main loop
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ============================================================
// WI-FI SETUP using WiFiManager
// On first use: creates "Solar-clean" hotspot for configuration.
// On subsequent use: auto-connects to saved network.
// ============================================================
void setupWiFi() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(PORTAL_TIMEOUT_SEC);

  if (!wm.autoConnect(AP_SSID, AP_PASSWORD)) {
    Serial.println("Wi-Fi connection failed. Restarting...");
    ESP.restart();
  }

  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());
}

// ── Serial Debug Print ───────────────────────────────────────
void printSerial() {
  Serial.println("─────────────────────────");
  Serial.print("Temperature : "); Serial.print(temperature);  Serial.println(" °C");
  Serial.print("Humidity    : "); Serial.print(humidity);     Serial.println(" %");
  Serial.print("Dust Density: "); Serial.println(dustDensity);
  Serial.print("Voltage     : "); Serial.print(voltage);      Serial.println(" V");
  Serial.print("Current     : "); Serial.print(current);      Serial.println(" mA");
  Serial.print("Power       : "); Serial.print(powerValue);   Serial.println(" W");
  Serial.print("Mode        : "); Serial.println(autoMode ? "AUTO" : "MANUAL");
}

// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_POWER,   OUTPUT);
  pinMode(MOTOR_PIN,   OUTPUT);
  pinMode(POWERON_LED, OUTPUT);

  dht.begin();
  brushServo.attach(SERVO_PIN);
  Wire.begin();

  // INA219 power sensor initialization
  if (!ina219.begin()) {
    Serial.println("WARNING: INA219 not found. Using static power values.");
  }

  setupWiFi();

  // Register web server routes
  server.on("/",       handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/clean",  handleClean);
  server.begin();

  digitalWrite(POWERON_LED, HIGH);  // System ON indicator
  Serial.println("System Ready. Web server started.");
}

// ============================================================
void loop() {
  server.handleClient();  // Handle incoming web requests

  // ── Read all sensors ──────────────────────────────────────
  readDust();
  readTemp();
  readPower();
  printSerial();

  // ── AUTO MODE: Clean when dust exceeds threshold ──────────
  if (autoMode) {
    if (dustDensity > threshold) {
      Serial.println("AUTO: Dust threshold exceeded. Cleaning...");
      digitalWrite(MOTOR_PIN, HIGH);   // Turn on blower fan
      sweepBrush();                    // Sweep servo brush
    } else {
      digitalWrite(MOTOR_PIN, LOW);    // Fan off — panel clean
    }

  // ── MANUAL MODE: Clean only on user request ───────────────
  } else {
    if (manualCleanRequest) {
      Serial.println("MANUAL: Cleaning triggered by user.");
      digitalWrite(MOTOR_PIN, HIGH);
      sweepBrush();
      manualCleanRequest = false;      // Reset request flag
    } else {
      digitalWrite(MOTOR_PIN, LOW);
    }
  }

  delay(500);  // 500ms loop cycle
}
