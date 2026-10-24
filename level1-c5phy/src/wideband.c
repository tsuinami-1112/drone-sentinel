#include "wideband.h"
#include <math.h>
#include <string.h>

int wb_candidate(const WbSectorView* s, const WbRules* r)
{
    return s->windows == r->sector_windows && s->level_min >= r->detect_level_db &&
           s->level_max - s->level_min <= r->level_spread_db && s->cv2 >= r->cv2_min && s->cv2 <= r->cv2_max;
}

const char* wb_class(const IqLagFeatures* f, const WbRules* r)
{
    if (f->r2667 >= r->r2667_min && f->r128 < r->r128_lte_max) return "lte";
    if (f->r128 >= r->r128_min) return "dot11";
    return "wb";
}

/* Noise decorrelates at lag 1, so r1 reads S/(S+N) of the waveform's own value
 * (model: LTE-like 0.85 at S/N 30 dB, 0.80 at 12 dB, under the 0.78 edge by
 * 10 dB): the bucket is taken on r1 scaled back by (S+N)/S from the level, the
 * reported r1 stays the raw feature. */
int wb_bucket_mhz(float r1, float level_db)
{
    float r = r1 * (1.0f + powf(10.0f, -level_db / 10.0f));
    if (r > 1.0f) r = 1.0f;
    return r >= 0.78f ? 10 : (r >= 0.52f ? 20 : (r >= 0.30f ? 30 : 40));
}

/* Confidence from the duty and the deciding feature's margin: high needs the
 * link on in 90 % of the windows and the feature at its _HIGH mark. */
static void classify(WbSummary* s, const WbRules* r)
{
    float key = 0.0f, key_high = 1.0f;
    s->cls = wb_class(&s->f, r);
    if (s->cls[0] == 'l') { key = s->f.r2667; key_high = r->r2667_high; }
    else if (s->cls[0] == 'd') { key = s->f.r128; key_high = r->r128_high; }
    int named = s->cls[0] != 'w';
    s->conf = named && s->duty >= 90 && key >= key_high ? "high" : (named && s->duty >= r->duty_min ? "med" : "low");
    s->bw_mhz = wb_bucket_mhz(s->f.r1, s->level_db);
}

void wb_summarise(const WbWindow* w, int n, const WbRules* r, WbSummary* out)
{
    memset(out, 0, sizeof(*out));
    out->cls = "wb";
    out->conf = "low";
    out->bw_mhz = 40;
    if (n <= 0) return;
    out->windows = n;
    out->level_min = out->level_max = w[0].level_db;
    for (int i = 1; i < n; i++) {
        if (w[i].level_db < out->level_min) out->level_min = w[i].level_db;
        if (w[i].level_db > out->level_max) out->level_max = w[i].level_db;
    }
    float lsum = 0, csum = 0, fsum = 0, qsum = 0;
    for (int i = 0; i < n; i++) {
        if (w[i].level_db < out->level_max - r->duty_db) continue;
        out->on++;
        lsum += w[i].level_db; csum += w[i].cv2; fsum += w[i].cfo_khz; qsum += w[i].q_phase;
        out->f.r1 += w[i].f.r1; out->f.r4 += w[i].f.r4; out->f.r128 += w[i].f.r128;
        out->f.r512 += w[i].f.r512; out->f.r2667 += w[i].f.r2667;
    }
    float inv = 1.0f / (float)out->on;          /* on >= 1: the loudest window itself */
    out->duty = 100 * out->on / n;
    out->level_db = lsum * inv;
    out->cv2 = csum * inv;
    out->q_phase = qsum * inv;
    out->cfo_khz = fsum * inv;
    out->f.r1 *= inv; out->f.r4 *= inv; out->f.r128 *= inv; out->f.r512 *= inv; out->f.r2667 *= inv;
    classify(out, r);
}

int wb_confirmed(const WbSummary* s, const WbRules* r)
{
    return s->duty >= r->duty_min;
}
