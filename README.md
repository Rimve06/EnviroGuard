# EnviroGuard
EnviroGuard is a smart IoT-based indoor environment and occupancy monitoring system that tracks air quality, temperature, humidity, lighting, noise, gas levels&amp;room occupancy in real time.It supports safety inspections,emergency evacuation by indicating occupied rooms &amp; continuous monitoring of homes, offices, classrooms, restaurants &amp;laboratories.


# 🌿 EnviroGuard 
### Smart Indoor Environment Monitoring & Occupancy Management System

EnviroGuard is an Arduino-based smart environmental monitoring system that continuously measures indoor environmental conditions, estimates room occupancy, evaluates overall comfort, detects potential hazards, and logs sensor data for analysis. A companion Python application records sensor data into CSV files and automatically sends email notifications whenever room occupancy changes.

The system combines multiple environmental sensors with occupancy detection to provide a complete overview of indoor conditions. During emergencies such as fire or gas leakage, occupancy information helps indicate whether a monitored room is occupied or has likely been evacuated, supporting emergency response. It can also be used as a portable environmental assessment tool for inspectors to quickly evaluate indoor conditions in restaurants, commercial kitchens, laboratories, offices, classrooms, and other workplaces.


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

Two ultrasonic sensors determine whether a person is entering or leaving the room. Environmental sensors collect temperature, humidity, gas concentration, light intensity, and sound level. The Arduino processes these readings, evaluates room comfort, updates the LCD display, activates alarms when necessary, and continuously tracks room occupancy.

A Python application receives sensor data through serial communication, stores every reading in a CSV file, and automatically sends email notifications whenever the occupancy changes.

During emergencies, the occupancy count can help indicate whether a monitored room is still occupied or has likely been evacuated. For inspections, the prototype provides a quick overview of environmental conditions such as gas leakage, smoke, temperature, humidity, lighting, and noise using a single portable device.

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

```
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
- Restaurant and commercial kitchen environmental assessment
- Consumer rights and safety inspections for evaluating indoor environmental conditions
- Workplace safety monitoring
- Educational IoT projects
- Emergency evacuation support by indicating whether a monitored room is occupied
- Disaster response support by providing occupancy information alongside environmental conditions
- Environmental data collection for research and analysis

---

# Limitations

- Designed primarily for single-person entry/exit detection.
- Ultrasonic counting accuracy decreases if multiple people pass simultaneously.
- Occupancy count estimates whether a monitored room is occupied but cannot confirm someone is trapped or determine their exact location.
- During emergencies, the system should be used only as a decision-support tool alongside professional rescue procedure.
- The system evaluates environmental conditions but cannot directly determine food hygiene,       cleanliness, or sanitation standards.
- MQ sensors require calibration and periodic recalibration.
- Email notifications require a connected computer with internet access.
- Monitoring is limited to the sensor coverage area.
- Uses wired serial communication.
- Fire detection primarily relies on smoke concentration.



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
# Notes
**The calibration values in given EnviroGuard.ino are given as per to the used sensor modules, Arduino board, and hardware configuration used during the development of this project. Hence different versions of sensors or arduino may not work properly under this given code. If you use different hardware, recalibration of the sensors and threshold values is recommended to ensure accurate performance. **
---

If you find this project useful, feel free to ⭐ the repository.
