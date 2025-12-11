# ecu-edge-ai-can-anomaly
# Automotive Safety & Security ECU Prototype
**ISO 26262 Functional Safety | TinyML Intrusion Detection | Zephyr RTOS**

[![Build Firmware](https://github.com/Saket-Titan/ecu-edge-ai-can-anomaly/actions/workflows/ci.yml/badge.svg)](https://github.com/Saket-Titan/ecu-edge-ai-can-anomaly/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Executive Summary
This project demonstrates a production-grade automotive architecture integrating **Functional Safety (ASIL-B)** and **Cybersecurity (Edge AI)**.

The system consists of a distributed architecture with two ECUs communicating over **CAN-FD** using the **SAE J1939** protocol. It features a "Safety Node" (Engine ECU) that enforces real-time plausibility checks and a "Security Node" (Body ECU) that utilizes an on-device **Autoencoder Neural Network** to detect CAN bus anomalies in under 2ms.

## System Architecture

```mermaid
graph LR
    subgraph "Engine ECU (Safety Node)"
    A[STM32G474RE] -- "J1939 (PGN 61444)" --> CAN
    A -- "Watchdog & Plausibility" --> A
    end

    subgraph "CAN-FD Bus"
    CAN[MCP2562 Transceivers]
    end

    subgraph "Body ECU (Security Node)"
    CAN --> B[STM32H723ZG]
    B -- "TinyML Inference" --> AI[Anomaly Detector]
    AI -- "Score > 0.1" --> Alert[Safe State Alert]
    end
```
## Key Features
| Domain | Feature | Implementation Details |
| :--- | :--- | :--- |
| Functional Safety | Fault Detection | Implemented Watchdog Timer (WDT) and Data Plausability Check (Range/Gradient) to meet ASIL-B goals. |
| Networking | J1939 & UDS | Custom J1939 PGN construction and UDS Service 0x10 (Diagnostic Session Control) implementation. |
| AI/Security | TinyML IDS | Custom C-based Inference Engine running an Autoencode (76 parameters) to detect malicious CAN injection. |
| Lifecycle | Secure Boot | Implemented MCUboot chain-of-trust with RSA-2048 signed firmware and A/B patition OTA swapping |
| Architecture | Edge Computing | Designed edge-first architecture to process anomalies locally,reducing cloud bandwith dependencies |


## Hardware & Software Stack
 * Microcontrollers: STM32G474RE (Engine), STM32H723ZG (Body)
 * RTOS: Zephyr RTOS (v3.7+)
 * Transceivers: Microchip MCP2562FD
 * Languages: C (Firmware), Python (ML Training)
 * Tools: West, CMake, Keras/TensorFlow, Imgtool, Ztest, Github Actions

 ## Repository Structure
      ├── firmware/
      │   ├── ecu_engine/       # Project A: Safety-critical Engine Node
      │   ├── ecu_body/         # Project A: Body Control Node
      │   └── ids_node/         # Project B: AI Security Node (with MCUboot)
      ├── ml/                   # Python scripts for Autoencoder training
      ├── ota/                  # RSA Keys and Signing Scripts
      ├── docs/                 # Engineering Artifacts (HARA, FMEA, Verification Reports)
      ├── tests/                # Unit Tests (Ztest) for AI logic
      └── .github/workflows/    # CI/CD Pipeline configuration


## Verification & Results
  This Project focuses on verification-driven development. Key artifacts include:
    **1. Safety Analysis : **
        * [HARA](./docs/HARA.md) (Hazard Analysis): Identified top-level hazards (e.g., Unintended Acceleration).
        * FMEA (Failure Mode Analysis): Mapped software safety mechanisms to physical failures.
    **2. Testing:**
        * Unit Testing: AI Inference logic mathematically verified using Ztest. View Report
        * System Testing: Validated system stability via 2-hour Soak Test.
        * Security Testing: Verified Secure Boot and OTA ROllback via manual image swapping.
        
## Getting Started
  *1.Prerequisites
      * Install Zephyr SDK
      * Install Python dependencies: pip install -r ml/requirements.txt
  *2.Build the Safety Node (Engine) 
      ```west build -p always -b nucleo_g474re firmware/ecu_engine
         west flash
         ```
 *3.Build the Security Node(Body)
     Includes MCUboot and TinyML Model
     ```west build --sysbuild -p always -b nucleo_h723zg firmware/ids_node
        west flash
        ```
 *4. Run Unit Tests 
     ```west build -p always -b nucleo_h723xg test/unit/anomaly_score```

## Demo
  * Video Demonstration:
  * Live Metrics: System detects "Attack" traffic with a confidence score > 0.4 (Normal traffic < 0.1)

Author: Saket Kumar License: MIT
