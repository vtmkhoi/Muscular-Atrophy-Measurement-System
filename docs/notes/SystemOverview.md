# SystemOverview – Muscular Atrophy Measurement System (MAMS)

> **Project type:** Research prototype — portable, non-invasive, multi-parameter muscle assessment device.
> **Status:** Active hardware development. EIM module is the primary focus; NIRS and Ultrasound are TBD/planned.
> **⚠️ Disclaimer:** This is **not yet a certified medical device**. Any human-subject testing must follow IEC 60601-style safety review, current limiting, electrical isolation, and supervisor approval.

---

## 1. Project Scope & Mission

MAMS is a handheld/portable device that combines three complementary sensing modalities to assess **skeletal muscle health**, **muscle atrophy**, **fibrosis/tissue integrity**, and **neuromuscular disease progression**.

The three modalities are designed to be **complementary**, each capturing a different physical aspect:

| Modality | Physical principle | What it measures |
|---|---|---|
| **EIM** (Electrical Impedance Myography) | AC current/voltage in tissue at MHz | Resistance R, Reactance X, Phase θ, |Z| → cell membrane integrity, anisotropy, fibrosis |
| **NIRS** (Near-Infrared Spectroscopy) | Optical absorption at 700–900 nm | Hemoglobin/deoxyhemoglobin → SmO₂ (muscle oxygen saturation), tissue composition |
| **Ultrasound** | Acoustic pulse-echo | Muscle thickness, cross-section, fat layer, structural geometry |

### Clinical rationale
- EIM phase angle and reactance correlate with muscle cell membrane integrity (decreases in ALS, DMD, sarcopenia)
- NIRS provides functional info on oxygen delivery/consumption (active vs atrophic muscle)
- Ultrasound gives geometric ground truth (skin-fat thickness affects EIM, must be corrected)

> [!important] Combined value
> Skin-fat thickness (from US) is needed to **normalize EIM readings** because subcutaneous fat thickness has been shown to significantly affect EIM parameters. NIRS and EIM together give both **structural** (membrane) and **functional** (perfusion) muscle information.

---

## 2. High-Level System Architecture

```
                    ┌───────────────────────────────────────────────┐
                    │                MAMS Device                    │
                    │                                               │
                    │  ┌────────────┐ ┌────────────┐ ┌────────────┐ │
                    │  │   EIM      │ │   NIRS     │ │ Ultrasound │ │
                    │  │  Module    │ │  Module    │ │  Module    │ │
                    │  │ (active)   │ │   (TBD)    │ │   (TBD)    │ │
                    │  └─────┬──────┘ └─────┬──────┘ └─────┬──────┘ │
                    │        │              │              │        │
                    │        └──────────────┼──────────────┘        │
                    │                       │                       │
                    │              ┌────────▼────────┐              │
                    │              │   MCU (TBD)     │              │
                    │              │ STM32 or ESP32  │              │
                    │              └────────┬────────┘              │
                    │                       │                       │
                    │   ┌───────────────────┼───────────────────┐   │
                    │   │                   │                   │   │
                    │ ┌─▼──┐  ┌────▼────┐  ┌────▼────┐  ┌────▼────┐ │
                    │ │BLE │  │  WiFi   │  │Ethernet │  │ Display │ │
                    │ │5.0 │  │ /Server │  │  /PoE   │  │  /USB   │ │
                    │ └────┘  └─────────┘  └─────────┘  └─────────┘ │
                    └───────────────────────────────────────────────┘
                                          │
                                          ▼
                              Server / Mobile App / PC
```

---

## 3. Target Specifications

### 3.1 EIM Module (confirmed targets)

| Parameter | Target | Source / rationale |
|---|---|---|
| Operating frequency | **3 MHz** | β-dispersion region; deep cellular info |
| Injection current range | 100 µA – 1 mA | Medical safety; SNR optimization |
| **Design point** | **500 µA peak** (= 353.6 µA rms) at 3 MHz | Trade-off SNR vs safety margin |
| Output voltage (DDS) | 2 Vpp | AD9834 reference design |
| Tissue impedance range | ~50 Ω – 5 kΩ | Typical skin/muscle; localized measurement |
| Phase resolution | < 0.5° | Required for clinical EIM phase angle |
| Magnitude accuracy | < 1% | Required for atrophy progression tracking |
| Calibration | Open / short / known load (RC networks) | Standard practice |
| Electrode configuration | **Tetrapolar (4-electrode)** | Eliminates electrode-skin impedance error |

**Safety check (IEC 60601-1, Type BF):**
- Allowed leakage current at 3 MHz = `f_kHz × 10 µA = 3000 × 10 = 30 mA rms`
- Design current 353.6 µA rms = **1.18% of limit** → very safe with large engineering margin
- Allowed patient auxiliary current at 3 MHz: similar derating, well within bounds

### 3.2 NIRS Module (TBD — proposed targets)

| Parameter | Proposed | Status |
|---|---|---|
| Wavelengths | 740 nm + 850 nm (min); 760/810/850 nm (better) | TBD |
| Geometry | Reflectance, source-detector distance 2–4 cm | TBD |
| Modulation frequency | 1–10 kHz (lock-in detection) | TBD |
| ADC resolution | ≥ 16-bit | TBD |
| Output | SmO₂ (%), HbO₂ trend, HHb trend | TBD |

### 3.3 Ultrasound Module (TBD — proposed targets)

| Parameter | Proposed | Status |
|---|---|---|
| Mode | A-mode (single-element) for thickness, or B-mode (linear array) for imaging | TBD |
| Frequency | 5–10 MHz (skin/fat/muscle layer resolution) | TBD |
| Transducer | Single-element 7.5 MHz (simple) or 64–128 element linear array (B-mode) | TBD |
| Pulse voltage | 50–200 V (high-voltage pulser) | TBD |
| Depth range | 0–5 cm (sufficient for limb muscles) | TBD |
| Output | Skin/fat thickness (mm), muscle thickness (mm), echogenicity index | TBD |

---

## 4. Medical Safety Standards

### 4.1 Primary applicable standard

**IEC 60601-1: Medical Electrical Equipment – Part 1: General Requirements for Basic Safety and Essential Performance**
- Defines patient leakage current, dielectric isolation, mechanical safety, software lifecycle (in conjunction with IEC 62304)
- **Type BF (Body Floating) classification** is the standard target for surface-electrode bioimpedance devices. Type BF requires patient circuit isolation from earth ground.

### 4.2 Relevant collateral & particular standards

| Standard | Topic | Relevance to MAMS |
|---|---|---|
| **IEC 60601-1** | General safety + essential performance | All modules (mandatory) |
| **IEC 60601-1-2** | EMC (electromagnetic compatibility) | All modules (3 MHz EIM is sensitive to EMI) |
| **IEC 60601-1-6** | Usability engineering | Handheld UX |
| **IEC 60601-2-40** | Electromyographs and evoked-response equipment | EIM (closest particular standard) |
| **IEC 60601-2-37** | Ultrasonic medical diagnostic and monitoring equipment | Ultrasound module |
| **IEC 62304** | Medical device software lifecycle | MCU firmware |
| **ISO 10993-1, -5, -10** | Biocompatibility (electrodes contact skin) | Electrodes (skin contact materials) |
| **ISO 14971** | Risk management | Whole device |
| **FCC Part 15 / CE radio** | Wireless emissions | BLE/WiFi |

### 4.3 Patient leakage current limits (IEC 60601-1, Type BF, Normal Condition)

| Frequency | Patient leakage (rms) | Patient auxiliary (rms) |
|---|---|---|
| DC – 1 kHz | 100 µA | 100 µA |
| 1 kHz – 100 kHz | f(kHz) × 100 µA, max 10 mA | f(kHz) × 100 µA, max 10 mA |
| > 100 kHz | 10 mA fixed | 10 mA fixed |

**For MAMS at 3 MHz:** The 10 mA rms ceiling applies. Design current (353.6 µA rms) is well below this.

### 4.4 Single-fault condition
The device must remain safe under single-fault conditions (e.g. one capacitor short, one op-amp output stuck). For VCCS, this typically means **AC-coupling to the patient electrodes** (DC-blocking capacitors on the current path) so a single op-amp DC failure cannot inject DC current into the body.

> [!warning] Implementation note
> The current MAMS VCCS design uses C_coupling = 47 nF in series with the current-injecting electrodes — this is the DC-blocking safety capacitor. Make sure this cap has high voltage rating (≥ 50 V) and X7R or C0G dielectric, and that there is **no DC path bypassing it**.

---

## 5. MCU Selection: STM32F411 vs ESP32 (with Ethernet considerations)

### 5.1 Critical clarification on Ethernet support

| MCU | Built-in Ethernet MAC? | PHY needed externally? | Notes |
|---|---|---|---|
| **STM32F411CE** | ❌ No | ❌ Cannot do Ethernet natively | Only via SPI Ethernet module (W5500) |
| STM32F407/F429/F446 | ✅ Yes (10/100 MAC) | ✅ Yes (LAN8720, KSZ8081) | Widely used for Ethernet-IoT |
| STM32F746/F767/H7 | ✅ Yes (10/100 MAC) | ✅ Yes | Higher performance, Ethernet+ DMA |
| **ESP32 (classic)** | ✅ Yes (10/100 MAC) | ✅ Yes (LAN8720, IP101, KSZ8041) | Built-in MAC + WiFi + BLE |
| **ESP32-S3** | ❌ No | ❌ Cannot do Ethernet natively | Only via SPI Ethernet module (W5500) — has WiFi + BLE 5.0 |
| ESP32-P4 | ✅ Yes | ✅ Yes | Newer, Ethernet + RISC-V |

> [!important] Conclusion on MCU
> If **Ethernet is mandatory**, neither STM32F411 nor ESP32-S3 has it built-in. Three viable paths:
> 1. **STM32F411 + W5500 SPI Ethernet module** — keeps F411, adds Ethernet via SPI (slower, ~10 Mbit usable)
> 2. **Upgrade MCU to STM32F407/F429** — built-in Ethernet MAC + external LAN8720 PHY
> 3. **ESP32 classic (not S3)** — built-in MAC + LAN8720 + WiFi + BLE in one chip ← **recommended for connectivity**
>
> If **Ethernet is optional / WiFi is enough** → **ESP32-S3 is excellent**: dual-core 240 MHz, 512 KB SRAM + 8 MB PSRAM, BLE 5.0, WiFi, USB native, AI vector instructions.

### 5.2 STM32F411CE vs ESP32-S3 — feature comparison

| Feature | STM32F411CE (LQFP100) | ESP32-S3-WROOM-1 |
|---|---|---|
| Core | ARM Cortex-M4F @ 100 MHz | Xtensa LX7 dual-core @ 240 MHz |
| Flash | 512 KB | 8 MB external (QSPI) |
| RAM | 128 KB SRAM | 512 KB SRAM + up to 8 MB PSRAM |
| FPU | Single-precision | Single-precision + vector |
| Internal ADC | 12-bit, 1 MSPS, **good linearity** | 12-bit SAR, **noisy & non-linear** (~9–10 ENOB) |
| SPI | 5× (up to 50 Mbit/s) | 4× (up to 80 MHz) |
| I²C | 3× | 2× |
| UART | 3× | 3× |
| USB | OTG FS (12 Mbit) | Native USB OTG + JTAG |
| WiFi | ❌ | ✅ 802.11 b/g/n |
| BLE | ❌ | ✅ BLE 5.0 |
| Ethernet | ❌ (need W5500) | ❌ (need W5500) |
| Power (active) | ~30 mA @ 100 MHz | ~120 mA WiFi active |
| Sleep | 4 µA standby | 7 µA deep sleep |
| Tooling | STM32CubeIDE, HAL/LL | ESP-IDF, Arduino, PlatformIO |

### 5.3 Recommendation matrix

| If priority is… | Recommended MCU |
|---|---|
| Lowest analog noise, best ADC | STM32F411 (with external 16-bit SPI ADC anyway) |
| WiFi/BLE built-in, single-chip | ESP32-S3 |
| Ethernet built-in | ESP32 classic OR STM32F407 |
| Existing prototype already on STM32 | STM32F411 + ESP32 module via UART (both worlds) |
| Lowest BOM cost | ESP32-S3 (replaces MCU + WiFi + BLE) |

> [!recommendation] Practical approach for MAMS
> **Hybrid: STM32F411 (master) + ESP32-S3 module (communication slave) over UART**
> - STM32F411 handles all timing-critical analog work: AD9834 SPI control, ADC sampling of AD8302 outputs, EIM signal-chain control
> - ESP32-S3 acts as communication co-processor: receives processed data from STM32 over UART, then transmits via WiFi/BLE
> - This decouples analog precision (STM32 ADC is cleaner) from RF noise (WiFi RF stays away from analog circuit)
> - If Ethernet is later required, add W5500 SPI Ethernet on the STM32 SPI bus

### 5.4 But — if user must use external 16-bit ADC anyway…
In practice, EIM at 3 MHz with phase resolution < 0.5° requires **better than 12-bit ADC** for the AD8302 VMAG/VPHS outputs (DC voltages, but small range needed). Recommended external ADCs:
- **ADS8881 (18-bit, 1 MSPS, SPI)** — excellent for VMAG/VPHS sampling
- **ADS1115 (16-bit, 860 SPS, I²C)** — slower but cheap, fine for slow DC readout
- **ADS131M04 (24-bit, simultaneous 4-channel)** — overkill but useful if also reading current sense

When using an external ADC, the **noise advantage of STM32 internal ADC disappears** — both MCUs can drive an external SPI ADC equally well. → This shifts the recommendation toward **ESP32-S3** for its built-in WiFi/BLE.

---

## 6. Pin Assignment Plan (preliminary)

### 6.1 EIM module pin count

| Block | Interface | Pins required |
|---|---|---|
| AD9834 (DDS) | SPI: SCLK, MOSI, FSYNC | 3 (shared SPI bus) |
| AD9834 control | RESET, FSELECT, PSELECT | 3 GPIO |
| AD8302 outputs | VMAG, VPHS (DC voltages) | 2 ADC analog OR 2 SPI ADC channels |
| External ADC (recommended) | SCLK, MISO, MOSI, CS, DRDY | 5 (shared SPI) |
| VCCS enable | EN/DIS | 1 GPIO |
| LED status / button | Various | 2–4 GPIO |
| **EIM total** | | **~12–15 pins** |

### 6.2 NIRS module pin count (estimate)

| Block | Interface | Pins required |
|---|---|---|
| LED drivers (2× wavelengths) | PWM or DAC + enable | 4 |
| Photodiode → TIA → ADC | SPI to 16-bit ADC | 4 (shared SPI) |
| Sync/timing | Timer trigger | 1 |
| **NIRS total** | | **~6–8 pins** |

### 6.3 Ultrasound module pin count (estimate, A-mode single element)

| Block | Interface | Pins required |
|---|---|---|
| HV pulser trigger | PWM or GPIO | 2 |
| T/R switch control | GPIO | 1 |
| LNA + VGA control | I²C or GPIO | 2 |
| Fast ADC (≥ 40 MSPS) | LVDS or parallel + clock | 8–16 |
| **Ultrasound total** | | **~12–18 pins** (A-mode) |
| **B-mode (array)** | | **far more — typically requires AFE chip like AFE5808A** |

### 6.4 Recommended STM32F411 pin assignment (LQFP100)

| Function | STM32F411 pin | Notes |
|---|---|---|
| AD9834 SPI1 SCK | PA5 | SPI1 |
| AD9834 SPI1 MOSI | PA7 | SPI1 |
| AD9834 FSYNC (CS) | PA4 | SPI1 NSS, manual control |
| AD9834 RESET | PB0 | GPIO |
| AD9834 FSELECT | PB1 | GPIO |
| AD9834 PSELECT | PB2 | GPIO |
| External ADC SPI2 SCK | PB13 | SPI2 |
| External ADC SPI2 MISO | PB14 | SPI2 |
| External ADC SPI2 MOSI | PB15 | SPI2 |
| External ADC CS | PB12 | SPI2 NSS |
| External ADC DRDY | PA0 | EXTI interrupt |
| AD8302 VMAG (backup direct read) | PA1 (ADC1_IN1) | If skip external ADC |
| AD8302 VPHS (backup direct read) | PA2 (ADC1_IN2) | If skip external ADC |
| VCCS enable | PC4 | GPIO |
| Status LED green | PC13 | GPIO |
| Button | PC14 | GPIO with EXTI |
| ESP32 UART TX | PA9 (USART1_TX) | UART1 |
| ESP32 UART RX | PA10 (USART1_RX) | UART1 |
| ESP32 reset | PC15 | GPIO |
| SWD debug | PA13 (SWDIO), PA14 (SWCLK) | Reserved |

> [!note] This is preliminary
> Final pin assignment depends on:
> - Whether external ADC is used (highly recommended)
> - PCB layout constraints (route SPI traces away from analog)
> - NIRS and Ultrasound modules added later
> - DMA channel availability (SPI1+ADC+SPI2 should be on different DMA streams)

---

## 7. Power Architecture Overview

```
USB 5V or Li-ion 3.7V (battery)
         │
         ├─► ADP2303 Buck → 5V system bus (digital, MCU, ESP32)
         │
         ├─► ADP5070 Boost+Inverter → ±5V raw
         │       │
         │       ├─► ADP7142 LDO → +5V_AVDD (clean analog: AD8014, AD8130, AD8066, AD8302)
         │       └─► ADP7182 LDO → −5V_AVDD (clean analog)
         │
         ├─► ADP7118 LDO → +3.3V_AVDD (AD9834 AVDD, AD8302 VPS)
         │
         └─► TPS7A2033 LDO → +3.3V digital (MCU, ESP32)
```

**Decoupling rules (for MHz-range analog):**
- Each IC supply pin: 100 nF C0G/NP0 (close, < 2 mm) + 1 µF X7R + 10 µF bulk
- Supply pour planes, not just traces
- Keep DGND and AGND on separate copper pours, joined at a single star point near the ADC

---

## 8. Module Independence & Inter-module Coupling

> [!tip] Design principle
> Each module (EIM, NIRS, US) should be **electrically independent** at the analog level — they share only:
> - Power supply (well-decoupled per module)
> - MCU SPI/I²C/UART (isolated by IC's chip-select)
> - Common ground reference (single-point AGND star)

Specifically avoid:
- EIM 3 MHz coupling into NIRS photodiode (use shielded photodiode head)
- Ultrasound HV pulse coupling into EIM front-end (separate ground returns)
- WiFi 2.4 GHz coupling into 3 MHz EIM (shield the analog section under a copper can if needed)

---

## 9. Calibration & Verification Strategy

### 9.1 Bench calibration (mandatory before any human testing)

| Test | Reference load | What it validates |
|---|---|---|
| Open circuit | No load | Verifies VCCS does not oscillate / saturate |
| Short circuit | 0 Ω wire | Verifies offset / minimum |
| Pure R | 100 Ω, 1 kΩ, 4.7 kΩ ±0.1% | Magnitude accuracy |
| Series RC | 1 kΩ + 1 nF, 1 kΩ + 10 nF | Phase accuracy at known impedance |
| Cole-Cole phantom | Banana, beef, agar phantom | Realistic biological-like load |

### 9.2 Calibration procedure
1. Measure each known load → record raw VMAG, VPHS
2. Compute calibration coefficients α₁, α₂, offsets per AD8302 datasheet equations
3. Store in MCU non-volatile memory
4. Apply correction in firmware before reporting Z

---

## 10. Development Roadmap & Status

| Phase | Status | Description |
|---|---|---|
| 1. EIM signal chain — schematic | 🟡 In progress | DDS + LPF + AD8130 + AD8014/AD8066 + VCCS + AD8302 — under review |
| 2. EIM PCB v1 | ⏳ Planned | Single-layer or 4-layer mixed-signal PCB |
| 3. EIM bench calibration | ⏳ Planned | Known R / RC loads |
| 4. EIM phantom test | ⏳ Planned | Agar/banana phantoms |
| 5. NIRS module — schematic | ⏳ Not started | Wavelength selection, TIA design |
| 6. Ultrasound module — schematic | ⏳ Not started | Decide A-mode vs B-mode |
| 7. MCU firmware integration | ⏳ Not started | RTOS, BLE/WiFi data pipeline |
| 8. Mechanical / handheld housing | ⏳ Not started | Probe ergonomics |
| 9. Pre-clinical safety validation | ⏳ Future | IEC 60601 review |
| 10. Human pilot testing | ⏳ Future | After supervisor + safety approval |

---

## 11. Risks & Open Issues

| Risk | Impact | Mitigation |
|---|---|---|
| 3 MHz EIM is sensitive to PCB parasitics | Reduced accuracy at high frequency | Tight layout, ground plane, short traces, controlled impedance |
| Cable & electrode parasitic capacitance | Phase error at 3 MHz | Use shielded cables, calibrate with electrodes attached |
| Skin-fat layer variability | EIM reading varies with subject | Use Ultrasound to normalize fat thickness |
| AD8302 cannot distinguish positive/negative phase | Sign ambiguity | Use known reference resistor for sign info, or use I/Q demodulation as alternative |
| ESP32 RF noise into EIM analog | SNR degradation | Place ESP32 antenna far from analog, use shielded section |
| Patient safety (any leakage > limit) | Critical safety hazard | DC-blocking caps mandatory, current-limit resistors, isolation review |
| AD8014/AD8066 stability at 3 MHz | Oscillation | Decouple supplies aggressively, follow datasheet feedback resistor |

---

## 12. Related Documents

- [[ModuleEIM]] — Detailed EIM hardware design
- [[ModuleNIRS]] — Detailed NIRS hardware design
- [[ModuleUltrasound]] — Detailed Ultrasound hardware design
- IEC 60601-1 (general safety standard)
- IEC 60601-2-40 (EMG/evoked response — closest particular standard for EIM)
- IEC 60601-2-37 (medical ultrasound)
- Project files referenced:
  - `EIM_Architecture_and_Calibration_of_a_Multichannel_Electrical_Impedance_Myographer.pdf`
  - `EIMA3MHzLowErrorAdaptive_Howland_Current_Source_for_high_frequency_bioimpedence_applications.pdf`
  - `EIM__Compar_Arms.pdf`
  - `Potential_Utility_of_Electrical_Impedance_Myography_in_Evaluating_AgeRelated_Skeletal_Muscle_Function_Deficits.pdf`
  - `NIRNearinfrared_spectroscopyderived_muscle_oxygen_saturation.pdf`

---

**Document version:** 1.0 — initial knowledge consolidation
**Last updated:** 2026-05-01