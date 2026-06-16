# ModuleEIM – Electrical Impedance Myography (Zero to Hero)

> **Module status:** Active development.
> **Target:** 3 MHz tetrapolar bioimpedance measurement, 100 µA – 1 mA injection current, design point 500 µA peak.
> **Goal of this doc:** End-to-end reference covering theory, signal-chain design, component-level engineering, and verification.

---

## Table of Contents

1. [What is EIM and why it works](#1-what-is-eim-and-why-it-works)
2. [Bioimpedance theory — first principles](#2-bioimpedance-theory--first-principles)
3. [Tetrapolar measurement principle](#3-tetrapolar-measurement-principle)
4. [System block diagram](#4-system-block-diagram)
5. [Block 1: DDS signal source (AD9834)](#5-block-1-dds-signal-source-ad9834)
6. [Block 2: Anti-imaging LPF](#6-block-2-anti-imaging-lpf)
7. [Block 3: Differential-to-single conversion (AD8130)](#7-block-3-differential-to-single-conversion-ad8130)
8. [Block 4: Buffer amplifier (AD8014 / AD8066)](#8-block-4-buffer-amplifier-ad8014--ad8066)
9. [Block 5: VCCS (Voltage-Controlled Current Source)](#9-block-5-vccs-voltage-controlled-current-source)
10. [Block 6: Tetrapolar electrodes](#10-block-6-tetrapolar-electrodes)
11. [Block 7: Voltage sense path (AD8065 + AD8130)](#11-block-7-voltage-sense-path-ad8065--ad8130)
12. [Block 8: Current sense path](#12-block-8-current-sense-path)
13. [Block 9: Gain/phase detector (AD8302)](#13-block-9-gainphase-detector-ad8302)
14. [Block 10: ADC and DSP](#14-block-10-adc-and-dsp)
15. [Signal chain budget — full numerical trace](#15-signal-chain-budget--full-numerical-trace)
16. [Calibration](#16-calibration)
17. [Layout & PCB practices for 3 MHz](#17-layout--pcb-practices-for-3-mhz)
18. [Common pitfalls & risks](#18-common-pitfalls--risks)

---

## 1. What is EIM and why it works

**Electrical Impedance Myography (EIM)** is a **non-invasive** technique that injects a weak high-frequency AC current into muscle tissue and measures the resulting voltage to derive complex impedance Z = R + jX.

The technique exploits the fact that **muscle tissue is not purely resistive**:
- Cell membranes act as capacitors (lipid bilayer)
- Intracellular and extracellular fluids act as resistors (ion conductivity)
- The frequency response of |Z|, R, X, θ reveals **cellular structure**

### Clinical utility
- **Phase angle θ** correlates with cell membrane integrity. Healthy muscle has higher phase angle; atrophic / fibrotic muscle has lower.
- **Reactance X** drops as muscle fibers are replaced by fat or connective tissue.
- **Anisotropy** (impedance along vs across fiber direction) is a fingerprint of muscle architecture.
- Used as biomarker in: ALS, Duchenne muscular dystrophy (DMD), spinal muscular atrophy (SMA), sarcopenia, peripheral nerve injury.

> [!warning] Medical claims
> EIM is a **research biomarker**, not a standalone diagnostic. Do not claim clinical diagnostic certainty unless validated against gold-standard methods (MRI, CT, biopsy) in your specific population.

---

## 2. Bioimpedance theory — first principles

### 2.1 Cell-level model

A biological cell can be modeled as:

```
        ┌─── R_e (extracellular fluid) ────┐
        │                                  │
   ●────┤                                  ├────●
        │   ┌── R_i (intracellular) ───┐   │
        │   │                          │   │
        └───┤        C_m (membrane)    ├───┘
            │                          │
            └──────────────────────────┘
```

**Equivalent impedance (single Cole element):**

```
Z(ω) = R_∞ + (R_0 - R_∞) / (1 + (jωτ)^α)
```

Where:
- `R_0` = impedance at DC (current bypasses cells, only extracellular path)
- `R_∞` = impedance at very high frequency (membranes look like short, current goes through both compartments)
- `τ = R·C` time constant of membrane
- `α` = Cole-Cole exponent (0 < α ≤ 1), describing distribution of relaxation times

### 2.2 Frequency dispersion regions

Biological tissue exhibits three classical dispersion regions (Schwan):

| Region | Frequency range | Physical mechanism |
|---|---|---|
| **α-dispersion** | 1 Hz – 10 kHz | Counter-ion polarization at membrane |
| **β-dispersion** | 10 kHz – 100 MHz | Cell membrane charging/discharging |
| **γ-dispersion** | > 100 MHz | Water dipole rotation |

> **MAMS targets 3 MHz → middle of β-dispersion** = best sensitivity to muscle membrane / cellular structure.

### 2.3 Why higher frequency is better (up to a point)

| Frequency | Behavior |
|---|---|
| Low (< 10 kHz) | Current bypasses cells → only extracellular info, dominated by skin-electrode impedance |
| Middle (10 kHz – 1 MHz) | β-dispersion onset, partial cell penetration |
| **High (1–10 MHz)** | Full cellular penetration, phase angle reveals membrane health |
| Very high (> 100 MHz) | Stray capacitance dominates, hard to measure accurately |

### 2.4 Measurement equation

Once injection current `I(ω)` and resulting voltage `V(ω)` are known as phasors:

```
Z(ω) = V(ω) / I(ω) = |Z|·e^(jθ)
|Z| = |V| / |I|       (magnitude)
θ = ∠V − ∠I            (phase)
R = |Z|·cos(θ)         (resistance, real part)
X = |Z|·sin(θ)         (reactance, imaginary part)
```

---

## 3. Tetrapolar measurement principle

### 3.1 Why two-electrode (bipolar) fails at low impedance

In a bipolar setup, the same two electrodes inject current AND measure voltage:

```
V_measured = I × (Z_electrode_1 + Z_tissue + Z_electrode_2)
```

The **electrode-skin contact impedance** can be 10 kΩ to several MΩ at low frequency — far larger than the tissue Z (~50–500 Ω). The measurement is dominated by electrode artifacts.

### 3.2 Tetrapolar (four-electrode) solution

Use **separate electrodes for current injection vs voltage sense**:

```
          ┌── I_inject ───┐                    ┌─── I_return ──┐
          │               │                    │               │
       [HC]            [HP]                 [LP]            [LC]
   (high current)  (high potential)    (low potential) (low current)
          │               │                    │               │
          └─►─────────────●────tissue──────────●──────────────►
                         V_sense+              V_sense−
```

- **HC, LC = current electrodes (outer)** — inject and return I
- **HP, LP = voltage electrodes (inner)** — measure V across tissue
- Voltage measurement uses **high-input-impedance amplifier** → almost zero current flows through HP/LP → electrode-skin Z does NOT cause voltage drop on the sense path

This **eliminates the dominant error source** at low frequencies and is the standard method for accurate bioimpedance.

### 3.3 Standard electrode arrangement (Sanchez et al., 2016)

Order along the muscle: **LC – LP – HP – HC**, equally spaced (typical d = 1–7 cm depending on muscle size).

### 3.4 Effect of electrode displacement
Per Sanchez 2016 (project file `EIM__Guildine_to_electrode_positioning_for_human.pdf`):
- Bringing current electrodes closer to potential electrodes → R and X **increase exponentially**
- Misplacement in the longitudinal axis affects readings far more than transverse
- For ICC ≥ 0.9 reproducibility, electrode placement precision should be ≤ 0.5 cm
- For small muscles (< 5 cm), use a **rigid metal electrode array** with fixed inter-electrode distance

---

## 4. System block diagram

```
                                  EIM SIGNAL CHAIN
                                  ────────────────

  ┌─────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
  │  AD9834 DDS │───►│ 7th-order    │───►│   AD8130     │───►│   AD8014     │
  │ (sine, 3MHz)│    │ Elliptic LPF │    │ Diff→Single  │    │ Buffer (CFA) │
  │  2 Vpp diff │    │  fc≈4.5MHz   │    │  G = 1 or 2  │    │  G = 1 or 3  │
  └─────────────┘    └──────────────┘    └──────────────┘    └──────┬───────┘
                                                                    │
                                                                    ▼
                                                       ┌──────────────────────┐
                                                       │     VCCS             │
                                                       │ (Improved Howland or │
                                                       │  AD8130-based)       │
                                                       │  500 µA peak out     │
                                                       └──────────┬───────────┘
                                                                  │
                              ┌───────────────────────────────────┴──────────┐
                              │                                              │
                              ▼ I_inject                                     ▼ I_return
                ┌──── HC ────────TETRAPOLAR ELECTRODES─────── LC ─────┐
                │       │                                  │          │
                │       ●── HP                       LP ──●           │
                │       │      [tissue (R+jX)]          │             │
                │       │                              │             │
                │       └─►V+    V−◄─┘                                │
                │          ▼                ▼                         │
                │   ┌────────────┐    ┌────────────┐                  │
                │   │  AD8065    │    │  AD8065    │                  │
                │   │ (high-Z    │    │ (high-Z    │                  │
                │   │  buffer)   │    │  buffer)   │                  │
                │   └─────┬──────┘    └──────┬─────┘                  │
                │         │                  │                        │
                │         ▼                  ▼                        │
                │      ┌─────────────────────┐                        │
                │      │   AD8130 V-sense    │     ┌────── R_sense ──┘
                │      │   (Diff→Single,     │     │
                │      │    G = 10)          │     ▼
                │      └────────┬────────────┘   ┌─────────────────┐
                │               │                │   AD8130 I-sense│
                │               │                │   (Diff→Single, │
                │               │                │    G = 5)       │
                │               │                └────────┬────────┘
                │               │                         │
                │               ▼                         ▼
                │       V_TISSUE_RAW              I_SENSE_RAW
                │       (≈ 50 mV rms)             (≈ 50 mV rms)
                │               │                         │
                │               ▼                         ▼
                │       ┌────────────┐            ┌────────────┐
                │       │  AD8066    │            │  AD8066    │
                │       │ (buffer    │            │ (buffer    │
                │       │  G = 1)    │            │  G = 1)    │
                │       └─────┬──────┘            └──────┬─────┘
                │             │                          │
                │             ▼ INPA                     ▼ INPB
                │       ┌─────────────────────────────────────┐
                │       │           AD8302 GPD                │
                │       │   VMAG = 30 mV/dB · 20·log(|VA|/|VB|)│
                │       │   VPHS = 10 mV/° · |θA − θB|         │
                │       └────────┬─────────────────┬──────────┘
                │                │                 │
                │                ▼                 ▼
                │            VMAG (DC)         VPHS (DC)
                │                │                 │
                │                ▼                 ▼
                │          ┌─────────────────────────┐
                └──────────►│  External 16-bit ADC    │──── SPI ───► MCU
                           │  (e.g. ADS8881)          │
                           └─────────────────────────┘
```

---

## 5. Block 1: DDS signal source (AD9834)

### 5.1 Why DDS over other generators?

| Method | Pros | Cons |
|---|---|---|
| Crystal oscillator | Low jitter | Fixed frequency, no sweep |
| LC oscillator | Cheap | Drift, hard to tune precisely |
| AD5933 (integrated AFE) | All-in-one | Limited to ~100 kHz, not suitable for 3 MHz |
| AD9833 | Cheap DDS | Lower MCLK (25 MHz), poorer SFDR at high freq |
| **AD9834** | 75 MHz MCLK, 28-bit phase, FSK/PSK, sweepable | Slightly more complex SPI |

> **AD9834 is the chosen DDS for MAMS** — its 75 MHz MCLK provides good Nyquist headroom at 3 MHz output and supports impedance spectroscopy applications (per ADI's bioimpedance app notes).

### 5.2 Key AD9834 parameters (from datasheet)

| Parameter | Value | Unit |
|---|---|---|
| Max MCLK | 75 | MHz |
| Output frequency range | 0 – 37.5 (Nyquist/2) | MHz |
| Phase resolution | 28-bit |  |
| DAC resolution | 10-bit |  |
| Output current full-scale (IOUT) | 3.0 | mA |
| Compliance voltage | 0.8 | V |
| VOUT max | 0.6 | V |
| Power | 20 mW @ 3V | |
| SFDR (narrowband) | -78 dBc (B grade) | |

### 5.3 Frequency tuning equation

```
f_out = (Δphase × f_MCLK) / 2^28
```

For f_out = 3 MHz, f_MCLK = 75 MHz:

```
Δphase = 3e6 × 2^28 / 75e6 = 10,737,418 (decimal)
```

This 28-bit value is loaded via SPI into FREQ0 or FREQ1 register.

### 5.4 Output amplitude

AD9834 outputs current. With R_SET = 6.8 kΩ and a load resistor R_LOAD = 200 Ω:

```
V_OUT_pk = 18 × (V_REF / R_SET) × R_LOAD = 18 × (1.18 / 6.8e3) × 200 = 0.625 V peak
V_OUT_pp ≈ 1.25 V (single-ended), or 2.5 Vpp differential
```

This matches the design target of **2 Vpp**.

### 5.5 SPI control sequence
1. Pulse RESET high
2. Write control register: bit 13 (B28=1) for 28-bit writes, bit 11 (FSEL=0), bit 10 (PSEL=0)
3. Write FREQ0 register: 14 LSBs first, then 14 MSBs (with FREQ0 address bits 14–15 = 01)
4. Optionally write PHASE0 register
5. Clear RESET to start output

### 5.6 Output is filtered AFTER DDS → see Block 2 (LPF)

> [!important] Why AD9834 needs LPF after it
> AD9834's DAC output contains:
> - Desired sine at f_out
> - **Image at (f_MCLK − f_out)** = 72 MHz (very high, easy to filter)
> - **DAC sin(x)/x roll-off** (~ -3 dB at f_MCLK/2)
> - Quantization harmonics
> The LPF must reject all of these while passing 3 MHz cleanly.

---

## 6. Block 2: Anti-imaging LPF

### 6.1 Why we need it

The DDS DAC produces a **stair-step approximation** of a sine. This generates:
- Spectral images at f_MCLK ± f_out, 2·f_MCLK ± f_out, etc.
- Quantization noise spread across DC – f_MCLK/2

For a clean sine drive into the VCCS, we need to **suppress everything above 3 MHz**.

### 6.2 Cutoff frequency selection

Common mistake: setting fc = f_signal = 3 MHz. This causes:
- ~3 dB attenuation at the signal frequency itself
- Phase shift of ~45° at fc (varies with filter type)

**Correct rule of thumb:**

```
fc = 1.5 × to 2 × f_signal  →  fc = 4.5 to 6 MHz for 3 MHz signal
```

This places the signal in the **flat passband** while still rejecting the first DDS image (at f_MCLK − f_out = 72 MHz, which is far in the stopband).

### 6.3 Filter type selection

| Type | Passband flat? | Stopband sharp? | Phase linear? | Ringing? |
|---|---|---|---|---|
| Butterworth | Flat | Moderate | No | Mild |
| Chebyshev I | Ripple | Sharp | No | Yes |
| **Elliptic (Cauer)** | Ripple | **Very sharp** | No | Yes |
| Bessel | Flat | Slow | **Yes** | None |

For DDS image rejection, **elliptic** is most efficient (highest order-per-dB rejection). Project notes mention a **7th-order elliptic filter at fc = 4.5 MHz** as the implementation choice.

### 6.4 Practical 7th-order elliptic LPF tips

- Use **C0G/NP0 capacitors** — X7R changes value vs voltage and frequency, ruining the filter response
- Use **1% tolerance** components or better (passband ripple is sensitive)
- LC ladder with high-Q inductors (Coilcraft 0805CS or similar)
- If specific cap values are unavailable, **parallel two close values** (e.g. 100 pF + 47 pF for 147 pF) — this doubles part count but improves precision

### 6.5 Active vs passive LPF

| Active (op-amp based) | Passive (LC) |
|---|---|
| No DC drift, easy to design | Needs precision Ls and Cs |
| Limited by op-amp GBW (need GBW > 50 × fc → 225 MHz min) | No active component noise |
| Adds gain | Lossy (insertion loss ~1–2 dB) |
| Sensitive to op-amp distortion at high freq | Distortion-free if linear components |

For 3 MHz with 4.5 MHz cutoff, **passive LC ladder is preferred** (matches what's seen in commercial designs and what the project is using).

### 6.6 Differential vs single-ended LPF

The AD9834 IOUT is single-ended (current to ground). Common topologies:
- **Single-ended LPF** then convert to single signal — simpler, but more sensitive to ground noise
- **Differential LPF using IOUT and IOUTB** — better PSRR and CMRR, this is what AD9834 evaluation board does

If using both IOUT and IOUTB → the next stage (AD8130) does diff-to-single conversion.

---

## 7. Block 3: Differential-to-single conversion (AD8130)

### 7.1 Why AD8130?

After the differential LPF, the signal needs to be converted to single-ended for the VCCS input. The AD8130 is a **high-speed differential receiver** with:

| Parameter | AD8130 |
|---|---|
| -3 dB BW | 270 MHz @ G = 1, 100 MHz @ G = 10 |
| Slew rate | 1090 V/µs |
| CMRR | 95 dB @ 100 kHz, 70 dB @ 10 MHz |
| Input impedance | High (FET inputs) |
| Power supply | ±5 V to ±12 V |
| Package | SOIC-8 |

### 7.2 Gain configuration

AD8130 is configured like a difference amplifier:

```
    V_OUT = (V+ − V−) × (1 + Rf/Rg)
```

For G = 1: leave Rg open (∞), keep Rf small (typically 499 Ω as datasheet recommends for stability)
For G = 2: Rf = Rg = 1 kΩ (this is what MAMS VCCS uses for U2)
For G = 5: Rf = 4 kΩ, Rg = 1 kΩ
For G = 10: Rf = 9 kΩ, Rg = 1 kΩ

> [!important] Critical pin connection
> **PD (Power-Down) pin** must be tied to **+VS (active high enables the part)**, NOT left floating. A floating PD pin can let the part power down or behave erratically. This was a previously-identified issue on U8 in the MAMS schematic.

### 7.3 Decoupling
Each AD8130 supply pin: 100 nF C0G + 10 µF X5R, placed within 2 mm of the pin. AD8130 has high BW and will oscillate without proper decoupling.

### 7.4 Reference pin (REF)
Pin 4 (REF) sets the output offset. Tie to AGND for ground-referenced output.

---

## 8. Block 4: Buffer amplifier (AD8014 / AD8066)

### 8.1 Purpose
Isolate the LPF output from the VCCS input, provide gain (if needed), drive the VCCS without loading the LPF.

### 8.2 AD8014 vs AD8066 — which to choose?

| Parameter | AD8014 (CFA) | AD8066 (VFA, dual) |
|---|---|---|
| -3 dB BW (G=+1) | 400 MHz | 145 MHz |
| Slew rate | 4000 V/µs | 180 V/µs |
| Input bias current | 15 µA (BJT) | 1 pA (FET) |
| Input voltage noise | 3.5 nV/√Hz | 7 nV/√Hz |
| Single or dual? | Single | **Dual** |
| THD @ 5 MHz | -70 dB | Per datasheet (verify) |
| Channel matching | N/A (single) | Excellent (single die) |
| Topology | Current-feedback | Voltage-feedback |

### 8.3 Why MAMS chose AD8066 over 2× AD8014

For the AD8302 input stage, **two matched buffers** are needed (one for INPA, one for INPB). Using a **dual VFA AD8066** has key advantages:

1. **Single die** → matched temperature drift, matched offsets, matched gain
2. **JFET inputs** → very low bias current → no DC offset issue, simplified bias network
3. **Phase tracking** → both channels see the same phase shift, which **cancels in AD8302's differential measurement**
4. **Fewer parts** → smaller PCB footprint, fewer decoupling caps needed

The CFA per-channel phase error of AD8014 at 3 MHz is irrelevant for AD8302 because the phase error tracks between channels.

### 8.4 Gain setting for VFA at G=+1 (AD8066)

For voltage-feedback op-amps at G = +1 (unity gain buffer):
- Rg = ∞ (open, not present)
- Rf = 0 (direct connection of OUT to −IN)

**This is different from CFA** where Rf is required even at G=+1 to set internal stability — for CFA op-amps like AD8014, datasheet usually specifies Rf ≈ 750 Ω at G=+1.

> [!warning] Earlier confusion clarified
> Saying "Rf = 499 Ω gives G = +1" is **imprecise**. G = +1 means Rg is absent. Rf has no role in setting gain at G=+1; it sets bandwidth and stability for CFAs only. For AD8066 (VFA), G=+1 buffer literally connects OUT → −IN with zero resistance.

### 8.5 AC coupling capacitors C64, C65
Between LPF output and AD8066 buffer inputs, AC coupling caps remove DC offset.

> [!critical] Capacitor dielectric
> Must be **C0G/NP0** dielectric, NOT X7R. X7R has voltage coefficient of capacitance (DC bias reduces capacitance by 30–50%) and large temperature drift. At 3 MHz with mV-level signals, X7R will introduce distortion. Use 22 nF or 47 nF C0G in 0603/0805.

### 8.6 Decoupling (CFA stability)
**For AD8014 CFA at 3 MHz, missing decoupling = oscillation.** Always:
- 100 nF C0G ≤ 2 mm from each supply pin
- 1 µF X5R close
- 10 µF tantalum or X5R bulk

This was previously flagged as a critical issue in the MAMS U10/U11 schematic.

---

## 9. Block 5: VCCS (Voltage-Controlled Current Source)

### 9.1 Why current source (not voltage source)?

In bioimpedance, current injection is preferred:
- Output current `I_inject` is fixed → measured tissue voltage `V_tissue` is directly proportional to Z
- Current limiting is straightforward → patient safety
- Voltage source would require knowing electrode impedance (variable) to compute current

### 9.2 Required VCCS specifications (for MAMS)

| Parameter | Target |
|---|---|
| Output current amplitude | 100 µA – 1 mA (programmable), design point 500 µA peak |
| Frequency range | DC – 3 MHz |
| Output impedance | > 1 MΩ at 3 MHz (or as high as practical) |
| Amplitude error vs frequency | < 1% |
| Amplitude error vs load (50 Ω – 5 kΩ) | < 1% |
| Common-mode voltage on load | Minimized (< 100 mV) |
| Compliance voltage | Sufficient to drive max load × max current |

### 9.3 VCCS topology comparison

#### A. Basic Howland Current Source (HCS)

```
        Rf
   ┌────/\/\────┐
   │            │
Vin────\/\─────┤+\
       R3      │  >─── V_out ──[R_sense]── Z_load ─┐
        ┌──────┤−/                                  │
        │      └───────────── R5 ──────────────────┘
        │      
       /\/\ R4
       /\/\ R1   (R1 || R4 to ground)
        │
       GND
       
Condition for ideal current source: R3/R4 = Rf/R5 (perfect ratio match)
I_out = Vin / R5
```

**Pros:** Simple, only one op-amp.
**Cons:** Output impedance degrades quickly above 100 kHz; very sensitive to resistor matching (1% mismatch → 100 dB output Z drop).

#### B. Enhanced Howland (EHCS) — most common in literature

Same as basic, but with R5 separate from feedback resistors → better output swing. Still grounded-load (ungrounded-load EHCS exists for biomedical safety).

#### C. Mirrored Enhanced Howland (MEHCS) — fully differential

Two op-amps mirrored top/bottom → load is differential, no common-mode current. Better for biomedical, but still degrades > 100 kHz.

#### D. Adaptive Howland Current Source (AHCS) — Nwokoye 2024 (project file)

```
Adds AGC (Automatic Gain Control) loop that monitors I_out and corrects amplitude error.
Achieves < 1% error up to 3 MHz at both 100 µA and 1 mA.
```

This is the **most relevant topology for MAMS** because the project's target (3 MHz, < 1% error, 100 µA – 1 mA) matches exactly.

#### E. AD8130-based current source (current MAMS implementation)

The MAMS schematic uses an **AD8130 differential receiver + AD8065 buffer** to form a VCCS:

```
V_in_filtered ──► AD8130 (G=2 with Rf=Rg=1kΩ) ──► AD8065 (unity buffer)
                                                         │
                                                         ▼
                                              [R_sense=2kΩ]
                                                    │
                                          ┌─────────┤
                                          │         │
                                          │      R_isolation = 47 Ω
                                          │         │
                                          │      C_coupling = 47 nF
                                          │         │
                                          │      Tetrapolar electrode (HC)
                                          │
                                  Voltage feedback to AD8130 −IN
```

Key principles:
- AD8130 senses the voltage across R_sense → applies feedback to maintain constant current
- R_sense converts voltage command to current: `I_out = V_in / R_sense`
- AD8065 buffer drives the load with low output impedance
- C_coupling (47 nF) provides DC isolation for patient safety
- R_isolation (47 Ω) limits output current under fault conditions

### 9.4 MAMS VCCS final values (from memory updates)

| Component | Value | Notes |
|---|---|---|
| R_sense | 2 kΩ | Sets I_out = V_in / R_sense |
| AD8130 G | 2 | Rf = Rg = 1 kΩ |
| AD8065 G | 1 | Unity buffer driving filter |
| R_isolation | 47 Ω | Series safety / current limit |
| C_coupling | 47 nF | DC blocking, patient safety |
| Supply | ±5 V or ±12 V | ±12 V gives more compliance |

### 9.5 Compliance check
With V_in = 2 Vpp = 1 V peak applied:
- I_out = 1 V / 2 kΩ = **500 µA peak** ✓ matches design point
- V across R_sense = 1 V peak
- V across tissue (assume 500 Ω) = 500 µA × 500 Ω = 0.25 V peak
- Total ≈ 1.25 V peak, well within AD8065 ±3.8 V output swing on ±5 V supply ✓

### 9.6 Common-mode voltage minimization
For tetrapolar measurement, the common-mode voltage at HP/LP electrodes should be near 0 V to minimize CMRR error in the voltage sense path. Strategies:
- **Symmetric drive:** push-pull current source (one op-amp sourcing, one sinking) → load floats, CM stays at 0
- **Driven right leg (DRL)** equivalent: feedback loop senses CM and drives a third electrode to cancel
- For MAMS, **AC coupling + R_isolation** + symmetric layout is the practical approach

---

## 10. Block 6: Tetrapolar electrodes

### 10.1 Electrode types

| Type | Pros | Cons | Use case |
|---|---|---|---|
| Ag/AgCl gel | Low contact Z, biocompatible | Single-use, dries out | Clinical trials |
| Stainless steel (rigid array) | Reusable, fixed spacing | Higher contact Z, needs gel | Handheld probe |
| Gold-plated (rigid) | Low Z, reusable | Expensive | Premium handheld |
| Microneedle array | Penetrates stratum corneum, low Z | Fragile, biocompat. unclear | Research |

For MAMS handheld probe, **rigid stainless steel or gold-plated electrode array** with conductive paste is standard.

### 10.2 Spacing for human limbs

From project literature (Sanchez 2016, Briko 2024):

| Application | Inter-electrode distance |
|---|---|
| Forearm muscles (small) | 7 mm between adjacent electrodes |
| Biceps, triceps | 1.5–3 cm |
| Quadriceps, calf | 3–7 cm |
| Whole-segment BIA | 50–100 cm |

For MAMS handheld measuring localized muscles, **7 mm to 3 cm spacing** is typical.

### 10.3 Skin preparation

To minimize electrode-skin Z and improve repeatability:
1. Clean skin with alcohol → removes oil, dead skin
2. Optionally exfoliate (light scrub) on stratum corneum
3. Apply electrode paste / gel
4. Apply consistent pressure on the probe (Briko 2024 used force sensors to control this)

### 10.4 Cable parasitic capacitance

At 3 MHz, every pF of cable capacitance matters:
- Coaxial cable typical: 100 pF/m
- For 30 cm cable: 30 pF
- At 3 MHz: |Xc| = 1 / (2π × 3e6 × 30e-12) = 1.77 kΩ → comparable to tissue Z!

**Mitigation:**
- Use **driven shield (active guarding)** — shield is driven by buffer to follow the inner conductor → cancels capacitance to shield
- Use **short cables** (ideally < 10 cm) or integrate amplifier into the probe head

---

## 11. Block 7: Voltage sense path (AD8065 + AD8130)

### 11.1 Purpose
Measure the differential voltage between HP and LP electrodes with **high input impedance** so almost no current flows through the sense electrodes.

### 11.2 Two-stage approach

```
HP ──► AD8065 (unity buffer, FET input, ZIN > 1 GΩ) ──┐
                                                       ├──► AD8130 (Diff→Single, G=10)
LP ──► AD8065 (unity buffer, FET input, ZIN > 1 GΩ) ──┘
```

**Why two stages?**
- AD8065 (FET-input op-amp) presents very high input impedance to the electrode
- AD8130 (high-CMRR diff amp) takes the difference and amplifies (G=10) to bring signal up

### 11.3 Why AD8065 for the buffer?

| Parameter | AD8065 |
|---|---|
| Input impedance | 10^12 Ω (FET) |
| -3 dB BW | 145 MHz @ G=1 |
| Slew rate | 180 V/µs |
| Input bias current | 6 pA typ |
| Voltage noise | 7 nV/√Hz |
| Package | SOIC-8 single, SOIC-8 dual (AD8066) |

The **FET input** is critical — BJT input op-amps have nA bias currents that flow through the high-impedance path (electrode-skin Z), causing voltage offsets and saturation.

### 11.4 Why G=10 on AD8130 voltage sense?

The expected V across tissue at I=500 µA peak, Z_tissue=50 Ω:
- V_tissue = 500 µA × 50 Ω = **25 mV peak** (or about 5 mV rms for low-Z tissue)

After AD8065 buffer (G=1): still 25 mV peak.
After AD8130 with G=10: 250 mV peak ≈ 50 mV rms.

This brings the signal up to a **comfortable level for AD8302 input** (which prefers 10 mV – 316 mV rms input range).

> [!note] Per memory updates, MAMS uses:
> - V-sense path: AD8065 (G=1) → AD8130 (G=10) → 50 mV rms at AD8302 INPA
> - I-sense path: 100 Ω shunt → AD8130 (G=5) → 50 mV rms at AD8302 INPB
> - Both inputs matched for AD8302 log-ratio operation

---

## 12. Block 8: Current sense path

### 12.1 Two methods to sense injection current

**Method A: Current-sensing resistor in series with the load**
```
VCCS_OUT ── R_sense=100Ω ── HC electrode
              ▲
              │
              └── voltage across R_sense → AD8130 (G=5) → AD8302 INPB
```
Voltage across R_sense = I_inject × R_sense (Ohm's law). At 500 µA × 100 Ω = 50 mV rms — feeds AD8302 INPB directly with proper gain.

**Method B: Sense across the VCCS internal R_sense (the 2 kΩ feedback resistor)**
The voltage across the VCCS's internal R_sense already equals V_in (by VCCS feedback), so reading V_in is the same as reading I_out. Less hardware but couples in DDS noise.

**MAMS uses Method A** with a separate sense resistor in series with the current-injecting electrode, which is more accurate and isolates measurement from drive.

### 12.2 R_sense considerations
- Must be **non-inductive** (use thin-film SMD, not wirewound)
- Tolerance: 0.1% or better for accuracy
- Power: 500 µA² × 100 Ω = 25 µW (negligible)
- Self-resonant frequency must be > 30 MHz (avoid above 3 MHz)

---

## 13. Block 9: Gain/Phase Detector (AD8302)

### 13.1 What AD8302 does

AD8302 is a **monolithic integrated circuit** that compares two AC input signals VA and VB and outputs:
- **VMAG** = DC voltage ∝ 20 × log10(|VA| / |VB|), scale = **30 mV/dB**
- **VPHS** = DC voltage ∝ phase difference ∠VA − ∠VB, scale = **−10 mV/°** (decreasing with phase)

### 13.2 Output equations

```
V_MAG = 30 mV/dB × 20·log10(|VA|/|VB|) + 900 mV
V_PHS = −10 mV/° × (|θA − θB| − 90°) + 900 mV
```

The 900 mV offset means:
- VMAG = 900 mV when |VA| = |VB| (0 dB ratio)
- VPHS = 900 mV when |θA − θB| = 90°

### 13.3 Solving for impedance

Given the MAMS architecture where INPA = V_tissue (after gain G_v) and INPB = V_R_sense (after gain G_i):

```
|VA|/|VB| = (G_v × V_tissue) / (G_i × V_R_sense)
         = (G_v × I × |Z|) / (G_i × I × R_sense)
         = (G_v / G_i) × (|Z| / R_sense)
```

So |Z| can be solved from VMAG by:
```
|Z| = R_sense × (G_i/G_v) × 10^((V_MAG − 900) / 600)
```

And θ from VPHS:
```
θ = θA − θB = ±(900 mV − V_PHS)/(10 mV/°) + 90°
```

### 13.4 Critical AD8302 limitations

> [!warning] Sign ambiguity
> AD8302 outputs |θA − θB| (absolute value). It **cannot distinguish positive from negative phase**. For bioimpedance where reactance can be negative (capacitive) or positive (inductive, rare), this is an issue.
>
> **Solutions:**
> - Use a known reference resistor in series → compare phase against it
> - Add a separate I/Q demodulator for sign info (more complex)
> - Use AD8302 phase output ±90° as a sign indicator (since reactance is dominantly capacitive in tissue)

> [!warning] Low-frequency limit
> AD8302 was designed for RF/IF (1 MHz – 2.7 GHz). Below ~20 kHz, output becomes inaccurate. **At 3 MHz, AD8302 is well within its operating range** — this is not an issue for MAMS.

### 13.5 Input dynamic range

| Input range | dBm (50 Ω) | mV peak |
|---|---|---|
| Min | −60 dBm | 316 µV |
| Max | 0 dBm | 316 mV |

> [!important] Designing for AD8302 input level
> Both INPA and INPB should be in the **10–316 mV peak range** for accurate operation. Below 10 mV peak, log accuracy degrades; above 316 mV, soft compression starts.
>
> **MAMS targets ~50 mV rms (~70 mV peak)** at both inputs → comfortably mid-range.

### 13.6 Pin configuration (TSSOP-14)

| Pin | Name | Connection |
|---|---|---|
| 1 | COMM | AGND |
| 2 | INPA | AC-coupled input from V-sense path |
| 3 | OFSA | 1 nF cap to AGND (offset filtering) |
| 4 | VPOS | +3 V or +5 V supply with decoupling |
| 5 | OFSB | 1 nF cap to AGND |
| 6 | INPB | AC-coupled input from I-sense path |
| 7 | COMM | AGND |
| 8 | PSET | Tie to VPHS (pin 9) for measurement mode |
| 9 | VPHS | Phase output (DC) → ADC |
| 10 | VREF | 1.8 V reference output, decouple with 10 nF |
| 11 | PFLT | Phase filter cap to AGND (typically 1 nF) |
| 12 | MFLT | Magnitude filter cap to AGND (typically 1 nF) |
| 13 | VMAG | Magnitude output (DC) → ADC |
| 14 | MSET | Tie to VMAG (pin 13) for measurement mode |

### 13.7 Decoupling
- VPOS: 100 nF C0G ≤ 2 mm + 10 µF bulk
- VREF: 10 nF C0G to AGND
- MFLT, PFLT: 1 nF C0G to AGND (sets averaging time constant ~1 µs)
- OFSA, OFSB: 1 nF C0G to AGND

---

## 14. Block 10: ADC and DSP

### 14.1 Why external ADC over MCU internal?

For VMAG and VPHS (DC levels with 0–1.8 V range, slow-changing), accuracy < 1% requires:
- ≥ 16-bit resolution (1.8 V / 65536 = 27 µV LSB → 0.001% of full-scale)
- Low noise (< 1 mV RMS effective)
- Linearity: < 1 LSB INL

**STM32F411 12-bit ADC**: 1.8 V / 4096 = 440 µV LSB → 0.024% — **insufficient for high-precision EIM**.

**Recommended external ADCs:**

| ADC | Resolution | SPS | Channels | Interface |
|---|---|---|---|---|
| **ADS8881** | 18-bit | 1 MSPS | 1 differential | SPI |
| **ADS8688** | 16-bit | 500 kSPS | 8 single-ended | SPI |
| **ADS131M04** | 24-bit | 32 kSPS | 4 simultaneous | SPI |
| **MCP3564** | 24-bit ΔΣ | 153 kSPS | 8 channels | SPI |

For VMAG/VPHS reading, **ADS8688 (16-bit, 8-ch)** is good fit — covers AD8302 outputs + headroom for future expansion.

### 14.2 Sampling strategy

For DC outputs of AD8302:
- Sample at ~1 kSPS per channel
- Average 100 samples → effective resolution improves by √100 = 10× → 19+ effective bits
- Output one Z measurement every 100 ms (10 Hz update)

### 14.3 Calibration in firmware

```c
// Pseudocode
float compute_Z_magnitude(uint16_t v_mag_raw) {
    float v_mag = adc_to_voltage(v_mag_raw);  // raw → mV
    float ratio_dB = (v_mag - 900.0) / 30.0;   // mV → dB
    float ratio = pow(10.0, ratio_dB / 20.0);  // dB → linear
    return R_SENSE * (G_I / G_V) * ratio;       // |Z| in ohms
}

float compute_phase_deg(uint16_t v_phs_raw) {
    float v_phs = adc_to_voltage(v_phs_raw);
    float theta = 90.0 - (v_phs - 900.0) / (-10.0);  // mV → degrees
    return theta;  // sign ambiguous!
}
```

Apply per-frequency calibration coefficients from the bench calibration step.

---

## 15. Signal chain budget — full numerical trace

This trace is from project memory updates; documents the end-to-end voltage levels at the design point.

| Stage | V level | Notes |
|---|---|---|
| AD9834 output (sine) | 600 mV peak ≈ 424 mV rms | After R_LOAD (200 Ω) on IOUT |
| After AD8130 diff-to-single (G=1) | 424 mV rms | (G=2 in some configurations → 848 mV rms) |
| After AD8014/AD8066 buffer (G=3) | 1.27 V rms | Boosts before VCCS |
| After LPF (insertion loss ~1.5 dB) | 1.07 V rms | Slight passband attenuation |
| VCCS output current | 100 µA rms (or 500 µA peak design point) | I = V_in / R_sense |
| **At tissue (Z=50 Ω, 100 µA rms)** | V_tissue = 5 mV rms | Across HP-LP electrodes |
| AD8065 V-sense buffer (G=1) | 5 mV rms | High-Z buffer |
| AD8130 V-sense (G=10) | 50 mV rms | → AD8302 INPA |
| Across R_sense (100 Ω, 100 µA rms) | 10 mV rms | I-sense voltage |
| AD8130 I-sense (G=5) | 50 mV rms | → AD8302 INPB |
| AD8302 VMAG | ≈ 900 mV (since |VA|=|VB|) | DC output |
| AD8302 VPHS | depends on θ | DC output |

> [!note]
> When tissue Z deviates from R_sense (after gain normalization), VMAG deviates from 900 mV. This is the actual measurement signal.

---

## 16. Calibration

### 16.1 Why calibrate

Real components have:
- Resistor tolerance (1% common, 0.1% precision)
- Op-amp offset, gain error
- Cable/PCB parasitics
- AD8302 ±0.5 dB and ±1° intrinsic error
- Frequency-dependent gain rolloff

Without calibration, |Z| error can be 10–20%. With proper calibration → < 1%.

### 16.2 Calibration procedure (recommended)

1. **Open** (no load): record VMAG, VPHS → these are noise floor / leakage
2. **Short** (0 Ω wire): record VMAG, VPHS → minimum |Z|
3. **Multiple known R loads** (100 Ω, 470 Ω, 1 kΩ, 4.7 kΩ ±0.1%): build |Z| → V_MAG curve, fit polynomial or LUT
4. **Series RC loads** (1 kΩ + 1 nF, 1 kΩ + 10 nF): characterize phase response at known angles
5. **Cole-Cole phantoms** (banana, agar+saline): verify with biological-like load

### 16.3 Three-point calibration formula (per Yang et al.)

If three measurements at known impedances Z₁, Z₂, Z₃ give VMAG values M₁, M₂, M₃:

```
|Z|_corrected = a × VMAG² + b × VMAG + c
```

Solve for a, b, c using the three known points.

For phase, similar polynomial in VPHS.

### 16.4 Per-frequency calibration

If sweeping multiple frequencies (impedance spectroscopy), calibrate at **each frequency** because:
- AD8302 magnitude/phase scale factors are frequency-dependent
- LPF insertion loss varies
- Cable capacitance becomes more dominant at higher f

Store a calibration table indexed by frequency in MCU non-volatile memory.

---

## 17. Layout & PCB practices for 3 MHz

### 17.1 General principles for MHz analog

- **Solid ground plane** under all analog signals — return current finds shortest path naturally
- **Star-point AGND/DGND junction** — connect at a single point, ideally at the ADC reference
- **Short traces** for high-frequency signals (DDS output, AD8302 inputs) — every cm of trace adds ~10 pF and a bit of inductance
- **Decoupling close to pins** — within 2 mm, on the same side as the IC if possible
- **Signal trace impedance** — for 3 MHz on FR4, λ/10 ≈ 5 m → trace impedance not critical, but parasitic capacitance to GND is

### 17.2 Specific layout rules for MAMS

| Signal | Layout rule |
|---|---|
| AD9834 IOUT trace | < 5 cm, away from digital | 
| LPF stage | Compact, all components on one side, GND pour underneath |
| AD8130 inputs | Symmetric trace lengths for + and − inputs |
| AD8014/AD8066 feedback | Tight loop, < 1 cm |
| VCCS output to electrodes | Shielded cable, length matched between current and voltage paths |
| AD8302 INPA/INPB traces | Equal length within 1 mm to preserve phase matching |
| AD8302 VMAG/VPHS to ADC | Add 100 Ω + 100 pF RC filter to remove residual RF |
| Power planes | Separate +5V_AVDD pour from +3.3V_DVDD pour, single bridge |

### 17.3 Test points (mandatory for bringup)

Add 2-pin headers / SMD pads for these signals:
- DDS raw output (before LPF)
- LPF output
- Buffer output
- VCCS V_in command
- Across R_sense (current sense voltage)
- HP and LP electrode signals
- AD8302 INPA, INPB
- AD8302 VMAG, VPHS
- All power rails (+5 V, −5 V, +3.3 V)

### 17.4 EMC and shielding

- WiFi/BLE 2.4 GHz section physically separated from analog (at least 5 cm)
- Optional copper can over the analog section (especially DDS, LPF, VCCS, AD8302)
- Antenna farthest possible from analog
- Ferrite beads on power lines entering analog section

---

## 18. Common pitfalls & risks

| Pitfall | Symptom | Fix |
|---|---|---|
| **Floating PD pin on AD8130** | Random output, instability | Tie PD to +VS |
| **Missing decoupling on CFA op-amp (AD8014)** | Oscillation at MHz, wandering DC | 100 nF C0G ≤ 2 mm from each supply pin |
| **X7R coupling caps in signal path** | Distortion, gain error | Use C0G/NP0 only |
| **750Ω + 50Ω attenuator copied from RF design** | Signal at AD8302 too small (< 10 mV) | Remove 750Ω, increase AD8130 gain to ~3 |
| **Setting LPF fc = f_signal** | 3 dB loss + 45° phase shift | Set fc = 1.5–2× f_signal |
| **Long unshielded electrode cables** | Phase error, capacitive loading | Use shielded coax, < 30 cm; consider driven shield |
| **Trying to use AD5933 for 3 MHz** | Limited to ~100 kHz | Don't — use AD9834 + discrete chain instead |
| **No calibration** | 10–20% Z error | Open/short/load calibration mandatory |
| **No DC blocking at electrodes** | Patient safety violation | C_coupling 47 nF in series with current electrode |
| **Sign ambiguity from AD8302** | Wrong reactance sign | Use known reference, or replace with I/Q demod |
| **Common-mode at electrodes high** | CMRR error in V-sense | Use symmetric current source, AC coupling |

---

## 19. Related Documents

- [[SystemOverview]]
- [[ModuleNIRS]]
- [[ModuleUltrasound]]
- Project files:
  - `AD9834.pdf` — DDS datasheet
  - `AD8129_8130.pdf` — Diff-to-single amp
  - `ad8014.pdf` — CFA buffer
  - `ad8065_8066.pdf` — Single/dual VFA buffer
  - `EIMA3MHzLowErrorAdaptive_Howland_Current_Source_for_high_frequency_bioimpedence_applications.pdf` — Modern high-freq VCCS
  - `EIM__four_Electrodes_implement.pdf` — Tetrapolar implementation
  - `EIM__Guildine_to_electrode_positioning_for_human.pdf` — Electrode placement guidance
  - `EIM__Design_and_preliminary_evaluation_of_a_portable_device_for_the_measurement_of_bioimpedance_spectroscopy.pdf` — Yang et al. AD8302-based device
  - `EIMElectrical_impedance_myography_at_frequencies_up_to_2_MHz.pdf` — High-freq EIM reference

---

**Document version:** 1.0 — initial knowledge consolidation
**Last updated:** 2026-05-01