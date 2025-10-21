/*
  AGR12 Pressure Sensor (0–100 kPa) – Example for Arduino UNO (IDE 1.8.19)

  Datasheet (5 V supply):
  - 0 kPa   -> approx. 0.5 V
  - 100 kPa -> approx. 4.5 V
  => Linear range 0.5–4.5 V corresponds to 0–100 kPa
*/

const uint8_t PIN_SENSOR = A0;

// --- Sensor and ADC parameters ---
const float V_SUPPLY   = 5.0;     // ADC reference voltage (default = 5 V)
const int   ADC_MAX    = 1023;    // 10-bit ADC resolution
const float V_MIN      = 0.5;     // Sensor output voltage at 0 kPa (from datasheet)
const float V_MAX      = 4.5;     // Sensor output voltage at 100 kPa
const float FS_KPA     = 100.0;   // Full-scale pressure range: 0–100 kPa

// --- Measurement / filtering parameters ---
const uint16_t NSAMPLES = 50;     // Number of samples for averaging (reduces noise)
const uint32_t PERIOD_MS = 200;   // Output interval in milliseconds

// Zero-point offset (measured automatically at startup)
float vOffset = 0.0;
bool  offsetInitialized = false;

// Reads multiple samples and returns the averaged voltage
float readAveragedVoltage(uint8_t pin, uint16_t nSamples) {
  uint32_t acc = 0;
  for (uint16_t i = 0; i < nSamples; i++) {
    acc += analogRead(pin);
    delayMicroseconds(300); // Small delay between samples
  }
  float adc = acc / float(nSamples);
  return (adc / ADC_MAX) * V_SUPPLY;
}

// Converts sensor voltage to pressure in kPa
float voltageToPressureKpa(float v) {
  if (v < V_MIN) v = V_MIN;
  if (v > V_MAX) v = V_MAX;
  return ((v - V_MIN) / (V_MAX - V_MIN)) * FS_KPA;
}

void setup() {
  analogReference(DEFAULT); // Use 5 V as ADC reference
  Serial.begin(115200);
  delay(300);

  // Measure offset at ~0 kPa (open to atmosphere)
  const uint16_t N0 = 200;
  float accV = 0.0;
  for (uint16_t i = 0; i < N0; i++) {
    accV += readAveragedVoltage(PIN_SENSOR, 1);
    delayMicroseconds(200);
  }
  float v0 = accV / N0;
  vOffset = v0 - V_MIN;
  offsetInitialized = true;

  Serial.println(F("AGR12 Pressure Sensor – Initialization complete"));
  Serial.println(F("Columns: Raw(ADC)\tU[V]\tU_corrected[V]\tp[kPa]"));
}

void loop() {
  static uint32_t t0 = 0;
  if (millis() - t0 < PERIOD_MS) return;
  t0 = millis();

  // Read averaged sensor voltage
  float v = readAveragedVoltage(PIN_SENSOR, NSAMPLES);

  // Apply zero offset correction
  float v_corr = v - (offsetInitialized ? vOffset : 0.0);

  // Convert to pressure in kPa
  float p_kpa = voltageToPressureKpa(v_corr);

  // Display raw and processed data
  int raw = analogRead(PIN_SENSOR);
  Serial.print(raw); Serial.print('\t');
  Serial.print(v, 4); Serial.print('\t');
  Serial.print(v_corr, 4); Serial.print('\t');
  Serial.println(p_kpa, 2);
}