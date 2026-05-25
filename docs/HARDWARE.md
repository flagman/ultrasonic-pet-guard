# Ultrasonic Pet Guard — v0.7 Hardware Specification

Final prototype form: rectangular base + cylindrical drum head on a single tilt axis, concentric with the drum surface. A single clamping thumbscrew locks the tilt by friction. The cable runs through a hollow stub axle — it only rotates, it does not stretch.

> **Power note (as-built).** The built device is powered by a **12 V DC adapter (≥1–2 A) through a panel-mount barrel jack**. The 12 V feeds the L298N H-bridge directly — this is what gives the high SPL — while an **XL4015 DC-DC step-down (buck) converter drops 12 V → 5 V** to power the ESP32. The earlier USB-C-5V-plus-step-*up* scheme described in older notes is *not* used in the shipped build.

## Dimensions (updated v0.7.1)

| Part | Dimensions |
|---|---|
| Base (outer) | **120 × 85 × 50 mm** |
| Base (inner) | **115 × 80 × 44.5 mm** |
| Drum head | **Ø 65 × 85 mm** *(enlarged for the Ø50 speaker)* |
| Yoke | 8 × **45 mm** (height above the top of the base), spacing between yokes **86 mm** |
| Pivot axis from top of base | **38 mm** |

---

## 1. 3D-printed parts

| # | Part | Material | Print orientation | Layer | Infill | Walls | Note |
|---|---|---|---|---|---|---|---|
| **P1** | Base housing (lower box) | PETG light grey (RAL 7035) | flat on the rear face, openings up | 0.2 mm | 25% gyroid | 3 | Printed with integrated yokes |
| **P2** | Base bottom cover | PETG light grey | flat | 0.2 mm | 25% gyroid | 3 | With keyhole slots for wall mounting |
| **P3** | Drum head | PETG light grey | flat on one Ø50 end face | **0.16 mm** | 30% gyroid | 4 | Hollow inside; right stub axle is hollow (Ø5) for the cable |
| **P4** | Front insert (chord/flat face) | PETG dark grey | flat | 0.16 mm | 20% | 3 | Openings for the PIR dome, horn throat, LED |
| **P5** *(opt., v0.7.3)* | Waveguide horn | PETG light grey | mouth down on the bed, no supports | 0.16 mm | 15% | 4 (2.5 mm wall) | Conical flare 50→70 mm, L=30, 80×2 mm flange with 4 M3 holes on PCD 52. Gives +3–4 dB on-axis. |
| ~~**P6**~~ | ~~Protective grille~~ | **excluded** | – | – | – | – | The off-the-shelf speaker has its own protection |
| **P7** *(opt.)* | Tilt limiters | PETG | – | – | solid | – | 2×3 mm micro-stops on the yoke to block at ±60° |

**Total: 6 mandatory STLs + 1 optional.**

---

## 2. Off-the-shelf hardware

| # | Part | Specification | Qty | Where |
|---|---|---|---|---|
| **H1** | Knurled M3×16 thumbscrew | knob Ø~12 mm, knurled | 1 | Tilt clamp through the left yoke |
| **H1a** *(opt.)* | M3×24 countersunk screws | for mounting the speaker + horn | 4 (instead of 4× M3×16) | If the optional P5 horn is fitted — longer screws than standard are required |
| **H2** | M3×16 axle screw, hex socket | countersunk | 1 | Fixed axle through the right yoke |
| **H3** | M3×5 brass heat-set insert | pressed in with a soldering iron | 2 | In each axial end face of the drum |
| **H4** | M2.5×6 countersunk screw | DIN 7991, stainless | 4 | Bottom cover ↔ base |
| **H5** | **Ø1.75 mm self-tapping plastic screws** | small plastic/wood screws | 4–6 | ESP32 and L298N driven directly into the printed standoffs |
| **H6** | **M3 brass heat-set insert** | for the lid/top-plate | 4 | In the base, for fastening the top plate/cover |
| **H7** | Rubber foot Ø6×3 mm | self-adhesive, grey | 4 | Corners of the bottom cover |
| **H8** | Silicone O-ring Ø3×1 mm | black | 1 *(opt.)* | Under the thumbscrew — springs the friction |
| **H9** | Cable grommet Ø5 mm | silicone/rubber | 1 | Cable entry into the hollow axle |
| **H10** | M3 nylon washer (3/6/0.5) | black | 2 | Between yoke ↔ drum on each side |

---

## 3. Electronic components

| # | Component | Specification | **Measurements** | Qty | Placement |
|---|---|---|---|---|---|
| **E1** | ESP32 DevKit | ESP32-WROOM-32, USB-C | **52 × 28.68 × h10** (53.23 with USB-C) | 1 | Rear-left in the base, USB-C toward the rear wall |
| **E2** | L298N | module with heatsink | **43.5 × 43.5 × h26** (with heatsink!) | 1 | Front-center in the base |
| **E3** | PIR sensor | AM312 (3-pin: VCC/GND/OUT) | **dome Ø13.4, PCB 19.2 long, pins 9** | 1 | Behind the PIR dome in the drum |
| **E4** | **Ultrasonic speaker/tweeter** | **Ø50.1 × h18, 25 kHz resonance, 4 mounting holes Ø3.2 on PCD 52** | **Ø50.1 × 18** | 1 | Recessed into the drum's chord/flat face, fastened with 4 M3 screws into brass heat-set inserts |
| **E5** | LED indicator | 3 mm green + 220 Ω | – | 1 | Behind the light pipe in the drum |
| **E6** | SPDT switch | mini panel toggle | – | 1 | Rear wall of the base |
| **E7** | **12 V DC adapter** | **≥ 1–2 A**, 5.5 × 2.1 mm barrel plug | – | 1 | External power supply |
| **E8** | Silicone wire | AWG28–30, colored | – | ~1 m | Internal wiring |
| **E9** | Heat-shrink tubing | 2 mm + 3 mm | – | 30 cm each | On the harness |
| **E10** | Through-axle harness | 7 conductors, overall Ø ~4 mm | – | 12 cm | Hollow axle → base |
| **E11** | **12 V barrel-jack socket** | panel-mount, 5.5 × 2.1 mm | – | 1 | Rear wall of the base (power input) |
| **E12** | **DC-DC step-down (buck) — XL4015** | 12 V → 5 V, output trimmer set to 5 V | ~52 × 27 × h12 | 1 | In the base, powers the ESP32 |

**Harness pinout** through the hollow stub axle (7 wires):

| Color | Function |
|---|---|
| red | +5 V to PIR + LED anode |
| black | GND to PIR + LED cathode |
| yellow | PIR_OUT → GPIO 27 ESP32 |
| white-1 | piezo: H-bridge OUT1 |
| white-2 | piezo: H-bridge OUT3 |
| blue | LED control (opt., if the LED is driven from an ESP32 GPIO) |
| green | spare |

**Power (as-built, 12 V):**

```
12 V adapter → barrel jack ─┬→ L298N Vs (12 V, drives the H-bridge / piezo)
                            └→ XL4015 buck (12 V → 5 V) → ESP32 5V pin
common GND
```

Driving the H-bridge from 12 V (rather than 5 V) is what gives the piezo its high SPL — much greater range and deterrence. The ESP32 is powered from the XL4015 buck converter (set its output trimmer to 5 V *before* wiring it to the ESP32). During flashing the ESP32 can also be powered over USB.

---

## 4. Tilt mechanism

### 4.1 Geometric basis

```
side profile head and base:

                  flat chord:
                  PIR ● horn ⊃ LED ●     "ULTRASONIC PET GUARD v0.7"
              ┌────────────────────────┐
            ╱                            ╲
           ╱                              ╲
          │                                │
          │       ◯  ────── axis ───────── │   ← geometric center
          │      M3                        │     of the Ø 50 mm circle
           ╲                              ╱
            ╲                            ╱
             ╲__________________________╱
                                              ↑
                                  cylindrical surface
                                  at a constant 25 mm radius
   ════════════════════════════════════════════ from the axis at any
            top face of the base                head tilt
```

**Principle:** the axis of rotation passes through the center of the circle → any point on the drum's cylindrical surface stays at the same distance from the axis during rotation → **the gap between the moving part (drum) and the fixed part (base) does not change at any tilt angle.** This completely eliminates the collision risk that limited v0.6.

### 4.2 Assembly composition

```
top view (section along the axis):

   ┌───────┐  ┌──────────────────────────────┐  ┌───────┐
   │       │  │                              │  │       │
   │  M3   │  │      drum head Ø50           │  │  M3   │
   │ thumb ├══╪══◀ M3 brass insert (L)       │  │ pivot │
   │ screw │  │  wall      wall    insert ▶  ╪══╡ screw │
   │       │  │                              │  │ socket│
   └───────┘  └──────────────────────────────┘  └───────┘
       ▲      ▲                                ▲      ▲
       │      │                                │      │
       │   M3 nylon washer           M3 nylon washer
       │   Ø3/6/0.5                  Ø3/6/0.5
       │                                                ▲
   LEFT yoke                                        RIGHT yoke
   (clamping)                                       (supporting)
```

### 4.3 How the lock works

1. The **M3×16** thumbscrew passes through the hole in the left yoke, the nylon washer, and threads into the **M3 brass insert** in the left end face of the drum.
2. Mirrored on the right: the M3×16 axle screw is not used for clamping, it only retains the second axle.
3. As the thumbscrew is tightened it **pulls the drum to the left** → the normal force presses the drum's end face against the nylon washer and the yoke → static friction develops.
4. **Friction force × friction radius = holding torque**, which exceeds the head's gravitational torque hundreds of times over.

### 4.4 Holding-torque estimate (order of magnitude)

| Parameter | Value |
|---|---|
| Hand-tightening effort on the thumbscrew | ~0.3 N·m of torque |
| Resulting axial force (via the M3 thread) | ~300 N |
| PETG–nylon friction coefficient | μ ≈ 0.3 |
| Effective contact radius | ~15 mm |
| **Holding torque** | ≈ **1.35 N·m** |
| | |
| Head mass | ~80 g |
| CoG offset from the axis | ~5 mm (horn + PIR shift it forward) |
| **Gravitational torque at maximum** | ≈ **0.004 N·m** |

**Margin: ~330×.** The head will not slip even with the thumbscrew barely tightened.

### 4.5 Usage scenario

```
   1. loosen the thumbscrew 1–2 turns ↺   (no tools, by hand)
                  │
                  ▼
   2. turn the head by hand to the desired angle
                  │
                  ▼
   3. hand-tighten the thumbscrew until snug ↻
                  │
                  ▼
   4. done — the position holds until the next adjustment
```

A pattern familiar to anyone who has used an IP camera, a projector, a monitor stand, or a microphone mount.

### 4.6 Cable routing — why it does not break

In v0.6 the cable ran as a loop *over the top* of the base → it bent at a small radius during tilting → a durability problem.

In v0.7:

```
the cable passes through the HOLLOW AXLE:

   ┌─ base housing ┐                ┌─ drum ────┐
   │  ESP32        │                │  PIR      │
   │  L298N        │                │  Piezo    │
   │               │   hollow axle  │  LED      │
   │  harness ═════╪══(Ø5 inside    ╪═══ harness│
   │  service      │   Ø6 outside)  │  service  │
   │  loop 15mm    │                │  loop 15  │
   └───────────────┘                └───────────┘
                  ▲                ▲
                  │                │
              grommet Ø5     passage inside
                             the axle stub
```

When the drum rotates, the cable **only twists about its own axis** inside the hollow stub, it does not stretch and does not bend at a small radius. This is a "swivel pivot" mode of cable operation (as in fixed swivel mechanisms), not a "flex cable" mode.

**Service life:** tens of thousands of tilt cycles.

### 4.7 Optional upgrade — detent clicks

If tactile feedback at fixed angles is wanted:

- Steel ball Ø3 mm + a spring in a socket on the side face of the drum.
- Ø3 mm dimples on the inner face of the left yoke, arranged along arcs at 0°, ±15°, ±30°, ±45°.
- The thumbscrew still provides the final lock.

For the first prototype this can be skipped. The friction clamp is sufficient.

---

## 5. Assembly sequence (brief)

1. Print P1–P5 (P6, P7 — optional).
2. Heat-set the brass inserts (H3 × 2 in the drum; H6 × 4 in the base for the top-plate/lid screws).
3. Inside the base, mount the ESP32 (E1, rear-left), the L298N (E2, front-center), and the XL4015 buck (E12) with Ø1.75 mm self-tapping screws (H5) driven into the printed standoffs.
4. Wire the power: 12 V adapter (E7) → barrel jack (E11) → L298N Vs (12 V), and a branch to the XL4015 buck (E12) input. Set the XL4015 output to 5 V, then wire its output to the ESP32 5V pin. Common GND.
5. On the rear wall of the base: mount the barrel jack (E11) and the SPDT switch (E6) in the 12 V line.
6. Inside the drum:
   - glue the piezo (E4) into the horn throat (P5),
   - screw down the PIR board (E3) behind the dome (4 M2 screws onto h=2 mm standoffs),
   - mount the LED (E5) behind the light pipe.
7. Connect the drum and base with the 7-conductor harness (E10) through the drum's hollow stub axle and the grommet (H9) in the top face of the base.
8. Set the drum into the yokes; insert the axle screw (H2) on the right and the thumbscrew (H1) on the left with the nylon washers (H10).
9. Screw on the bottom cover (P2) with M2.5 screws (H4); stick on the feet (H7).

**Assembly time for an experienced builder: ~25 minutes.**

---

## 6. Printing: general parameters

- **Printer:** FDM, build volume ≥ 120×120×120 mm (any modern one).
- **Nozzle:** 0.4 mm.
- **Material:** PETG (PCTG / PETG-CF — better for the mechanical part).
- **Post-processing:** snap-fit clearances are designed for "as-printed," nothing needs drilling.
- **Total print time** on a Bambu A1 / Prusa MK4: ~12–14 hours.
