/* Where the sweep's stuck windows happen. Bench1 (2026-10-24/25): about 5-7 %
 * of the windows a scan takes read stuck (every sample identical on the wired
 * lanes) with nothing on any pin, on 3 lanes and on 8, with an 8 ms or a 30 ms
 * settle after the tune; none in quiet holds, a few in a hold under phone-
 * hotspot traffic. These counts tell the candidate causes apart: a tune that
 * freezes the stream for a whole visit (visits_all), the sector switch
 * (by_sector: sector 0 is the first window after the tune), back-to-back
 * captures (the later windows of a sector: by_window, which only fills past
 * window 0 when a sector's first window comes within QUIET_MARGIN_DB of the
 * threshold, so on a quiet bench by_sector carries it), or particular channels
 * (by_ch). main.cpp feeds it the sweep's measured windows only, not holds,
 * v / w or confirmation passes or gain retakes, so its total reads under the
 * bus_stuck difference; a hold shows stuck on every bench line. Plain C, host
 * tested. */
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define STUCK_MAX_CH   64       /* MAX_CHANNELS of config.h */
#define STUCK_MAX_SEC  4        /* SECTOR_COUNT */
#define STUCK_MAX_WIN  3        /* WINDOWS_PER_SECTOR */

typedef struct {
    uint32_t windows, stuck;
    uint32_t ch_windows[STUCK_MAX_CH], ch_stuck[STUCK_MAX_CH];
    uint32_t sec_windows[STUCK_MAX_SEC], sec_stuck[STUCK_MAX_SEC];
    uint32_t win_windows[STUCK_MAX_WIN], win_stuck[STUCK_MAX_WIN];   /* by window of its sector: 0 = after the switch */
    uint32_t visits, visits_any, visits_all;                         /* tunes measured; with one stuck; all stuck */
    int      ch;                        /* the visit in progress: table channel, -1 for a pull-in target */
    int      visit_windows, visit_stuck;
} StuckStats;

void stuck_reset(StuckStats* s);
/* A visit is one tune and the four sectors measured on it. ch < 0: not a table
 * channel (a pull-in target), counted everywhere but by_ch. */
void stuck_visit_begin(StuckStats* s, int ch);
void stuck_window(StuckStats* s, int sector, int win, int stuck);
void stuck_visit_end(StuckStats* s);

/* The bench line, {"info":"stuck",...}. since_s: seconds the counts cover.
 * by_ch lists the table channels with a stuck window, most first, by their
 * fpv_channels.c names ("R1":[stuck,windows]) while they fit in cap; "more"
 * counts the ones left out. Returns the length written, 0 when cap is too
 * small for the fixed part. */
int stuck_report_json(char* out, size_t cap, const StuckStats* s, uint32_t since_s);

#ifdef __cplusplus
}
#endif
