#pragma once

#include <stdint.h>
#include <stdlib.h>

struct opl_voice;
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

struct opn_voice_operator {
	uint8_t dt_mul, tl, ks_ar, am_dr, sr, sl_rr, ssg_eg;
};

struct opn_voice {
	char name[256];
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

struct fm_voice_bank {
	struct opl_voice *opl_voices;
	int num_opl_voices;
	struct opm_voice *opm_voices;
	int num_opm_voices;
	struct opn_voice *opn_voices;
	int num_opn_voices;
};

enum fm_voice_file_format {
	FORMAT_AUTO = 0,
	FORMAT_BNK,
	FORMAT_DMP,
	FORMAT_SYX_DX21,
	FORMAT_SYX_FB01,
	FORMAT_INS,
	FORMAT_OP3,
	FORMAT_OPM,
	FORMAT_SBI,
	FORMAT_Y12,
};

void fm_voice_bank_init(struct fm_voice_bank *bank);
void fm_voice_bank_clear(struct fm_voice_bank *bank);
int fm_voice_bank_append_opl_voice(struct fm_voice_bank *bank, struct opl_voice *voice);
struct opl_voice *fm_voice_bank_reserve_opl_voices(struct fm_voice_bank *bank, int num_voices);
int fm_voice_bank_append_opm_voice(struct fm_voice_bank *bank, struct opm_voice *voice);
struct opm_voice *fm_voice_bank_reserve_opm_voices(struct fm_voice_bank *bank, int num_voices);
int fm_voice_bank_append_opn_voice(struct fm_voice_bank *bank, struct opn_voice *voice);
struct opn_voice *fm_voice_bank_reserve_opn_voices(struct fm_voice_bank *bank, int num_voices);
void fm_voice_bank_dump(struct fm_voice_bank *bank);
int fm_voice_bank_load(struct fm_voice_bank *bank, uint8_t *data, size_t data_len);
int fm_voice_bank_save(struct fm_voice_bank *bank, enum fm_voice_file_format format, size_t (*write)(void *, size_t, void *), void *data_ptr);
