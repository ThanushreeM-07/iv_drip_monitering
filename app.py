import serial
import time
from flask import Flask, jsonify, render_template

app = Flask(__name__)

# --- Setup Serial ---
try:
    ser = serial.Serial('COM11', 9600, timeout=1)
    time.sleep(2)
    print("✅ Arduino Connected on COM11")
except Exception as e:
    ser = None
    print(f"❌ Serial Connection Failed: {e}")

# Updated data (added status)
data = {"volume": "0", "rate": "0", "time": "0", "status": "Waiting"}

def read_from_arduino():
    global ser
    if ser:
        try:
            for _ in range(10):
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()

                    # Ignore junk lines
                    if not line or "," not in line:
                        continue

                    print(f"📡 Arduino: {line}")

                    parts = line.split(",")

                    # 🔥 UPDATED: accept 3 or 4 values
                    if len(parts) >= 3:
                        data["volume"] = parts[0]
                        data["rate"] = parts[1]
                        data["time"] = parts[2]

                    if len(parts) >= 4:
                        data["status"] = parts[3]

                    break

                time.sleep(0.01)

        except Exception as e:
            print(f"❌ Serial Error: {e}")

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/data")
def get_data():
    read_from_arduino()
    return jsonify(data)

if __name__ == "__main__":
    app.run(debug=True, use_reloader=False)