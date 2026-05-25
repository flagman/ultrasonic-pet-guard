# Ultrasonic Pet Guard — Hardware

Parts list, wiring, and print settings. The geometry and assembly are defined by
the CAD (`hardware/cad/`) and the print files (`hardware/print/`).

## Electronics

| Component | Key spec |
|---|---|
| ESP32 DevKit | ESP32-WROOM-32, USB-C (~52 × 28.7 mm) |
| L298N dual H-bridge module | with heatsink (~43.5 × 43.5 × 26 mm) |
| AM312 PIR sensor | 3-pin (VCC / GND / OUT), dome Ø13.4 mm |
| Wide-band piezo tweeter | Ø50 × 18 mm, ~1–45 kHz (must reproduce 25 kHz well — **not** a unit capped at 25 kHz) |
| 12 V DC adapter | ≥ 1–2 A, 5.5 × 2.1 mm barrel plug |
| 12 V barrel-jack socket | panel-mount, 5.5 × 2.1 mm |
| XL4015 DC-DC buck | 12 V → 5 V step-down |
| Silicone hookup wire + heat-shrink | AWG 28–30, ~3 m |

> The LED and on/off switch shown in the concept render are **not** in the current enclosure design (no cutouts; firmware drives no LED). Power is switched at the adapter.

## Fasteners & adhesives

| Part | Qty | Use |
|---|---|---|
| M8 bolt | 2 | tilt pivot (one per yoke) |
| M8 extended (coupling) nut | 1 | captured in one drum end; the M8 bolt threads into it (other end is a plain Ø8 hole) |
| M3 heat-set brass insert | 1 | holds the top-plate |
| M2.5 × 6 countersunk screw | 4 | bottom cover ↔ base |
| Ø1.75 mm self-tapping screws | 6–8 | ESP32, L298N, XL4015 into the printed standoffs |
| Cable grommet Ø10 mm | 1 | base↔drum harness pass-through |
| Rubber feet | 4 | base |
| Hot-melt glue | — | PIR + tweeter into the front panel |
| Superglue (CA) | — | drum halves + front panel |

## Wiring

**Pinout (ESP32):**

| GPIO | To |
|---|---|
| 27 | PIR `OUT` |
| 13 | L298N `IN1` (PWM) |
| 12 | L298N `IN3` (inverted PWM, via GPIO matrix) |
| 14 | L298N `ENA` + `ENB` |

- L298N `IN2` and `IN4` → **GND** (unused inputs, must not float).
- Transducer connects **across L298N OUT1 ↔ OUT3** (full H-bridge).

**Power:**

```
12 V adapter → barrel jack ─┬→ L298N Vs (12 V → H-bridge / piezo)
                            └→ XL4015 buck (12 V → 5 V) ─┬→ ESP32 5V
                                                          ├→ L298N +5V (logic)
                                                          └→ PIR VCC
common GND
```

Set the XL4015 output trimmer to 5 V before wiring it to anything. The L298N's onboard 5 V regulator is not used.

## Printing

- FDM, 0.4 mm nozzle, **PETG**.
- Layers: **0.2 mm** (all parts).
- Infill: **100 %**.
- Print files (`hardware/print/`): `body`, `top-plate`, `drum-left`, `drum-right`, `front-panel`, `bolt-holder`.

## Assembly notes

- The ESP32 is mounted **upside down** in the base (board flipped onto its standoffs).
- Heat-set the single M3 insert into the base that holds the top-plate; capture the extended M8 nut in one drum end.
- Hot-glue the PIR and the tweeter into the front panel; superglue the two drum halves and the front panel.
- Route the harness out through the drum's hollow stub axle and the base grommet before closing up.
- Mount the drum on the two M8 bolts; tighten the bolt on the captured-nut side to set the tilt friction.
