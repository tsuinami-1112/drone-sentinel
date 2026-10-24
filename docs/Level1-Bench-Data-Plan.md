# Level 1 station v3 — bench data plan

The working memory for the bench campaign. Written 2026-10-08, before any
hardware had run, at firmware commit `bd98938` on `level1`. A session that is
told "I'm going ahead with the bench tests now" reads this file first, in
full, and then works the loop in section 1 for every batch of data the user
brings. Everything a fresh session needs is here or linked: what each stage
measures, which constant, line of code, document or mapper row rests on that
measurement, what to change for each outcome, how to verify, and where to log
what was done. Keep this file current: it is edited in the same commit as the
change it describes.

Companion documents: the procedure itself is the bench guide
(`docs/Level1-Station-v3-C5PHY-Bench-Guide.html`, rendered to the `.pdf`
beside it; stages 0–8 on sheets 6–9, the kit index and the skirt record on
sheet 10); the reasoning behind every threshold is
`docs/Level1-Detection-Internals.md`; the root `README.md` carries the
condensed bring-up checklist and the troubleshooting table. The records the
user fills go to `docs/bench/` (see its README). The order the stages are
run in, with every assumption ranked by how likely it is wrong today and the
first sessions written out for helpers, is `docs/Level1-Bench-Run-Order.md`;
the decisions after each measurement stay here. What to buy for the bench,
per stage and in purchase order, with the corrections found when the list was
reviewed against the stage 8 rules, is `docs/Level1-Bench-Shopping-List.md`.

---

## 0. Ground truth

**Branches.** `level1` holds the station firmware (`level1-c5phy/`), these
docs and the root README; it is pushed directly. The mapper and the home node
live on `level2-main` (`mesh-mapper.py`, `node-mode-dualcore/`, `mapper_test/`,
`docs/REFERENCE.md`) and the mapper alone on `standalone-mapper-meshtastic`
(`mesh-mapper.py`, `mapper_test/`, `docs/REFERENCE.md`, `README.md`); both
take changes through a feature branch and a pull request, never a direct
push, and their shared level 1 functions stay byte-identical
(`docs/tools/mapper-parity.py` checks). The mixed level 1 + 2 station branch
is retired and will be restarted from `level1` plus `level2-main` later.

**Builds and tests.** From `level1-c5phy/`: `pio run -e seeed_xiao_esp32c5`
and `pio run -e seeed_xiao_esp32c5_dualband` (PlatformIO Core 6.2.0 or
newer, the minimum the pinned platform's `engines` field accepts, on Python
3.10 or newer, which the platform's `penv_setup.py` insists on; pioarduino
platform pinned in `platformio.ini` to release 55.03.312-1, Arduino core
3.3.12 / ESP-IDF 5.5). The pin replaced pioarduino's `stable` URL on
2026-10-08, when the two assets were byte-identical: PlatformIO never
re-downloads a platform it installed from a URL, so a `stable` URL can leave
an older copy building for ever, and `level2-main` pins the same release.
Host tests: `make -C level1-c5phy/test/host` (gcc, python3; runs the suite
twice, default plan and `-DDUAL_BAND=1 -DLOWBAND=2`, then `check_json.py`).
CI (`.github/workflows/firmware.yml`) runs the same on every push to
`level1`. Nothing is pushed red: host tests and both builds first. In the
Anthropic cloud container the PlatformIO toolchain download fails TLS
verification until the proxy CA (`/root/.ccr/ca-bundle.crt`) is appended to
the certifi `cacert.pem` files pioarduino's `penv_setup.py` points at; the
user authorised that edit on 2026-10-07; never disable verification instead.
On a Windows machine whose VPN or security product injects a Winsock LSP,
the `uv.exe` that `penv_setup.py` runs crashes with exit 3221225622
(`0xC0000096`) and the build stops at "Failed to install Python dependencies
into penv"; the root `README.md` troubleshooting table has the fix and the
detour.

**Where the numbers live.** Every tunable is a `#define` in
`level1-c5phy/include/config.h`, overridable per station with `-D` in
`stations.ini` (gitignored; `stations.example.ini` documents it; the Station
setup task `tools/station.py` writes it from `custom_*` options:
`custom_node_id`, `custom_station_lat/lon`, `custom_station_heading`,
`custom_antenna_dbi[_24]`, `custom_antenna_beamwidth_deg[_24]`,
`custom_bearing_k[_24]`). Three numbers are not in `config.h`: the bandwidth
bucket edges 0.78 / 0.52 / 0.30 on `r1` (`wb_bucket_mhz`, `src/wideband.c`,
which the host test calls directly), the lag set 1 / 4 / 128 / 512 / 2667
(`iq_lag_features`, `src/demod.c`), and the per-window metric constants
(`Q_POWER_MIN` 8, `COH_LIMIT` 32 = ±45°, `MOD_STD_DEG` 4, `MOD_Q_MIN_PCT` 20,
the video search `W_MIN/W_MAX/W_IDEAL`, `PAL_PERIOD`, `NTSC_PERIOD`,
`PERIOD_TOL` 2.5 %, `MIN_SWING` 4, all in `src/demod.c`). The radio ceiling
`C5PHY_MAX_MHZ` defaults to 5945 in `src/fpv_channels.c`.

**The host test runs the firmware's own wideband rules.** Since 2026-10-25
the class rule, the confidence, the bucket, the candidate gate and the
confirmation's on/duty decision live in `src/wideband.c` (plain C, called by
`main.cpp`), and `test/host/test_level1.c` includes `config.h` and calls
`wideband.c` directly: a threshold changed in `config.h` is what the test
checks, with no copy to mirror. (Before, the test carried its own fallbacks
and a copy of the class rule and bucket edges.)

**Where a changed number is quoted** (grep the old value across these before
committing; the list is the usual places, not a guarantee):

| Constant / fact | Quoted in |
|---|---|
| `DETECT_LEVEL_DB` 8, `Q_MIN` 40, `ANALOG_CV2_MAX` 0.5 | `config.h`; boot line (`threshold_level_db`, `q_min`); root README "How it works" and bring-up table; `level1-c5phy/README.md` detection section; internals §3; bench guide stages 1, 3, 5; `test_level1.c` gate assertions |
| `RSSI_CAL_*`, `RF_NOISE_POWER[_24]`, the −87 dBm threshold, the −95 reference | `config.h`; sample boot/heartbeat lines in both READMEs and bench guide sheet 4; internals §2–3; bench guide stage 5 ("expect −85 to −90"); `mapper_test/level1_bearing_sim.py` (`level = rssi + 95`) on both mapper branches |
| `WB_*` thresholds, duty 75 / 90, bucket edges | `config.h`; `wideband.c` (`wb_class`, `classify`, `wb_bucket_mhz`); internals §4 tables; `level1-c5phy/README.md` wideband table; bench guide stage 8 expected rows; mapper `WIDEBAND_SYSTEMS` (buckets, `duty_min` 90) and both `docs/REFERENCE.md` |
| `WB_ANALOG_OWN_MHZ` 30, `WB_FOLD_MHZ` 25, `PEAK_PICK_MHZ` 20 | `config.h`; `sweep_decide.c` comments; `test_level1.c` `test_sweep_decide` (`sw_rules`); internals §3–5; root README troubleshooting; bench guide sheet 10 |
| Antenna model: 8 dBi → 72°, K 2.97–3.0 °/dB, σ 10°, `BEARING_K_SCALE` 2.5 | `config.h`; `station.py` (`DEFAULT_DBI`, `BEAMWIDTH_MIN_DEG`); `stations.example.ini`; root README antenna section; `level1-c5phy/README.md` setup table; internals §8; bench guide stage 6 and the boot line's `bearing_k` |
| Channel counts 50 / 55 / 58, `C5PHY_MAX_MHZ` 5945, the 60 MHz pull | `fpv_channels.c`; `config.h` comments; both READMEs; internals §6; bench guide stage 2; `test_channels` (expects 50 / 58) |
| `SECTOR_SWITCH_TABLE` | `config.h`; root README "Line states for each sector"; bench guide sheet 2 and stage 6 |
| `WB_ON_SNR_DB` 3, the decode floor (`snr_from`) | `config.h`; `wideband.c` (`window_on`, `wb_candidate`); `main.cpp` (`snr_from`, the `scan_freq` comment); internals §4 (candidate, decode floor, constants table); root README "How it works" item 4; `test_wideband_floor` |
| `C5PHY_STRONG_PHY_SYMBOLS` 1 | `config.h`; `c5phy_rf.cpp` header; `platformio.ini` comment; root README troubleshooting (`"rf":false`); this plan's stage 0 rule; run order Part 4 |
| Stuck-window background (S4 above) | `stuck_stats.h`; root README bring-up table, `k` row and troubleshooting; `level1-c5phy/README.md` `k` row; run order Parts 3 and 5; bench guide stage 0 and 1 rows |
| Power and timing (≈ 200–240 mA; sweep 2.07 s measured quiet, 2.5 s estimated busy; 2 s `w`) | root README power row, build table and sector section; bench guide sheets 2 and 3 (sweep) and stage 7 (current); internals §6 (sweep time) and §10 (`w`) |
| Status: "nothing bench validated", "nothing has run on hardware", *(model)* markers | bench guide footers (ten, one per sheet), the cover's status row and callout; root README build table ("Proven on hardware"); `level1-c5phy/README.md` line near the top; internals intro note and §10 "what only the bench can prove"; `CLAUDE.md`'s bench paragraph; `config.h` comments marked `(model)` |

**Model harness.** `level1-c5phy/test/model/sim_skirt.c` is the FM-through-
filter-skirt model that set `WB_ANALOG_OWN_MHZ` (two filter shapes, three
levels, two video deviations). Re-run it with a filter shape fitted to the
stage 5 skirt record when deciding the radius.

**Settled on the bench: do not re-test.** What the bench has established, so
that no later session puts it back on a bench list (the run order, the bench
guide, the README checklist). An entry is re-opened only for the reason in its
last column; a session that measures one again says why in the log. Bench1
is the XIAO ESP32-C5 `FFCB`, bare, stock antenna; each record names its
firmware.

| # | Finding (bench1, 2026-10-24/25) | Record (`docs/bench/`) | What rests on it now | Re-open if |
|---|---|---|---|---|
| S1 | The build links all eight undocumented PHY/MAC calls (`nm` on the flashed `firmware.elf`, a weak-symbol build then: all `T`), the PHY holds receive-only and PARLIO streams: ready line `"rf":true`, `"phy_set_freq":true`; about 97 captures a second while scanning, `cap_err` 0, `tune_fail` 0, heap steady at 67 912 B | `2026-10-24-bench1-stage0.md` | strong symbols are the default (`C5PHY_STRONG_PHY_SYMBOLS` 1): the build is the link check, so the archive `nm` step of session A and the proposal to report the two unreported calls in the ready line are retired | the platform pin moves, or a board prints `"rf":false` |
| S2 | Bring-up values: 50 channels, `iq_lane_bits` 3, `gain_step` 3, `bearing_k` 2.97; the first heartbeat about 10 s after boot. The ready line prints once, at boot: the monitor has to be open before RESET. Heartbeat `seq` is the last detection's number (0 on a quiet bench). The ESP-IDF line before the ready line, `MSPI Timing: Failed to allocate dummy cacheline for PSRAM memory barrier!`, changes nothing the station does (`cap_err` 0) | `2026-10-24-bench1-stage0.md` | the expected values of the run order and the guide's sheet 6 | that line comes with failed captures or a shrinking heap |
| S3 | The receiver is quantization-limited at gain 62. No signal, 3 lanes: every sample decodes to ±1, so `p_mean` 2.00, `cv2` 0.00, `q_phase` 0, `noise` 0, `level_db` 0.0 on every line. 8 lanes: the noise stays mostly under one 4-bit step, with a positive DC bias: `p_mean` 0.25–0.60, `cv2` 1.0–3.5, `noise` 1, `level_db` −5 to −9 | `2026-10-24-bench1-stage1.md` | `RF_NOISE_POWER` 2.0 is the 3-lane decode floor, not a measured noise; the noise statistics of stage 1 (`cv2` about 1, `q_phase` under 15) cannot be read on 3 lanes; an empty window at gain g reads `GAIN_MAX − g` dB, which is why the wideband rules test `snr_from` (`WB_ON_SNR_DB`) | the chain or the decode changes (LNA, 8 lanes: §4); the floor is then measured again |
| S4 | Stuck windows (every sample identical on the wired lanes) are a background of the scan: 7.3 % of the windows on 3 lanes, 4.9 % on 8, 4.4 % on 8 with a 30 ms settle, so not the wiring (nothing is wired) and not the settle time. None in quiet holds: A1 0 of 936 lines (3 lanes), A7 0 of 101 (8 lanes), the 18 channels held in C1. But A7 held in phone-hotspot traffic on 3 lanes showed them (2 of 136 lines, 10 of 112), and a 3-lane scan with the hotspot up counted 1.3–2.2 a sweep against 12–17 without it. On 8 lanes a stuck window decodes to (0, 0), `p_mean` 0.05 after the clamp: the `nf_dbm` −111 of every 8-lane heartbeat | stage 0 records of both days; `2026-10-25-bench1-stage1-hotspot.md` | a few percent of the scan's windows stuck is not a stop condition (run order, README, guide). The cause is open: candidates in stage 0's status, counted by `k`. The scan takes a stuck window as a reading (sector level, `nf_dbm`): §4 | a quiet hold shows `stuck` 1, or the scan's rate goes well over 10 % |
| S5 | The "US" table refuses Wi-Fi 169 on this core and accepts 173 and 177; `rf_tune` then parks on the next-ranked centre: A2 on 165, a +20 MHz pull (held). By the same ranking R6 (+18) and F6 (+15) park on 165 and B7 on 173 (−18), not held. The other 17 channels of the C1 list park where the code predicts (E8 on 177 at +60 MHz, D1 on 36, L4 on 100); none replies "tune failed" | `2026-10-25-bench1-stage2.md` | internals §6, the `fpv_channels.c` and `RF_COUNTRY_CC` comments | another country code or platform. Not settled: whether the synthesizer follows the pulls (a VTX on R3, B1, E8) |
| S6 | RF reaches the ADC: an iPhone hotspot on Wi-Fi 149 at bench distance clips the 3-lane decode at gain 62 on A7 and steps the gain down to 44 | `2026-10-25-bench1-stage1-hotspot.md` | — | — |
| S7 | A real 20 MHz 802.11 source reads as the model says: `w` on the loaded hotspot (A7) gives `cls` `dot11` with `r128` 0.15–0.25 and `scan_lag` 128 (512 once), `r2667` about 0.02, `r1` 0.69–0.71 and `bw_mhz` 20, `fc_mhz` within 0.5 MHz of 5745 | `2026-10-25-bench1-stage1-hotspot.md`, `…-stage3.md` | the 802.11 class rule and the 20 MHz bucket; nothing about a saturated link, OpenIPC or 40 MHz (stages 3 and 8) | `w` on a 20 MHz 802.11 source reads another class or bucket |
| S8 | A false `wideband` report from the 3-lane floor, found in the scan under the hotspot's load (an empty R3 at gain 44, `duty` 100), fixed in `168ab5f`: a window counts (in the candidate, and as on in the confirmation) only when it stands `WB_ON_SNR_DB` 3 dB over the no-signal power at its own gain; `test_wideband_floor` rebuilds the case. The audit the same day removed a mean-level test that came with the fix (it lost links near the threshold) and rebuilt the four reports under today's rules: R3 and A4 would not be reported, A7 very likely not, and B5's confirmation still passes (`duty` 75; whether its sweep windows still make it a candidate cannot be read from the report) | `2026-10-25-bench1-stage3.md` | stage 3's status; the bench confirmation is still to do (run order, session C) | a `wideband` line on an empty channel on firmware from `168ab5f` on |
| S9 | A quiet sweep (one window per sector, no confirmation pass) takes 2.07 s at the 8 ms settle, about 41 ms a channel where 20 ms was estimated; 3.2 s at a 30 ms settle, so the extra 20 ms a channel is in the tune or the metrics, not the settle. A busy sweep is not timed yet | stage 0 records of both days | the sweep times in the root README and internals §6 | the settle, window count or channel count changes |

**First looks, not settled.** These stay on the bench lists: no `wideband`
report with the phone hotspot idle (62 sweeps of unconfirmed timing, from a
phone rather than a beaconing access point), and a saturated 802.11 link at
`duty` ≥ 90, not demonstrated (the loaded hotspot read 62 and 75). `duty`
taken before `168ab5f` could count floor windows as on whenever the pass ran
under gain 62 (the scan reports at gain 41–47 did; a `w` line does not print
its gain), so the same source can read a lower `duty` on today's firmware:
not a regression.

---

## 1. The loop for every batch of data

1. **Record first.** Put what the user brings into `docs/bench/` exactly as it
   came (console lines in a fenced block, record-sheet values as a table, a
   photo described in words), one file per stage and station:
   `docs/bench/YYYY-MM-DD-<board or station id>-stage<N>.md`, with the build
   (commit, environment, extra `-D` flags), the chain (VTX, attenuator, LNA,
   cables, antenna) and the ambient conditions the sheet asks for. Do this
   before interpreting anything; a half-read value is still data.
2. **Decide with the stage's rules** in section 2. Where the rule says "ask",
   put the question and the data to the user and stop at that item; carry on
   with the rest.
3. **Edit.** Constants in `config.h` change only when the data comes from the
   reference build (3-lane decode, `GAIN_MAX` 62, no LNA unless the sheet says
   the LNA was in the chain, in which case follow the LNA rules of stage 5).
   Station-specific numbers (switch table of a different switch, cable loss,
   measured K of one box) go to that station's `stations.ini` as `custom_*`
   options or `-D` flags, and into `stations.example.ini` as a documented
   example, not into the defaults. Every changed number is also changed where
   it is quoted (table above). Replace a *(model)* marker with "measured X
   (model said Y)" rather than deleting the model figure.
4. **Verify.** `make -C level1-c5phy/test/host`, both `pio run` environments,
   and, when the guide's HTML changed, re-render the PDF and check the sheet
   fill (section 3). When a mapper branch changed: `python3 -m py_compile`,
   the parity check, the standalone `mapper_test/test_mesh_direct.py`.
5. **Docs and status.** Move the stage's line in the bench guide footer
   status, the internals §10 bench list and this plan's status fields from
   "model" / "unproven" to what was measured, with the date and the board.
6. **Push `level1`; open PRs for the mapper branches.** Commit message says
   which stage's data drove the change. Then append to the log in section 5.

---

## 2. Stage by stage

Each stage lists what the user delivers, the claims it tests, what in the
repo rests on it, the decision rules and the edits, then a status line.
"Reference build" = `seeed_xiao_esp32c5`, defaults, bare XIAO with the stock
antenna until stage 6.

### Stage 0 — flash and boot (sheet 6)

**Delivers:** the boot line (`"rf"`, `"phy_set_freq"`, `iq_lane_bits`,
`channels`), any `{"info":"error","stage":…,"call":…,"err":…}` line, two `?`
status lines 60 s apart (`captures`, `cap_err`, `bus_stuck`).

**Tests:** that this core's libphy exports the undocumented calls
(`phy_disable_agc`, `phy_rfagc_disable`, `phy_wifi_fbw_sel`,
`phy_force_rx_gain`, `phy_set_freq`), that the PHY can be held receive-only
and that PARLIO delivers I/Q windows. The whole station rests on this; it is
the first of the "what only the bench can prove" bullets.

**Rules.**
- `"rf":true`, `"phy_set_freq":true`, `captures` climbing, `cap_err` 0: record
  the platform and core version the build used (from `pio run -v` or
  `platformio.ini`'s pin) in the record and in the bench guide's cover status
  row; update the status strings (section 3) to "stage 0 passed on <board>,
  <date>".
- `"rf":false`: the error line names the call. Since 2026-10-25 the default
  build references the undocumented PHY/MAC symbols strongly
  (`C5PHY_STRONG_PHY_SYMBOLS` 1), so a core without one fails to build, and
  `"phy_set_freq"` in the ready line is a build fact, always true. A
  `(not linked in this weak-symbol build)` error only comes from a
  `-DC5PHY_STRONG_PHY_SYMBOLS=0` build and means the linker did not pull the
  symbol in, not that the library lacks it (a weak reference does not pull an
  archive member): check `nm` on that `firmware.elf`, then build strong. A
  core whose libraries really lack a symbol fails the default build: do not
  patch around it; report to the user with the core version, and look at
  whether a different pioarduino platform version exports it (the pin is in
  `platformio.ini`).
  Any other failing stage of `rf_start` is a bring-up bug: the sequence is in
  `src/c5phy_rf.cpp`, modelled on C5VRX.
- `"phy_set_freq":false` with `"rf":true` (only a `-DC5PHY_STRONG_PHY_SYMBOLS=0`
  build can print it since 2026-10-25): the radio can only park on public
  centres; `tune_fail` will climb on every off-centre channel. Report; the
  plan has no fallback plan for this (the exact-MHz step is the receiver).
- `cap_err` climbing: PARLIO (`cap_err_last` in `?` names the error).
  `bus_stuck`: a few percent of the scan's windows is bench1's background
  (§0 S4), not wiring. Wiring, or a bus that does not stream, only when a
  quiet hold shows `stuck` 1 on most lines or the scan's rate goes well over
  10 %; `IQ_LANE_GPIOS` / `IQ_LANE_DIAG` in `config.h` are the knobs, the rule
  that the six lane pads stay unconnected is in both READMEs.

**Status:** passed on bench1, 2026-10-24 (§0 S1, S2). Open: the stuck windows
of the scan (§0 S4). Ruled out: the wiring, a pause of the stream after the
retune (a 30 ms settle left 4.4 % against 4.9 % at 8 ms), a fixed offset of
particular channels (none while held on any of the 18 channels of C1). Left
as candidates: a retune that leaves the stream constant for a whole visit
(`visits_all` in `k`), the sector switch lines toggling (sector 0 is the
first window after the tune; on a quiet bench each sector takes one window,
so `by_sector` carries the information and `by_window` stays near 0 past
window 0), back-to-back captures, and the RF environment (the hotspot moved
the counts both ways, S4). Next bench, on firmware from `6550094` on: `k 0`,
two minutes of scanning, `k`. `k` counts the sweep's measured windows only,
so it reads under the `bus_stuck` difference whenever a hold, a `v` or `w`
pass, a gain retake or a confirmation pass ran in between.

### Stage 1 — receiver alive (sheet 6)

**Delivers:** on a quiet channel: `level_db`, `p_mean`, `q_phase`, `cv2`,
`gain`, `clip`, `noise`, `stuck`, `nf_dbm`; with a VTX keyed on A1: the rise
in `level_db`, `gain`, `q_phase`, `cv2`, `mod`, `cfo_khz`; dual-band: the same
on `h G3`.

**Tests:** the noise reference (`RF_NOISE_POWER` 2.0 is C5VRX's figure, not
this board's), the noise statistics the gates assume (`q_phase` under 15,
`cv2` about 1 on noise; the model gave 0.8), that FM video reads coherent
(`q_phase` ≥ 50) with a steady envelope (`cv2` ≤ 0.3, gate limit 0.5), that
the offset estimate is right (`cfo_khz` within ±2000 on-channel), and on the
dual-band build that the I/Q bus works at BW40 on 2.4 GHz.

**Rules.** Bench1 found the quiet channel at the decode floor (§0 S3): on 3
lanes the first two rules have nothing to measure until the chain lifts the
noise over one decode step (the LNA or 8 lanes, §4); they apply to that chain.
- `p_mean` on a quiet channel at gain 62, 3-lane build: set `RF_NOISE_POWER`
  to it when it differs from 2.0 by more than 10 %; `RF_NOISE_POWER_24` from
  G3 on the dual-band build. Update the bench guide's "p_mean ≈ 2" and the
  internals §2 row; the host test's 2.75 is the synthetic model's figure and
  stays. (Bench1, 3 lanes: 2.00 exactly, the floor; nothing to set.)
- `cv2` on noise: write the measured value into internals §2 and the guide.
  Under 0.6 it crowds the `noise` flag (0.5) and `WB_CV2_MIN` (0.5): ask the
  user before moving the analog / digital splitter (`ANALOG_CV2_MAX` and
  `WB_CV2_MIN` move together; the cost is on the digital candidate side).
  (Bench1, 3 lanes: 0.00, every sample the same size; not a noise reading.)
- `q_phase` on noise above 15: the margin to `Q_MIN` 40 shrinks and
  `MOD_Q_MIN_PCT` 20 (demod.c) may read noise as modulated; report with the
  number; a `Q_MIN` change is a design change (ask).
- Keyed VTX `cv2` above 0.3 with the camera on: real video sits closer to
  the splitter than the model; if above 0.4 ask before anything else, since
  `ANALOG_CV2_MAX` 0.5 would start losing analog hits in multipath.
- `cfo_khz` sign: if the VTX is known to sit above the channel and `cfo_khz`
  reads negative (or `freq_peak` in stage 3 lands on the wrong side), I and Q
  are swapped in the lane map; fix in `IQ_LANE_GPIOS` order or the nibble
  decode in `demod.c`, never by negating `cfo_khz` downstream. Ask first:
  it changes every captured value.
- `stuck` 1 on most lines of a quiet hold: lane wiring; see stage 0 (a few
  percent of the scan's windows is the background, §0 S4).
- Dual-band `h G3` shows no rise with a 2.4 GHz source: the dual-band
  environment is not usable; mark it so in both READMEs and the bench guide
  until solved, and keep stations on the single-band build.

**Status:** board-only half done on bench1, 2026-10-24/25 (§0 S3, S6): the
quiet channel reads the decode floor on 3 lanes and noise under one step on
8, so `RF_NOISE_POWER` stays 2.0 as the 3-lane floor; RF reaches the ADC. No
constant moved. Open: everything with the VTX (keyed `level_db`, `q_phase`,
`cv2`, `cfo_khz` and its sign, the walk-about `cv2`; dual-band `h G3`), and
the lane count with the LNA as one decision for the user (§4), measured with
the VTX and the attenuators.

### Stage 2 — channel coverage (sheet 6)

**Delivers:** `tune_fail` after a heartbeat with the deployment country,
which channels were refused and the error text, `h E4` / `h E8` holds with
their `wifi_ch`, the E8-versus-E5 mirror test, `h D1` / `h L4` (`h G1` /
`h G5`), `alias_drop`.

**Tests:** the regulatory table under `RF_COUNTRY_CC`, that the synthesizer
follows `phy_set_freq` 60 MHz past the last public centre (E8 5945 from
5885), that the D, L and G bootstrap centres hold, and whether the alias
guard ever fires.

**Rules.**
- Refusals fixed by the country code: set `RF_COUNTRY_CC` in `config.h` to
  the deployment country if the user says every station is in that country;
  otherwise it stays per station in `stations.ini`. Refusals of the D or L
  points that no country code fixes: the station drops them with
  `-DSCAN_5G1=0` / `-DLOWBAND=0`; note it in the README troubleshooting row
  that already covers `tune_fail`.
- The synthesizer does not follow (E8 deaf, a VTX on E5 heard on E5, E7, E8
  alike): ask the user whether `C5PHY_MAX_MHZ` 5885 becomes the default (all
  C5 modules share the PHY, so probably yes). If yes: change the default in
  `src/fpv_channels.c`, update the channel counts everywhere (50 → 46,
  55 → 51, "58 at most" → 54 in `config.h`, `main.cpp` and the docs), the
  `test_channels` expectations, the bench guide stage 2 and stage 8 rows
  (HDZero R8 5917 and Walksnail 5914 unreachable) and the root README
  hardware notes; keep `ALIAS_GUARD` on. The mapper rows for 5914 / 5917 stay
  (another receiver may reach them).
- The synthesizer follows 60 MHz: record it; the `LOWBAND` 2 pull of 140 MHz
  (L1–L3) is a separate test only if the user cares about 5362–5436 MHz.
- `alias_drop` climbing on a board where E8 does follow: the guard is
  dropping real carriers; look at the levels (a second emitter within 2 dB of
  one near 5885); `ALIAS_LEVEL_DB` can come down to 1.0 before the guard is
  turned off.

**Status:** C1 steps 1–2 done on bench1, 2026-10-25 (§0 S5): "US" refuses
Wi-Fi 169, and the code's own fallback parks A2, R6 and F6 on 165 and B7 on
173, reached with a 15–20 MHz pull instead of 0–5; everything else parks as
predicted, with no tune failure. No constant moved; internals §6 and the
`fpv_channels.c` comment corrected. Open: whether the synthesizer follows
the pulls (E8, R3 / B1 with a VTX: B1b), C1 step 3 (`alias_drop` with a VTX
on E5).

### Stage 3 — selectivity (sheet 6)

**Delivers:** channels reported with a VTX on R3 (R3 alone, or B1 / F1
folded), any hit more than 40 MHz away with its `q_phase` and `cfo_khz`,
`cfo_khz` → `freq_peak` on `h R3`, and the access-point control: `cv2` on
the AP's channel idle, `cls` and `duty` under a long file copy.

**Tests:** `PEAK_PICK` 20 MHz folding, that coherence rejects everything
off-channel (`Q_MIN`), the offset estimate against a known VTX frequency,
and the two Wi-Fi claims: idle traffic never becomes a report
(`WB_CV2_MAX` 1.5 or `WB_DUTY_MIN` 75 stop it), a saturated link is reported
as `dot11` with `duty` ≥ 90 (the mapper labels that "802.11 video link";
under 90 "Wi-Fi traffic").

**Rules.**
- A hit more than 40 MHz from the VTX: if `q_phase` ≥ 40 there, the VTX is
  overloading the front end (check `clip` and `gain` on `h`) or the lane
  decode is wrong; note the level at which it starts and treat it as the
  station's overload point in the README; no constant moves for it.
- `freq_peak` off by more than 1 MHz from the VTX's set frequency on a
  stable `cfo_khz`: see the I/Q swap rule in stage 1 if the sign is wrong;
  if the magnitude is wrong, `DEMOD_SAMPLE_RATE_HZ` (40 MS/s) is not what the
  bus delivers: report, do not tune.
- Idle AP produces a `wideband` report: find which gate let it through from
  the `w` output (`cv2` inside 0.5–1.5 and `duty` ≥ 75 on beacons would mean
  very busy air). First move is `WB_DUTY_MIN` 75 → 90 (the mapper already
  treats under 90 as traffic); `WB_CV2_MAX` stays unless `cv2` on idle Wi-Fi
  reads under 1.5 consistently, then ask. Since `168ab5f` a window only
  counts as on 3 dB over the floor at its own gain (§0 S8), so a report from
  idle air means real signal in six of eight windows: check the report's
  `gain` and `rssi_min` before the duty rule.
- Loaded AP reports `dot11` with `duty` under 90: the 90 rule in the mapper's
  802.11 rows is too high for real traffic; hold the change until the OpenIPC
  link in stage 8 gives the duty of a real video link, then set the row's
  `duty_min` between the two (both mapper branches, both REFERENCE tables,
  the root README troubleshooting row).
- B1 / F1 never hit at all with a VTX on R3: fine, note it; `PEAK_PICK` stays
  for the 1 MHz R3 / B1 case.

**Status:** started on bench1, 2026-10-25, with a phone hotspot on Wi-Fi 149
standing in for the access point (C2's Wi-Fi steps;
`docs/bench/2026-10-25-bench1-stage1-hotspot.md` and `…-stage3.md`): §0 S7
(class, bucket and centre on a real 20 MHz 802.11 source) and S8 (the floor
report, fixed). The scan under the hotspot's load gave four `wideband`
reports. R3 was false: a burst from the hotspot 13 MHz away clipped the
sector's first window, the gain stepped down to 44 and stayed there, burst
fragments in two of three windows passed the candidate gate (an empty 3-lane
window at gain g reads `GAIN_MAX − g` dB), and the confirmation pass at that
gain counted eight floor windows as on, `duty` 100. A7 (the hotspot) and A4
(an AP on Wi-Fi 161) were real traffic with `duty` 100 and `conf` high
inflated the same way, which the mapper would label "802.11 video link"; A7
was also bucketed at 30 MHz (`r1` 0.51). B5 (the 161 AP, `duty` 75) was honest
traffic. Fixed 2026-10-25 with the user's go-ahead (`168ab5f`; the decisions
now live in `src/wideband.c`, `14f6465`; the audit the same day took out a
mean-level test that came with it, §0 S8); `test_wideband_floor` rebuilds the
R3 case next to real links at reduced gain and a TDD link, which still report.
The analog path takes no false hit from the floor (`q_phase` gates it), but a
floor-inflated sector can still win the best-sector choice there (§4). Left as
they were: the bucket's (S+N)/S correction from `level_db`, and the bearing
windows of a wideband report (§4). Open (run order Part 6): C0, the rescan
under the hotspot's load on today's firmware (no `wideband` line for an empty
channel, no `duty` 100 on bursty Wi-Fi); C2 with a VTX; C2w with a router (an
idle access point that beacons, and a saturated LAN copy for the `duty` ≥ 90
rule).

### Stage 4 — video check (sheet 7)

**Delivers:** `v` output with an NTSC camera (windows with `video:1`,
`std`, `line_hz`, `present`), the same with PAL, the keyed-no-camera case
(`present:0`, `carrier`), and the detection line's `video`, `sync_hz`, `fp`.

**Tests:** the sync-pulse detector on real video (`W_MIN` 128 to `W_MAX` 288
samples, `PERIOD_TOL` 2.5 %, `MIN_SWING` 4, polarity search), PAL in
particular (unproven on this receiver), `VIDEO_MIN_WINDOWS` 3 of 8, and the
`carrier` split (`MOD_STD_DEG` 4° on the box-averaged discriminator).

**Rules.**
- NTSC fails: a detector problem, not a threshold; the per-window `swing`,
  `pulses`, `periods`, `period` from `v` go into the record and to the user
  with a proposed change to `video_window` in `demod.c`; add the failing
  shape to the host test's synthetic cases when the cause is known.
- PAL fails, NTSC passes: same data; look first at `PAL_PERIOD` 2560 against
  the measured `period` and at the swing on the PAL camera; a tolerance
  change is a one-line edit plus the test's PAL cases.
- Keyed, no camera, reads `carrier:"fm"`: `MOD_STD_DEG` 4 is under the VTX's
  idle deviation; raise it to half of the measured `step_std` on the bare
  carrier. Camera on reads `cw`: lower it to half the measured video value.
  Update the internals §2 `mod` row.
- `sync_hz` more than 15 Hz from 15 734 / 15 625 on a real camera: the
  camera's clock, not a bug; widen the `fp` tolerance note in the README if
  the user wants two stations' `fp` to agree on the mapper.

**Status:** not started.

### Stage 5 — sensitivity, calibration and the filter skirt (sheets 7 and 10)

**Delivers:** the level record (input dBm against `level_db`, `gain`,
`q_phase`, `cv2`, hit or not, at −90 … −30 dBm), the measured slope and
floor, the same floor on a v2 RX5808 station, the optional LNA sweep (floor
improvement, clipping onset, `RF_NOISE_POWER` re-measured), the quiet-band
60 s maximum, the dual-band repeat on a G channel, and the skirt record
(six channels 11–31 MHz off a VTX on R4 at `level_db` 35–40: `level_db`,
`q_phase`, `cv2`, pass or not; then two minutes of scanning with the VTX on).

**Tests:** `level_db` linearity (1 dB per dB), the dBm line
(`RSSI_CAL_DBM_AT_NOISE` −95, `RSSI_CAL_SLOPE` 1.0), the floor and so the
v2 / v3 verdict, the gain loop near threshold (`GAIN_STEP` 3,
`CLIP_MAX_PCT` 3: no notch where a carrier just above threshold is lost when
the gain steps), the detection margin (`DETECT_LEVEL_DB` 8 over the quiet
maximum by ≥ 6 dB), the LNA's worth (`RF_FRONTEND_GAIN_DB`), the `_24`
calibration, and the skirt: the real reach of a strong FM carrier's image,
which set `WB_ANALOG_OWN_MHZ` 30 on a model.

**Rules.**
- Slope within 1.0 ± 0.05: keep `RSSI_CAL_SLOPE` 1.0. Outside: set it to the
  fitted slope and say so in the internals §3 table.
- `RSSI_CAL_DBM_AT_NOISE`: the input level at which the fitted line gives
  `level_db` 0 (two points well inside the linear range, e.g. −70 and −50
  dBm). Set it from the first calibrated reference board; a second board
  within 1.5 dB confirms it as the default, otherwise the spread goes into
  the docs as the per-station uncertainty. Then update every "−87 dBm
  threshold" and "−95 reference" quote (table in section 0) to the measured
  figures and mark them measured.
- Floor against the RX5808 station: write the difference into the root README
  hardware section and the bench guide stage 5 line; this is the user's
  decision input, not a constant. If v3 is more than 6 dB deafer, raise the
  LNA question with the LNA sweep's numbers.
- LNA sweep: calibration taken with the LNA in the chain keeps
  `RF_FRONTEND_GAIN_DB` 0 and gets its own `RSSI_CAL_DBM_AT_NOISE` in that
  station's `stations.ini`; a station that adds the LNA to a calibration taken
  without it sets `RF_FRONTEND_GAIN_DB` to the measured gain (the comment in
  `config.h` says 20; replace with the measurement). `RF_NOISE_POWER` with
  the LNA goes to that station too, not to the default. If the LNA improves
  the floor by ≥ 3 dB on the single-band build, the README's "fit it if
  stage 5 shows…" becomes "fit it: +X dB", and the bench guide's optional
  marks on the LNA items go.
- Clipping onset: record the dBm at which `clip` stays above 3 % at
  `GAIN_MIN` 2; that is the station's overload point for the README. Nothing
  to tune: the gain range is the PHY's.
- Notch: a "hit while scanning" column that reads no between two yes
  columns means the gain step still loses a carrier near threshold; try
  `CLIP_MAX_PCT` 10 before `GAIN_STEP` 2 (the internals §3 explains the
  trade), and run the host test's gain-loop cases after.
- Quiet-band maximum: `DETECT_LEVEL_DB` = that maximum + 6 dB, rounded up to
  the half dB, when it comes out above 8; below 8 the default stays (do not
  lower it: the digital candidate rule shares it). A change moves
  `threshold_level_db` in every sample line and the gate quotes (section 0).
- `_24` set: the same four rules on the G-channel sweep →
  `RSSI_CAL_DBM_AT_NOISE_24`, `RSSI_CAL_SLOPE_24`, `RF_NOISE_POWER_24`,
  `RF_FRONTEND_GAIN_DB_24`.
- Per-station offset: `RSSI_CAL_OFFSET_DB` has no `custom_*` option yet; the
  first station that needs one (stage 6 insertion loss, cable loss) gets a
  `custom_rssi_offset_db` option added to `tools/station.py`
  (`ANTENNA_OPTIONS`-style entry, define `RSSI_CAL_OFFSET_DB`, range −10 … 30),
  `tools/station_setup.html`, `stations.example.ini` and the Station setup
  table in `level1-c5phy/README.md`. Convention: the passive loss between the
  antenna's SMA and the U.FL, positive, so `rssi_dbm` refers to the antenna
  connector.
- **Skirt record → `WB_ANALOG_OWN_MHZ`.** A held channel "passes" when
  `level_db` ≥ 8 and `cv2` is within 0.5–1.5 at the same time. Radius needed
  = the largest passing offset + 5 MHz. Over 30: raise `WB_ANALOG_OWN_MHZ`
  to it (round up to 5). Nothing passes beyond 20: set 25 (keep the margin
  over `PEAK_PICK_MHZ`). Between: keep 30. Whatever the outcome, replace the
  model wording in `config.h`, internals §4, the root README troubleshooting
  row and bench guide sheet 10 with the measured offsets, update `sw_rules()`
  and the B4 / A7 case comment in `test_sweep_decide` if the number moved,
  and re-run `test/model/sim_skirt.c` with a filter edge fitted to the
  measured levels (the `design()` calls set cutoff and tap count) so the
  model's numbers in the docs match a filter shape that reproduces the
  record. The two-minute scan must show no `wideband` line: if a `DF:` line
  appears more than the radius away from R4, record its channel, `cls`,
  `cv2` and `fc_mhz`; a second mechanism is at work and needs the user.
- If the skirt passes at offsets where `q_phase` is above 40 (it should not):
  that is an analog hit on a neighbour, i.e. `PEAK_PICK` territory, and
  `PEAK_PICK_MHZ` would need to grow with it; ask.

**Status:** not started.

### Stage 6 — RF switch and pattern (sheet 8)

**Delivers:** the truth table (`t 000` … `t 110` → RF port), `s 0` … `s 3`
following the patch, insertion loss (and with the LNA), the 15° rotation
sweep at 30 m (all four `sectors` per step), the derived K against the boot
line's `bearing_k`, the measured K, and the residuals of `bearing_deg`
against truth.

**Tests:** `SECTOR_SWITCH_TABLE`, the antenna model (`sqrt(32400 / G)`
beamwidth, K = `BEARING_K_SCALE` 2.5 × beamwidth² / 4320, σ base 10° at 72°),
the ±45° clamp, and the loss budget between the patch and the U.FL.

**Rules.**
- Truth table differs from the default table for the PE42442: if the
  reference design uses that board, change `SECTOR_SWITCH_TABLE` in
  `config.h` and the root README "Line states" table; a different switch
  gets its table in `stations.ini` and a line in the README's "Any other
  switch" section.
- Insertion loss: becomes that station's `RSSI_CAL_OFFSET_DB` (see stage 5;
  add the option first); with the LNA in the path the net gain goes to
  `RF_FRONTEND_GAIN_DB` rules of stage 5.
- Measured K from the neighbour-sector difference slope: the station's
  `custom_bearing_k`; `BEARING_K_SCALE` default = 2.5 × measured / derived
  for the antenna family, once two boxes with the same patches agree within
  15 %. Update the §8 table values, the boot-line samples' `bearing_k`
  (2.97) and the station setup docs. If the patch datasheet beamwidth
  differs from the `sqrt(32400 / G)` figure by more than 15°, prefer the
  datasheet in `custom_antenna_beamwidth_deg` and say so in the README's
  antenna section.
- Bearing residuals: `BEARING_SIGMA_BASE_DEG` = the RMS error at good S/N
  over the sweep (currently 10° at 72°); if the error grows past ±30° off
  the sector axis the `BEARING_MAX_OFFSET_DEG` 45 clamp is hiding it and the
  model's K is wrong off-axis, which is a finding for the user rather than a
  constant. The mapper's `_clamp_sigma` floor (2°) and ceiling (60°) only
  move if the station's σ ever leaves that range.
- Save the sweep table to `docs/bench/` as data, not only the derived K.

**Status:** not started.

### Stage 7 — field acceptance (sheet 8)

**Delivers:** two stations' fix results on ten points (hits inside the 90 %
circle), the mesh copy's completeness (`fp` arriving), 72 h on solar
(brownouts, heartbeat gaps), station current at 5 V.

**Tests:** the whole chain (mesh pacing `MESH_REPORT_INTERVAL_MS` 8 s,
`HEARTBEAT_MESH_S` 120, the 191-byte line, the home node forwarding), the
mapper's fusion (`BEARING_FUSE_WINDOW_S` 30, `BEARING_OBS_KEEP_S` 300,
`BEARING_MIN_CROSSING_DEG` 8, `BEARING_OUTLIER_DEG` 20, `BEARING_MAX_RANGE_M`
20 km) against the station's σ model, and the power budget.

**Rules.**
- Fewer than 9 of 10 inside the circle with bearings individually sane: the
  σ model is optimistic; raise `BEARING_SIGMA_BASE_DEG` (and `WB_SIGMA_EXTRA_DEG`
  for digital) until the circle holds; too many inside with large circles
  the other way. More than one station's line regularly excluded as an
  outlier: look at `BEARING_OUTLIER_DEG` (mapper branches, PR) only after
  headings and positions are confirmed.
- `mesh_drop` or `usb_drop` climbing in heartbeats: the Heltec queue; raise
  `MESH_REPORT_INTERVAL_MS` or `MESH_LINE_GAP_MS` 350 and document the
  airtime cost.
- Measured current: replace the estimates in the root README power table and
  the bench guide power table; re-check the solar sizing text (20 W / 50 Wh)
  against the measured daily energy.

**Status:** not started.

### Stage 8 — digital video links (sheet 9), one block per kit

**Delivers, per kit:** held channel; `level_db`, `q_phase`, `cv2` on `h`;
three `w` results (`duty`, `cls`, `conf`, `bw_mhz`, `fc_mhz`, `cv2`, `r1`,
`r4`, `r128`, `r512`, `r2667`, `scan_lag`, `scan_r`); one `wideband` line per
sweep under `x` and the mapper's `system` / `system_conf`; the rotation
(bearing error at 0 / 15 / 30 / 45°, σ, the digital K); the kit's own
setting (channel, bandwidth, power). The dual-band block repeats it at 2.4
GHz.

**Tests:** everything the wideband path assumes, all of it from a synthetic
model so far: the candidate gate on real links (`cv2` 0.5–1.5, `q_phase`
under 40, level steady within 6 dB over three windows), `WB_DUTY_MIN` 75
against real TDD duty cycles, the class rule (`WB_R128_MIN` 0.10 for 802.11,
`WB_R2667_MIN` 0.03 with `WB_R128_LTE_MAX` 0.05 for the LTE-like class, the
`_HIGH` marks 0.15 / 0.04), the lag 2667 itself (a 66.7 µs symbol is an
assumption about DJI), the bucket edges on `r1`, the centre estimate against
the mapper's 2 MHz match, the `WIDEBAND_SYSTEMS` rows (centres, buckets,
class, `quality`), the 802.11 rows' `duty` 90 split, the mesh line keeping
`cls` / `fc_mhz` / `bw_mhz`, the digital bearing (σ + 5° + 0.2° per missing
duty percent, one K per band), and HDZero's unknown modulation.

**Rules, per kit.**
- *Gate.* A kit that reads `cv2` under 0.5 on `h` with `q_phase` ≥ 40 is
  passing the analog gate and will be reported as an `AF:` carrier marked
  `cw` (the single-carrier case the internals §4 names). That is a design
  item, not a constant: bring the user the `w` features (`r1`, `r4`,
  `step_std` from the bench line) and propose a discriminator (`r1` below
  the FM range, `mod` 0 with a wide `cfo_khz` spread, or a symbol-rate line
  in the lag scan); no edit without that discussion. A kit with `q_phase`
  40–45 and `cv2` about 1 (DJI is expected near 40) is safe: `cv2` fails it
  out of the analog gate; note the margin.
- *Duty.* `duty` under 75 on a real link on most `w` presses: lower
  `WB_DUTY_MIN` to the observed value minus one step of 12.5 (never below
  50), and widen the bench guide and README expectations; the σ term stays.
  First check which windows are off: if they sit 1–3 dB over the floor at the
  pass's gain (a link that stepped the gain down sits only 5.5–7.5 dB over it
  on 3 lanes, so a fade of 2.5–4.5 dB between sweep and pass turns windows
  off), `WB_ON_SNR_DB` can come down to 2 before the duty moves (bench1's
  floor windows and burst fragments read under 1 dB). A TDD kit that is
  rarely a candidate at all: its three sweep windows must all fall in its on
  time, which the duty of the kit sets; note the rate per sweep.
- *Class, 802.11 (OpenIPC).* `r128` ≥ 0.10 and `scan_lag` 128: the model
  holds; mark the row verified. `r128` between 0.05 and 0.10: lower
  `WB_R128_MIN` to 0.6 × the observed value and `WB_R128_HIGH` to the
  observed value (the host test reads them from `config.h`), re-check
  that FM video's `r128` (0.10–0.12 in the model, measured in stage 1's `w`
  if taken) stays clear of the new minimum; if it does not, the `lte`-first
  rule is no longer a safe guard and the user decides. `scan_lag` not 128:
  the link is not on a 3.2 µs symbol (a 10 MHz half-clocked wfb-ng mode would
  show 256); add that lag to `iq_lag_features` and the class rule only with
  the user, since it changes the report fields.
- *Class, DJI O4.* `r2667` ≥ 0.03 with `r128` under 0.05 and `scan_lag`
  2665–2669: verified; raise the O4 row's `quality` from `med` to `high` in
  `WIDEBAND_SYSTEMS` on both mapper branches (O3 stays `med` unless an O3
  unit is tested), and change "expected, unverified" in both REFERENCE
  tables and the internals. `scan_lag` elsewhere: DJI's symbol is not
  66.7 µs; the peak lag is the new feature (rename `r2667` and
  `WB_R2667_*` to the measured lag, update `iq_lag_features`, the class rule,
  `report.c` field names only if the user agrees to break the mapper's
  `r2667` key, the fixtures and both mappers' normalisation list); this is
  a session's work and goes to the user as a proposal with the data first.
  `r2667` present but under 0.03: lower `WB_R2667_MIN` to 0.6 × observed, the
  `_HIGH` mark to observed; check the noise floor of
  the feature from a quiet `w` (0.01 in the model) keeps a 2:1 margin.
- *Bucket.* The reported `bw_mhz` against the kit's set bandwidth: a 20 MHz
  link reading 10 or 30 means `r1` sits outside 0.52–0.78 for that spectrum
  shape. Move the edge that is wrong to the midpoint between the measured
  `r1` of the two neighbouring widths (`wb_bucket_mhz` in `wideband.c`, which
  the test calls), and record each kit's `r1` per mode in the internals §4 bucket
  table; the mapper tolerates one bucket step, so the rows need no change
  for a single mis-bucket.
- *Centre.* `fc_mhz` against the kit's centre: within 2 MHz, fine. 2–4 MHz
  off consistently: raise the mapper's `WIDEBAND_MATCH_MHZ` to 4 only after
  checking that 5768.5 (O4), 5769 (HDZero) and 5770 (Walksnail) still
  resolve by class and bucket (the ranking rules in `_wb_match`), and note
  which way the bias runs (a 40 MHz link is cut by the filter and biases
  toward the tuned channel). Larger: the wrap past Nyquist or a lane problem;
  user.
- *HDZero and Walksnail.* Whatever `cls`, bucket and `scan_lag` they read is
  the result: set the row's `cls` to it (`'cls': 'wb'` is allowed, the
  matcher then requires that class), keep `quality` `high` only for a row
  whose class and bucket were both measured, and put the measured features
  into the bench guide's "expected" rows for the next bench. If HDZero reads
  as an analog `cw` carrier, see the gate rule above.
- *Mapper label.* `system` / `system_conf` as the mapper printed them go into
  the record. A wrong label with correct firmware fields is a table problem:
  centres (the kit's real channel plan), buckets, class or ranking; fix in
  both mapper branches, run the parity check, update both REFERENCE tables
  and the standalone `test_mesh_direct.py` expectation, and the
  `level1_bearing_sim.py` fixture features on both branches to measured DJI
  values when O4 is done.
- *Wi-Fi split.* Combine with stage 3: the loaded AP's `duty` and the OpenIPC
  link's `duty` set `duty_min` of the "802.11 video link" row and `duty_max`
  of "Wi-Fi traffic" to the midpoint; if they overlap, the split needs a
  second feature (`r128` at `conf` high, or the 5 MHz grid alone) and the
  user decides.
- *Bearing.* First the floor windows (§4): below gain 62 a sector with
  nothing on it reads `GAIN_MAX − gain` dB in the bearing windows, so fit the
  digital K only on reports whose neighbours stand over that floor, or after
  that decision. Digital K within 20 % of the analog K: fine, note. Further off:
  the one-K-per-band assumption is wrong for noise-like levels; bring the two
  numbers to the user (a per-type K is a small change in `report_wideband`
  plus a `BandCal` field). σ from the residuals: `WB_SIGMA_EXTRA_DEG` so that
  the analog base plus it covers the digital RMS error; the 0.2°-per-percent
  duty term is checked against the low-duty kits (DJI at range).
- *Mesh.* The `wideband` mesh line with the real node id must still carry
  `cls` and `fc_mhz`; if a long node id cuts them, the order in
  `report_wideband_json` is the knob and `test_wideband_report` the proof.
- *2.4 GHz block.* The same rules on the `_24` constants; a 2.4 GHz kit that
  cannot be heard at all while G3 passed stage 1 is a bandwidth or filter
  question for the user.

**Status:** not started for every kit (OpenIPC, HDZero, DJI O4, Walksnail;
2.4 GHz block).

### After all stages

- The status strings (section 3) say what was measured, on which board,
  when; the *(model)* markers that remain are listed in the internals §10 as
  still open.
- The bench guide's "expected" rows for stage 8 carry measured values, so
  the next station's bench is a comparison, not an experiment.
- A short "Measured on hardware" section at the top of the internals doc
  replaces the "Nothing here has run on hardware" note.

---

## 3. Mechanics

**Status strings.** The bench guide footer text is repeated on all ten
sheets, and the cover's status row says the same in its own words: edit the
footer with one replace over the file (since 2026-10-25 it reads "stage 0
passed on bench1 (2026-10-24); stages 1–3 started, no VTX yet"); keep it no
longer, the footer is 9 px on one line. The cover callout, the root README's
"Proven on hardware" row, `level1-c5phy/README.md`'s "On hardware so far" and
the internals intro note were moved off "nothing has run on hardware" at
stage 0 (2026-10-25); they move again with the first transmitter.

**Rendering the bench guide.** After editing the HTML:

```
node docs/tools/bench-guide-measure.js docs/Level1-Station-v3-C5PHY-Bench-Guide.html
```

prints the free millimetres on every sheet under print media (needs
Playwright's Chromium; the sheets are fixed A4 boxes with `overflow: hidden`
and the footer pinned, so a negative number means text runs into the
footer). Then

```
chrome --headless --no-sandbox --disable-gpu --print-to-pdf=docs/Level1-Station-v3-C5PHY-Bench-Guide.pdf \
       --no-pdf-header-footer file:///<abs path>/docs/Level1-Station-v3-C5PHY-Bench-Guide.html
```

(in the cloud container `chrome` is
`/opt/pw-browsers/chromium-1194/chrome-linux/chrome`, add
`--user-data-dir=<scratch dir>`), and look at the changed pages with
`pdftoppm -f N -l N -r 80 -png`. Sheets 2, 5 and 7 are within a line of
their footers (0.7, 1.9 and 0.5 mm free on 2026-10-25; sheets 8 and 9 had run
1 and 0.1 mm into theirs and were trimmed by a line each that day): anything
added there needs something removed. Commit the HTML and the PDF together.

**Mapper changes.** Worktrees from `origin/level2-main` and
`origin/standalone-mapper-meshtastic`, a feature branch each, the same edit
in both `mesh-mapper.py` files, `python3 docs/tools/mapper-parity.py <l2>
<standalone>` must print ALL SAME, `python3 -m py_compile` both, run the
standalone `mapper_test/test_mesh_direct.py`, update both `docs/REFERENCE.md`
(the standalone README's one-line mention if the behaviour changed), push,
open two pull requests with the usual footer; the user merges.

**Commit hygiene.** One commit per data batch where possible, message
naming the stage and the board; this plan's status and log edited in the
same commit; `level1` pushed directly; CI green before the next batch.

---

## 4. Decisions that go to the user, with the data

Do not make these alone, whatever the numbers say:

- the analog / digital splitter (`ANALOG_CV2_MAX` = `WB_CV2_MIN` 0.5) and
  `Q_MIN` 40 (stages 1, 3, 8);
- the radio ceiling default `C5PHY_MAX_MHZ` 5885 if the synthesizer does not
  pull (stage 2);
- the v2 / v3 verdict and whether the LNA becomes standard (stage 5);
- the lane count (6 or 8 lanes) together with the LNA (stages 1 and 5):
  bench1 is quantization-limited at gain 62 on both decodes (§0 S3), so
  which of the two, or both, lifts the noise over a decode step is one
  decision, taken on the VTX and attenuator data;
- stuck windows as readings (§0 S4): `measure_sector` keeps a stuck window as
  a sector level and the `nf_dbm` minimum takes it (an 8-lane heartbeat's
  −111 is the clamp). Proposed 2026-10-25, not made: leave stuck windows out
  of the `nf_dbm` minimum, and retake or skip them in `measure_sector`, once
  `k` has shown where they come from;
- floor windows read as levels at reduced gain, wherever no `snr_from` test
  guards them (found by the audit of 2026-10-25; one design, two places).
  The analog path: `measure_sector` keeps the minimum level over a sector's
  windows, so a sector whose first window clipped on a Wi-Fi burst reads the
  floor of its later windows at the stepped gain (18 dB at 44) and can win
  the best-sector choice over a sector with a real carrier at 12 dB; the hit
  test then runs on the floor sector (`q_phase` 0) and the carrier is missed
  for that sweep, and `report_hit` takes every sector's level into the
  bearing. The wideband bearing windows: `report_wideband` takes one window
  per sector with `level_from` at the pass's gain, so a neighbour further under
  the strongest sector than the link's S/N reads the floor and the offset
  collapses to the axis; on 3 lanes nearly every report runs under gain 62
  (a steady link clips from about 8 dB, model), so this is the common case.
  Before stage 8's digital K and stage 6's analog K;
- an I/Q lane swap (stages 1, 3);
- a new or renamed lag feature, a single-carrier class, a per-type bearing
  constant, the Wi-Fi split's second feature (stage 8);
- any constant whose measured value lies inside the margin the docs give
  for a *different* gate (the two gates share `DETECT_LEVEL_DB`; the
  envelope limits meet at 0.5).

Everything else in section 2 is a rule: apply it, verify, push, log.

---

## 5. Log

Append one line per batch: date, board or station, stage, what came in,
what changed (commit), what is still open.

| Date | Board / station | Stage | Data | Edits (commit) | Open |
|---|---|---|---|---|---|
| 2026-10-08 | — | plan written | no hardware yet | `docs/Level1-Bench-Data-Plan.md`, `CLAUDE.md`, `docs/bench/`, `docs/tools/`, `test/model/` | all stages |
| 2026-10-08 | user's Windows bench PC; cloud container | session A (desk) | `pio run` on the PC stopped at the penv dependency step: a VPN's Winsock LSP crashes `uv.exe` (exit 3221225622); after the pin, the PC's PlatformIO Core 6.1.19 refused the platform (`IncompatiblePlatform`, needs 6.2.0), and `pio upgrade` stayed at 6.1.19 because that Core's Python is older than 3.9: Core goes into a venv of a current Python (README, Install the tools). In the container, on the pinned platform: host tests pass, both environments and the strong-symbol build link (RAM 44.8 %, flash 28.8 %), the six PHY calls are global in `esp32c5/ld/libphy.a`, `lmac_stop_hw_txq` and `phy_track_pll_deinit` in `lib/`; libs 5.5.5+sha.b774170ff46, Arduino 3.3.12; sdkconfig: `CONFIG_BT_ENABLED=y`, `CONFIG_ESP_COEX_SW_COEXIST_ENABLE=y`, `CONFIG_ESP_PHY_CALIBRATION_MODE=0`, no `CONFIG_PM_ENABLE` | platform pinned to pioarduino 55.03.312-1; README troubleshooting rows; run-order session A step 3 path; this note | the user's own build after the `uv.exe` mitigation, then stage 0 |
| 2026-10-08 | user's Windows bench PC (PlatformIO Core 6.2.0 on Python 3.14, pinned platform 55.03.312) | session A (desk) closed | Host tests pass (WSL). `seeed_xiao_esp32c5` SUCCESS, RAM 44.8 % (146692 B), flash 28.8 % (962751 B); `seeed_xiao_esp32c5_dualband` SUCCESS, RAM 44.8 % (146708 B), flash 28.8 % (963047 B); strong-symbol build links. `esp32c5/ld/libphy.a`: `phy_disable_agc`, `phy_rfagc_disable`, `phy_wifi_fbw_sel`, `phy_force_rx_gain`, `phy_set_freq`, `phy_chip_set_chan_offset` all `T`; `lib/libpp.a` `lmac_stop_hw_txq` `T`; `lib/libesp_phy.a` `phy_track_pll_deinit` `T`. sdkconfig: `CONFIG_BT_ENABLED=y`, `CONFIG_ESP_COEX_SW_COEXIST_ENABLE=y`, `CONFIG_ESP_PHY_CALIBRATION_MODE=0`, no `CONFIG_PM_ENABLE`, no `PHY_DISABLE_PLL`. versions.txt: lib-builder master 6671d0b, esp-idf v5.5.5 b774170ff46, arduino master e7c1e8cd4. Head 615af11. Identical to the container run | this row; run order marks session A done and D1–D4 confirmed | session B / stage 0 on the bare board |
| 2026-10-08 | — | bench kit review, before session B | The shopping list another session drafted, reviewed against the wideband path and the stage 8 rules; the user ruled out goggles unless unavoidable and large purchases. Corrections: HDZero Race V3 / Freestyle V2 take MIPI cameras only (an analog camera needs the ≈ €45 NTSC-only adapter) and have no goggles-side control; the DJI O4 Air Unit is inert without goggles, so the `lte` class is checked on a borrowed OcuSync 3/4 drone first; wfb-ng video is a one-way broadcast, so the OpenIPC kit can be emitter-only, and the level 1 firmware never decodes frames, so STBC/LDPC support is not a level 1 question; Walksnail needs a VRX, deferred. Analog kit for session B: a full 40- or 48-channel VTX with pit mode (SpeedyBee TX800; fallbacks Eachine TX805, TBS Unify Pro32 Nano), a Caddx Ratel 2, fixed SMA attenuators 10 / 20 / 30 dB, one u.FL–SMA pigtail | `docs/Level1-Bench-Shopping-List.md`; links from the run order, this file and the README | session B / stage 0 on the bare board |
| 2026-10-24 | bench1 (bare XIAO, stock antenna) | stage 0 pre-check | `nm` on the default (weak-symbol) build's `firmware.elf` on the bench PC: `phy_disable_agc`, `phy_rfagc_disable`, `phy_wifi_fbw_sel`, `phy_force_rx_gain`, `phy_set_freq`, `phy_chip_set_chan_offset`, `phy_track_pll_deinit`, `lmac_stop_hw_txq` all `T`. A weak reference does not pull an archive member, so session A's archive `nm` and strong-symbol link did not prove this; the two calls the ready line does not report are linked | `docs/bench/2026-10-24-bench1-stage0.md`; stage 0 status | confirm the ELF was today's B0 build, not session A's strong-symbol build; the B0 lines |
| 2026-10-24/25 | bench1 (XIAO ESP32-C5 `FFCB`, bare, stock antenna; no VTX) | session B board-only half (stages 0, 1, B1d no-signal), C1 (stage 2), C2 Wi-Fi steps with a phone hotspot (stage 3), stuck-window tests | Eight PHY/MAC calls resolved in the flashed default ELF; ready line `rf` and `phy_set_freq` true, 50 channels; captures about 97 a second, `cap_err` 0, `tune_fail` 0. Empty channel on 3 lanes: every sample at the decode floor (`p_mean` 2.00, `cv2` 0.00); on 8 lanes noise mostly under one 4-bit step (`p_mean` 0.25–0.60, `cv2` 1.0–3.5): quantization-limited at gain 62. `bus_stuck` 7.3 % scanning on 3 lanes, 4.9 % on 8, 4.4 % with a 30 ms settle, none held. "US" refuses Wi-Fi 169 (A2 on 165), C1 list otherwise as predicted. Phone hotspot on 149: RF reaches the ADC (clips at gain 62); `w` reads `dot11`, `r128` 0.15–0.25, `bw_mhz` 20, `fc_mhz` within 0.5 MHz. Scan under hotspot load: one false `wideband` report (R3, all floor windows at gain 44) and two with `duty` 100 inflated by the same 3-lane floor + gain-step effect. ESP-IDF boot error on the PSRAM memory-barrier workaround | records `docs/bench/2026-10-24-bench1-stage0.md`, `…-stage1.md`, `2026-10-25-bench1-stage0.md`, `…-stage1-hotspot.md`, `…-stage2.md`, `…-stage3.md`; status lines of stages 0–3; internals §6 parking (A2, R6 on 165) | VTX steps of session B (B1 step 4–6, B1b, B1c, B1d with signal); the stuck-window cause (per-channel diagnostic proposed); for the user: the wideband floor fix, strong PHY symbols by default, 6 vs 8 lanes with the LNA; the `fpv_channels.c` comment on A2; the chip revision from the upload log |
| 2026-10-25 | cloud container (pinned platform) | firmware from session B's findings, with the user's go-ahead | — | `14f6465` the wideband decisions to `src/wideband.c`, called by the host test (output byte-identical); `168ab5f` no wideband report from windows at the decode floor (`WB_ON_SNR_DB`, `wb_confirmed`, `test_wideband_floor`); `87fddcd` strong PHY symbols by default; `6550094` `k` counts the sweep's stuck windows, A2 parking comments. Host tests and both builds green on each | the floor fix and `k` on the bench |
| 2026-10-25 | cloud container | audit and housekeeping after the session | — | `fba85ff`, `cb9e801`, `fadd651`: §0 register "Settled on the bench" (S1–S9) and first looks; the contradicted rules of stages 0, 1 and 3 annotated; three decisions added to §4 (lanes with the LNA, stuck windows as readings, wideband bearing windows); the run order trimmed to what is left; READMEs, internals, `config.h` comments and the bench guide corrected to the bench data (guide: footers and cover at "stage 0 passed on bench1", `k` in sheet 4's console list, stage 0 and 1 expectations from bench1, sheets 8 and 9 trimmed back inside their footers); `r128` in the row above corrected from 0.21–0.31 to 0.15–0.25 (the `w` lines) | session B's VTX steps, session C's VTX and router steps, the hotspot rescan; the §4 decisions; the chip revision |
| 2026-10-25 | cloud container | the audit's findings (`b46702a`) | an independent audit of `14f6465`…`6550094` (builds and host tests at each commit, `-Wall -Wextra`, ASan/UBSan, the 8-lane and weak-symbol builds, 3 000 000 cases of the old against the new wideband logic, the `k` line fuzzed) | `wb_confirmed` back to the duty alone: the mean-level test of `168ab5f` dropped links near the threshold and was not monotonic (a regression test fails on it at gains 62 and 59); `test_wideband_floor` now also checks the candidate gate on links at reduced gain and on a TDD sector; docs: the 3 dB test is over `RF_NOISE_POWER`, not "the decode floor" (8 lanes differ), the stepped-link margin 5.5–7.5 dB, `conf` low in `w`, the `k` window index, `phy_set_freq` false only in the weak build; §4 widened to the analog best-sector choice and bearings | the floor-window design (§4); fade tolerance and TDD candidates at stage 8 |
