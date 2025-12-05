# Hazard Analysis and Risk Assessment (HARA)
**Project:** edge-ai-can-anomaly
**Item Definition:** Secure Automotive Gateway System (Engine & Body Control)
**Standard:** ISO 26262

# 1. Item Definition
This is a distributed automotive control system consisting of an Engine Control Unit (ECU), 
a Body Control Unit , and a Gateway communication over CAN-FD. The system managers simulated torque Requested,
and critical body actuators (brake lights) while monitoring for cyber-physical threats.

# 2. Hazard Analysis
Analysed potential malfunctioning behaviors at the vehicle level.

| ID | Malfunctioning | Behaviour | Opertional Situation | Seerity (S) | Exposure (E) | Controllability (C) | ASIL |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **H-01** | **Unintended Acceleration** (Engine ECU sends max torque without input) | Highway Driving (>100 km/h) | **S3** (Life-threatening) | **E4** (High Probability) | **C3** (Difficult to control) | **ASIL D** |
| **H-02** | **Loss of Deceleration/Brake Light** (Body ECU fails to actuate light) | Heavy City Traffic | **S2** (Severe Injuries) | **E4** (High Probability) | **C2** (Normally Controllable) | **ASIL B** |
| **H-03** | **System Freeze / Communication Loss** (Gateway/Engine stops responding) | Overtaking Maneuver | **S3** (Life-threatening) | **E3** (Medium Probability) | **C3** (Difficult to control) | **ASIL D** |

* **S3:** Life-threatening injuries (survival uncertain).
* **E4:** Situation occurs >10% of drive time.
* **C3:** Average driver cannot control the outcome (requires expert level skill).

## 3. Safety Goals (SG)
  This is derived from ASIL ratings table, the system must meet these goals:

  * **SG-01 (ASIL D):** The system shall prevent unintended torque requests exceeding 10% of the driver demand.
  * **SG-02 (ASIL B):** The system shall indicate braking intent to following vehicles with >99% reliability.
  * **SG-03 (ASIL D):** The system shall detect a loss of communication or frozen ECU within **100ms** and transition to a Safe State.
    

