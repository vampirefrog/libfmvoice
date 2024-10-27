#pragma once

#include <stdint.h>

#include "opl_voice.h"
#include "opn_voice.h"

struct opl_voice;
struct opn_voice;

/* For OPM we also support OPZ wave sel */
struct opm_voice_operator {
	uint8_t dt1_mul, tl, ks_ar, ams_d1r, dt2_d2r, d1l_rr, ws;
};

struct opm_voice {
	char name[256];
	// per chip fields
	uint8_t lfrq, amd, pmd, w, ne_nfrq;
	// per channel fields
	uint8_t rl_fb_con, pms_ams, slot;
	struct opm_voice_operator operators[4];
};
void opm_voice_init(struct opm_voice *);
void opm_voice_dump(struct opm_voice *);
void opm_voice_normalize(struct opm_voice *);
int opm_voice_compare(struct opm_voice *, struct opm_voice *);
int opm_operator_is_silent(struct opm_voice_operator *);
int opm_voice_is_silent(struct opm_voice *);
void opm_voice_compute_md5_sum(struct opm_voice *, uint8_t *digest);
int opm_voice_load_opl_voice(struct opm_voice *opmv, struct opl_voice *oplv);
int opm_voice_load_opn_voice(struct opm_voice *opmv, struct opn_voice *opnv);
int opm_pitch_to_kc_kf(float pitch, int clock);
float opm_kc_kf_to_pitch(uint8_t kc, uint8_t kf, int clock);

uint8_t opm_voice_get_lfrq(struct opm_voice *v);
uint8_t opm_voice_get_rl(struct opm_voice *v);
uint8_t opm_voice_get_fb(struct opm_voice *v);
uint8_t opm_voice_get_con(struct opm_voice *v);
uint8_t opm_voice_get_pms(struct opm_voice *v);
uint8_t opm_voice_get_ams(struct opm_voice *v);
uint8_t opm_voice_get_slot(struct opm_voice *v);
uint8_t opm_voice_operator_get_dt1(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_mul(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_tl(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_ks(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_ar(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_ams(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_d1r(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_dt2(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_d2r(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_d1l(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_rr(struct opm_voice *v, int op);
uint8_t opm_voice_operator_get_ws(struct opm_voice *v, int op);
