# ⚡ STABILIZER-X // AUTONOMOUS SELF-LEVELING PLATFORM
### *Gesture-Controlled Terrain-Adaptive Robotic Carrier System*

```
╔══════════════════════════════════════════════════════════════════╗
║   ██╗  ██╗ ██████╗ ██████╗ ██╗███████╗ ██████╗ ███╗   ██╗██╗  ██╗║
║   ██║  ██║██╔═══██╗██╔══██╗██║╚══███╔╝██╔═══██╗████╗  ██║╚██╗██╔╝║
║   ███████║██║   ██║██████╔╝██║  ███╔╝ ██║   ██║██╔██╗ ██║ ╚███╔╝ ║
║   ██╔══██║██║   ██║██╔══██╗██║ ███╔╝  ██║   ██║██║╚██╗██║ ██╔██╗ ║
║   ██║  ██║╚██████╔╝██║  ██║██║███████╗╚██████╔╝██║ ╚████║██╔╝ ██╗║
║   ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝
╚══════════════════════════════════════════════════════════════════╝

  STATUS: [ ONLINE ]  IMU: [ ACTIVE ]  SERVOS: [ ARMED ]
```

---

## 🔩 SYSTEM OVERVIEW

**Horizon-X** is a real-time, IMU-driven self-leveling robotic platform mounted atop a hand-gesture-controlled ground vehicle. Regardless of the terrain the carrier traverses — gravel, ramps, debris, or uneven surfaces — the top platform maintains a **perfectly level horizontal plane** using a closed-loop complementary filter and dual-axis servo correction system.

The vehicle base is commanded wirelessly via hand gestures. The upper platform is mechanically **decoupled** from the base dynamics using an **Orthogonal Stacked-Stage mechanism**, actively counteracting roll and pitch disturbances in real time.

---

## 🏗️ MECHANICAL ARCHITECTURE: STACKED REVOLUTE STAGE

Unlike a traditional suspended gimbal, this project utilizes a **bottom-up pedestal design** to maximize payload stability and center of gravity:

*   **Base Stage (Roll Control):** The lower platform layer is mounted to the vehicle chassis and handles side-to-side compensation.
*   **Top Stage (Pitch Control):** Mounted directly onto the roll platform, this stage manages forward and backward tilt.
*   **Kinematic Advantage:** This serial configuration allows for higher structural integrity when carrying heavy equipment compared to lightweight suspended rings.

---

## 🧠 HOW IT WORKS

```
  [ HAND GESTURE ] ──► [ RF MODULE ] ──► [ BASE VEHICLE ]
                                                │
                                         ┌──── ▼ ────┐
                                         │  TERRAIN   │
                                         │ DISRUPTION │
                                         └──── ▼ ────┘
                                                │
                                         ┌──── ▼ ────────────────────┐
                                         │      ESP32 (MAIN MCU)     │
                                         │  ┌─────────────────────┐  │
                                         │  │    MPU-6050 IMU      │  │
                                         │  │   Accel + Gyro       │  │
                                         │  └────────┬────────────┘  │
                                         │           │               │
                                         │  Complementary Filter     │
                                         │  Roll + Pitch Fusion      │
                                         │           │               │
                                         │  ┌────────▼────────────┐  │
                                         │  │  4x PWM Servo Drive  │  │
                                         │  │  Stacked Stage Corr. │  │
                                         │  └─────────────────────┘  │
                                         └───────────────────────────┘
                                                │
                                         ┌──── ▼ ────┐
                                         │  PLATFORM  │
                                         │  STAYS ▲   │
                                         │  LEVEL  ✓  │
                                         └────────────┘
```

### Signal Processing Pipeline

| Stage | Process | Detail |
|---|---|---|
| **1. Sensing** | Raw IMU read | 16-bit accel + gyro at 100 Hz |
| **2. Fusion** | Complementary filter | 95% gyro + 5% accelerometer |
| **3. Smoothing** | Low-pass filter (EMA) | α = 0.1 on fused angle output |
| **4. Dead-band** | Noise rejection | ±3° threshold before corrections |
| **5. Mapping** | Angle → PWM | ±45° input → 40°–140° servo range |
| **6. Output** | Servo drive | Opposing pairs for roll and pitch |

---

## 🛠️ HARDWARE SPECIFICATIONS

```
┌─────────────────────────────────────────────────────────────┐
│                     SYSTEM HARDWARE MAP                     │
├─────────────────────┬───────────────────────────────────────┤
│ COMPONENT           │ SPECIFICATION                         │
├─────────────────────┼───────────────────────────────────────┤
│ MCU                 │ ESP32 (Dual-core 240 MHz, Wi-Fi/BT)   │
│ IMU                 │ MPU-6050 (6-DOF, I2C @ 400 kHz)      │
│ Stabilizer Servos   │ 4x MG90S / SG90 (GPIO 16,17,18,19)  │
│ Motor Driver        │ L298N / L293D Dual H-Bridge           │
│ Drive Motors        │ 4x DC TT Gear Motors                  │
│ Gesture Controller  │ ESP32 + MPU-6050 (Transmitter Unit)   │
│ RF Link             │ ESP-NOW / NRF24L01 / HC-12            │
│ Power               │ 7.4V 2S LiPo + 5V LDO for logic      │
│ Chassis             │ 4WD Acrylic / Aluminium Frame         │
│ Platform Mount      │ 3D Printed Stacked Stage Bracket      │
└─────────────────────┴───────────────────────────────────────┘
```

---

## ⚙️ TUNING PARAMETERS

Adjust these constants in the firmware to calibrate for your specific build:

```cpp
// ── Complementary Filter ──────────────────────────────────────────
// Higher = trust gyro more (smoother but drifts over time)
// Lower  = trust accelerometer more (noisier but self-corrects)
roll  = 0.95f * (roll  + gyroX * dt) + 0.05f * rollAcc;
pitch = 0.95f * (pitch + gyroY * dt) + 0.05f * pitchAcc;

// ── Dead Band ─────────────────────────────────────────────────────
// Prevents servo jitter in near-level conditions
if (fabsf(smoothRoll) < 3.0f) targetRoll = 90.0f;

// ── Servo Travel ──────────────────────────────────────────────────
// ±45° platform tilt maps to 40°–140° servo sweep (±50° from center)
targetRoll = 90.0f + (smoothRoll / 45.0f) * 50.0f;
```

---

## 🌍 REAL-WORLD APPLICATIONS

### 🏥 Medical & Emergency Response
Mobile field robots transport fragile diagnostic equipment over uneven disaster-zone terrain.

### 🎥 Broadcast & Cinematography
Professional camera rovers use this architecture to keep lenses perfectly level during outdoor shoots.

### 🔬 Scientific Field Robotics
Geological survey rovers carry sensitive instruments that demand sub-degree level accuracy.

### 🚜 Precision Agriculture
Agricultural robots keep sprayers perpendicular to gravity regardless of chassis tilt.

---

## 📁 PROJECT STRUCTURE

```
horizon-x/
├── src/
│   └── stabilizer_main.ino      ← Main stabilizer firmware
    └── gesture_transmitter.in

├── docs/
│   ├── wiring_diagram.png
│   └── stacked_stage_mount.stl
├── README.md

```

---



---

```
╔══════════════════════════════════════════════════════════╗
║          STABILIZER-X // END OF DOCUMENTATION           ║
║    [ PLATFORM ARMED ] [ IMU LOCKED ] [ SERVOS READY ]   ║
╚══════════════════════════════════════════════════════════╝
```
