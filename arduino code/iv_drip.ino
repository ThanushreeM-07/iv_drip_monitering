#include "HX711.h"

HX711 scale;
#define DOUT  A1
#define CLK   A0

float calibration_factor = 10500;
float maxWeight = 0;
float weight = 0;
float lastWeight = 0;
float intervalStartWeight = 0;
unsigned long intervalStartTime = 0;
int step = 1;
float dripRate_ml_min = 0;
float remainingTime = 0;
unsigned long stagnationTimer = 0;
String status = "SETUP";

void setup() {
  Serial.begin(9600);
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  // 1. Initial Prompt
  Serial.println("0.00,0.00,0.00,SETUP: PLACE BOTTLE");
  delay(5000); // 5s wait for user to place bottle

  // 2. Read Initial Weight (100%)
  maxWeight = scale.get_units(20);
  if (maxWeight < 5) maxWeight = 100; // Safety fallback
  
  lastWeight = maxWeight;
  intervalStartWeight = maxWeight;
  intervalStartTime = millis();
  stagnationTimer = millis();
  status = "NORMAL";
}

void loop() {
  weight = scale.get_units(10);
  if (weight < 0) weight = 0;
  unsigned long currentTime = millis();

  float volumePercent = (weight / maxWeight) * 100.0;
  if (volumePercent > 120 || volumePercent < 0) return;

  // -------- LOGIC FOR 5% DROPS --------
  // Check if current weight has dropped by another 5% segment
  float nextThreshold = maxWeight * (1.0 - (0.05 * step));

  if (weight <= nextThreshold && step <= 20) {
    float timeTakenSec = (currentTime - intervalStartTime) / 1000.0;
    float volumeLostPercent = ((intervalStartWeight - weight) / maxWeight) * 100.0;

    if (timeTakenSec > 1) {
      float ratePerSec = volumeLostPercent / timeTakenSec;
      dripRate_ml_min = ratePerSec * 60.0; 
      
      if (dripRate_ml_min > 0.01) {
        remainingTime = volumePercent / ratePerSec; // in seconds
      }
    }
    intervalStartWeight = weight;
    intervalStartTime = currentTime;
    step++;
  }

  // -------- STATUS CHECK --------
  if (weight > lastWeight + 1.0) {
    status = "BACKFLOW";
  } else if (abs(weight - lastWeight) < 0.05) {
    if (millis() - stagnationTimer > 10000) status = "NO FLOW / BLOCKED";
  } else {
    stagnationTimer = millis();
    if (volumePercent <= 10) status = "CRITICAL";
    else if (volumePercent <= 25) status = "LOW";
    else status = "NORMAL";
  }

  lastWeight = weight;

  // CSV Output: Volume%, Rate, TimeRemaining, Status
  Serial.print(volumePercent, 2);
  Serial.print(",");
  Serial.print(dripRate_ml_min, 2);
  Serial.print(",");
  Serial.print(remainingTime, 2);
  Serial.print(",");
  Serial.println(status);

  delay(2000);
}
