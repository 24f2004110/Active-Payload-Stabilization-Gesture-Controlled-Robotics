#include <Wire.h>
#include <MPU6050.h>
#include <RCSwitch.h>

MPU6050 mpu;
RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.begin(21, 22);
  mpu.initialize();

  mySwitch.enableTransmit(4);

  Serial.println("System Ready");
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  int command = 5; // default = STOP (never 0)

  // Dead zone
  if (abs(ax) < 3000 && abs(ay) < 3000) {
    command = 5; // STOP
  }
  else if (abs(ax) > abs(ay)) {
    if (ax > 6000) command = 1;       // RIGHT
    else if (ax < -6000) command = 2; // LEFT
  }
  else {
    if (ay > 6000) command = 3;       // FORWARD
    else if (ay < -6000) command = 4; // BACKWARD
  }


  if (command != 0) {
    mySwitch.send(command, 24);
  }

  Serial.print("Command: ");
  Serial.println(command);

  delay(200);
}