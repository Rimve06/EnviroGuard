import serial
import csv
from datetime import datetime

# ── CONFIG ──────────────────────────────────────────
PORT     = "COM7"    # change to your Arduino's COM port
BAUD     = 9600      # match your Serial.begin()
CSV_FILE = "enviroguard_log.csv"
# ────────────────────────────────────────────────────

# ── EMAIL CONFIG ─────────────────────────────────────
SEND_EMAIL        = True
EMAIL_FROM        = "abc@gmail.com"
EMAIL_PASSWORD    = "abc"  # NOT your real Gmail password
EMAIL_TO          = "abc@gmail.com"
# ─────────────────────────────────────────────────────

HEADER = [
    "timestamp",
    "people", "temp_c", "humidity_pct",
    "mq2_ppm", "mq2_raw", "mq2_state",
    "mq5_ppm", "mq5_raw", "mq5_state",
    "ldr_raw", "light_level",
    "sound_raw", "sound_level",
    "fire_confirm", "comfort", "suggestion",
    "count_state", "in_debounce",
    "dist_a_cm", "dist_b_cm"
]

def parse_block(lines):
    """Parse one printSerial() block into a dict."""
    data = {}
    for line in lines:
        line = line.strip()  # removes trailing spaces like "Safe    "
        if not line:
            continue
        
        if line.startswith("People:"):
            data["people"] = line.split()[-1]
        
        elif line.startswith("Temp:"):
            data["temp_c"] = line.split()[1]
        
        elif line.startswith("Humidity:"):
            data["humidity_pct"] = line.split()[1]
        
        elif line.startswith("MQ2 ppm:"):
            parts = line.split()
            # parts: ['MQ2', 'ppm:', '4', 'raw:484', 'Safe']
            data["mq2_ppm"]   = parts[2]
            data["mq2_raw"]   = parts[3].replace("raw:", "")
            data["mq2_state"] = " ".join(parts[4:])
        
        elif line.startswith("MQ5 ppm:"):
            parts = line.split()
            # parts: ['MQ5', 'ppm:', '0', 'raw:561', 'Safe']
            data["mq5_ppm"]   = parts[2]
            data["mq5_raw"]   = parts[3].replace("raw:", "")
            data["mq5_state"] = " ".join(parts[4:])
        
        elif line.startswith("LDR raw:"):
            parts = line.split()
            # parts: ['LDR', 'raw:', '951', 'Bright']
            data["ldr_raw"]     = parts[2]
            data["light_level"] = " ".join(parts[3:])
        
        elif line.startswith("Sound raw:"):
            parts = line.split()
            # parts: ['Sound', 'raw:', '950', 'Normal']
            data["sound_raw"]   = parts[2]
            data["sound_level"] = " ".join(parts[3:])
        
        elif line.startswith("FireConfirm:"):
            data["fire_confirm"] = line.split()[-1]
        
        elif line.startswith("Comfort:"):
            data["comfort"] = line.split(":", 1)[1].strip()
        
        elif line.startswith("Suggestion:"):
            data["suggestion"] = line.split(":", 1)[1].strip()
        
        elif line.startswith("Count state:"):
            data["count_state"] = line.split()[-1]
        
        elif line.startswith("In debounce:"):
            data["in_debounce"] = line.split()[-1]
        
        elif line.startswith("DistA:"):
            data["dist_a_cm"] = line.split()[-1]
        
        elif line.startswith("DistB:"):
            data["dist_b_cm"] = line.split()[-1]
    
    return data

import smtplib
from email.mime.text import MIMEText

def send_email(subject, body):
    if not SEND_EMAIL:
        return
    try:
        msg = MIMEText(body)
        msg["Subject"] = subject
        msg["From"]    = EMAIL_FROM
        msg["To"]      = EMAIL_TO

        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as smtp:
            smtp.login(EMAIL_FROM, EMAIL_PASSWORD)
            smtp.send_message(msg)
        print(f"  [EMAIL SENT] {subject}")
    except Exception as e:
        print(f"  [EMAIL FAILED] {e}")

def main():
    print(f"Opening {PORT} at {BAUD} baud...")
    ser = serial.Serial(PORT, BAUD, timeout=2)
    print(f"Logging to {CSV_FILE}  (Ctrl+C to stop)\n")

    # open CSV, write header only if file is new
    import os
    file_exists = os.path.isfile(CSV_FILE)
    csvfile = open(CSV_FILE, "a", newline="", encoding="utf-8")
    writer  = csv.DictWriter(csvfile, fieldnames=HEADER, extrasaction="ignore")
    if not file_exists:
        writer.writeheader()

    block = []
    inside = False
    last_people = [-1]  # using list so it's mutable inside the loop

    try:
        while True:
            raw = ser.readline()
            if not raw:
                continue
            line = raw.decode("utf-8", errors="ignore").strip()

            if "======" in line and not inside:
                inside = True
                block  = []
            elif "======" in line and inside:
                # end of block — parse and write
                parsed = parse_block(block)
                if parsed:
                    parsed["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    writer.writerow(parsed)
                    csvfile.flush()

                    # ── people change detection ──────────────────────
                    current_people = int(parsed.get("people", 0))
                    if current_people != last_people[0]:
                        direction = "increased" if current_people > last_people[0] else "decreased"
                        subject = f"EnviroGuard: People count {direction} → {current_people}"
                        body = (
                            f"People count changed at {parsed['timestamp']}\n\n"
                            f"  Previous : {last_people[0]}\n"
                            f"  Current  : {current_people}\n"
                            f"  Change   : {direction}\n\n"
                            f"  Temp     : {parsed.get('temp_c','?')} C\n"
                            f"  Humidity : {parsed.get('humidity_pct','?')} %\n"
                            f"  Comfort  : {parsed.get('comfort','?')}\n"
                            f"  Suggestion: {parsed.get('suggestion','?')}\n"
                        )
                        send_email(subject, body)
                        last_people[0] = current_people
                    # ─────────────────────────────────────────────────

                    print(f"[{parsed['timestamp']}] "
                          f"Temp={parsed.get('temp_c','?')}C  "
                          f"Humidity={parsed.get('humidity_pct','?')}%  "
                          f"People={parsed.get('people','?')}")
                inside = False
                block  = []
            elif inside:
                block.append(line)

    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        csvfile.close()
        ser.close()

if __name__ == "__main__":
    main()
