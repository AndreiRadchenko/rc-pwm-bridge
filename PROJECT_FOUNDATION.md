# rc-pwm-bridge — Project Foundation

Recorded: 2026-10-28  
Status: requirements and design proposals; hardware selection and load wiring are not approved or validated.

## 1. Purpose and source of truth

Build a local, two-channel RC PWM bridge for the Scopa system: one channel switches the ignition/glow-plug function ON/OFF, and a separate channel converts RC pulse width into duty-cycle PWM for a 12 V brushed DC fuel pump. The motivation is to investigate replacing the existing ESC function with a controller plus a suitable power-switch stage. This document does not establish that a particular replacement is electrically suitable.

Chosen GitHub repository name: **`rc-pwm-bridge`**. The existing local repository is `/Users/mac/Projects/Scopa/rc-pwm-bridge`; its configured origin at the time of writing is [AndreiRadchenko/rc-pwm-bridge](https://github.com/AndreiRadchenko/rc-pwm-bridge). This records the local configuration, not verification of the remote repository's public availability.

Primary discussion: [Обговорення ESC контролерів](chatgpt-conversation://6adb3b2f-b72c-83ed-a45f-1232eb0fbd19), conversation ID `6adb3b2f-b72c-83ed-a45f-1232eb0fbd19`.

Source limitations and interpretation:

- All available conversation pages were retrieved. User messages were readable; assistant replies were returned as content-reference placeholders rather than their full text. Attached files were made available separately.
- The user's continuation request supplies the detailed discussion summary, including proposed thresholds, pin examples, controller preference and reference repository names. These details are preserved with their stated uncertainty; they are not presented as independently recovered quotations or verified engineering conclusions.
- The uploaded `esp32-mosfet.pdf` was inspected, including its product description, board illustrations and test-mode description. The uploaded initial driver schematic image was also inspected. The conversation additionally includes `CytronMDA20.pdf`; this foundation does not claim a completed review of that driver's ratings or protections.
- User measurements and operating descriptions are recorded as reported observations. Product-document claims are distinguished from validation. Additional architecture, milestones and tests below are proposals for subsequent work.
- No new hardware measurements, firmware implementation, load tests or external repository audits were performed to create this document.

## 2. Requirements and reported operating context

### Functional requirements

| Item | Requirement / intended behavior | Status |
| --- | --- | --- |
| Input arrangement | Two separate RC PWM pulse inputs, CH1 and CH2 | Project requirement |
| Nominal input span | 1000–2000 microseconds (µs) pulse width on each channel | Project requirement; actual source timing and electrical levels still need measurement |
| CH1 function | Ignition/glow-plug ON/OFF control | Project requirement; load interface and ratings unconfirmed |
| CH2 function | Linear conversion: 1000 µs → 0%; 2000 µs → 100% pump PWM | Project requirement |
| Pump PWM frequency | 1000 Hz at the pump power-switch control interface | Project requirement; confirm compatibility with the selected driver |
| Calibration | Fixed mappings; no automatic endpoint calibration | Explicit user requirement |
| Connectivity | Local-only operation; Wi-Fi and Bluetooth disabled on ESP32 | Intended operating constraint |
| Initial signal source | RC servo tester before integration with the actual receiver | Test plan from discussion |

RC PWM input pulse width and the 1000 Hz motor PWM output are different signal formats. Do not assume the RC source repeats at 1000 Hz. Receiver/tester frame rate, jitter and behavior on disconnection must be measured.

### Load and supply context

- The system supply is nominally **12 V**.
- The fuel-pump motor is **brushed DC**.
- The user reported motor current up to **15 A during normal operation**. This is not a verified startup or stall-current limit.
- The motor normally operates around **80% PWM**, with periods at **40% PWM**.
- The user reported **2.5 A at 40% PWM** in the 12 V context. Measurement method, instantaneous current waveform and hydraulic load conditions were not established here.
- Do not infer the 80% current by linearly scaling the 40% measurement. Preserve the 15 A normal-operation design concern until characterized.
- **Ignition/glow-plug voltage, steady current, cold/inrush current, grounding and required switching interface remain unconfirmed.** A 12 V system does not prove that the plug itself accepts 12 V.
- The discussion uses ignition/glow-plug terminology. Confirm whether this is direct plug power, an ignition-unit supply, or a control input before selecting or wiring its output stage.

## 3. Proposed control behavior — not all parameters are final

### CH1 hysteresis

Proposed ON/OFF thresholds for a valid, fresh CH1 signal:

- Pulse width **≥1600 µs**: command ON.
- Pulse width **≤1400 µs**: command OFF.
- Between 1400 and 1600 µs: retain the previous valid ON/OFF state.

The startup state must be OFF. Hysteresis must never retain an ON state through a signal fault or reset. These thresholds are a proposal from the discussion, not a finalized hardware safety specification.

### CH2 fixed linear map

For a validated input within the nominal range:

```text
pump_duty_percent = 100 × (CH2_pulse_us − 1000) / 1000
```

| Valid CH2 pulse | Requested pump duty |
| --- | --- |
| 1000 µs | 0% |
| 1250 µs | 25% |
| 1400 µs | 40% |
| 1500 µs | 50% |
| 1800 µs | 80% |
| 2000 µs | 100% |

Limit the final duty command to 0–100% after signal validation. An invalid pulse must not be converted into full duty by blind endpoint clamping. Any tolerance outside 1000–2000 µs, associated clamping behavior, filtering and rounding must be explicitly chosen and tested. No automatic calibration or learned endpoints are intended.

At 0%, command continuous OFF; at 100%, command the selected driver's supported full-ON behavior. Verify physical output polarity: logical zero duty must mean the load is OFF even if a particular module uses an inverted control input.

### Signal validity and failsafe

The intended behavior is cutoff on missing or invalid RC signal. **100 ms was proposed as a timeout; it is not a final, verified value.**

Proposed conservative baseline, pending an explicit decision: a fault on either input forces **both outputs OFF**. The discussion does not resolve whether faults should instead affect only the corresponding channel. Resolve this policy before implementation acceptance.

Each channel needs its own last-valid-pulse timestamp and validity state. Validate pulse width and freshness; define handling of malformed edges, stuck HIGH/LOW, implausible repetition rate, capture overflow and excessive jitter. Invalid activity must not keep refreshing the last-valid timestamp indefinitely.

Timeout, width tolerances, required valid-frame count, fault debounce, response latency and recovery/arming behavior remain open. As a proposed recovery policy, require stable valid signals and deliberate safe/low commands before re-enabling outputs. A receiver may continue sending valid-looking pulses after radio-link loss; the bridge cannot detect that from pulse freshness alone. Verify and configure the receiver's own failsafe accordingly.

## 4. Controller and power-stage choices

### Preferred controller and alternative

**ESP32-WROOM-32E-N4** is the preferred controller direction; **Arduino Nano** remains an alternative. This preference does not finalize a carrier board or authorize direct 12 V power to a bare ESP32 module.

Proposed ESP32 implementation:

- Use **RMT receive or MCPWM capture** to measure both RC pulse widths. Select one after checking the exact ESP32 variant, framework version, peripheral resources and driver APIs.
- Use **LEDC** for the 1000 Hz pump PWM output. The plug output is static ON/OFF and may use a normal GPIO; if routed through an LEDC output abstraction, explicitly verify steady 0%/100% behavior and safe initialization.
- Keep capture and failsafe handling nonblocking. Serial diagnostics must not delay cutoff.
- Disable Wi-Fi and Bluetooth and avoid starting network services. Radio operation is outside the intended design.
- Decide ESP-IDF versus Arduino-ESP32, version pinning, PWM resolution and watchdog configuration during firmware setup; they are not settled by this document.

Arduino Nano would require its own verified capture/timer allocation and electrical/power design. It is an alternative platform, not a promise of drop-in compatibility.

### Illustrative pin plan only

| Function | Example ESP32 GPIO | Verification required |
| --- | --- | --- |
| CH1 RC input | GPIO18 | Availability, header location, input conditioning, board conflicts |
| CH2 RC input | GPIO19 | Availability, header location, input conditioning, board conflicts |
| Pump PWM control | GPIO25 | Actual driver connection, polarity and startup state |
| Plug ON/OFF control | GPIO26 | Actual driver connection, polarity and startup state |

**These are illustrative assignments, not a wiring plan.** An integrated MOSFET board may connect its gates to entirely different GPIOs. Verify the exact board revision, schematic, continuity, exposed headers, boot behavior and firmware assignment before connecting any load. Do not infer gate routing from this table or a product photograph.

### Candidate hardware — no final selection

| Candidate | Why it was discussed | Unresolved checks |
| --- | --- | --- |
| ESP32-32E Dual MOS Switch Module, with dual/quad variants | Compact controller plus two or four MOS switch channels and integrated power conversion | Actual MOSFET part numbers, gate drive, continuous current, thermal path, terminals/traces, channel sharing, inductive protection, GPIO routing and reset behavior |
| Existing custom test rig: MCP1407 + IRF3205 | Separate gate driver and power MOSFET; initial schematic is available | Actual assembly, drive waveform, component ratings, protection, heat dissipation and wiring versus schematic |
| Cytron MD20A | Dedicated motor-driver alternative | Exact model/document identity, 12 V and 1000 Hz compatibility, logic interface, continuous/peak current, cooling and protection behavior; a separate plug switch may still be needed |
| Dual AOD4184 MOSFET board | Compact power-switch alternative | Exact devices, 3.3 V control suitability, current/thermal performance, protection and connector limits |
| Advertised 5–36 V, 15 A MOSFET module | Separate power-switch candidate used in the early controller/tester discussion | Whether advertised current is continuous under actual cooling; peak current, control thresholds, PWM capability and inductive protection |

“15 A” or “400 W” in a listing is not proof of suitability for this pump, especially with startup current and enclosure heating. No candidate is selected by inclusion in this table.

### What the uploaded ESP32 module PDF actually claims

The uploaded `esp32-mosfet.pdf` describes:

- An onboard ESP32-32E module with 4 MB flash.
- Two-channel and four-channel MOS switch versions.
- **DC 5–60 V input support**, with **12 V recommended**.
- An integrated DC-to-5 V conversion circuit and Type-C USB 5 V power support.
- Exposed I/O and reserved UART programming connections.
- N-channel MOSFETs described as having 60 V drain-source voltage capability.

These are product-document claims, not measured capabilities of the user's board. A MOSFET voltage rating does not establish the safe operating range of every board component or its motor-current capability. **Suitability for 15 A normal pump current and adequate inductive-load protection remain unknown.** The PDF does not substitute for a verified board schematic and MOSFET datasheets.

The PDF's module test-mode description includes modes that turn outputs ON or run a channel at 50% brightness. Treat factory/demo firmware as unsuitable for direct connection to fuel or ignition loads until its behavior is replaced or fully verified. Do initial programming and startup checks with both loads disconnected. Verify USB/DC power-path and backfeed behavior before simultaneous connection; USB support alone does not prove safe power sharing or integrated USB programming.

## 5. Electrical architecture and protection

### Proposed architecture

```text
RC tester initially / actual RC receiver later
  CH1 ── verified input conditioning ── capture CH1 ─┐
  CH2 ── verified input conditioning ── capture CH2 ─┤
                                                    v
                         ESP32: validation, mapping, failsafe,
                         safe startup, arming and watchdog
                           │                         │
                  1000 Hz pump PWM             plug ON/OFF
                           │                         │
                 verified motor driver     verified plug interface
                           │                         │
                      fuel pump                 ignition/plug

12 V source ── fuse(s) ── independent emergency load cutoff ── load branches
      └── protected DC-DC / verified integrated converter ── controller rails
```

This is a functional proposal, not a construction schematic. Final supply routing, protective devices, grounding and emergency-switch topology must be reviewed for the actual loads and fault currents.

### Supply, logic and switching

- A bare ESP32 requires an appropriate regulated supply; a 12 V system needs conversion unless the selected board explicitly integrates a suitable 12 V input stage. Verify the exact carrier's input specification. Do not apply 12 V to a 3.3 V pin or assume a development board's power input is suitable.
- Measure RC tester and receiver output levels. If the RC signal is 5 V, provide a verified level-shifting/input-protection circuit for the ESP32. Verify common-ground requirements or design an intentional isolated interface.
- A MOSFET switch is **not a relay dry contact**. It has polarity, leakage and a body diode; ordinary low-side modules share an electrical reference and provide **no galvanic isolation**. Confirm that the plug/ignition circuit permits this topology. If it requires isolated dry contacts, select an appropriate isolated interface rather than assuming a MOSFET is equivalent.
- Keep high-current motor return paths from disturbing logic ground. Plan conductor sizes, connector ratings, local bypassing, bulk capacitance and transient/noise suppression from actual measurements and component data.
- Characterize pump startup current and supply sag. The reported 15 A normal maximum is not a fuse, connector, MOSFET or emergency-switch selection value by itself.

### Flyback and inductive-load protection

The initial custom driver schematic shows **MCP1407-E/SN**, **IRF3205** and a **B540C** diode across the load path. This confirms a diode is drawn, not that the installed diode or circuit is adequate for the pump.

**30SQ050, advertised as 30 A / 50 V in an R-6 package**, was discussed as a candidate external Schottky flyback diode. The user preferred a part without a heatsink. Neither the nominal current label nor that preference proves heatsink-free operation. Exact manufacturer, datasheet conditions, average/RMS/peak current, forward loss, reverse-voltage margin, lead/PCB heat removal and ambient temperature need verification. Thermal suitability remains unresolved.

For a conventional single low-side MOSFET stage, a flyback path across the motor is normally oriented to be reverse-biased during powered operation; the final circuit must be checked before wiring. Do not generalize that connection to an H-bridge or integrated driver without its documentation. The MOSFET body diode alone does not establish a suitable motor recirculation path. Check what protection the selected board already provides and whether an external diode, TVS or other suppression is required.

Evaluate protection under actual current ripple, startup, switching and enclosure conditions. Diode heating depends on recirculation current during PWM OFF time, not just the supply-current reading or duty percentage. No diode choice is finalized here.

### Required safety direction

- **Safe startup:** both loads OFF during power application, reset, bootloader entry, firmware upload, brownout and watchdog recovery. Keep them OFF until the defined arming conditions are satisfied.
- **Hardware fail-off:** use a verified gate/control bias and, where required, an independent enable or interlock so floating pins, controller reset or a stalled program cannot leave an unintended ON command. Audit any factory firmware and active-low interfaces.
- **Watchdog and fault handling:** supervise the control task and prove that fault/reset handling actually removes the physical command. A watchdog alone is not independent load isolation.
- **Independent emergency cutoff:** provide a physical means to remove hazardous load power independently of the ESP32 and its firmware. Consider power-stage short-circuit failure as well as a stuck software command.
- **Fusing:** select and locate suitable protection for supply wiring and load branches using measured operating/startup currents and conductor/device limits.
- **No reliance solely on software:** software failsafe, GPIO logic and a MOSFET command cannot guarantee shutdown after every hardware fault.
- Start with disconnected loads and dummy-load tests; progress to controlled pump/plug testing only after electrical review. Prevent unintended fuel delivery or ignition throughout bench work.

## 6. Unresolved decisions and evidence needed

1. Confirm the exact ignition/glow-plug function, voltage, current, inrush, grounding and whether dry-contact isolation is required.
2. Select the controller carrier and power stages. Obtain the exact board revision/schematic and MOSFET/driver datasheets; verify ratings, gate drive, terminals, traces and thermal design before load wiring.
3. Verify the final pin assignments and polarity against the physical board. The GPIO18/19/25/26 plan must remain illustrative until then.
4. Measure pump current at representative operating points, startup and other credible worst cases using a safe method; establish supply transient and thermal limits. Do not perform uncontrolled stall tests.
5. Resolve flyback/recirculation protection and the thermal suitability of the B540C or any replacement, including the 30SQ050 candidate.
6. Select the power-conversion and grounding/noise design; verify USB/DC interaction on an integrated board.
7. Measure RC levels, pulse endpoints, frame rate and jitter from tester and receiver. Choose input conditioning and exact validity limits.
8. Finalize CH1 thresholds, failsafe timeout (100 ms remains proposed), fault scope across channels, recovery/arming and any ignition/pump sequencing or interlocks. No automatic ignition sequence is specified yet.
9. Choose firmware framework, peripheral APIs, PWM resolution, diagnostics and watchdog settings. Confirm endpoint behavior and actual 1000 Hz output at the power-stage input.
10. Define numerical timing tolerances, allowable temperatures, test durations and acceptance limits before declaring the design ready.

## 7. Proposed firmware and integration milestones

1. **Hardware definition:** identify boards and load interfaces, review documentation, approve power/protection design and assign verified pins. Record evidence and open issues.
2. **Safe firmware skeleton:** outputs OFF before peripheral setup; no radio services; explicit startup/fault state; local serial diagnostics and watchdog. Verify with loads disconnected.
3. **Two-channel capture:** implement RMT or MCPWM capture with separate validity/freshness tracking. Log measured pulse widths and faults using the RC tester. No load enabling yet.
4. **Deterministic control:** implement fixed CH2 mapping, CH1 hysteresis and selected invalid-signal handling. Generate and measure the 1000 Hz output with dummy loads or a logic analyzer.
5. **Failsafe and arming:** implement the explicitly selected timeout, channel fault policy, recovery conditions and watchdog behavior; demonstrate physical output OFF under faults.
6. **Power-stage bench validation:** verify switching polarity, gate drive, protection, startup transients, emergency cutoff and thermal behavior in staged, current-limited tests appropriate to the equipment.
7. **Controlled load integration:** validate pump and separately specified plug interface, then actual receiver behavior and combined electrical noise. Record test conditions, firmware version and results.

Each milestone is a proposed work package. None is marked completed by the creation of this document.

## 8. Proposed acceptance tests

All tests below are **planned, not passed**. Numerical tolerances, safe load conditions and final failsafe settings must be recorded before execution.

| Test | Expected outcome / evidence |
| --- | --- |
| Cold start, reset, firmware upload, bootloader and brownout | Both physical load commands remain OFF; no unsafe pulse. Observe from power application, not only after firmware initialization. |
| Arming and recovery | Outputs remain OFF until the approved valid-signal and safe-command conditions are met; reconnecting a high command does not bypass the selected policy. |
| CH1 thresholds | With the proposed values, 1600 µs switches ON, 1400 µs switches OFF; values strictly between retain state only while signal remains valid. Startup in the middle band remains OFF. |
| CH2 mapping | Verify 1000/1250/1400/1500/1800/2000 µs against 0/25/40/50/80/100% within defined capture/PWM tolerances. No endpoint learning occurs after power cycling. |
| PWM frequency and endpoints | Measure 1000 Hz at intermediate duty settings; confirm continuous OFF at 0% and supported full ON at 100%, with correct physical polarity. |
| Channel independence | Changing CH1 does not change the commanded CH2 duty and vice versa during valid operation; selected safety interlocks override this only as explicitly specified. |
| Loss and invalid input | Disconnect each channel separately and both together; test stuck HIGH/LOW, short/long pulses, malformed streams and invalid activity. Outputs follow the selected fault scope and cutoff deadline. |
| Timeout boundary | Measure last-valid-pulse-to-OFF latency against the finalized timeout and execution allowance. If 100 ms is adopted, test it explicitly; do not assume it is already approved. |
| Tester and receiver loss behavior | Verify actual pulse levels, endpoints and frame rates. Check whether receiver radio-link loss produces absent pulses or a programmed valid pulse; demonstrate safe system behavior in both cases. |
| Capture load and watchdog | Fault detection remains timely with diagnostics and both inputs active; a deliberately induced firmware stall triggers the intended reset/fail-off behavior. |
| Input electrical interface | Verify conditioned voltage and edge quality at the MCU without exceeding its limits; check common-reference or isolation design as applicable. |
| Pump startup and representative loading | Record current, supply sag, switching transients and controller stability; no unintended plug activation or reset-induced restart. |
| Thermal endurance | At representative 40%/80% duty and maximum expected normal load, measure MOSFET, diode, terminals and converter temperatures against verified limits for the chosen duration and ambient conditions. |
| Power-stage protection | Demonstrate an adequate recirculation/transient path and voltage margins using a safe measurement setup; no claim based solely on visible diodes or nominal listing ratings. |
| Emergency cutoff and fuse design | Physical cutoff removes load power independently of MCU state; verify device/wiring ratings and the documented fuse selection without uncontrolled destructive testing. |
| Local-only operation | Wi-Fi/Bluetooth are not initialized or used; normal control and fault handling require no network connection. |

Retain test records and unresolved failures. Documentation or simulation alone does not establish that a fuel/ignition load is safe to connect.

## 9. References from the discussion

### GitHub inspiration — unverified

The following repository names were mentioned in the discussion summary. They have **not been audited or independently verified for existence, current contents, licensing, compatibility or safety** during this task. They are leads for later investigation, not selected dependencies or copied libraries:

- [fazeelahmad72/ESP32_PWM_MOSFET_Trigger_Switch](https://github.com/fazeelahmad72/ESP32_PWM_MOSFET_Trigger_Switch)
- [slopeztech/CyberBoat](https://github.com/slopeztech/CyberBoat)
- [rengels/rc_functions_controller](https://github.com/rengels/rc_functions_controller)
- [nhutchison/StealthTank](https://github.com/nhutchison/StealthTank)
- [RomanLut/hx_espnow_rc](https://github.com/RomanLut/hx_espnow_rc)

Review each independently before reuse. A wireless reference does not change this project's local-only, radios-disabled requirement.

### Hardware and document leads — suitability unverified

- Uploaded `esp32-mosfet.pdf`: ESP32-32E dual/quad MOS switch module product description and illustrations. A local reference copy was found at `/Users/mac/Projects/Scopa/esp32-mosfet.pdf`; the uploaded attachment was used for inspection.
- Uploaded initial test-rig driver schematic image: MCP1407-E/SN + IRF3205, with B540C shown. No assembly or ratings verification is implied.
- Uploaded `CytronMDA20.pdf`; local reference copy at `/Users/mac/Projects/Scopa/CytronMDA20.pdf`. The conversation calls the candidate Cytron MD20A; verify exact model identity against the document and purchased unit.
- [ESP32 MOS switch module listing mentioned](https://www.aliexpress.com/item/1005009794900318.html)
- [5–36 V / 15 A MOSFET module listing mentioned](https://ly-tron.com/modul-mosfet-536v-15a-400w-pwm-kliuch-dlia-keruvannia-navantazhenniam-mosfet-15a-400w-pwm-ua)
- [RC servo tester listing mentioned](https://prom.ua/ua/p3135252857-tsifrovoj-tester-servoprivodov.html)
- [Cytron MD20A listing mentioned](https://bigben.net.ua/ua/p3042173466-cytron-md20a-odnokanalnyj.html)
- [Dual AOD4184 board listing mentioned](https://arduino.ua/prod2540-modyl-silovogo-klucha-na-dvoh-mosfet-aod4184-50w)
- [30SQ050 diode listing mentioned](https://arduino.ua/prod7819-diod-shotki-30sq050-30a-50v-r6-1sht)

These links preserve discussion context, not endorsements or verified present-day specifications. **Before wiring either load, verify pin assignment, board schematic, exact MOSFET datasheets, protection, power path and all relevant electrical/thermal ratings.**
