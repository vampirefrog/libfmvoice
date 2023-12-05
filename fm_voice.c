#include <stdio.h>
#include <string.h>

#include "fm_voice.h"
#include "bnk_file.h"
#include "dmp_file.h"
#include "op3_file.h"
#include "opm_file.h"
#include "ins_file.h"
#include "sbi_file.h"
#include "tfi_file.h"
#include "y12_file.h"
#include "syx_dx21.h"
#include "syx_fb01.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void opl_voice_init(struct opl_voice *v) {
	memset(v, 0, sizeof(*v));
}

void opm_voice_init(struct opm_voice *v) {
	memset(v, 0, sizeof(*v));
}

void opn_voice_init(struct opn_voice *v) {
	memset(v, 0, sizeof(*v));
}

void opl_voice_dump(struct opl_voice *v) {
	printf("name=%.256s\n", v->name);
	printf("4OP=%d percussion=%d\n", v->en_4op, v->perc_inst);
	for(int i = 0; i < 2; i++) {
		printf(
			"%d: ch=%c%c%c%c fb=%d c=%d  ",
			i,
			v->ch_fb_cnt[i] & 0x80 ? 'D' : '-',
			v->ch_fb_cnt[i] & 0x40 ? 'C' : '-',
			v->ch_fb_cnt[i] & 0x20 ? 'B' : '-',
			v->ch_fb_cnt[i] & 0x10 ? 'A' : '-',
			v->ch_fb_cnt[i] >> 1 & 0x07,
			v->ch_fb_cnt[i] & 0x01
		);
	}
	printf("\n");
	printf("DAM=%d DVB=%d RYT=%d BD=%d SD=%d TOM=%d TC=%d HH=%d\n",
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 7,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 6 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 5 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 4 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 3 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 2 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 1 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh & 1
	);
	printf("OP: AR DR SL RR TL MUL EG AM VIB KSR KSL WS\n");
	for(int i = 0; i < 4; i++) {
		if(!v->en_4op && i >= 2) break;
		struct opl_voice_operator *op = &v->operators[i];
		printf(
			"%2d: %2d %2d %2d %2d %2d %3d %2d %2d %3d %3d %3d %2d\n",
			i,
			op->ar_dr >> 4,
			op->ar_dr & 0x0f,
			op->sl_rr >> 4,
			op->sl_rr & 0x0f,
			op->ksl_tl & 0x3f,
			op->am_vib_eg_ksr_mul & 0x0f,
			op->am_vib_eg_ksr_mul >> 5 & 1,
			op->am_vib_eg_ksr_mul >> 7,
			op->am_vib_eg_ksr_mul >> 6 & 1,
			op->am_vib_eg_ksr_mul >> 4 & 1,
			op->ksl_tl >> 6,
			op->ws & 0x07
		);
	}
}

void opm_voice_dump(struct opm_voice *v) {
	printf("%.256s\n", v->name);
	printf("lfrq=%d amd=%d pmd=%d w=%d ne=%d nfrq=%d\n", v->lfrq, v->amd, v->pmd, v->w, v->ne_nfrq >> 7, v->ne_nfrq & 0x1f);
	printf("pan=%c%c fb=%d con=%d slot=0x%02x\n", v->rl_fb_con & 0x80 ? 'R' : '-', v->rl_fb_con & 0x40 ? 'L' : '-', v->rl_fb_con >> 3 & 0x07, v->rl_fb_con & 0x07, v->slot);
	printf("OP AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS WS\n");
	for(int i = 0; i < 4; i++) {
		struct opm_voice_operator *o = v->operators + i;
		printf(
			"%2d %2d %3d %3d %2d %2d %3d %2d %3d %3d %3d %3d %2d\n",
			i,
			o->ks_ar & 0x1f,
			o->ams_d1r & 0x1f,
			o->dt2_d2r & 0x1f,
			o->d1l_rr & 0x0f,
			o->d1l_rr >> 4,
			o->tl & 0x7f,
			o->ks_ar >> 6,
			o->dt1_mul & 0x0f,
			o->dt1_mul >> 4 & 0x07,
			o->dt2_d2r >> 6,
			o->ams_d1r >> 7,
			o->ws & 0x07
		);
	}
}

void opn_voice_dump(struct opn_voice *v) {
	printf("%.256s\n", v->name);
	printf("lfo=%d slot=%c%c%c%c fb=%d con=%d pan=%c%c ams=%d pms=%d\n", v->lfo, v->slot & 0x80 ? '4' : '-', v->slot & 0x40 ? '3' : '-', v->slot & 0x20 ? '2' : '-', v->slot & 0x10 ? '1' : '-', v->fb_con >> 3 & 0x07, v->fb_con & 0x07, v->lr_ams_pms & 0x80 ? 'L' : '-', v->lr_ams_pms & 0x40 ? 'R' : '-', v->lr_ams_pms >> 4 & 0x03, v->lr_ams_pms & 0x07);
	printf("OP DT MUL  TL KS AR AM DR SR SL RR SSG\n");
	for(int i = 0; i < 4; i++) {
		struct opn_voice_operator *op = &v->operators[i];
		printf(
			"%2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %3d\n",
			i,
			op->dt_mul >> 4 & 0x07,
			op->dt_mul & 0x0f,
			op->tl & 0x7f,
			op->ks_ar >> 6,
			op->ks_ar & 0x1f,
			op->am_dr >> 7,
			op->am_dr & 0x1f,
			op->sr & 0x1f,
			op->sl_rr >> 4,
			op->sl_rr & 0x0f,
			op->ssg_eg & 0x0f
		);
	}
}

int opl_voice_load_opm_voice(struct opl_voice *oplv, struct opm_voice *opmv) {
	return 0;
}

int opl_voice_load_opn_voice(struct opl_voice *oplv, struct opn_voice *opnv) {
	return 0;
}

int opm_voice_load_opl_voice(struct opm_voice *opmv, struct opl_voice *oplv) {
	return 0;
}

int opm_voice_load_opn_voice(struct opm_voice *opmv, struct opn_voice *opnv) {
	return 0;
}

int opn_voice_load_opl_voice(struct opn_voice *opnv, struct opl_voice *oplv) {
	return 0;
}

int opn_voice_load_opm_voice(struct opn_voice *opnv, struct opm_voice *opmv) {
	/* per chip registers */
	opnv->lfo = opmv->lfrq >> 4;

	/* per channel registers */
	opnv->fb_con = (opmv->rl_fb_con & 0x38) | (opmv->rl_fb_con & 0x07);
	opnv->lr_ams_pms = (opmv->rl_fb_con & 0xc0) | (opmv->pms_ams & 0x03) << 4 | (opmv->pms_ams & 0x70) >> 3;

	/* slot mask */
	opnv->slot = opmv->slot << 1;

	/* operators */
	for(int j = 0; j < 4; j++) {
		struct opn_voice_operator *nop = &opnv->operators[j];
		struct opm_voice_operator *mop = &opmv->operators[j];

		nop->dt_mul = mop->dt1_mul & 0x7f;
		nop->tl = mop->tl & 0x7f;
		nop->ks_ar = mop->ks_ar;
		nop->am_dr = mop->ams_d1r;
		nop->sr = mop->dt2_d2r & 0x1f;
		nop->sl_rr = mop->d1l_rr;
		nop->ssg_eg = 0;
	}
	return 0;
}

void fm_voice_bank_init(struct fm_voice_bank *bank) {
	memset(bank, 0, sizeof(*bank));
}

void fm_voice_bank_clear(struct fm_voice_bank *bank) {
	if(bank->opl_voices) free(bank->opl_voices);
	if(bank->opm_voices) free(bank->opm_voices);
	if(bank->opn_voices) free(bank->opn_voices);
	fm_voice_bank_init(bank);
}

int fm_voice_bank_append_opl_voice(struct fm_voice_bank *bank, struct opl_voice *voice) {
	bank->num_opl_voices++;
	struct opl_voice *newvoice = fm_voice_bank_reserve_opl_voices(bank, 1);
	if(!newvoice) return -1;
	memcpy(newvoice, voice, sizeof(*voice));
	return 0;
}

struct opl_voice *fm_voice_bank_reserve_opl_voices(struct fm_voice_bank *bank, int num_voices) {
	int old_num = bank->num_opl_voices;
	bank->num_opl_voices += num_voices;
	bank->opl_voices = realloc(bank->opl_voices, bank->num_opl_voices * sizeof(struct opl_voice));
	if(!bank->opl_voices) return 0;
	return bank->opl_voices + old_num;
}

int fm_voice_bank_append_opm_voice(struct fm_voice_bank *bank, struct opm_voice *voice) {
	bank->num_opm_voices++;
	struct opm_voice *newvoice = fm_voice_bank_reserve_opm_voices(bank, 1);
	if(!newvoice) return -1;
	memcpy(newvoice, voice, sizeof(*voice));
	return 0;
}

struct opm_voice *fm_voice_bank_reserve_opm_voices(struct fm_voice_bank *bank, int num_voices) {
	int old_num = bank->num_opm_voices;
	bank->num_opm_voices += num_voices;
	bank->opm_voices = realloc(bank->opm_voices, bank->num_opm_voices * sizeof(struct opm_voice));
	if(!bank->opm_voices) return 0;
	return bank->opm_voices + old_num;
}

int fm_voice_bank_append_opn_voice(struct fm_voice_bank *bank, struct opn_voice *voice) {
	bank->num_opn_voices++;
	struct opn_voice *newvoice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!newvoice) return -1;
	memcpy(newvoice, voice, sizeof(*voice));
	return 0;
}

struct opn_voice *fm_voice_bank_reserve_opn_voices(struct fm_voice_bank *bank, int num_voices) {
	int old_num = bank->num_opn_voices;
	bank->num_opn_voices += num_voices;
	bank->opn_voices = realloc(bank->opn_voices, bank->num_opn_voices * sizeof(struct opn_voice));
	if(!bank->opn_voices) return 0;
	return bank->opn_voices + old_num;
}

void fm_voice_bank_dump(struct fm_voice_bank *bank) {
	for(int i = 0; i < bank->num_opl_voices; i++) {
		printf("OPL %d: ", i);
		opl_voice_dump(&bank->opl_voices[i]);
	}
	for(int i = 0; i < bank->num_opm_voices; i++) {
		printf("OPM %d: ", i);
		opm_voice_dump(&bank->opm_voices[i]);
	}
	for(int i = 0; i < bank->num_opn_voices; i++) {
		printf("OPN %d: ", i);
		opn_voice_dump(&bank->opn_voices[i]);
	}
}

int opl_voice_from_op3_file_instrument(struct opl_voice *v, struct op3_file_instrument *inst) {
	v->en_4op = inst->en_4op;
	v->perc_inst = inst->percnotenum;
	v->ch_fb_cnt[0] = 0xf0 | inst->fb_con12;
	v->ch_fb_cnt[1] = 0xf0 | inst->fb_con34;
	v->dam_dvb_ryt_bd_sd_tom_tc_hh = 0;
	for(int i = 0; i < 4; i++) {
		struct opl_voice_operator *op = &v->operators[i];
		struct op3_file_instrument_op *op3 = &inst->op[i];
		op->am_vib_eg_ksr_mul = op3->ave_kvm;
		op->ksl_tl = op3->ksl_tl;
		op->ar_dr = op3->ar_dr;
		op->sl_rr = op3->sl_rr;
		op->ws = op3->ws;
	}
	return 0;
}

int fm_voice_bank_append_op3_file(struct fm_voice_bank *bank, struct op3_file *f) {
	struct opl_voice *voice = fm_voice_bank_reserve_opl_voices(bank, f->count_melodic + f->count_percussive);
	if(!voice) return -1;
	for(int i = 0; i < f->count_melodic; i++) {
		opl_voice_from_op3_file_instrument(voice, &f->melodic[i]);
		voice++;
	}
	for(int i = 0; i < f->count_percussive; i++) {
		opl_voice_from_op3_file_instrument(voice, &f->percussive[i]);
		voice++;
	}
	return 0;
}

int fm_voice_bank_append_opm_file(struct fm_voice_bank *bank, struct opm_file *f) {
	struct opm_voice *voice = fm_voice_bank_reserve_opm_voices(bank, f->num_voices);
	if(!voice) return -1;
	for(int i = 0; i < f->num_voices; i++) {
		struct opm_file_voice *v = &f->voices[i];
		memcpy(voice->name, v->name, 256);
		voice->lfrq = v->lfo_lfrq;
		voice->amd = v->lfo_amd & 0x7f;
		voice->pmd = v->lfo_pmd & 0x7f;
		voice->w = v->lfo_wf & 0x03;
		voice->ne_nfrq = v->ch_ne << 7 | (v->nfrq & 0x1f);
		voice->rl_fb_con = (v->ch_pan & 0xc0) | (v->ch_fl & 0x07) << 3 | (v->ch_con & 0x07);
		voice->pms_ams = (v->ch_ams & 0x07) << 4 | (v->ch_pms & 0x03);
		voice->slot = v->ch_slot >> 3;
		for(int j = 0; j < 4; j++) {
			struct opm_voice_operator *op = &voice->operators[j];
			struct opm_file_operator *fop = &v->operators[j];
			op->dt1_mul = fop->dt1 << 4 | fop->mul;
			op->tl = fop->tl;
			op->ks_ar = fop->ks << 6 | fop->ar;
			op->ams_d1r = fop->ame << 7 | fop->d1r;
			op->dt2_d2r = fop->dt2 << 6 | fop->d2r;
			op->d1l_rr = fop->d1l << 4 | fop->rr;
		}
		voice++;
	}
	return 0;
}

int fm_voice_bank_append_bnk_file(struct fm_voice_bank *bank, struct bnk_file *f) {
	struct opl_voice *voice = fm_voice_bank_reserve_opl_voices(bank, f->num_used);
	if(!voice) return -1;
	for(int i = 0, u = 0; i < f->num_instruments; i++) {
		struct bnk_file_name *n = &f->names[i];
		if((n->flags & 1) == 0) continue;
		struct bnk_file_instrument *inst = &f->instruments[n->index];
		memcpy(voice->name, n->name, 8);
		voice->en_4op = 0;
		voice->perc_inst = inst->percussive;
		voice->ch_fb_cnt[0] = (inst->operators[0].fb & 0x07) << 1 | (inst->operators[0].con & 1);
		voice->ch_fb_cnt[1] = 0;
		voice->dam_dvb_ryt_bd_sd_tom_tc_hh = 0;
		for(int j = 0; j < 2; j++) {
			struct opl_voice_operator *op = &voice->operators[j];
			struct bnk_file_operator *fop = &inst->operators[j];
			op->am_vib_eg_ksr_mul = (fop->am & 1) << 7 | (fop->vib & 1) << 6 | (fop->eg & 1) << 5 | (fop->ksr & 1) << 4 | (fop->mul & 0x0f);
			op->ksl_tl = (fop->ksl & 0x03) << 6 | (fop->tl & 0x3f);
			op->ar_dr = (fop->ar & 0x0f) << 4 | (fop->dr & 0x0f);
			op->sl_rr = (fop->sl & 0x0f) << 4 | (fop->rr & 0x0f);
			op->ws = fop->wave_sel & 0x03;
		}
		voice++;
		u++;
		if(u >= f->num_used) break;
	}

	return 0;
}

int fm_voice_bank_append_dmp_file(struct fm_voice_bank *bank, struct dmp_file *f) {
	return -1;
}

int fm_voice_bank_append_ins_file(struct fm_voice_bank *bank, struct ins_file *f) {
	struct opn_voice *voice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!voice) return -1;
	memcpy(voice->name, f->name, MIN(f->name_len, 256));
	voice->lfo = 0;
	voice->slot = 0x0f;
	voice->fb_con = f->fb_alg;
	voice->lr_ams_pms = 3 << 6;
	for(int i = 0; i < 4; i++) {
		struct opn_voice_operator *op = &voice->operators[i];
		struct ins_file_operator *fop = &f->operators[i];
		op->dt_mul = fop->mul_dt;
		op->tl = fop->tl;
		op->ks_ar = fop->rs_ar;
		op->am_dr = fop->dr;
		op->sr = fop->sr;
		op->sl_rr = fop->sl_rr;
		op->ssg_eg = 0;
	}
	return 0;
}

int fm_voice_bank_append_sbi_file(struct fm_voice_bank *bank, struct sbi_file *f) {
	struct opl_voice *voice = fm_voice_bank_reserve_opl_voices(bank, 1);
	if(!voice) return -1;
	memcpy(voice->name, f->name, 32);
	voice->en_4op = 0;
	voice->perc_inst = f->perc_voice;
	voice->ch_fb_cnt[0] = f->fb_con;
	voice->ch_fb_cnt[1] = 0;
	for(int i = 0; i < 2; i++) {
		struct opl_voice_operator *op = &voice->operators[i];
		op->am_vib_eg_ksr_mul = f->am_vib_eg_ksr_mul[i];
		op->ksl_tl = f->ksl_tl[i];
		op->ar_dr = f->ar_dr[i];
		op->sl_rr = f->sl_rr[i];
		op->ws = f->ws[i];
	}
	return 0;
}

int fm_voice_bank_append_tfi_file(struct fm_voice_bank *bank, struct tfi_file *f) {
	struct opn_voice *voice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!voice) return -1;
	voice->lfo = 0;
	voice->slot = 0x0f;
	voice->fb_con = (f->fb & 0x07) << 3 | (f->alg & 0x07);
	for(int i = 0; i < 4; i++) {
		struct opn_voice_operator *op = &voice->operators[i];
		struct tfi_file_operator *fop = &f->operators[i];
		op->dt_mul = (fop->dt & 0x07) << 3 | (fop->mul & 0x07);
		op->tl = fop->tl & 0x7f;
		op->ks_ar = fop->rs << 6 | (fop->ar & 0x1f);
		op->am_dr = fop->dr & 0x1f;
		op->sr = fop->sr;
		op->sl_rr = fop->sl << 4 | (fop->rr & 0x0f);
		op->ssg_eg = fop->ssg_eg;
	}
	return 0;
}

int fm_voice_bank_append_y12_file(struct fm_voice_bank *bank, struct y12_file *f) {
	struct opn_voice *voice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!voice) return -1;
	snprintf(voice->name, 256, "%.*s / %.*s / %.*s", MIN(f->name_len, 16), f->name, MIN(f->dumper_len, 16), f->dumper, MIN(f->game_len, 16), f->game);
	voice->fb_con = (f->fb & 0x07) << 3 | (f->alg & 0x07);
	for(int i = 0; i < 4; i++) {
		struct opn_voice_operator *op = &voice->operators[i];
		struct y12_file_operator *fop = &f->operators[i];
		op->dt_mul = fop->mul_dt;
		op->tl = fop->tl & 0x7f;
		op->ks_ar = fop->ar_rs;
		op->am_dr = fop->dr_am;
		op->sr = fop->sr;
		op->sl_rr = fop->rr_sl;
		op->ssg_eg = fop->ssg;
	}
	return 0;
}

/* DX21 and some other synths use a lookup table for MUL and DT values, indexed by the "FREQUENCY" parameter (0-63)
 *
 * This table can be found in their ROMS:
 * DX100 1.1 0x0b63
 * DX21  1.4 0x5184
 * DX21  1.5 0x51b8
 * DX21  1.6 0x51b1
 * TX81Z 1.0 0x34cb
 * TX81Z 1.1 0x34de
 * TX81Z 1.2 0x34e9
 * TX81Z 1.3 0x34e9
 * TX81Z 1.4 0x34e9
 * TX81Z 1.5 0x34e9
 * TX81Z 1.6 0x34ea
 *
 * Thanks to andlabs for pointing this out!
 */
static const uint8_t vced_mul_dt2_table[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0C, 0x0A, 0x0B, 0x10, 0x0D, 0x0E,
	0x14, 0x0F, 0x11, 0x18, 0x12, 0x13, 0x1C, 0x15,
	0x16, 0x20, 0x19, 0x17, 0x24, 0x1A, 0x1D, 0x28,
	0x1B, 0x1E, 0x2C, 0x21, 0x30, 0x1F, 0x22, 0x25,
	0x34, 0x23, 0x38, 0x29, 0x26, 0x3C, 0x2D, 0x27,
	0x2A, 0x31, 0x2E, 0x2B, 0x35, 0x32, 0x2F, 0x39,
	0x36, 0x33, 0x3D, 0x3A, 0x37, 0x3E, 0x3B, 0x3F,
};

int fm_voice_bank_append_dx21_vced(struct fm_voice_bank *bank, struct dx21_vced_voice_bank *f) {
	struct opm_voice *voice = fm_voice_bank_reserve_opm_voices(bank, f->num_voices);
	if(!voice) return -1;
	for(int i = 0; i < f->num_voices; i++) {
		struct dx21_vced_voice *v = &f->voices[i];
		memcpy(voice->name, v->name, 10);
		voice->rl_fb_con = 3 << 6 | (v->feedback & 0x07) << 3 | (v->algorithm & 0x07);
		voice->lfrq = 255 * v->lfo_speed / 99;
		voice->amd = v->lfo_amd * 127 / 99;
		voice->pmd = v->lfo_pmd * 127 / 99;
		voice->w = v->lfo_wave;
		voice->pms_ams = (v->pm_sens & 0x07) << 4 | (v->am_sens && 0x03) >> 1;
		voice->slot = 0x0f;
		for(int j = 0; j < 4; j++) {
			struct opm_voice_operator *op = &voice->operators[j];
			struct dx21_vced_voice_op *fop = &v->op[j];
			uint8_t mul_dt2 = vced_mul_dt2_table[fop->freq & 0x3f];
			op->dt1_mul = (fop->detune & 0x07) << 4 | mul_dt2 >> 2;
			op->tl = fop->ol * 127 / 99;
			op->ks_ar = fop->ksr << 6 | (fop->ar & 0x1f);
			op->ams_d1r = fop->ame << 7 | (fop->d1r & 0x1f);
			op->dt2_d2r = (mul_dt2 & 0x03) << 6 | (fop->d2r & 0x1f);
			op->d1l_rr = fop->d1l << 4 | (fop->rr & 0x0f);
			op->ws = fop->opw;
		}
		voice++;
	}
	return 0;
}

int fm_voice_bank_append_fb01_bulk(struct fm_voice_bank *bank, struct fb01_bulk_voice_bank *f) {
	struct opm_voice *voice = fm_voice_bank_reserve_opm_voices(bank, f->num_voices);
	if(!voice) return -1;
	for(int i = 0; i < f->num_voices; i++) {
		struct fb01_bulk_voice *v = &f->voices[i];
		memcpy(voice->name, v->name, 7);
		voice->lfrq = v->lfo_speed;
		voice->amd = v->am_depth;
		voice->pmd = v->pm_depth;
		voice->w = v->lfo_waveform;
		voice->ne_nfrq = 0;
		voice->rl_fb_con = 3 << 6 | v->fb_level << 3 | v->algorithm;
		voice->pms_ams = v->pm_sens << 4 | v->am_sens;
		voice->slot = 0x0f;
		for(int j = 0; j < 4; j++) {
			struct opm_voice_operator *op = &voice->operators[j];
			struct fb01_bulk_voice_op *fop = &v->op[j];
			op->dt1_mul = fop->detune << 4 | fop->freq;
			op->tl = fop->tl & 0x7f;
			op->ks_ar = fop->ks_rate_depth << 6 | fop->ar;
			op->ams_d1r = fop->d1r & 0x1f;
			op->dt2_d2r = (fop->inharmonic_freq & 0x03) << 6 | (fop->d2r & 0x1f);
			op->d1l_rr = fop->sl << 4 | fop->rr;
			op->ws = 0;
		}
		voice++;
	}
	return 0;
}

int fm_voice_bank_load(struct fm_voice_bank *bank, uint8_t *data, size_t data_len) {
#define TRY_FMT(fmt) struct fmt##_file fmt; fmt##_file_init(&fmt); if(fmt##_file_load(&fmt, data, data_len) == 0) return fm_voice_bank_append_##fmt##_file(bank, &fmt);

	TRY_FMT(op3)
	TRY_FMT(opm)
	TRY_FMT(bnk)

	struct dmp_file dmp;
		if(dmp_file_load(&dmp, data, data_len, 1) == 0)
			return fm_voice_bank_append_dmp_file(bank, &dmp);

	TRY_FMT(ins)
	TRY_FMT(sbi)
	TRY_FMT(tfi)
	TRY_FMT(y12)

	struct dx21_vced_voice_bank dx21_vced_voice_bank;
	dx21_vced_voice_bank_init(&dx21_vced_voice_bank);
	if(dx21_vced_voice_bank_from_buffer(&dx21_vced_voice_bank, data, data_len) == 0)
		return fm_voice_bank_append_dx21_vced(bank, &dx21_vced_voice_bank);

	struct fb01_bulk_voice_bank fb01_bulk_voice_bank;
	fb01_bulk_voice_bank_init(&fb01_bulk_voice_bank);
	if(fb01_bulk_voice_bank_from_buffer(&fb01_bulk_voice_bank, data, data_len) == 0)
		return fm_voice_bank_append_fb01_bulk(bank, &fb01_bulk_voice_bank);

	return -1;
}
