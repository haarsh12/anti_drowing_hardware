#include <Wire.h>
#include <MPU6050.h>
#include <MAX30100_PulseOximeter.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <LoRa.h>
#include <ESP32Servo.h>

MPU6050 mpu;
PulseOximeter pox;
TinyGPSPlus gps;

HardwareSerial GPSserial(1);
Servo myServo;

// Thresholds
float tiltThreshold = 40.0;
float bpmThreshold = 50;

void setup() {
  Serial.begin(115200);

  // I2C (STANDARD ESP32 PINS)
  Wire.begin(21, 22);

  mpu.initialize();
  Serial.println("MPU6050 Initialized");

  if (!pox.begin()) {
    Serial.println("MAX30100 FAILED");
  } else {
    Serial.println("MAX30100 READY");
  }

  // GPS UART
  GPSserial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

  //  LoRa SPI (STANDARD)
  SPI.begin(18, 19, 23, 5);
  LoRa.setPins(5, 14, 26);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Failed");
    while (1);
  }

  Serial.println("LoRa Initialized");

  // Servo setup
  myServo.setPeriodHertz(50);
  myServo.attach(27, 500, 2400);   // GPIO 27 works on ESP32
  myServo.write(0);
}

void loop() {
  delay(1);  // watchdog safe

  pox.update();

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float tilt = atan2(ay, az) * 180 / PI;
  float bpm = pox.getHeartRate();

  //  Limit GPS read (avoid blocking)
  int count = 0;
  while (GPSserial.available() && count < 50) {
    gps.encode(GPSserial.read());
    count++;
  }

  float lat = gps.location.isValid() ? gps.location.lat() : 0;
  float lng = gps.location.isValid() ? gps.location.lng() : 0;

  Serial.println("\n===== SENSOR DATA =====");
  Serial.print("AX: "); Serial.println(ax);
  Serial.print("AY: "); Serial.println(ay);
  Serial.print("AZ: "); Serial.println(az);
  Serial.print("Tilt: "); Serial.println(tilt);
  Serial.print("BPM: "); Serial.println(bpm);
  Serial.print("Lat: "); Serial.println(lat, 6);
  Serial.print("Lng: "); Serial.println(lng, 6);

  bool danger = false;

  if (abs(tilt) > tiltThreshold) {
    Serial.println("⚠️ Tilt abnormal!");
    danger = true;
  }

  if (bpm < bpmThreshold && bpm > 0) {
    Serial.println("⚠️ Low BPM!");
    danger = true;
  }

  //  Servo trigger (non-blocking style)
  if (danger) {
    Serial.println("🚨 SERVO ACTIVATED");
    myServo.write(180);

    unsigned long start = millis();
    while (millis() - start < 2000) {
      delay(10);
    }

    myServo.write(0);
  }

  // LoRa send
  LoRa.beginPacket();
  LoRa.print(danger);
  LoRa.print(",");
  LoRa.print(lat, 6);
  LoRa.print(",");
  LoRa.print(lng, 6);
  LoRa.endPacket();

  Serial.println("📡 Data Sent");

  delay(1000);
}