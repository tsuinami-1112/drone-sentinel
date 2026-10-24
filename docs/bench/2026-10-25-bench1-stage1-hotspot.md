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
hotspot nearby; distance, hotspot channel (`netsh wlan show interfaces`) and
whether a download was running during these lines: not yet given.

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

- The user asked "Does this show any activity?"; no description of what the
  phone was doing during the lines yet.
