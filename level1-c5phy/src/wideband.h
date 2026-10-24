/* The wideband (digital video link) decisions: the sweep's candidate gate, and
 * over a confirmation pass which windows carry the link, its duty, class,
 * confidence and bandwidth bucket. Plain C over per-window measurements so the
 * host tests drive exactly what main.cpp runs; main.cpp measures and reports. */
#pragma once
#include "demod.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float detect_level_db;  /* DETECT_LEVEL_DB */
    int   sector_windows;   /* WINDOWS_PER_SECTOR: a candidate needs all of them */
    float level_spread_db;  /* WB_LEVEL_SPREAD_DB */
    float cv2_min, cv2_max; /* WB_CV2_MIN / WB_CV2_MAX: the candidate's envelope window */
    float duty_db;          /* WB_DUTY_DB: "on" within this of the loudest window ... */
    float on_snr_db;        /* WB_ON_SNR_DB: ... and this far over the no-signal power at its own gain */
    int   duty_min;         /* WB_DUTY_MIN: percent of the windows on for a report */
    float r128_min, r2667_min, r128_lte_max, r128_high, r2667_high;   /* WB_R* */
} WbRules;

/* One window of a confirmation pass, all taken at the same gain. */
typedef struct {
    float level_db;         /* level_from(): (GAIN_MAX - gain) + snr_db */
    float snr_db;           /* 10 log10(p_mean / noise power): the window's own power over the no-signal
                               power. An empty window reads ~0 at every gain on a quantization-limited
                               receiver, where level_db reads GAIN_MAX - gain */
    float cv2, cfo_khz, q_phase;
    IqLagFeatures f;
} WbWindow;

typedef struct {
    int   windows, on, duty;            /* windows, windows on, percent on */
    float level_db, level_min, level_max;   /* mean of the on windows; min / max over all */
    float cv2, cfo_khz, q_phase;        /* means over the on windows */
    IqLagFeatures f;                    /* features averaged over the on windows */
                                        /* (with no window on: over every window, duty 0) */
    const char* cls;                    /* "lte" | "dot11" | "wb" */
    const char* conf;                   /* "high" | "med" | "low" */
    int   bw_mhz;                       /* 10 | 20 | 30 | 40 */
} WbSummary;

/* The strongest sector of a swept channel, as measure_sector left it. */
typedef struct {
    int   windows;
    float level_min, level_max;
    float snr_min;                      /* lowest snr_db over the windows */
    float cv2;                          /* median over the windows */
} WbSectorView;

/* Candidate: every window of the sector above threshold and on_snr_db over the
 * no-signal power at its own gain, within level_spread_db (continuous over
 * ~1.5 ms), with a noise-like envelope. The caller adds "WIDEBAND and not an
 * analog hit". */
int wb_candidate(const WbSectorView* s, const WbRules* r);

/* Class from the lag features: lte (r2667 with no 802.11 peak), dot11 (r128),
 * else wb. */
const char* wb_class(const IqLagFeatures* f, const WbRules* r);

/* Bandwidth bucket from the one-sample autocorrelation, r1 scaled back by
 * (S+N)/S from the level (model: LTE-like 9 MHz 0.85, 802.11 20 MHz 0.67,
 * 40 MHz 0.06). */
int wb_bucket_mhz(float r1, float level_db);

/* Summarise n windows (n >= 1): a window is on within duty_db of the loudest
 * and at least on_snr_db over the no-signal power; duty is their share, level /
 * cv2 / cfo / q and the features are averaged over them, then class,
 * confidence and bucket. With no window on, duty is 0 and the means run over
 * every window, so a quiet pass still reads its features' noise floor. */
void wb_summarise(const WbWindow* w, int n, const WbRules* r, WbSummary* out);

/* Report or not: the duty the confirmation pass must reach, and the on
 * windows' mean level at least the detection threshold. */
int wb_confirmed(const WbSummary* s, const WbRules* r);

#ifdef __cplusplus
}
#endif
