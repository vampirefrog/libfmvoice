#pragma once

#include <stdint.h>

#include "opl_voice.h"
#include "opm_voice.h"

struct opl_voice;
struct opm_voice;

struct opn_voice_operator {
	uint8_t dt_mul, tl, ks_ar, am_dr, sr, sl_rr, ssg_eg;
};

struct opn_voice {
	char name[256], dumper[256], game[256];
	uint8_t lfo, slot, fb_con, lr_ams_pms;
	struct opn_voice_operator operators[4];
};

void opn_voice_init(struct opn_voice *);
void opn_voice_dump(struct opn_voice *);
void opn_voice_normalize(struct opn_voice *);
int opn_voice_compare(struct opn_voice *, struct opn_voice *);
int opn_operator_is_silent(struct opn_voice_operator *);
int opn_voice_is_silent(struct opn_voice *);
void opn_voice_compute_md5_sum(struct opn_voice *, uint8_t *digest);
int opn_voice_load_opl_voice(struct opn_voice *opnv, struct opl_voice *oplv);
int opn_voice_load_opm_voice(struct opn_voice *opnv, struct opm_voice *opmv);
int opn_pitch_to_block_fnum(float pitch, int clock); // YM2203 (OPN)
float opn_block_fnum_to_pitch(uint8_t block_fnum2, uint8_t fnum1, int clock);
int opnx_pitch_to_block_fnum(float pitch, int clock); // OPNA, OPNB, OPN2
float opnx_block_fnum_to_pitch(uint8_t block_fnum2, uint8_t fnum1, int clock);

uint8_t opn_voice_get_lfo(struct opn_voice *v);
uint8_t opn_voice_get_slot(struct opn_voice *v);
uint8_t opn_voice_get_fb(struct opn_voice *v);
uint8_t opn_voice_get_con(struct opn_voice *v);
uint8_t opn_voice_get_lr(struct opn_voice *v);
uint8_t opn_voice_get_ams(struct opn_voice *v);
uint8_t opn_voice_get_pms(struct opn_voice *v);
uint8_t opn_voice_get_operator_dt(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_mul(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_tl(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_ks(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_ar(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_am(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_dr(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_sr(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_sl(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_rr(struct opn_voice *v, int op);
uint8_t opn_voice_get_operator_ssg_eg(struct opn_voice *v, int op);
