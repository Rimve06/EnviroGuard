# EnviroGuard
EnviroGuard is a smart IoT-based indoor environment and occupancy monitoring system that tracks air quality, temperature, humidity, lighting, noise, gas levels&amp;room occupancy in real time.It supports safety inspections,emergency evacuation by indicating occupied rooms &amp; continuous monitoring of homes, offices, classrooms, restaurants &amp;laboratories.


# 🌿 EnviroGuard 
### Smart Indoor Environment Monitoring & Occupancy Management System

EnviroGuard is an Arduino-based smart environmental monitoring system that continuously measures indoor environmental conditions, estimates room occupancy, evaluates overall comfort, detects potential hazards, and logs sensor data for analysis. A companion Python application records data to CSV files and sends email notifications whenever the room occupancy changes.

---

## 📌 Features

- 👥 Real-time people counting using dual ultrasonic sensors
- 🌡 Temperature monitoring
- 💧 Humidity monitoring
- 🔥 Smoke and combustible gas detection (MQ-2)
- ⛽ LPG/Natural gas detection (MQ-5)
- ☀ Ambient light monitoring (LDR)
- 🔊 Noise level monitoring
- 🚨 Fire/Smoke alarm with LED and buzzer
- 😊 Indoor comfort assessment
- 💡 Environmental suggestions based on sensor readings
- 📊 Automatic CSV data logging
- 📧 Email notifications on occupancy changes
- 📺 LCD interface with rotating information screens

---

# System Overview

The system continuously monitors multiple environmental parameters inside a room.

Two ultrasonic sensors determine whether a person is entering or leaving the room. Environmental sensors collect temperature, humidity, gas concentration, light intensity, and sound level. The Arduino processes these readings, evaluates room comfort, updates the LCD display, and activates alarms when necessary.

A Python application receives sensor data through serial communication, stores it in a CSV file, and sends email notifications whenever the occupancy changes.

---

# Hardware Components

| Component | Purpose |
|------------|---------|
| Arduino Uno | Main controller |
| LCD 16×2 | User display |
| HC-SR04 Ultrasonic Sensor ×2 | People counting |
| MQ-2 Gas Sensor | Smoke detection |
| MQ-5 Gas Sensor | LPG/Natural gas detection |
| HSM-20G Sensor | Temperature & Humidity |
| LDR | Ambient light sensing |
| Sound Sensor | Noise monitoring |
| Active Buzzer | Audible alerts |
| LED | Fire warning indicator |
| Breadboard & Jumper Wires | Circuit connections |

---

# Software Stack

- Arduino IDE
- C++
- Python 3
- PySerial
- CSV
- SMTP (Email Notifications)

---

# Functional Modules

## People Counting

- Dual ultrasonic sensors detect movement direction.
- Entry:
  - Sensor A → Sensor B
- Exit:
  - Sensor B → Sensor A
- Built-in debounce logic prevents false counts.

---

## Environmental Monitoring

The system measures:

- Temperature
- Humidity
- Smoke concentration
- LPG concentration
- Ambient light
- Noise level

Sensor values are refreshed periodically.

---

## Comfort Evaluation

The system computes an overall comfort level using:

- Temperature
- Humidity
- Gas concentration
- Noise
- Light intensity

Comfort categories include:

- Excellent
- Good
- Fair
- Poor
- Bad

Suggestions are displayed on the LCD when environmental conditions are outside the recommended range.

---

## Fire Detection

Fire detection is based on:

- MQ-2 sensor readings
- Consecutive confirmation logic

When smoke exceeds the threshold:

- Buzzer activates
- Fire LED turns on
- LCD displays emergency warning

---

## Data Logging

The Python application stores every sensor reading inside:

``
enviroguard_log.csv
```

Logged information includes:

- Timestamp
- People count
- Temperature
- Humidity
- MQ-2 values
- MQ-5 values
- Light level
- Sound level
- Comfort level
- Suggestions
- Sensor states

---

## Email Notifications

Whenever the occupancy changes, the Python application automatically sends an email containing:

- Previous occupancy
- Current occupancy
- Temperature
- Humidity
- Comfort status
- Suggestions

---

# Repository Structure

```
EnviroGuard/
│
├── Arduino/
│   └── EnviroGuard.ino
│
├── Python/
│   └── logger.py
│
├── Images/
│   ├── prototype.jpg
│   ├── lcd.jpg
│   ├── circuit.png
│   └── system_architecture.png
│
├── Report/
│   └── EnviroGuard_Report.pdf
│
├── README.md
└── LICENSE
```

---

# How to Run

## Arduino

1. Open the Arduino sketch.
2. Install the required libraries.
3. Upload the code to the Arduino Uno.
4. Open Serial Monitor (9600 baud) if needed.

---

## Python Logger

Install dependencies:

```bash
pip install pyserial
```

Update the configuration:

```python
PORT = "COM7"
EMAIL_FROM = "your_email@gmail.com"
EMAIL_PASSWORD = "your_app_password"
EMAIL_TO = "recipient@gmail.com"
```

Run:

```bash
python logger.py
```

---

# Applications

- Smart classrooms
- Offices
- Laboratories
- Libraries
- Smart homes
- Hostels
- Warehouses
- Indoor environmental monitoring
- Educational IoT projects

---

# Limitations

- Designed for single-person entry/exit detection.
- Ultrasonic counting accuracy decreases if multiple people pass simultaneously.
- MQ sensors require calibration and periodic recalibration.
- Email notifications require a connected computer with internet access.
- Monitoring is limited to a single room.
- Uses wired serial communication.
- Fire detection relies primarily on smoke concentration.

---

# Future Improvements

- ESP32/ESP8266 Wi-Fi connectivity
- Cloud database integration
- Web dashboard
- Mobile application
- SMS or push notifications
- More accurate environmental sensors
- AI-based occupancy prediction
- Multi-room monitoring
- Automatic HVAC control

---

# Sample Output

```
People:      3
Temp:        28.6 C
Humidity:    61.2 %

MQ2: Safe
MQ5: Safe

Comfort: Good

Suggestion:
Slightly Humid

---

# Authors

**Tasmin Rubaiyat Rimve**
**Sadman Sakib Mugdho**

Computer Science & Engineering

Khulna University of Engineering & Technology (KUET)

Bangladesh

---

If you find this project useful, feel free to ⭐ the repository.
