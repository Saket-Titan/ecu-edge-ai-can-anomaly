# Verification Report

**Project** AI-CAN Intrusion Detector
**Date:** 04-12-2025
**Target:** NUCLEO-H723ZG

# Method
TinyML Inference logic ('manual_inference.c') without hardware dependencies.
Utilized the **Zephyr Ztest** framework, validating the Autoencoder weights and
C-based math functions correctly distinguish between normal J1939 frames and anomaly inputs.

## Test Cases
|  Test ID  |   Description  |               Input Data               | Expected Result |  Status  |

| **TC-01** | Normal Traffic | '[00 00 00 2E E0 00 00 00]' (1500 RPM) |   Score < 0.1   | **PASS** |

| **TC-02** | Attack Traffic | '[00 12 55 AA BB CC DD EE]' (Noise)    |   Score > 0.1   | **PASS** |


## Execution Logs

*Captured from physical hardware (STM32H723ZG) via Serial Console.*
```
*** Booting Zephyr OS build v4.2.0-1599-g6558eca56cd3 ***
Running TESTSUITE anomaly_suite
===================================================================
START - test_attack_data
Attack Data Score: 419/1000 PASS - test_attack_data in 0.003 seconds
===================================================================
START - test_normal_data
Normal Data Score: 36/1000 
 PASS - test_normal_data in 0.003 seconds
===================================================================
TESTSUITE anomaly_suite succeeded

------ TESTSUITE SUMMARY START ------

SUITE PASS - 100.00% [anomaly_suite]: pass = 2, fail = 0, skip = 0, total = 2 duration = 0.006 seconds
 - PASS - [anomaly_suite.test_attack_data] duration = 0.003 seconds
 - PASS - [anomaly_suite.test_normal_data] duration = 0.003 seconds

------ TESTSUITE SUMMARY END ------

===================================================================
PROJECT EXECUTION SUCCESSFUL


 
