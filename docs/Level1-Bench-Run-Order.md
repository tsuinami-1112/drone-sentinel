# Level 1 station v3: bench run order and session instructions

Branch `level1`, firmware `level1-c5phy/`, written 2026-10-08 against the
firmware at commit `f38f624` (50-channel plan, analog gate with the envelope
condition, wideband path, pull-in, dual-band variant). Companion to:

- the [bench guide](Level1-Station-v3-C5PHY-Bench-Guide.html): the procedure
  itself, stages 0–8 on sheets 6–9, the kit index and the filter-skirt record on
  sheet 10;
- the [bench data plan](Level1-Bench-Data-Plan.md): what each measurement
  changes, the decision rule for every outcome, what goes back to the lead for
  a decision, and the log. **Every decision after a measurement is taken from
  that plan, not from this document.**
- [`docs/bench/`](bench/README.md): where every raw record goes, one file per
  stage and board, before anything is concluded from it.

This document decides **in which order** the stages are run and **why**, and
writes the first sessions out in full for helpers who are comfortable with a
computer but are not the project lead. Trimmed on 2026-10-25 after bench1's
first sessions: what the bench has settled is listed once, in the data plan's
§0 "Settled on the bench", and is not on the lists below; the steps here are
the ones still to run. Parts 1 and 2 are for the lead; Part 3
onward is for whoever is at the bench. Everything here is on the reference
build (`seeed_xiao_esp32c5`, defaults, bare XIAO with its stock antenna until
the patches come in); the dual-band variant repeats stages 1, 5, 6 and 8 later,
as the data plan describes.

How Part 1 was built: every file of the firmware and its tests was read; the
port was compared line by line with its source, the C5VRX project (its
production `rf.c`/`video.c`, docs and legacy research); and the binaries the
Arduino build links were inspected directly (Espressif `esp-phy-lib`
release/v5.5 and master, `esp32-wifi-lib` release/v5.5, the Arduino
lib-builder configs, pioarduino's `platform.json`), together with the IDF 5.5
PARLIO RX driver and PHY glue and the arduino-esp32 `XIAO_ESP32C5` variant.

---

## Part 1 — How sure are we? (for the lead)

"Chance wrong" is my estimate that the assumption is wrong **today**, from the
code, C5VRX's hardware record and the IDF 5.5 binaries. Settled means 10 % or
less.

### 1a. Settled from the desk (no bench time needed)

| # | Assumption the firmware leans on | What we already have | Chance wrong |
|---|---|---|---|
| D1 | The six undocumented PHY calls exist in the IDF 5.5 C5 `libphy.a` (`phy_disable_agc`, `phy_rfagc_disable`, `phy_wifi_fbw_sel`, `phy_force_rx_gain`, `phy_set_freq`, `phy_chip_set_chan_offset`) | `nm` on `esp-phy-lib` release/v5.5 `esp32c5/libphy.a`: all six global, byte-identical sizes to the IDF 6 blob C5VRX used | 2 % |
| D2 | `lmac_stop_hw_txq` exists | `esp32-wifi-lib` release/v5.5 `libpp.a`: global | 2 % |
| D3 | `phy_track_pll_deinit` exists and stops the PHY's 1 s PLL-tracking timer | A non-static function in IDF's open-source `esp_phy/phy_common.c`, built on every non-ESP32 target. C5VRX never called it (its build removed the timer), so the port's call is its first use, but the symbol is real | 5 % |
| D4 | pioarduino "stable" = Arduino 3.3.12 on IDF 5.5 | `platform.json` 55.03.312 pins Espressif's `esp32-core-3.3.12-libs`; lib-builder branch release/v5.5 | 2 % |
| D5 | XIAO pin map D0=1 D1=0 D2=25 D3=7 D4=23 D5=24 D6=11 D7=12 D8=8 D9=9 D10=10, LED 27 | arduino-esp32 `variants/XIAO_ESP32C5/pins_arduino.h` | 2 % |
| D6 | PARLIO RX accepts two unwired lanes | IDF 5.5 `parlio_rx.c` skips negative pins with a warning; no all-lanes check | 5 % |
| D7 | Register recipe, lane order (Q bits on DIAG 6–9, I bits on DIAG 16–19), 40 MS/s, rising edge, LSB packing, SRAM-ownership write, no heap reservation | Identical to C5VRX-3 production; C5VRX-3 also runs an unreserved heap in that mode. Same board (XIAO C5) | 5 % |
| D8 | `phy_set_freq(uint16_t mhz, int offset)` signature | C5VRX disassembly of the 6.0 blob; same symbol size in 5.5 | 5 % |
| D9 | Gain index 62 = maximum; noise does not clip at 62 | C5VRX walk-around log: `G_act=62 P_med=1 clip=0` with the VTX off | 5 % |

Session A confirmed D1–D4 on the installed package itself on 2026-10-08, in
the cloud container and on the bench PC (Arduino 3.3.12, libs 5.5.5
b774170ff46, platform 55.03.312): all nine symbols global, the strong-symbol
build links, both environments build. Results in the Data Plan log.

Bench1 (2026-10-24/25) confirmed D6 (the 3-lane build runs with two lanes
unwired) and D9 (no clipping at gain 62 with no signal: the noise does not
even reach one decode step); D7's lane order waits for the `cfo_khz` sign
with a VTX. It also settled three of the open ranks, which are off the table
below: **11**, the receive chain runs under Arduino / IDF 5.5 (data plan §0
S1); **3**, `RF_NOISE_POWER` 2.0 is the 3-lane decode floor exactly, with
the noise under one decode step, so there is no noise to measure on 3 lanes
(S3); **9**, "US" refuses Wi-Fi 169 and the fallback parks A2, R6, F6 and B7
15–20 MHz away, no channel lost (S5). Ranks 2, 5, 7, 8, 12 and 14 carry what
bench1 added.

### 1b. Open, most uncertain first

| Rank | Assumption | Chance wrong | Why that number | If wrong: impact / fix size | Earliest test |
|---|---|---|---|---|---|
| **1** | The bearing model: K derived from the antenna gain through a Gaussian main lobe and an empirical scale (`BEARING_K_SCALE` 2.5; 8 dBi → 72° → 2.97 °/dB), applied as strongest sector + K × neighbour difference | **60 %** that K is off by more than half; **40 %** that the model is marginal | No measurement behind it: the scale 2.5 was chosen to reproduce the earlier guess of 3 °/dB. The two neighbours sit 45°–135° off their own axes, in the side-lobe/back region where patch patterns are irregular and box diffraction dominates | The bearing is the project's output; sigma feeds the mapper's fix circle. Scale only: one constant. Non-linear model: new estimator in `bearing.c` (medium) | **Session D**: single-patch pattern sweep (needs the receiver alive, one patch, a pigtail, a VTX outdoors; no switch). Full: stage 6 |
| **2** | The wideband (digital-link) path as a group: candidate envelope `cv2` 0.5–1.5, duty ≥ 75, class edges `r128` ≥ 0.10 / `r2667` ≥ 0.03, the 66.7 µs symbol assumed for DJI (lag 2667), the bandwidth buckets, the 30 MHz ownership radius from a filter-skirt model, pull-in from 3 MHz off | **70 %** that at least one threshold moves (one rule was added on bench1: `WB_ON_SNR_DB`); **50 %** that DJI's symbol is not 66.7 µs | All of it from synthetic waveforms; no DJI, Walksnail or HDZero capture has ever been fed to the code. Bench1: a phone hotspot's 20 MHz 802.11 read `dot11`, 20 MHz, centre within 0.5 MHz (data plan S7); a false report from the 3-lane floor was found and fixed (S8) | Digital detection quality and false reports from busy Wi-Fi; the analog core only through the shared splitter (rank 10) and the rule that a digital link within 30 MHz of a live analog carrier is not reported. Constants via the data plan's rules; a new lag feature if DJI's symbol differs (a session's work) | Session C (Wi-Fi access point idle and loaded), stage 5 skirt record (session E), stage 8 kits (session H) |
| **4** | Gain index steps are about 1 dB each (`GAIN_STEP` 3 = 3 dB) | **40 %** off by more than 30 % | Not stated anywhere in C5VRX. Espressif's RF-test library forces gain by index too, so the index is real; the dB per index is unknown | `level_db` of strong signals; **a bearing bias** whenever sectors settle at different gains; the hit decision on strong carriers. Constants, a table if non-linear, maybe equal-gain sectors (small code) | **Session B** (B1c, 5 min); stage 5 slope |
| **5** | The synthesizer follows `phy_set_freq` 60 MHz above the last Wi-Fi centre (E8 5945) | **40 %** (the new +10 MHz pulls for D1–D3 and the −27 MHz pull for L4: about 15 %) | 5945 MHz is 20 MHz above the Wi-Fi band edge; range unknown. Bench1: E8 parks on 177 and holds without a tune error, which says nothing yet about the synthesizer (no signal) | Four channels (R8, E6–E8, with them HDZero R8 and Walksnail 5914). `-DC5PHY_MAX_MHZ=5885`, a lead decision (data plan stage 2) | **Session B** (B1b edge), session C |
| **6** | Video detection works on real FM video, PAL included | **35 %** needs changes | Only synthetic video tested; a window holds 6.4 lines; PAL and NTSC line periods differ by 0.7 %; PAL untested anywhere | `video`/`fp` fields and the stage 7 "never none" rule. `demod.c` tuning (medium); no raw capture dump exists to debug with | Quick look session C; full stage 4 (session E) |
| **7** | Sensitivity floor is within about 5 dB of an RX5808 station without the LNA | **35 %** (10 % with the LNA) | On paper kTB + ~6 dB noise figure lands near −95 dBm; the real noise figure and the 3-lane loss are unknown. Bench1: quantization-limited at gain 62 on both decodes (S3), so the floor is set by the decode step, not the noise: the LNA's case is stronger | BOM (LNA), range, whether v3 replaces v2: a lead decision (data plan stage 5) | Stage 5 (session E) |
| **8** | The 6-lane (3-bit) decode costs at most ~1 dB versus C5VRX's 8 lanes | **Raised after bench1: 60 %** that it costs more than 3 dB without the LNA (was 12 %) | Synthetic noise used sigma = 1.0 LSB. Bench1: the noise sits under one 4-bit step and both decodes are quantization-limited (S3); the no-signal floors differ by 5–9 dB (3-lane `p_mean` 2.00, 8-lane 0.25–0.60), so near the floor the dropped bit is not covered by noise. With the LNA it may not matter: one decision with rank 7 (data plan §4) | Pin plan and carrier PCB (4-lane needs the underside pads free, frees D6), or the LNA | B1d with the VTX (session B); stage 5 with the attenuators and the LNA (session E) |
| **10** | Real FM video keeps a constant envelope: `cv2` ≤ 0.5 (`ANALOG_CV2_MAX`), the third condition of every analog hit since the digital work | **25 %** that real video in multipath exceeds it at times | Model: FM video 0.02–0.3, OFDM about 1; nothing measured. New on the **analog core path** | Analog hits lost in multipath. The splitter is shared with the digital candidate rule, so moving it is a lead decision (data plan, section 4) | **Session B** (B1: `cv2` keyed, then walking the VTX about); stage 5 (`cv2` down to the threshold) |
| **12** | 8 ms tune settle and 200 µs switch settle are enough; the first sector after a retune is not systematically low | **15 %** | Public API retunes synchronously; PLL lock is microseconds. But sector 0 is always the first measurement after a tune, so a bias here is silent. Bench1: a 30 ms settle left the stuck windows as they were (4.4 % against 4.9 %, S4); where they fall by sector is the next `k` reading | A bias away from N in every bearing. `TUNE_SETTLE_MS` or a throw-away window (tiny) | `k` (session B); "first sector low" with the VTX (C2 step 2) |
| **13** | Mesh line, heartbeat, position, mapper integration, for analog and wideband reports | **15 %** | Desktop-tested JSON within 191 bytes; the Heltec serial path is the v2 proven setup | Fleet operation; configuration-level fixes | Stage 7 |
| **14** | `phy_set_freq` moves the synthesizer to any in-band MHz | **12 %** | C5VRX never validated it (its roadmap item is unchecked). But Espressif's own RF-test library calls `phy_set_freq` from its channel/frequency routines (`rf_test.o`, `wifi.o`), the disassembly shows a full retune, and the signature matches. Bench1: the call links and returns for every pull, up to +60 MHz (S5); whether the synthesizer moves needs a VTX | 32 of the 40 analog channels become unreachable; the station would be an 8-channel detector | **Session B** (B1b, 10 min) |
| **15** | PE42442 truth table: all lines low = RF4, default `SECTOR_SWITCH_TABLE` matches the cabling | **10 %** from the datasheet, plus human cabling error per station | Datasheet tables 5–6; README documents it | Every bearing rotated by 90°/180°/270°, silently. Config | Stage 6a |

Deviations from C5VRX the port introduces (cheap to flip; none of them
stopped the receiver on bench1, and they stay the first suspects if a later
board fails): PARLIO created before the PHY routes the
pads (C5VRX does the reverse); BLE/Thread/Zigbee/coexistence compiled in;
`esp_wifi_set_country_code("US")`; start at gain 62 (C5VRX starts at 52);
one-shot captures instead of a ring.

---

## Part 2 — Run order (for the lead)

Most uncertain first, inside the dependency gates. Session B exists because
nothing else can run before the receiver is alive; everything in it after B1 is
ordered by uncertainty. Session D is the main change versus the guide: the most
uncertain high-impact item (the bearing model) moves from last to third, tested
with one patch and no switch. The Wi-Fi part of the wideband checks moves into
session C because it is free there.

| Session | Who | What | Settles (Part 1 ranks) |
|---|---|---|---|
| **A** desk | lead | **Done 2026-10-08** (container and bench PC). Host tests (the suite runs twice); both builds; `nm` on the installed libs; libs' sdkconfig for the record; strong-symbol build | D1–D4 on the actual package: confirmed |
| **B** bench 1 | helpers + lead reachable | **Board-only half done 2026-10-24/25 on bench1** (data plan §0 S1–S4, S9; settled ranks 11, 3, 9). Left (Part 5): flash today's firmware and read `k` → B1 the VTX on A1, `cv2` keyed and walked → B1b `phy_set_freq` (R3, cross-checks, E4/E8 edges) → B1c gain scale → B1d 8 lanes with the VTX | 10, 14, 5, 4, 8, 12 (`k`) |
| **C** bench 2 | helpers | **C1 steps 1–2 done 2026-10-25** (S5) and C2's Wi-Fi steps with a phone hotspot (S7, S8). Left (Part 6): the hotspot rescan on today's firmware → `alias_drop` with a VTX on E5 → stage 3 selectivity, sector equality, sweep versus hold with a VTX → the Wi-Fi steps with a router, idle and saturated → quick video and `w` look | 5, 12, the Wi-Fi half of 2, first look at 6 |
| **D** outdoors | helpers measure, lead computes | Single-patch pattern sweep, 15° steps, 360° | 1 (early), patch gain |
| **E** bench 3 | helpers + lead | Stage 5 sensitivity and calibration (RX5808 comparison, LNA decision) and the filter-skirt record of sheet 10, **then** stage 4 video (NTSC, PAL, no camera) | 7, 4 (confirm), 10 (confirm), the skirt radius of 2, 6 |
| **F** bench 4 | lead + helper | Stage 6: truth table → sector table → full 4-patch K sweep | 15, 1 (final) |
| **G** field | all | Stage 7 as the guide | 13 |
| **H** kits | lead + helper | Stage 8, one record block per digital kit (OpenIPC, HDZero, DJI O4, Walksnail), as the guide's sheet 9 | 2 |

Hardware to line up in parallel so sessions D–H are not the long pole: four
patches and pigtails, a u.FL removal tool, a turntable or compass marks for 15°
steps, the PE42442 evaluation board, a step attenuator, NTSC and PAL cameras,
access to a v2 RX5808 station, a 5 GHz access point and a laptop to load it
(a phone hotspot stood in on bench1 for the idle and RF-alive checks, but it
cannot show a saturated link),
and the stage 8 kits. What to buy for all of this, with the analog VTX spec
session B needs, the attenuator set and the stage 8 kits in purchase order
under the no-goggles rule, is `docs/Level1-Bench-Shopping-List.md`.

---

## Part 3 — For helpers: words and what the screen shows

**The station** is the small XIAO ESP32-C5 board. It talks to the computer
over USB. The **serial monitor** is the program on the computer that shows what
the station prints and lets you type commands to it. Every line the station
prints is one `{ … }` record with `"name":value` pairs.

**Commands** are typed into the monitor and finished with Enter. The ones used
here:

| You type | What it does |
|---|---|
| `?` | Prints one status line (counters) |
| `h A1` | Hold channel A1 and print a **bench line** twice a second (also `h R3`, `h E8`, `h D1`, …) |
| `g 56` | Fix the receiver gain at 56 (any number 2–62). `g a` = automatic again |
| `v` | Run a video check on the held channel |
| `w` | Run a digital-link ("wideband") check on the held channel; takes about 2 s and prints one line |
| `k` | Prints one line saying where the scan's stuck windows happened (by sector, by channel). `k 0` starts the count again |
| `x` | Stop holding, go back to scanning all channels |

**A bench line** looks like this; this one is bench1 holding an empty A1 on
the normal build (2026-10-24), and yours will look much the same with no
transmitter:

```
{"info":"bench","ch":"A1","freq_mhz":5865,"wifi_ch":173,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":-5,"mod":0,"noise":0,"stuck":0,"step_std":0.9,"nf_dbm":-95,"captures":13884,"cap_err":0}
```

The fields you will be asked to read:

| Field | Meaning in plain words |
|---|---|
| `wifi_ch` | Which Wi-Fi channel number the radio was parked on before fine-tuning. Just write it down |
| `gain` | Receiver amplification setting, 2 (low) to 62 (high). Automatic mode turns it down in steps of 3 when the signal is very strong |
| `level_db` | How strong the signal is, in dB above "nothing there". 0.0 with no transmitter (bench1) |
| `p_mean` | Raw signal power. Exactly 2.00 with no transmitter: the smallest the normal build can read (bench1). On the 8-lane build of B1d it reads 0.25–0.60 instead |
| `q_phase` | How much the signal looks like a clean FM carrier, 0–100 %. 0 with no transmitter (bench1); a transmitter nearby is 50 or more |
| `cv2` | How much the signal's strength flickers. 0.00 with no transmitter on the normal build (bench1: every sample reads the same size); a steady FM transmitter reads under 0.3, a digital link about 1, bursty Wi-Fi above 1.5 |
| `clip` | Percent of samples that overloaded. Should be 0.0 unless the transmitter is very close |
| `cfo_khz` | How far the transmitter sits from the exact channel frequency, in kHz. With no transmitter a few kHz either side of 0, changing from line to line (bench1: −5 … +2); steady (same number within ±100) with one |
| `mod`, `noise` | 1/0 flags: `mod` 1 means "FM with content" (a camera), `noise` 1 means "noise-like". Both 0 with no transmitter on the normal build (bench1); `noise` 1 on the 8-lane build |
| `stuck` | 0 on almost every line while a channel is held. 1 means that window's raw data was frozen (every sample the same). While scanning, a few percent of windows read stuck on bench1 and nobody knows why yet: that is expected, and the `?` counter `bus_stuck` climbs with it. Write down any `stuck` 1 you see in a hold; **stop** only if most lines of a hold read 1 |
| `captures`, `cap_err` | How many data windows were taken, and how many failed. `cap_err` must stay 0 |

**Other lines you may see while scanning.** A line starting
`{"type":"analog_fm"` is a detected analog transmitter; `{"type":"wideband"`
is a detected digital link or busy Wi-Fi; `{"heartbeat":true` comes every
minute. Copy them when a step asks; otherwise let them pass.

**Jitter check.** With no transmitter the normal build reads `p_mean` 2.00 on
every line (bench1), so watch `cfo_khz` and `step_std` instead: they change a
little from line to line. A line with `cfo_khz` 0 and `step_std` 0.0 together
is what a stuck window looks like; several in a row while holding, write that
down even if `stuck` says 0.

**VTX** = the small FPV video transmitter used as the test signal. Use its
lowest power setting, keep it a few metres from the station, and keep its
antenna on (never power a VTX without an antenna). Know how to set its channel
(band letter + number, for example A1 = 5865 MHz) and confirm the channel on its
display or LED code.

**Stop rules.** Whenever a step says "stop", stop there, keep the station
running, save the log, and send the log plus the record sheet to the lead. Do
not try to fix things unless the step tells you what to change.

**Saving the log.** Start the monitor from the `level1-c5phy` folder with:

```
pio device monitor -b 115200 --echo -f log2file
```

`--echo` shows what you type; `log2file` writes everything to a file in the
`logs` folder under `level1-c5phy`, named `device-monitor-<date>-<time>.log`
(the folder is not committed). At the end of the session rename it
`YYYY-MM-DD-<board name>-stage<N>.log` (for example
`2026-10-12-bench1-stage1.log`) and hand it in with the record sheet; the lead
files both under `docs/bench/`.

**Open the monitor before you press RESET.** The ready line (B0) prints once,
when the board starts; a monitor opened afterwards misses it. Typing lines
while bench lines scroll twice a second is awkward but works: the station reads
what you type even when it is split by its own output.

---

## Part 4 — Session A: desk checks — done 2026-10-08

Done on the bench PC and in the cloud container (data plan log). A machine
that joins the bench later: install PlatformIO Core 6.2.0 or newer on Python
3.10 or newer as the root README's "Install the tools" says (its
troubleshooting table has the Windows `uv.exe` crash and its detour), then
from the checkout run `make -C level1-c5phy/test/host` (both runs end with
`ALL TESTS PASSED` and `check_json: … OK`; on Windows in WSL) and the two
builds, `pio run -e seeed_xiao_esp32c5` and
`pio run -e seeed_xiao_esp32c5_dualband` from `level1-c5phy/`. The build log
names pioarduino 55.03.312-1 and Arduino core 3.3.12. Since 2026-10-25 the
default build references the undocumented PHY calls strongly, so a build that
links is the symbol check: the archive `nm` and the separate strong-symbol
build of the original session A are retired (data plan §0 S1).

---

## Part 5 — Session B, second half: the VTX steps (helpers, lead reachable; about 1½ hours)

**Already done on bench1 (2026-10-24/25), not repeated:** B0 flash and boot,
B1 steps 1–3 (the empty channel), B1d without a signal. What they showed is in
the data plan's §0 (S1–S4); the expected values below come from it.

**Purpose.** Flash the firmware changed after that session, count where the
stuck windows happen, then everything that needs a transmitter: how a real FM
signal reads, whether the station can tune to any FPV channel, what one step
of gain is worth, and 6 lanes against 8 with a signal.

**You need.**
- The XIAO ESP32-C5 with its stock antenna clipped onto the tiny gold U.FL
  socket. Nothing else connected to any pin. Do **not** push the board into a
  breadboard or a socket: six of its pins must touch nothing at all.
- A USB-C **data** cable (a charge-only cable shows no port).
- The computer with PlatformIO and the `level1` branch **updated**
  (`git pull`), terminal opened in the `level1-c5phy` folder; in
  `platformio.ini` the line `; -DIQ_LANE_BITS=4` starts with `; ` (the normal
  build).
- A VTX with a camera attached, set to lowest power, that can be set to A1
  (5865), R3 (5732), A7 (5745), B1 (5733) and, if it has them, E4 (5645) and E8
  (5945). A full 40- or 48-channel unit has them, a 37-channel "US" unit does
  not; `docs/Level1-Bench-Shopping-List.md` names the unit, the camera and the
  attenuators that B1c needs.
- The record sheet at the end of this part, printed or open.

**Before you start.** Lay the board on a non-conductive surface (paper, wood,
plastic) with only the USB cable attached. VTX off. Note the time.

### B0 — Flash today's firmware and count the stuck windows (10 min)

1. Erase and flash, each from the `level1-c5phy` folder:
   ```
   pio run -e seeed_xiao_esp32c5 -t erase
   pio run -e seeed_xiao_esp32c5 -t upload
   ```
   If the first command cannot find the board: hold the board's **BOOT**
   button, tap **RESET**, release BOOT, and run the command again. Near its
   top the upload prints the chip and its revision (a line with `ESP32-C5` and
   `(revision v…)`): copy it onto the record sheet.
2. Start the monitor (`pio device monitor -b 115200 --echo -f log2file`),
   **then** press RESET once, so the ready line prints while the monitor is
   open.
3. The ready line starts with `{"info":"c5phy v3 station ready"`. Expected
   (bench1): `"rf":true`, `"phy_set_freq":true`, `"iq_lane_bits":3`,
   `"channels":50`, `"gain_step":3`, `"bearing_k":2.97`. A line
   `E (…) MSPI Timing: Failed to allocate dummy cacheline for PSRAM memory barrier!`
   before it is known and harmless.
   - `"rf":false` → **stop**. The line just before it starts with
     `{"info":"error"` and names what failed. Copy that whole line.
   - Text like `Guru Meditation` or `abort()`, or the ready line repeating by
     itself → the board is crashing and restarting → **stop**, save the log.
4. About 10 s later a line with `"heartbeat":true` comes: `tune_fail` and
   `cap_err` 0. `bus_stuck` is not 0: bench1 counts a few percent of the
   scan's windows stuck (12–17 a sweep on this build); write it down and go on.
5. Type `k 0` (the reply is `{"info":"stuck","reset":true}`). Leave the
   station scanning for two minutes without typing, then type `k` and copy the
   whole line it prints, then `?` and copy that line too.

### B1 — The receiver with a transmitter (15 min)

Channel A1 is special: it is exactly a Wi-Fi channel, so this test does not
involve the fine-tuning function. It tests the receiver alone.

1. VTX still off. Type `h A1`. The reply's `wifi_ch` should be 173 (if it is
   169 or 177, write it down and use A4 instead of A1 for the rest of B1, with
   the VTX on A4). Bench lines now come twice a second; with no transmitter
   they read like the sample in Part 3: `gain` 62, `level_db` 0.0, `p_mean`
   2.00, `q_phase` 0, `cv2` 0.00, `stuck` 0.
   - Most lines with `stuck` 1 → **stop** (check that truly nothing touches
     the board's pins, then save the log).
2. Switch the VTX on, on A1, a few metres away with the camera connected.
   Within a second the bench lines should change: `level_db` up by 30 or more;
   `gain` may drop below 62 in steps of 3; `q_phase` 50 or more; `cv2` under
   0.3; `mod` 1; `cfo_khz` now a steady number (same within ±100 from line to
   line). Write down typical `level_db`, `gain`, `q_phase`, `cv2`, `cfo_khz`,
   `clip`.
   - `level_db` went up but `q_phase` stayed under 20 → write it down and
     double-check the VTX is really on A1. If it is, **stop**.
   - `cv2` above 0.4 with the camera on → write it down with the `level_db`;
     continue, and make sure the lead sees it (it decides whether real video is
     steadier than the firmware assumes).
   - Nothing changed at all → try the VTX on A2 and type `h A2`. If still
     nothing, **stop**.
3. Switch the VTX off. The next lines should look like step 1 again. Write
   "yes" or "no".
4. Switch it on again and slowly walk it 1 m closer and 2 m further away, then
   around the room behind furniture and people. `gain` should go down when
   close and back up when far; `level_db` should follow the distance;
   `q_phase` should stay 50 or more. Write down the **highest** `cv2` you see
   during the walk and where the VTX was at that moment.

### B1b — Can it tune to any channel? (10 min)

This is the most important single reading of the session for the lead. R3
(5732 MHz) is 12 MHz away from the nearest Wi-Fi channel, so the station must
use its fine-tuning function to get there.

1. Set the VTX to R3 and leave it on. Type `h R3`. Write down `wifi_ch` from the
   reply (expected 144).
2. Watch ten bench lines. Write down `level_db`, `q_phase`, `cv2`, `cfo_khz`,
   `mod`.
   - Tuning works: it looks like B1 step 2 (`q_phase` 50 or more, `cfo_khz`
     steady within ±2000).
   - Tuning does not work: `level_db` is up but `q_phase` stays low (under 20)
     and `cfo_khz` jumps around.
3. Cross-check A: set the VTX to A7 (5745), type `h A7`. A7 needs no
   fine-tuning, so it must look like B1 step 2. Write down `q_phase` and
   `cfo_khz`.
4. Cross-check B: set the VTX back to R3, type `h B1` (B1 is 5733, one MHz above
   R3, and the radio reaches it from the other side: expected `wifi_ch` 149). If
   tuning works, `q_phase` is high and `cfo_khz` is about 1000 lower than the R3
   reading from step 2. Write down `wifi_ch`, `q_phase` and `cfo_khz`.
5. Edges, if the VTX has these channels: VTX on E4, type `h E4`, write down
   `wifi_ch` (expected 128), `q_phase`, `cfo_khz`. VTX on E8, type `h E8`, write
   down the reply (either a hold line with `wifi_ch` 177, or an error line that
   says `tune failed`) and, if it held, `q_phase` and `cfo_khz`.

### B1c — What is one step of gain worth? (5 min)

1. VTX on A1, type `h A1`. Type `g 62`. Look at `clip`: it must be 0.0 and
   `p_mean` should be under about 50. If `clip` is not 0.0, move the VTX further
   away (or add attenuation) until it is. Do not move the VTX again during this
   step.
2. Type each of these, wait for three bench lines, and write down `p_mean` from
   the third line: `g 62`, `g 56`, `g 50`, `g 44`, `g 38`, `g 32`, `g 26`,
   `g 20`. Stop early if `p_mean` falls below about 8.
3. Type `g a`.

(The lead turns this into dB per step: 10·log10 of the ratio between
neighbouring rows is the worth of 6 index steps; the firmware assumes 6.0 dB
there and steps by 3 indices at a time.)

### B1d — 8 lanes with a signal (15 min)

This repeats B1 steps 1–3 and two rows of B1c with the firmware reading the
radio through 8 pins instead of 6. Nothing is wired; only the firmware
changes. Bench1 already has the 8-lane numbers with no transmitter: `p_mean`
0.25–0.60, `level_db` −5 to −9, `cv2` 1–3.5, `noise` 1 and `nf_dbm` −111 are
all expected there, not faults.

1. Close the monitor (Ctrl+C). Open `platformio.ini` in the `level1-c5phy`
   folder. Find the line `; -DIQ_LANE_BITS=4` and delete the `; ` at its start
   so it reads `-DIQ_LANE_BITS=4`. Save.
2. Erase, upload, start the monitor and press RESET as in B0 steps 1–2. The
   ready line must now show `"iq_lane_bits":4`.
3. Repeat B1 steps 1–3 with the VTX in the same places as before, and write
   the values in the "8 lanes" column of the record sheet.
4. Repeat B1c for `g 62` and `g 56` only, with the VTX and attenuators exactly
   as in B1c.
5. Put the `; ` back in front of `-DIQ_LANE_BITS=4`, save, and run erase +
   upload again so the board is back on the normal build. Confirm
   `"iq_lane_bits":3` in the ready line.

### Session B record sheet (second half)

| Item | 6 lanes | 8 lanes |
|---|---|---|
| Upload: the chip line with its revision | | — |
| Ready line: `rf` / `phy_set_freq` / `iq_lane_bits` / `channels` | | |
| First heartbeat: `tune_fail` / `cap_err` / `bus_stuck` | | — |
| `k` after two minutes of scanning (whole line) and the `?` after it | | — |
| `h A1` → `wifi_ch`; quiet lines as expected? any `stuck` 1? | | |
| VTX on A1: `level_db` / `gain` / `q_phase` / `cv2` / `cfo_khz` / `clip` / `mod` | | |
| VTX off again → back to the quiet values? | | |
| Walk-about: gain follows? highest `cv2` and where | | — |
| `h R3`: `wifi_ch` / `level_db` / `q_phase` / `cv2` / `cfo_khz` / `mod` | | — |
| `h A7` (VTX on A7): `q_phase` / `cfo_khz` | | — |
| `h B1` (VTX on R3): `wifi_ch` / `q_phase` / `cfo_khz` | | — |
| `h E4`: `wifi_ch` / `q_phase` / `cfo_khz` | | — |
| `h E8`: held or failed / `wifi_ch` / `q_phase` / `cfo_khz` | | — |
| Gain table `p_mean` at g = 62 / 56 / 50 / 44 / 38 / 32 / 26 / 20 | | (62 / 56 only) |
| Any `{"info":"error"…}` or `"err":"capture…"` lines (copy one) | | |
| Log file name, start and end time, VTX model and power setting, attenuators used | | |

---

## Part 6 — Session C, what is left (helpers, about 1 hour)

**Already done on bench1 (2026-10-25), not repeated:** C1 steps 1–2 (every
listed channel parks where the lead expects; A2 on Wi-Fi 165, data plan §0
S5) and the Wi-Fi steps with a phone hotspot standing in for the access point
(S7, S8).

**Purpose.** Confirm the fix made after the hotspot scan (no `wideband` report
for an empty channel), check that the station reports a transmitter only on
the right channel and that the four "sector" readings agree, that an idle
access point is never reported and a saturated one reads `dot11`, and take a
first look at video detection. C0 and C2w need no VTX and can run on their
own.

**You need.** Same as session B, on today's firmware, plus: the phone hotspot
of bench1 (5 GHz, channel 149) with the bench PC on it; a 5 GHz Wi-Fi router
that can be set to a fixed channel (149 is easiest: A7 sits on it), with one
computer wired to it and a second on its Wi-Fi, to copy a large file between
them for a few minutes.

### C0 — The hotspot again, on today's firmware (10 min)

1. VTX off. Hotspot on (5 GHz, channel 149), the bench PC on it, a speedtest or
   a long download running through it. Type `x` and let the station scan for
   three minutes.
2. Copy every line that starts with `{"type":"wideband"`. Expected: none on a
   channel with nothing on it (bench1 got one on R3 before the fix). A7
   (5745, the hotspot's own channel) may appear while the load runs: that is a
   real emitter; its `duty` should no longer read 100 on every line.
3. Stop the load, hotspot still on, and let the station scan for two more
   minutes. Expected: no `wideband` line. Copy any that appears.
4. Type `h A7`, start the load again and type `w` three times, copying each
   line; then stop the load and type `w` once more. Expected: `cls` `dot11`
   with the load; with no load `duty` 0 or close to it (bench1's firmware read
   `duty` 100 on an empty pass; today's does not count windows at the floor).
   Type `x` when done.

### C1 — `alias_drop` (VTX on E5; 10 min)

1. If the VTX has E5 (5885): set it there, type `x`, and let the station scan
   for 10 minutes. Then wait for a heartbeat and write down `alias_drop` and
   the list of channels that appeared in `"type":"analog_fm"` lines (the
   `"ch"` and `"band"` fields).

### C2 — Selectivity and the sector check (VTX; 15 min)

1. VTX on R3, camera on, a few metres away. Type `x`. Within two sweeps (about
   5 s) lines with `"type":"analog_fm"` appear. For one minute, write down every
   distinct `"band"`+`"ch"` that appears. Expected: R3 only, or R3 with B1 and F1
   folded away; nothing more than 40 MHz away from 5732. Also note whether any
   `"type":"wideband"` line appeared (expected: none).
2. In three of those R3 lines copy the four numbers in `"sectors":[…]` and the
   `"sector":` value. On this bare board all four numbers should be within about
   1 dB of each other and `sector` should vary between 0 and 3 at random. If the
   first number is always clearly lower than the other three, write "first
   sector low".
3. Type `h R3`. Write down `rssi_dbm` from a bench line. Compare with
   `rssi_dbm` in the scanning lines from step 1: they should agree within about
   2 dB. Write both down.
4. Any channel more than 40 MHz away that appeared in step 1: type `h` + that
   channel and write down `q_phase`, `cv2` and `cfo_khz`.
### C2w — Wi-Fi with a router (20 min)

1. VTX off. Router on 5 GHz channel 149 (or hold the FPV channel nearest the
   router's channel: A-band channels A1–A7 sit on Wi-Fi channels 173–149; D1
   sits next to Wi-Fi 36). Type `h A7` (or that channel). Expected while the
   network is idle: most lines at the empty-channel values of Part 3, now and
   then a raised one (a beacon) with `q_phase` under 40. Write down
   `level_db`, `q_phase`, `cv2` of a raised line. Type `w` and copy the whole
   line it prints (expected `duty` 0 or close).
2. Start a long file copy between the wired computer and the Wi-Fi one
   (several minutes). Type `w` again, copy the line. Expected: `cls` `dot11`
   and `duty` 90 or more.
3. Stop the copy, type `x`, and let the station scan for two minutes with the
   network idle. Expected: no `"type":"wideband"` line. Copy any that appears.

### C3 — Quick video and digital-check look (5 min)

1. VTX on R3 with the camera connected. Type `h R3`, then `v`. Two lines come
   back: one starting `{"info":"video"` and one `{"info":"video_windows"`. Copy
   both lines whole.
2. Still holding R3, type `w` and copy its line. (On an analog transmitter it is
   expected to say `dot11`; that is known and is why the station only runs this
   check on channels that failed the analog test.)
3. Unplug the camera from the VTX (VTX still on), type `v` again, copy both
   lines.

### Session C record sheet (what is left)

| Item | Reading |
|---|---|
| C0: `wideband` lines in 3 min under the hotspot's load (copy all) | |
| C0: `wideband` lines in 2 min with the hotspot idle | |
| C0: `w` on A7, three with the load, one without | |
| C1: 10 min with the VTX on E5: `alias_drop`, channels reported | |
| VTX on R3 scanning: channels reported in 1 min; any `wideband` line? | |
| Three `sectors` arrays and `sector` values; "first sector low"? | |
| `rssi_dbm` held vs scanning | |
| Stray channel(s): `q_phase` / `cv2` / `cfo_khz` | |
| C2w: router idle, channel held: `level_db` / `q_phase` / `cv2` of a raised line; `w` line | |
| C2w: router under a file copy: `w` line | |
| C2w: two minutes scanning, router idle: `wideband` lines (expected none) | |
| `v` with camera: both lines; `w` on the held R3 | |
| `v` without camera: both lines | |

---

## Part 7 — Session D: single-patch pattern sweep (helpers outdoors; lead computes K)

**Purpose.** The bearing the station reports depends on how each patch
antenna's signal strength falls off as the transmitter moves off its centre
line. Nobody has measured this yet; the firmware derives its constant from the
antenna gain through a formula with an unmeasured fudge factor. This session
measures one patch's pattern all the way round. The lead then works out the
bearing constant and whether the method is sound, before any switch or box is
built.

**You need.**
- The XIAO with the stock antenna removed and one 5.8 GHz patch antenna
  connected through a u.FL-to-SMA pigtail. The u.FL socket is fragile: press
  the plug straight down until it clicks; to remove, lift straight up with a
  u.FL tool or a fingernail under the plug; never twist or pull on the cable.
  Rated for only a few dozen cycles.
- A laptop on battery (the monitor runs on it), and the patch fixed to
  something you can rotate in 15° steps: a turntable with angle marks, or a
  tripod with a compass rose taped on. The patch must stay at the same height
  and position while it turns; only its direction changes.
- The VTX with camera, lowest power, on A1, on a stand 20–30 m away at the same
  height, with clear line of sight, no people or cars moving between.
- An open outdoor space; ideally no buildings within 20 m to the sides.

**Steps.**
1. Point the patch straight at the VTX. This is 0°. Type `h A1`. Type `g a`
   (automatic gain). Watch ten bench lines: `q_phase` should be 50 or more.
   Write down `level_db`, `gain`, `q_phase`, `cv2`. If `q_phase` is low,
   something is wrong with the setup (channel, line of sight): fix it before
   continuing.
2. Turn the patch 15° clockwise (to 15°). Wait for five bench lines and write
   down the middle value of `level_db` and the `gain` and `q_phase` that go
   with it.
3. Repeat step 2 for every 15° step up to 345° (24 rows in total). Keep everyone
   still during each reading.
4. Go back to 0° and read again; it should match row 1 within 1 dB. Write it
   down.
5. If time allows, repeat the whole sweep with a second patch from the same
   batch (the four patches must behave alike).

**Record sheet.** One row per angle: angle, `level_db`, `gain`, `q_phase`,
note (for example "car passed"). Plus VTX distance, height, channel, weather,
patch serial/mark, and the stock-antenna `level_db` at the same spot if you
have time to swap back once.

**For the lead.** From the pattern g(θ) build the four sector responses
P_k(θ) = g(θ − az_k) for az = 0/90/180/270, compute the neighbour difference
D(θ) = P_{k+1} − P_{k−1} over θ ∈ [−45°, 45°] around each axis, fit θ ≈ K·D,
and check monotonicity and the side-lobe region. Correct `level_db` with the
measured dB-per-gain-step from B1c wherever `gain` changed during the sweep.
Compare K with the boot line's derived `bearing_k` (2.97 at 8 dBi); the data
plan's stage 6 rule turns the ratio into `BEARING_K_SCALE` (2.5 × measured /
derived) for the antenna family, and the on-axis level against the stock
antenna gives the patch gain to enter in Station setup.

---

## Part 8 — Later sessions (lead; the guide's own checklists plus these notes)

- **Session E, stage 5 first:** as the guide, including the filter-skirt record
  of sheet 10; fit the slope separately in the gain-stepped region and the
  fixed-62 region (confirms B1c); record `cv2` down to the threshold (confirms
  rank 10); measure the floor and the slope in both lane modes, with and
  without the LNA: bench1 found both decodes quantization-limited at gain 62,
  so the lane count and the LNA are one lead decision (data plan §4), as is
  the v2/v3 verdict (data plan stage 5). **Then
  stage 4:** as the guide; before the PAL attempt consider adding a raw-window
  dump command to the bench console so failed windows can be replayed through
  `test/host`.
- **Session F, stage 6:** as the guide and the README section "Checking the
  mapping on the bench" (`t 000/100/010/110` → ports; `s 0..3` → patches; walk
  the VTX clockwise → `sector` 0→1→2→3). Use the session D pattern to predict
  the sweep before running it; the data plan's stage 6 rules place the measured
  K (`custom_bearing_k` per station, `BEARING_K_SCALE` per antenna family).
- **Session G, stage 7:** as the guide.
- **Session H, stage 8:** one record block per digital kit as the guide's
  sheet 9; the data plan's stage 8 rules are long and specific, read them
  before the kit is switched on. Keep any analog VTX off or more than 30 MHz
  away during stage 8.
- **Dual-band:** stages 1, 5, 6 and 8 repeated on the `seeed_xiao_esp32c5_dualband`
  build with dual-band patches, per the data plan; not part of the sessions
  above.

Recommended small firmware addition (not part of this document; a proposal
for the lead): a raw I/Q window dump command. (The ready-line report of the two
optional calls proposed here before is moot since the build references them
strongly; the `k` stuck-window counts proposed after bench1 are in the
firmware since 2026-10-25.)
