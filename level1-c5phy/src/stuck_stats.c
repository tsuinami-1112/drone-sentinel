#include "stuck_stats.h"
#include "fpv_channels.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void stuck_reset(StuckStats* s)
{
    memset(s, 0, sizeof(*s));
    s->ch = -1;
}

void stuck_visit_begin(StuckStats* s, int ch)
{
    s->ch = ch >= 0 && ch < STUCK_MAX_CH ? ch : -1;
    s->visit_windows = s->visit_stuck = 0;
}

void stuck_window(StuckStats* s, int sector, int win, int stuck)
{
    stuck = stuck != 0;
    s->windows++;
    s->stuck += (uint32_t)stuck;
    s->visit_windows++;
    s->visit_stuck += stuck;
    if (s->ch >= 0) { s->ch_windows[s->ch]++; s->ch_stuck[s->ch] += (uint32_t)stuck; }
    if (sector >= 0 && sector < STUCK_MAX_SEC) { s->sec_windows[sector]++; s->sec_stuck[sector] += (uint32_t)stuck; }
    if (win >= 0 && win < STUCK_MAX_WIN) { s->win_windows[win]++; s->win_stuck[win] += (uint32_t)stuck; }
}

void stuck_visit_end(StuckStats* s)
{
    if (s->visit_windows > 0) {
        s->visits++;
        if (s->visit_stuck > 0) s->visits_any++;
        if (s->visit_stuck == s->visit_windows) s->visits_all++;
    }
    s->visit_windows = s->visit_stuck = 0;
    s->ch = -1;
}

/* Append to out at *n while it fits under limit; on overflow leave *n as it
 * was and return 0. */
static int put(char* out, size_t limit, size_t* n, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int k = vsnprintf(out + *n, limit - *n, fmt, ap);
    va_end(ap);
    if (k < 0 || *n + (size_t)k >= limit) { out[*n] = 0; return 0; }
    *n += (size_t)k;
    return 1;
}

#define TAIL_RESERVE 24         /* room kept for },"more":N} and the terminator */

int stuck_report_json(char* out, size_t cap, const StuckStats* s, uint32_t since_s)
{
    if (!out || cap <= TAIL_RESERVE) return 0;
    out[0] = 0;
    size_t n = 0, body = cap - TAIL_RESERVE;
    if (!put(out, body, &n, "{\"info\":\"stuck\",\"since_s\":%u,\"windows\":%u,\"stuck\":%u,\"visits\":%u,\"visits_any\":%u,\"visits_all\":%u,\"by_sector\":[",
             (unsigned)since_s, (unsigned)s->windows, (unsigned)s->stuck, (unsigned)s->visits, (unsigned)s->visits_any, (unsigned)s->visits_all))
        return 0;
    for (int i = 0; i < STUCK_MAX_SEC; i++)
        if (!put(out, body, &n, "%s[%u,%u]", i ? "," : "", (unsigned)s->sec_stuck[i], (unsigned)s->sec_windows[i])) return 0;
    if (!put(out, body, &n, "],\"by_window\":[")) return 0;
    for (int i = 0; i < STUCK_MAX_WIN; i++)
        if (!put(out, body, &n, "%s[%u,%u]", i ? "," : "", (unsigned)s->win_stuck[i], (unsigned)s->win_windows[i])) return 0;
    if (!put(out, body, &n, "],\"by_ch\":{")) return 0;

    /* Channels with a stuck window, most first (insertion sort, stable: ties in plan order). */
    int idx[STUCK_MAX_CH], m = 0, channels = fpv_channel_count();
    if (channels > STUCK_MAX_CH) channels = STUCK_MAX_CH;
    for (int i = 0; i < channels; i++) {
        if (!s->ch_stuck[i]) continue;
        int j = m++;
        while (j > 0 && s->ch_stuck[idx[j - 1]] < s->ch_stuck[i]) { idx[j] = idx[j - 1]; j--; }
        idx[j] = i;
    }
    int shown = 0;
    for (; shown < m; shown++) {
        const FpvChannel* c = fpv_channel(idx[shown]);
        if (!put(out, body, &n, "%s\"%c%d\":[%u,%u]", shown ? "," : "", c->band, c->number,
                 (unsigned)s->ch_stuck[idx[shown]], (unsigned)s->ch_windows[idx[shown]]))
            break;
    }
    if (!put(out, cap, &n, "},\"more\":%d}", m - shown)) return 0;
    return (int)n;
}
