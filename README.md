# ☀️ IoT-Enabled Waterless Solar Panel Cleaning System
### Dual Mode Operation with Power Efficiency Monitoring

> **College Project** — SCSBDPROJ: Design Thinking and Innovations  
> Department of Electronics and Communication Engineering  
> Sathyabama Institute of Science and Technology, Chennai  
> **Duration:** November 2025 – April 2026

---

## 👥 Team

| Name | Roll Number |
|---|---|
| M. Thanuja Sri | 44130246 |
| Melvin Ancy. X | 44130272 |
| Nave Swety. M | 44130302 |

**Guide:** Dr. G.D. Anbarasi Jebaselvi, M.E., Ph.D.

---

## 📌 Problem Statement

Dust accumulation on solar panels reduces energy output significantly, especially in dry regions. Manual cleaning is labour-intensive, water-based cleaning wastes resources, and neither provides real-time monitoring. This project builds an automated, waterless, IoT-monitored solution.

---

## ⚙️ System Overview

The system detects dust using an optical sensor and automatically cleans the panel using a **servo-driven brush + air blower** — no water needed. A **web dashboard** hosted on the ESP32 allows real-time monitoring and remote control from any phone or laptop over Wi-Fi.

---

## 🔀 Dual Mode Operation

| Mode | How it works |
|---|---|
| **AUTO** | Cleaning triggers automatically when dust density exceeds threshold (0.10) |
| **MANUAL** | User presses "Start Cleaning" on the web dashboard |

Toggle between modes instantly from the web interface.

---

## 🌐 Web Dashboard

The ESP32 hosts a **responsive dark-themed web interface** accessible via local IP address:

- Live dust level, temperature, humidity, and power readings
- Mode indicator (AUTO / MANUAL) with colour-coded badge
- Toggle Mode button
- Start Cleaning button (available in MANUAL mode)
- Auto-refreshes every 2 seconds

### First Time Setup
On first use, the ESP32 creates a Wi-Fi hotspot **"Solar-clean"** (password: `setup1234`). Connect to it and enter your home/office Wi-Fi credentials. The system then connects automatically on every restart.

---

## 🔧 Hardware Components

| Component | Purpose |
|---|---|
| ESP32 WROOM | Main microcontroller + Wi-Fi hosting |
| Optical Dust Sensor | Detects dust accumulation on panel surface |
| DHT11 Sensor | Temperature & humidity monitoring |
| SG90 Servo Motor | Drives brush sweep mechanism (0°–180°) |
| 5V DC Blower Fan | Air-based dust removal |
| Adafruit INA219 | Power consumption monitoring |
| Li-ion Battery + Boost Converter | Portable 5V regulated power supply |
| USB Type-C Module | Battery charging interface |
| Push Button + LED | Manual trigger & system status indicator |

---

## 🗺️ Pin Configuration

| GPIO Pin | Component |
|---|---|
| GPIO 4 | DHT11 Data |
| GPIO 34 | Dust Sensor Analog Output (ADC) |
| GPIO 18 | Servo Motor PWM |
| GPIO 25 | Blower Fan (via transistor) |
| GPIO 14 | Dust Sensor LED Power |
| GPIO 19 | System Power ON LED |
| SDA/SCL | INA219 (I2C) |

---

## 📊 Performance Results

| Parameter | Value |
|---|---|
| Dust threshold | 0.10 |
| Servo sweep range | 0° – 180° |
| Cleaning cycle time | 4 – 6 seconds |
| System response time | 2 – 4 seconds |
| Dashboard refresh rate | ~2 seconds |
| Temperature range tested | 28°C – 34°C |
| Humidity range tested | 55% – 72% |

---

## 📚 Libraries Required

Install via Arduino Library Manager:
- `WiFi` — built into ESP32 Arduino core
- `WebServer` — built into ESP32 Arduino core
- `WiFiManager` — by tzapu
- `ESP32Servo` — by Kevin Harrington
- `Wire` — built in
- `Adafruit INA219` — by Adafruit
- `DHT sensor library` — by Adafruit

---

## 🛠️ How to Run

1. Install all libraries listed above in Arduino IDE
2. Select board: **ESP32 Dev Module**
3. Upload the code to your ESP32
4. On first boot, connect to Wi-Fi hotspot **"Solar-clean"**
5. Open browser → enter your Wi-Fi credentials
6. Find the ESP32's IP address from Serial Monitor
7. Open that IP in any browser to access the dashboard

---

## ⚠️ Known Limitations

- Dust sensor gives relative values (not calibrated mg/m³)
- INA219 power monitoring not fully calibrated in prototype
- Wi-Fi dependent — dashboard unavailable in offline areas
- Cleaning mechanism works best on loose/dry dust
- Prototype scale — servo sweep area limited to small panel

---

## 📄 Project Report

Full project report including literature survey, system design, block diagram, circuit configuration, performance analysis, and results is available in this repository.

---

## 📄 License

This project is open source and free to use for educational purposes.
