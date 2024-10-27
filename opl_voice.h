#pragma once

#include <stdint.h>

#include "opm_voice.h"
#include "opn_voice.h"

struct opm_voice;
struct opn_voice;

struct opl_voice_operator {
	uint8_t am_vib_eg_ksr_mul, ksl_tl, ar_dr, sl_rr, ws;
};

struct opl_voice {
	char name[256];
	uint8_t en_4op, perc_inst; // is perc_inst covered by dam_dvb_..?
	uint8_t ch_fb_cnt[2];
	uint8_t dam_dvb_ryt_bd_sd_tom_tc_hh; // ??
	struct opl_voice_operator operators[4];
};
void opl_voice_init(struct opl_voice *);
void opl_voice_dump(struct opl_voice *);
void opl_voice_normalize(struct opl_voice *);
int opl_voice_compare(struct opl_voice *, struct opl_voice *);
int opl_operator_is_silent(struct opl_voice_operator *);
int opl_voice_is_silent(struct opl_voice *);
void opl_voice_compute_md5_sum(struct opl_voice *, uint8_t *digest);
int opl_voice_load_opm_voice(struct opl_voice *oplv, struct opm_voice *opmv);
int opl_voice_load_opn_voice(struct opl_voice *oplv, struct opn_voice *opnv);
int opl_pitch_to_block_fnum(float pitch, int clock);
float opl_block_fnum_to_pitch(uint8_t block_fnum2, uint8_t fnum1, int clock);

uint8_t opl_voice_get_ch(struct opl_voice *v);
uint8_t opl_voice_get_fb(struct opl_voice *v);
uint8_t opl_voice_get_cnt(struct opl_voice *v);
uint8_t opl_voice_get_ch1(struct opl_voice *v);
uint8_t opl_voice_get_fb1(struct opl_voice *v);
uint8_t opl_voice_get_cnt1(struct opl_voice *v);
uint8_t opl_voice_get_am_depth(struct opl_voice *v);
uint8_t opl_voice_get_vib_depth(struct opl_voice *v);
uint8_t opl_voice_get_ryt(struct opl_voice *v);
uint8_t opl_voice_get_perc(struct opl_voice *v);
uint8_t opl_voice_operator_get_am(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_vib(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_eg_typ(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_ksr(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_mul(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_ksl(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_tl(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_ar(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_dr(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_sl(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_rr(struct opl_voice *v, int op);
uint8_t opl_voice_operator_get_ws(struct opl_voice *v, int op);
