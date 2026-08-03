#include <LiquidCrystal.h>
#include <math.h>

// ENVIROGUARD v2 
// PIN MAP:
// D2  = ECHO_A (INT0)     D3  = ECHO_B (INT1)
// D4  = LCD RS            D5  = LCD EN
// D6  = LCD D4            D7  = LCD D5
// D8  = LCD D6            D9  = LCD D7
// D10 = TRIG_A            D11 = TRIG_B
// D12 = BUZZER            D13 = FIRE LED
// A0  = MQ2               A1  = MQ5
// A2  = LDR               A3  = SOUND
// A4  = HSM HUMIDITY      A5  = HSM TEMP


LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

#define TRIG_A          10
#define ECHO_A          2
#define TRIG_B          11
#define ECHO_B          3
#define BUZZER_PIN      12
#define FIRE_LED_PIN    13
#define MQ2_PIN         A0
#define MQ5_PIN         A1
#define LDR_PIN         A2
#define SOUND_PIN       A3
#define HSM_HUMID_PIN   A4
#define HSM_TEMP_PIN    A5


// PEOPLE COUNTING SETTINGS

#define DETECT_DIST_CM    60
#define MIN_DIST_CM       5
#define COUNT_WINDOW_MS   2000
#define DEBOUNCE_MS       2500


// MQ CALIBRATION

#define RL_OHM          10000.0
#define MQ2_CLEAN_RATIO 9.8
#define MQ5_CLEAN_RATIO 6.5
#define CALIB_SAMPLES   60
#define WARMUP_SEC      3

float mq2_R0 = 10000.0;
float mq5_R0 = 10000.0;

#define GAS_SAFE        300.0
#define GAS_MODERATE    600.0
#define GAS_POOR        1000.0
#define GAS_DANGER      2000.0

#define MQ2_FIRE_ON     1050
#define MQ2_FIRE_OFF    900


// LIGHT AND SOUND THRESHOLDS

#define LDR_DARK        300
#define LDR_BRIGHT      700
#define SND_QUIET       1000
#define SND_NORMAL      900


// INTERRUPT VARIABLES

volatile bool          echoA_received = false;
volatile bool          echoB_received = false;
volatile unsigned long echoA_time     = 0;
volatile unsigned long echoB_time     = 0;
volatile unsigned long echoA_start    = 0;
volatile unsigned long echoB_start    = 0;
volatile float         distA_cm       = 999.0;
volatile float         distB_cm       = 999.0;

void ISR_echoA() {
  if (digitalRead(ECHO_A) == HIGH) {
    echoA_start = micros();
  } else {
    unsigned long dur = micros() - echoA_start;
    distA_cm       = dur / 58.0;
    echoA_time     = millis();
    echoA_received = true;
  }
}

void ISR_echoB() {
  if (digitalRead(ECHO_B) == HIGH) {
    echoB_start = micros();
  } else {
    unsigned long dur = micros() - echoB_start;
    distB_cm       = dur / 58.0;
    echoB_time     = millis();
    echoB_received = true;
  }
}


// PEOPLE COUNTING STATE

byte          countState    = 0;
unsigned long firstTrigTime = 0;
bool          inDebounce    = false;
unsigned long debounceStart = 0;
byte          peopleCount   = 0;


// SENSOR DATA

float temperature  = 25.0;
float humidity     = 50.0;
float mq2_ppm      = 0.0;
float mq5_ppm      = 0.0;
int   mq2Raw       = 0;
int   mq5Raw       = 0;
int   ldrRaw       = 512;
int   soundRaw     = 0;

byte  mq2State        = 0;
byte  mq5State        = 0;
bool  fireDetected    = false;
bool  wasFireDetected = false;
byte  fireConfirmCount = 0;


// COMFORT

byte comfortBand = 0;
char comfortLabel[12] = "Excellent";
char suggestion[17]   = "All good!";


// TIMING

unsigned long lastTrigTime     = 0;
unsigned long lastEnvTime      = 0;
unsigned long lastScreenRotate = 0;
unsigned long lastBuzzerToggle = 0;
unsigned long lastSerialTime   = 0;
byte          lcdScreen        = 0;


// FIRE BOTH TRIGGERS SEQUENTIALLY — 30ms gap prevents
// ultrasonic cross-talk/interference between sensors

void fireBothTriggers() {
  // Fire sensor A
  echoA_received = false;
  digitalWrite(TRIG_A, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_A, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_A, LOW);

  delay(30); // wait for A's echo to complete before firing B

  // Fire sensor B
  echoB_received = false;
  digitalWrite(TRIG_B, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_B, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_B, LOW);
}


// SETUP

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_A,       OUTPUT);
  pinMode(ECHO_A,       INPUT);
  pinMode(TRIG_B,       OUTPUT);
  pinMode(ECHO_B,       INPUT);
  pinMode(BUZZER_PIN,   OUTPUT);
  pinMode(FIRE_LED_PIN, OUTPUT);

  digitalWrite(TRIG_A,       LOW);
  digitalWrite(TRIG_B,       LOW);
  digitalWrite(BUZZER_PIN,   LOW);
  digitalWrite(FIRE_LED_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(ECHO_A), ISR_echoA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ECHO_B), ISR_echoB, CHANGE);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("EnviroGuard v2  ");
  lcd.setCursor(0, 1);
  lcd.print("Warming up...   ");

  Serial.println("========================================");
  Serial.println("     EnviroGuard v2 Starting");
  Serial.println("========================================");
  Serial.print("Warm-up: ");
  Serial.print(WARMUP_SEC);
  Serial.println(" seconds");

  // Warmup countdown
  unsigned long wStart = millis();
  while (millis() - wStart < (unsigned long)WARMUP_SEC * 1000UL) {
    int remaining = WARMUP_SEC - (int)((millis() - wStart) / 1000);
    lcd.setCursor(0, 1);
    lcd.print("Wait: ");
    lcd.print(remaining);
    lcd.print("s   ");
    Serial.print("Warmup: ");
    Serial.print(remaining);
    Serial.println("s remaining");
    delay(1000);
  }

  // Calibrate MQ sensors
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...  ");
  lcd.setCursor(0, 1);
  lcd.print("MQ2 + MQ5       ");
  Serial.println("Calibrating MQ sensors in clean air...");

  float sum2 = 0.0, sum5 = 0.0;
  for (int i = 0; i < CALIB_SAMPLES; i++) {
    float v2 = analogRead(MQ2_PIN) * (5.0 / 1023.0);
    float v5 = analogRead(MQ5_PIN) * (5.0 / 1023.0);
    if (v2 < 0.01) v2 = 0.01;
    if (v5 < 0.01) v5 = 0.01;
    sum2 += RL_OHM * (5.0 - v2) / v2;
    sum5 += RL_OHM * (5.0 - v5) / v5;
    delay(100);
  }

  mq2_R0 = (sum2 / CALIB_SAMPLES) / MQ2_CLEAN_RATIO;
  mq5_R0 = (sum5 / CALIB_SAMPLES) / MQ5_CLEAN_RATIO;

  Serial.print("MQ2 R0 = "); Serial.println(mq2_R0, 1);
  Serial.print("MQ5 R0 = "); Serial.println(mq5_R0, 1);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EnviroGuard v2  ");
  lcd.setCursor(0, 1);
  lcd.print("Ready!          ");
  Serial.println("System ready.");
  delay(2000);
  lcd.clear();
}


// MAIN LOOP

void loop() {
  unsigned long now = millis();

  // Fire ultrasonics every 250ms (sequential = 30ms gap inside)
  if (now - lastTrigTime >= 250UL) {
    lastTrigTime = now;
    fireBothTriggers();
  }

  // People counting state machine — runs every loop
  runCountingStateMachine(now);

  // Reset debounce
  if (inDebounce && (now - debounceStart >= DEBOUNCE_MS)) {
    inDebounce     = false;
    countState     = 0;
    echoA_received = false;
    echoB_received = false;
  }

  // Environment sensors every 2 seconds
  if (now - lastEnvTime >= 2000UL) {
    lastEnvTime = now;
    readAllSensors();
    checkFireState();

    if (fireDetected) {
      handleFire();
      return;
    }

    if (wasFireDetected && !fireDetected) {
      wasFireDetected = false;
      digitalWrite(FIRE_LED_PIN, LOW);
      digitalWrite(BUZZER_PIN,   LOW);
      lcd.clear();
    }

    if (peopleCount > 0) {
      updateGasStates();
      computeComfort();
      updateLCD();
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("People: 0       ");
      lcd.setCursor(0, 1);
      lcd.print("Room Empty      ");
    }
  }

  // Rotate LCD every 4 seconds when occupied
  if (!fireDetected && peopleCount > 0) {
    if (now - lastScreenRotate >= 4000UL) {
      lastScreenRotate = now;
      lcdScreen = (lcdScreen + 1) % 4;
      updateLCD();
    }
    runBuzzer(now);
  }

  // Serial debug every 5 seconds
  if (now - lastSerialTime >= 5000UL) {
    lastSerialTime = now;
    printSerial();
  }
}


// PEOPLE COUNTING STATE MACHINE

void runCountingStateMachine(unsigned long now) {
  if (inDebounce) return;

  noInterrupts();
  bool  gotA = echoA_received;
  bool  gotB = echoB_received;
  float dA   = distA_cm;
  float dB   = distB_cm;
  unsigned long tA = echoA_time;
  unsigned long tB = echoB_time;
  interrupts();

  bool personA = gotA && (dA > MIN_DIST_CM) && (dA < DETECT_DIST_CM);
  bool personB = gotB && (dB > MIN_DIST_CM) && (dB < DETECT_DIST_CM);

  switch (countState) {

    case 0: // IDLE
      if (personA && !personB) {
        countState    = 1;
        firstTrigTime = tA;
        noInterrupts(); echoA_received = false; interrupts();
        Serial.println("[COUNT] A fired — waiting for B (entry)");
      }
      else if (personB && !personA) {
        countState    = 2;
        firstTrigTime = tB;
        noInterrupts(); echoB_received = false; interrupts();
        Serial.println("[COUNT] B fired — waiting for A (exit)");
      }
      else if (personA && personB) {
        noInterrupts();
        echoA_received = false;
        echoB_received = false;
        interrupts();
        Serial.println("[COUNT] Both simultaneously — discarded");
      }
      break;

    case 1: // Entry in progress — waiting for B
      if (personB) {
        peopleCount++;
        Serial.print("[ENTRY] People: ");
        Serial.println((int)peopleCount);
        showCountEvent("  >> Entered! <<");
        noInterrupts();
        echoA_received = false;
        echoB_received = false;
        interrupts();
        countState    = 0;
        inDebounce    = true;
        debounceStart = now;
      }
      else if (now - firstTrigTime > COUNT_WINDOW_MS) {
        Serial.println("[COUNT] Entry timeout — discarded");
        noInterrupts(); echoA_received = false; interrupts();
        countState = 0;
      }
      break;

    case 2: // Exit in progress — waiting for A
      if (personA) {
        if (peopleCount > 0) peopleCount--;
        Serial.print("[EXIT] People: ");
        Serial.println((int)peopleCount);
        showCountEvent(peopleCount == 0 ? "  Room Empty  " : "  << Exited!  ");
        noInterrupts();
        echoA_received = false;
        echoB_received = false;
        interrupts();
        countState    = 0;
        inDebounce    = true;
        debounceStart = now;
      }
      else if (now - firstTrigTime > COUNT_WINDOW_MS) {
        Serial.println("[COUNT] Exit timeout — discarded");
        noInterrupts(); echoB_received = false; interrupts();
        countState = 0;
      }
      break;
  }
}

void showCountEvent(const char* msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
  lcd.setCursor(0, 1);
  lcd.print("People: ");
  lcd.print((int)peopleCount);
  delay(1200);
  lcd.clear();
}


// READ ALL ENVIRONMENT SENSORS

void readAllSensors() {

  //  HSM-20G TEMPERATURE 
  float tempSum = 0.0;
  for (int i = 0; i < 10; i++) {
    int raw  = analogRead(HSM_TEMP_PIN);
    float v  = raw * (5.0 / 1023.0);
    if (v < 0.01) v = 0.01;
    if (v > 4.99) v = 4.99;
    float rt = 47000.0 * v / (5.0 - v);
    if (rt <= 0) rt = 1.0;
    float tK = 1.0 / ((1.0 / 298.15) + (1.0 / 4150.0) * log(rt / 47000.0));
    tempSum += tK - 273.15;
    delay(5);
  }
  temperature = constrain(tempSum / 10.0, -10.0, 60.0)+5;

  // HSM-20G HUMIDITY 
  float humSum = 0.0;
  for (int i = 0; i < 10; i++) {
    int raw  = analogRead(HSM_HUMID_PIN);
    float v  = raw * (5.0 / 1023.0);
    float rh = 32.65 * v - 14.49;
    humSum += rh;
    delay(5);
  }
  humidity = constrain(humSum / 10.0, 0.0, 100.0);

  // MQ-2 
  mq2Raw      = analogRead(MQ2_PIN);
  float v2    = mq2Raw * (5.0 / 1023.0);
  if (v2 < 0.01) v2 = 0.01;
  float rs2   = RL_OHM * (5.0 - v2) / v2;
  float ratio2 = rs2 / mq2_R0;
  mq2_ppm     = constrain(658.71 * pow(ratio2, -2.168), 0.0, 9999.0);

  //  MQ-5 
  mq5Raw      = analogRead(MQ5_PIN);
  float v5    = mq5Raw * (5.0 / 1023.0);
  if (v5 < 0.01) v5 = 0.01;
  float rs5   = RL_OHM * (5.0 - v5) / v5;
  float ratio5 = rs5 / mq5_R0;
  mq5_ppm     = constrain(1163.8 * pow(ratio5, -3.874), 0.0, 9999.0);

  // LDR 
  ldrRaw = analogRead(LDR_PIN);

  // --- SOUND — average over 25 samples ---
  int soundSum = 0;
  for (int i = 0; i < 25; i++) {
    soundSum += analogRead(SOUND_PIN);
    delay(2);
  }
  soundRaw = soundSum / 25;
}


// FIRE DETECTION — needs 3 consecutive high readings

void checkFireState() {
  if (mq2Raw >= MQ2_FIRE_ON) {
    fireConfirmCount++;
    if (fireConfirmCount >= 3) {
      fireDetected = true;
    }
  } else if (mq2Raw < MQ2_FIRE_OFF) {
    fireConfirmCount = 0;
    fireDetected     = false;
  }
}

void handleFire() {
  digitalWrite(FIRE_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN,   HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!! FIRE/SMOKE !!");
  lcd.setCursor(0, 1);
  lcd.print("EVACUATE NOW!   ");
  wasFireDetected = true;
  Serial.println("!!! FIRE / SMOKE DETECTED — EVACUATE !!!");
}


// GAS STATES

void updateGasStates() {
  mq2State = getPPMState(mq2_ppm);
  mq5State = getPPMState(mq5_ppm);
}

byte getPPMState(float ppm) {
  if (ppm < GAS_SAFE)     return 0;
  if (ppm < GAS_MODERATE) return 1;
  if (ppm < GAS_POOR)     return 2;
  return 3;
}

const char* gasLabel(byte s) {
  if (s == 0) return "Safe    ";
  if (s == 1) return "Moderate";
  if (s == 2) return "Poor    ";
  return "DANGER  ";
}


// LIGHT — FIXED: high ADC = bright (LDR pull-down circuit)

byte getLightLevel() {
  if (ldrRaw < LDR_DARK)   return 0;  // dark   — low ADC
  if (ldrRaw > LDR_BRIGHT) return 2;  // bright — high ADC
  return 1;                            // normal
}

const char* lightLabel(byte l) {
  if (l == 0) return "Dark  ";
  if (l == 1) return "Normal";
  return "Bright";
}


// SOUND

byte getSoundLevel() {
  if (soundRaw > SND_QUIET)  return 0;
  if (soundRaw > SND_NORMAL) return 1;
  return 2;
}

const char* soundLabel(byte s) {
  if (s == 0) return "Quiet ";
  if (s == 1) return "Normal";
  return "Loud! ";
}


// COMFORT BAND

void computeComfort() {
  float problems = 0.0;
  strcpy(suggestion, "");

  // Temperature
  if (temperature < 18.0 || temperature > 32.0) {
    problems += 1.0;
    strcpy(suggestion, temperature < 18.0 ? "Too cold!       " : "Too hot!        ");
  } else if (temperature < 20.0 || temperature > 26.0) {
    problems += 0.5;
    if (strlen(suggestion) == 0)
      strcpy(suggestion, temperature < 20.0 ? "Slightly cold   " : "Slightly warm   ");
  }

  // Humidity
  if (humidity < 25.0 || humidity > 75.0) {
    problems += 1.0;
    if (strlen(suggestion) == 0)
      strcpy(suggestion, humidity < 25.0 ? "Too dry!        " : "Too humid!      ");
  } else if (humidity < 40.0 || humidity > 60.0) {
    problems += 0.5;
    if (strlen(suggestion) == 0)
      strcpy(suggestion, humidity < 40.0 ? "Slightly dry    " : "Slightly humid  ");
  }

  // Gas
  byte worstGas = (mq2State > mq5State) ? mq2State : mq5State;
  if (worstGas == 3) {
    problems += 2.5;
    strcpy(suggestion, "GAS DANGER!     ");
  } else if (worstGas == 2) {
    problems += 1.5;
    if (strlen(suggestion) == 0) strcpy(suggestion, "Open window!    ");
  } else if (worstGas == 1) {
    problems += 0.5;
    if (strlen(suggestion) == 0) strcpy(suggestion, "Gas detected    ");
  }

  // Sound
  if (getSoundLevel() == 2) {
    problems += 0.5;
    if (strlen(suggestion) == 0) strcpy(suggestion, "Too noisy       ");
  }

  // Light
  byte ll = getLightLevel();
  if (ll == 0) {
    problems += 0.5;
    if (strlen(suggestion) == 0) strcpy(suggestion, "Too dark        ");
  } else if (ll == 2) {
    problems += 0.3;
    if (strlen(suggestion) == 0) strcpy(suggestion, "Too bright      ");
  }

  // Assign band
  if (problems < 0.5) {
    comfortBand = 0;
    strcpy(comfortLabel, "Excellent");
    if (strlen(suggestion) == 0) strcpy(suggestion, "All good!       ");
  } else if (problems < 1.5) {
    comfortBand = 1;
    strcpy(comfortLabel, "Good     ");
    if (strlen(suggestion) == 0) strcpy(suggestion, "All good        ");
  } else if (problems < 2.5) {
    comfortBand = 2;
    strcpy(comfortLabel, "Fair     ");
  } else if (problems < 4.0) {
    comfortBand = 3;
    strcpy(comfortLabel, "Poor     ");
  } else {
    comfortBand = 4;
    strcpy(comfortLabel, "Bad      ");
  }
}


// LCD UPDATE

void updateLCD() {
  if (fireDetected) return;
  lcd.clear();
  switch (lcdScreen) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Comfort:");
      lcd.print(comfortLabel);
      lcd.setCursor(0, 1);
      lcd.print(suggestion);
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(temperature, 1);
      lcd.print((char)223);
      lcd.print("C H:");
      lcd.print((int)humidity);
      lcd.print("%  ");
      lcd.setCursor(0, 1);
      lcd.print("People:");
      lcd.print((int)peopleCount);
      lcd.print("        ");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("MQ2:");
      lcd.print((int)mq2_ppm);
      lcd.print("ppm     ");
      lcd.setCursor(0, 1);
      lcd.print("MQ5:");
      lcd.print((int)mq5_ppm);
      lcd.print("ppm     ");
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Snd:");
      lcd.print(soundLabel(getSoundLevel()));
      lcd.print("        ");
      lcd.setCursor(0, 1);
      lcd.print("Lgt:");
      lcd.print(lightLabel(getLightLevel()));
      lcd.print("        ");
      break;
  }
}


// BUZZER

void runBuzzer(unsigned long now) {
  if (fireDetected) return;
  switch (comfortBand) {
    case 0:
    case 1:
      digitalWrite(BUZZER_PIN, LOW);
      break;
    case 2:
      if (now - lastBuzzerToggle >= 2000UL) {
        digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
        lastBuzzerToggle = now;
      }
      break;
    case 3:
      if (now - lastBuzzerToggle >= 500UL) {
        digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
        lastBuzzerToggle = now;
      }
      break;
    case 4:
      digitalWrite(BUZZER_PIN, HIGH);
      break;
  }
}


// SERIAL DEBUG

void printSerial() {
  Serial.println("========================================");
  Serial.print("People:      "); Serial.println((int)peopleCount);
  Serial.print("Temp:        "); Serial.print(temperature, 1); Serial.println(" C");
  Serial.print("Humidity:    "); Serial.print(humidity, 1);    Serial.println(" %");
  Serial.print("MQ2 ppm:     "); Serial.print((int)mq2_ppm);
  Serial.print("  raw:");       Serial.print(mq2Raw);
  Serial.print("  ");           Serial.println(gasLabel(mq2State));
  Serial.print("MQ5 ppm:     "); Serial.print((int)mq5_ppm);
  Serial.print("  raw:");       Serial.print(mq5Raw);
  Serial.print("  ");           Serial.println(gasLabel(mq5State));
  Serial.print("LDR raw:     "); Serial.print(ldrRaw);
  Serial.print("  ");           Serial.println(lightLabel(getLightLevel()));
  Serial.print("Sound raw:   "); Serial.print(soundRaw);
  Serial.print("  ");           Serial.println(soundLabel(getSoundLevel()));
  Serial.print("FireConfirm: "); Serial.println((int)fireConfirmCount);
  Serial.print("Comfort:     "); Serial.println(comfortLabel);
  Serial.print("Suggestion:  "); Serial.println(suggestion);
  Serial.print("Count state: "); Serial.println((int)countState);
  Serial.print("In debounce: "); Serial.println(inDebounce ? "YES" : "NO");
  Serial.print("DistA:       "); Serial.println(distA_cm);
  Serial.print("DistB:       "); Serial.println(distB_cm);
  Serial.println("========================================");
}
