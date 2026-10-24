# 2026-10-25 — bench1 — stage 1, phone hotspot as the signal (not in the guide)

A check added on the bench while no VTX is on hand: does RF from the antenna
reach the ADC at all? The source is the user's iPhone 13 mini as a 5 GHz
Personal Hotspot ("Maximize Compatibility" off), the station held on A7
(5745 MHz, Wi-Fi 149, no pull).

## Build

`level1` firmware unchanged since `615af11`, environment
`seeed_xiao_esp32c5`, back on the 3-lane decode after B1d (`; -DIQ_LANE_BITS=4`
restored and re-uploaded; the ready line of this boot was not pasted, the
`p_mean` 2.00 floor on the empty windows is the 3-lane decode's).

## Chain

Bare XIAO ESP32-C5, stock antenna on the U.FL, USB-C only. iPhone 13 mini
hotspot nearby, channel 149, band 5 GHz (`netsh wlan show interfaces` on the
bench PC). Load: a Twitch live stream on the bench PC through the hotspot,
with the Astrill VPN running. Distance not given.

## Record

| Item | Reading |
|---|---|
| `h A7` → `wifi_ch` | 149 |
| Bench lines | 136 (`captures` 4587 … 4730) and one heartbeat |
| Quiet lines: `gain` 62, `level_db` 0.0, `p_mean` 2.00, `cv2` 0.00, `cfo_khz` −4 … +3, `step_std` 0.0 … 1.8 | 114 |
| Windows that clipped at gain 62 (the gain stepped; `captures` skipped) | 8 windows in 7 events, lines 4623, 4632, 4664 (two steps, to 56), 4671, 4680, 4702, 4724 |
| Phase activity at gain 62: `p_mean` 2.00 … 2.39, `cfo_khz` −99 … +2906, `step_std` 2.2 … 17.3 | 13 lines: 4600, 4635, 4638, 4646, 4649, 4650, 4660, 4681, 4695, 4699, 4710, 4725, 4726 |
| `stuck` 1 | 2 lines: 4627, 4717 |
| Heartbeat during the hold | `bus_stuck` 258 at 21 sweeps, `uptime_s` 71 |

## Console lines

The hold line, the first three floor lines, then every line that is not at
the floor, verbatim; the 111 other floor lines are trimmed here (values in the
table above) and are in `logs/device-monitor-261025-001830.log` on the bench
PC.

```
Disconnected (ClearCommError failed (PermissionError(13, 'The device does not recognize the command.', None, 22)))
Reconnecting to COM23 --- Logging an output to C:\Users\seant\drone-sentinel\level1-c5phy\logs\device-monitor-261025-001830.log
         Connected!
 h A7
{"info":"hold","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":1.3,"nf_dbm":-95,"captures":4587,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.7,"nf_dbm":-95,"captures":4588,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":-2,"mod":0,"noise":0,"stuck":0,"step_std":1.4,"nf_dbm":-95,"captures":4589,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":812,"mod":0,"noise":0,"stuck":0,"step_std":11.0,"nf_dbm":-95,"captures":4600,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.4,"nf_dbm":-95,"captures":4623,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":1,"step_std":0.0,"nf_dbm":-95,"captures":4627,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.2,"rssi_dbm":-91.8,"p_mean":2.08,"q_phase":0,"cv2":1.53,"clip":0.1,"cfo_khz":28,"mod":0,"noise":1,"stuck":0,"step_std":0.7,"nf_dbm":-95,"captures":4632,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":-99,"mod":0,"noise":0,"stuck":0,"step_std":9.5,"nf_dbm":-95,"captures":4635,"cap_err":0}
{"heartbeat":true,"node_id":"FFCB","receiver":"c5phy","hw":"v3","scanning":false,"channels":50,"sectors":4,"heading":0,"threshold_dbm":-87.0,"threshold_level_db":8.0,"video_seen":0,"gain_max":62,"bw40":1,"fe_gain_db":0.0,"bands":"5.8","antenna_dbi":8.0,"beamwidth_deg":72,"bearing_k":2.97,"wb_seen":0,"pullin":0,"tune_fail":0,"cap_err":0,"bus_stuck":258,"alias_drop":0,"sweeps":21,"usb_drop":0,"mesh_drop":0,"nf_dbm":-95,"temp_c":43.8,"uptime_s":71,"seq":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":15,"mod":0,"noise":0,"stuck":0,"step_std":2.9,"nf_dbm":-95,"captures":4638,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":267,"mod":0,"noise":0,"stuck":0,"step_std":7.8,"nf_dbm":-95,"captures":4646,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.6,"rssi_dbm":-94.4,"p_mean":2.31,"q_phase":1,"cv2":0.45,"clip":0.0,"cfo_khz":2455,"mod":0,"noise":0,"stuck":0,"step_std":17.3,"nf_dbm":-95,"captures":4649,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":84,"mod":0,"noise":0,"stuck":0,"step_std":7.6,"nf_dbm":-95,"captures":4650,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":188,"mod":0,"noise":0,"stuck":0,"step_std":7.6,"nf_dbm":-95,"captures":4660,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":56,"level_db":6.0,"rssi_dbm":-89.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.7,"nf_dbm":-95,"captures":4664,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":-1,"mod":0,"noise":0,"stuck":0,"step_std":0.7,"nf_dbm":-95,"captures":4671,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.2,"rssi_dbm":-91.8,"p_mean":2.08,"q_phase":0,"cv2":1.43,"clip":0.1,"cfo_khz":-72,"mod":0,"noise":1,"stuck":0,"step_std":0.9,"nf_dbm":-95,"captures":4680,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.8,"rssi_dbm":-94.2,"p_mean":2.39,"q_phase":1,"cv2":0.53,"clip":0.0,"cfo_khz":2906,"mod":0,"noise":1,"stuck":0,"step_std":14.5,"nf_dbm":-95,"captures":4681,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":5,"mod":0,"noise":0,"stuck":0,"step_std":2.2,"nf_dbm":-95,"captures":4695,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.4,"rssi_dbm":-94.6,"p_mean":2.18,"q_phase":1,"cv2":0.29,"clip":0.0,"cfo_khz":1379,"mod":0,"noise":0,"stuck":0,"step_std":17.0,"nf_dbm":-95,"captures":4699,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":-0,"mod":0,"noise":0,"stuck":0,"step_std":1.5,"nf_dbm":-95,"captures":4702,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.2,"rssi_dbm":-94.8,"p_mean":2.07,"q_phase":0,"cv2":0.14,"clip":0.0,"cfo_khz":505,"mod":0,"noise":0,"stuck":0,"step_std":15.7,"nf_dbm":-95,"captures":4710,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":1,"step_std":0.0,"nf_dbm":-95,"captures":4717,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.1,"rssi_dbm":-91.9,"p_mean":2.07,"q_phase":0,"cv2":1.15,"clip":0.1,"cfo_khz":-78,"mod":0,"noise":1,"stuck":0,"step_std":1.7,"nf_dbm":-95,"captures":4724,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.1,"rssi_dbm":-94.9,"p_mean":2.06,"q_phase":0,"cv2":0.12,"clip":0.0,"cfo_khz":481,"mod":0,"noise":0,"stuck":0,"step_std":13.1,"nf_dbm":-95,"captures":4725,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.3,"rssi_dbm":-94.7,"p_mean":2.16,"q_phase":0,"cv2":0.27,"clip":0.0,"cfo_khz":1686,"mod":0,"noise":0,"stuck":0,"step_std":17.0,"nf_dbm":-95,"captures":4726,"cap_err":0}
```

## Observations

- The user asked "Does this show any activity?". Hotspot on 149 / 5 GHz,
  loaded with a Twitch live stream (VPN on) to simulate load. The user is
  restarting the logger to repeat the test with `w`.

## Run 2: repeat with `w` (logger restarted, board reset)

Same build and chain; the ready line shows `iq_lane_bits` 3. Load: a
speedtest.net run on the bench PC through the hotspot (the user, after
sending the lines: "This one used speedtest.net as a load"); which phase of
the test (download, upload, finished) each `w` fell in is not known. The
user typed `w` four times while the
bench lines scrolled ("it's hard to write w while the lines scroll twice a
second"); which `w` presses, if any, were taken with the hotspot off was not
said.

| Item | Reading |
|---|---|
| Bench lines | 112 (`captures` 1332 … 1532), two heartbeats, four `w` lines |
| `w` 1 | `level_db` 14.4, `duty` 62, `cls` dot11, `conf` low, `bw_mhz` 20, `fc_mhz` 5745.2, `cv2` 1.65, `r1` 0.71, `r128` 0.250, `r2667` 0.025, `scan_lag` 128 |
| `w` 2 | `level_db` 18.6, `duty` 75, `cls` dot11, `conf` med, `bw_mhz` 20, `fc_mhz` 5745.4, `cv2` 1.99, `r1` 0.70, `r128` 0.237, `r2667` 0.018, `scan_lag` 512 (`scan_r` 0.055) |
| `w` 3 | `level_db` 0.3, `duty` 100, `cls` wb, `conf` low, `bw_mhz` 30, `cv2` 0.11, `r1` 0.24, `r128` 0.029, `scan_lag` 42 (`scan_r` 0.000) |
| `w` 4 | `level_db` 15.9, `duty` 12, `cls` dot11, `conf` low, `bw_mhz` 20, `fc_mhz` 5744.9, `cv2` 0.10, `r1` 0.69, `r128` 0.154, `r2667` 0.019, `scan_lag` 128 |
| Lowest gain the bench lines stepped to | 44 (`level_db` 24.3, `rssi_dbm` −70.7, `clip` 2.6 at 1400) |
| `stuck` 1 in hold | 10 lines: 1333, 1441, 1447, 1452, 1455, 1496, 1498, 1513, 1519, 1529 (all `p_mean` 2.00, `cfo_khz` 0, `step_std` 0.0) |

Console lines, verbatim: the ready line, the hold line, every `w` line,
the heartbeats and every bench line that is not at the floor. The other
bench lines are at the 3-lane floor (`gain` 62, `level_db` 0.0, `p_mean`
2.00, `cv2` 0.00 or 0.02, `cfo_khz` −2 … +2, `step_std` 0.0 … 1.2) or are the
ten `stuck` lines above, and are in the session's log file on the bench PC.

```
{"info":"c5phy v3 station ready","node_id":"FFCB","receiver":"c5phy","hw":"v3","channels":50,"sectors":4,"heading":0,"fe_gain_db":0.0,"threshold_dbm":-87.0,"threshold_level_db":8.0,"q_min":40,"peak_pick":1,"video":1,"bw40":1,"gain_max":62,"window_us":409,"iq_lane_bits":3,"bands":"5.8","antenna_dbi":8.0,"beamwidth_deg":72,"bearing_k":2.97,"wideband":1,"pullin":1,"gain_step":3,"mesh_uart":"D4 TX / D5 RX 115200","phy_set_freq":true,"rf":true}
h A7{"heartbeat":true,"node_id":"FFCB","receiver":"c5phy","hw":"v3","scanning":true,"channels":50,"sectors":4,"heading":0,"threshold_dbm":-87.0,"threshold_level_db":8.0,"video_seen":0,"gain_max":62,"bw40":1,"fe_gain_db":0.0,"bands":"5.8","antenna_dbi":8.0,"beamwidth_deg":72,"bearing_k":2.97,"wb_seen":0,"pullin":0,"tune_fail":0,"cap_err":0,"bus_stuck":66,"alias_drop":0,"sweeps":4,"usb_drop":0,"mesh_drop":0,"nf_dbm":-95,"temp_c":43.8,"uptime_s":11,"seq":0}
{"info":"hold","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":56,"level_db":6.2,"rssi_dbm":-88.8,"p_mean":2.08,"q_phase":0,"cv2":1.55,"clip":0.1,"cfo_khz":-60,"mod":0,"noise":1,"stuck":0,"step_std":8.5,"nf_dbm":-95,"captures":1340,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.2,"rssi_dbm":-91.8,"p_mean":2.08,"q_phase":0,"cv2":1.64,"clip":0.1,"cfo_khz":-45,"mod":0,"noise":1,"stuck":0,"step_std":0.5,"nf_dbm":-95,"captures":1342,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":3.1,"rssi_dbm":-91.9,"p_mean":4.11,"q_phase":2,"cv2":8.45,"clip":2.9,"cfo_khz":315,"mod":0,"noise":1,"stuck":0,"step_std":1.7,"nf_dbm":-95,"captures":1344,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.1,"rssi_dbm":-94.9,"p_mean":2.05,"q_phase":0,"cv2":0.72,"clip":0.1,"cfo_khz":-7,"mod":0,"noise":1,"stuck":0,"step_std":0.2,"nf_dbm":-95,"captures":1345,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":110,"mod":0,"noise":0,"stuck":0,"step_std":9.0,"nf_dbm":-95,"captures":1352,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.2,"rssi_dbm":-91.8,"p_mean":2.09,"q_phase":0,"cv2":1.93,"clip":0.1,"cfo_khz":-69,"mod":0,"noise":1,"stuck":0,"step_std":4.4,"nf_dbm":-95,"captures":1354,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":18,"mod":0,"noise":0,"stuck":0,"step_std":2.8,"nf_dbm":-95,"captures":1357,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":53,"level_db":9.0,"rssi_dbm":-86.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.9,"nf_dbm":-95,"captures":1366,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":56,"level_db":6.1,"rssi_dbm":-88.9,"p_mean":2.07,"q_phase":0,"cv2":1.19,"clip":0.1,"cfo_khz":52,"mod":0,"noise":1,"stuck":0,"step_std":0.5,"nf_dbm":-95,"captures":1369,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.1,"rssi_dbm":-91.9,"p_mean":2.06,"q_phase":0,"cv2":0.98,"clip":0.1,"cfo_khz":-13,"mod":0,"noise":1,"stuck":0,"step_std":0.4,"nf_dbm":-95,"captures":1371,"cap_err":0}
w
{"info":"wideband","ch":"A7","freq_mhz":5745,"level_db":14.4,"duty":62,"cls":"dot11","conf":"low","bw_mhz":20,"fc_mhz":5745.2,"cv2":1.65,"r1":0.71,"r4":0.12,"r128":0.250,"r512":0.028,"r2667":0.025,"scan_lag":128,"scan_r":0.276}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":47,"level_db":20.1,"rssi_dbm":-74.9,"p_mean":6.54,"q_phase":16,"cv2":2.64,"clip":2.3,"cfo_khz":406,"mod":0,"noise":1,"stuck":0,"step_std":13.9,"nf_dbm":-95,"captures":1385,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":44,"level_db":18.0,"rssi_dbm":-77.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":1.2,"nf_dbm":-95,"captures":1392,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":44,"level_db":24.3,"rssi_dbm":-70.7,"p_mean":8.48,"q_phase":21,"cv2":2.03,"clip":2.6,"cfo_khz":20,"mod":1,"noise":1,"stuck":0,"step_std":12.0,"nf_dbm":-95,"captures":1400,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":56,"level_db":7.4,"rssi_dbm":-87.6,"p_mean":2.77,"q_phase":2,"cv2":3.96,"clip":0.6,"cfo_khz":83,"mod":0,"noise":1,"stuck":0,"step_std":2.8,"nf_dbm":-95,"captures":1403,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.1,"rssi_dbm":-91.9,"p_mean":2.04,"q_phase":0,"cv2":0.49,"clip":0.1,"cfo_khz":-6,"mod":0,"noise":0,"stuck":0,"step_std":0.5,"nf_dbm":-95,"captures":1405,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":44,"level_db":18.9,"rssi_dbm":-76.1,"p_mean":2.49,"q_phase":2,"cv2":0.61,"clip":0.0,"cfo_khz":896,"mod":0,"noise":1,"stuck":0,"step_std":13.8,"nf_dbm":-95,"captures":1413,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":53,"level_db":9.0,"rssi_dbm":-86.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.4,"nf_dbm":-95,"captures":1417,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":56,"level_db":6.1,"rssi_dbm":-88.9,"p_mean":2.04,"q_phase":0,"cv2":0.55,"clip":0.0,"cfo_khz":397,"mod":0,"noise":1,"stuck":0,"step_std":11.3,"nf_dbm":-95,"captures":1420,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":53,"level_db":9.1,"rssi_dbm":-85.9,"p_mean":2.04,"q_phase":0,"cv2":0.46,"clip":0.0,"cfo_khz":4,"mod":0,"noise":0,"stuck":0,"step_std":0.7,"nf_dbm":-95,"captures":1424,"cap_err":0}
w
{"info":"wideband","ch":"A7","freq_mhz":5745,"level_db":18.6,"duty":75,"cls":"dot11","conf":"med","bw_mhz":20,"fc_mhz":5745.4,"cv2":1.99,"r1":0.70,"r4":0.15,"r128":0.237,"r512":0.039,"r2667":0.018,"scan_lag":512,"scan_r":0.055}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.3,"nf_dbm":-95,"captures":1434,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.1,"nf_dbm":-95,"captures":1440,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":69,"mod":0,"noise":0,"stuck":0,"step_std":6.8,"nf_dbm":-95,"captures":1444,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":1.1,"rssi_dbm":-93.9,"p_mean":2.60,"q_phase":4,"cv2":0.78,"clip":0.0,"cfo_khz":306,"mod":0,"noise":1,"stuck":0,"step_std":7.8,"nf_dbm":-95,"captures":1445,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":56,"level_db":6.0,"rssi_dbm":-89.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.4,"nf_dbm":-95,"captures":1451,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":26,"mod":0,"noise":0,"stuck":0,"step_std":2.6,"nf_dbm":-95,"captures":1459,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.01,"clip":0.0,"cfo_khz":32,"mod":0,"noise":0,"stuck":0,"step_std":5.3,"nf_dbm":-95,"captures":1460,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.4,"rssi_dbm":-91.6,"p_mean":2.19,"q_phase":1,"cv2":0.31,"clip":0.0,"cfo_khz":557,"mod":0,"noise":0,"stuck":0,"step_std":11.2,"nf_dbm":-95,"captures":1462,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":-811,"mod":0,"noise":0,"stuck":0,"step_std":7.9,"nf_dbm":-95,"captures":1463,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":53,"level_db":9.1,"rssi_dbm":-85.9,"p_mean":2.03,"q_phase":0,"cv2":0.29,"clip":0.0,"cfo_khz":12,"mod":0,"noise":0,"stuck":0,"step_std":1.3,"nf_dbm":-95,"captures":1469,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.2,"rssi_dbm":-91.8,"p_mean":2.07,"q_phase":0,"cv2":1.37,"clip":0.1,"cfo_khz":-40,"mod":0,"noise":1,"stuck":0,"step_std":0.3,"nf_dbm":-95,"captures":1471,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.3,"nf_dbm":-95,"captures":1475,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.1,"nf_dbm":-95,"captures":1478,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.0,"rssi_dbm":-92.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":0,"mod":0,"noise":0,"stuck":0,"step_std":0.2,"nf_dbm":-95,"captures":1480,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":1.0,"rssi_dbm":-94.0,"p_mean":2.52,"q_phase":3,"cv2":0.64,"clip":0.0,"cfo_khz":-234,"mod":0,"noise":1,"stuck":0,"step_std":10.9,"nf_dbm":-95,"captures":1482,"cap_err":0}
w
{"info":"wideband","ch":"A7","freq_mhz":5745,"level_db":0.3,"duty":100,"cls":"wb","conf":"low","bw_mhz":30,"fc_mhz":5745.2,"cv2":0.11,"r1":0.24,"r4":0.06,"r128":0.029,"r512":0.010,"r2667":0.007,"scan_lag":42,"scan_r":0.000}
{"heartbeat":true,"node_id":"FFCB","receiver":"c5phy","hw":"v3","scanning":false,"channels":50,"sectors":4,"heading":0,"threshold_dbm":-87.0,"threshold_level_db":8.0,"video_seen":0,"gain_max":62,"bw40":1,"fe_gain_db":0.0,"bands":"5.8","antenna_dbi":8.0,"beamwidth_deg":72,"bearing_k":2.97,"wb_seen":0,"pullin":0,"tune_fail":0,"cap_err":0,"bus_stuck":95,"alias_drop":0,"sweeps":6,"usb_drop":0,"mesh_drop":0,"nf_dbm":-95,"temp_c":43.8,"uptime_s":75,"seq":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.0,"rssi_dbm":-95.0,"p_mean":2.00,"q_phase":0,"cv2":0.00,"clip":0.0,"cfo_khz":12,"mod":0,"noise":0,"stuck":0,"step_std":6.7,"nf_dbm":-95,"captures":1514,"cap_err":0}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":59,"level_db":3.2,"rssi_dbm":-91.8,"p_mean":2.08,"q_phase":0,"cv2":1.57,"clip":0.1,"cfo_khz":22,"mod":0,"noise":1,"stuck":0,"step_std":0.2,"nf_dbm":-95,"captures":1517,"cap_err":0}
w
{"info":"wideband","ch":"A7","freq_mhz":5745,"level_db":15.9,"duty":12,"cls":"dot11","conf":"low","bw_mhz":20,"fc_mhz":5744.9,"cv2":0.10,"r1":0.69,"r4":0.08,"r128":0.154,"r512":0.013,"r2667":0.019,"scan_lag":128,"scan_r":0.339}
{"info":"bench","ch":"A7","freq_mhz":5745,"wifi_ch":149,"sector":0,"switch":"000","gain":62,"level_db":0.2,"rssi_dbm":-94.8,"p_mean":2.07,"q_phase":0,"cv2":1.37,"clip":0.1,"cfo_khz":-17,"mod":0,"noise":1,"stuck":0,"step_std":0.2,"nf_dbm":-95,"captures":1528,"cap_err":0}
```
