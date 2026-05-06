#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>

MPU6050 mpu;

Servo servo1;  // Roll left
Servo servo2;  // Roll right (opposite)
Servo servo3;  // Pitch front
Servo servo4;  // Pitch back (opposite)

float roll  = 0.0f;
float pitch = 0.0f;

float smoothRoll  = 0.0f;
float smoothPitch = 0.0f;

float smoothAngleRoll  = 90.0f;
float smoothAnglePitch = 90.0f;

unsigned long prevTime = 0;
const unsigned long LOOP_INTERVAL_MS = 10;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  mpu.initialize();

  servo1.attach(18);  // Roll left
  servo2.attach(19);  // Roll right
  servo3.attach(12);  // Pitch front
  servo4.attach(14);  // Pitch back

  prevTime = millis();
  Serial.println("System Ready");
}

void loop() {
  // Non-blocking loop gate
  unsigned long now = millis();
  if (now - prevTime < LOOP_INTERVAL_MS) return;
  float dt = (now - prevTime) / 1000.0f;
  prevTime = now;

  // Read raw sensor data
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Convert to physical units
  float accX = ax / 16384.0f;
  float accY = ay / 16384.0f;
  float accZ = az / 16384.0f;
  float gyroX = gx / 131.0f;  // Roll rate
  float gyroY = gy / 131.0f;  // Pitch rate

  // ─────────────── ROLL ───────────────
  float rollAcc = atan2f(accY, accZ) * 180.0f / PI;
  roll = 0.95f * (roll + gyroX * dt) + 0.05f * rollAcc;

  // Smooth and clamp
  smoothRoll = 0.9f * smoothRoll + 0.1f * roll;
  smoothRoll = constrain(smoothRoll, -45.0f, 45.0f);

  // Float map
  float targetRoll;
  if (fabsf(smoothRoll) < 3.0f) {
    targetRoll = 90.0f;
  } else {
    targetRoll = 90.0f + (smoothRoll / 45.0f) * 50.0f;  // ±45° → 40–140
  }

  // Smooth angle
  smoothAngleRoll = 0.6f * smoothAngleRoll + 0.4f * targetRoll;
  int angleRoll1 = (int)roundf(smoothAngleRoll);
  int angleRoll2 = 180 - angleRoll1;

  // ─────────────── PITCH ───────────────
  float pitchAcc = atan2f(-accX, accZ) * 180.0f / PI;
  pitch = 0.95f * (pitch + gyroY * dt) + 0.05f * pitchAcc;

  // Smooth and clamp
  smoothPitch = 0.9f * smoothPitch + 0.1f * pitch;
  smoothPitch = constrain(smoothPitch, -45.0f, 45.0f);

  // Float map
  float targetPitch;
  if (fabsf(smoothPitch) < 3.0f) {
    targetPitch = 90.0f;
  } else {
    targetPitch = 90.0f + (smoothPitch / 45.0f) * 50.0f;  // ±45° → 40–140
  }

  // Smooth angle
  smoothAnglePitch = 0.6f * smoothAnglePitch + 0.4f * targetPitch;
  int anglePitch1 = (int)roundf(smoothAnglePitch);
  int anglePitch2 = 180 - anglePitch1;

  // ─────────────── WRITE SERVOS ───────────────
  servo1.write(angleRoll1);
  servo2.write(angleRoll2);
  servo3.write(anglePitch1);
  servo4.write(anglePitch2);

  // ─────────────── DEBUG ───────────────
  Serial.print("Roll: ");     Serial.print(roll, 2);
  Serial.print("  Pitch: ");  Serial.print(pitch, 2);
  Serial.print("  R1: ");     Serial.print(angleRoll1);
  Serial.print("  R2: ");     Serial.print(angleRoll2);
  Serial.print("  P1: ");     Serial.print(anglePitch1);
  Serial.print("  P2: ");     Serial.println(anglePitch2);
}