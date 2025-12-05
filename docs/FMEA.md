# Failure Mode and Effects Analysis (FMEA)

**System:** Firmware Safety Mechanisms
**Reviewer:** Saket Kumar

| ID | Component | Failure Mode | Effect (Vehicle Level) | Safety Mechanism (Detection & Control) | Diagnostic Coverage | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **F-01** | **Engine MCU** | Software Hang / Infinite Loop | **SG-03 Violation:** Loss of control; frozen RPM output. | **Watchdog Timer (WDT):** Hardware watchdog resets MCU if not fed within 100ms. | High (99%) | Implemented |
| **F-02** | **Sensor Input** | Electrical Short (Stuck High) | **SG-01 Violation:** Unintended Acceleration (Reads 25,000 RPM). | **Plausibility Check:** Software logic rejects values > MAX_RPM (8000). Flags "Safety Violation". | High (99%) | Implemented |
| **F-03** | **CAN Bus** | Wire Break / Disconnect | **SG-02 Violation:** Body ECU receives no brake commands. | **Timeout Monitor:** Body ECU detects bus silence > 100ms and enters Safe State (Default OFF). | Medium (90%) | Implemented |
| **F-04** | **Firmware Image** | Malware Injection / Tampering | **Security Violation:** Malicious code execution overriding safety logic. | **Secure Boot:** MCUboot verifies RSA-2048 signature before booting. Rejects unsigned images. | High (99%) | Implemented |
| **F-05** | **CAN Bus** | Injection Attack (Spoofing) | **SG-01 Violation:** Hacker sends fake "High RPM" frames. | **AI Intrusion Detection:** Autoencoder detects data anomaly (Score > 0.1) and triggers alert. | High (95%) | Implemented |
