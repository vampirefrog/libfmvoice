#ifdef HAVE_STDIO
#include <stdio.h>
#endif
#include <string.h>
#include <math.h>

#include "opm_voice.h"
#include "md5.h"

void opm_voice_init(struct opm_voice *v) {
	memset(v, 0, sizeof(*v));
}

#ifdef HAVE_STDIO
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
#endif

void opm_voice_normalize(struct opm_voice *v) {
	const uint8_t slot_masks[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };

	/* Fix volume */
	uint8_t slot_mask = slot_masks[v->rl_fb_con & 0x07];

	uint8_t min_tl = 0x7f;
	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m) {
			if((v->operators[i].ks_ar & 0x1f) < 1 && v->operators[i].tl > 100)
				v->operators[i].tl = 127;
			if(v->operators[i].tl < min_tl)
				min_tl = v->operators[i].tl;
		}
	}

	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m)
			v->operators[i].tl -= min_tl;
	}
}

int opm_voice_compare(struct opm_voice *v1, struct opm_voice *v2) {
	if((v1->rl_fb_con & 0x3f) != (v2->rl_fb_con & 0x3f)) return 1;
	for(int j = 0, m = 1; j < 4; j++, m <<= 1) {
		struct opm_voice_operator *o1 = &v1->operators[j];
		struct opm_voice_operator *o2 = &v2->operators[j];
		if((o1->dt1_mul & 0x7f) != (o2->dt1_mul & 0x7f)) return 1;
		if((o1->ks_ar & 0xdf) != (o2->ks_ar & 0xdf)) return 1;
		if((o1->ams_d1r & 0x9f) != (o2->ams_d1r & 0x9f)) return 1;
		if((o1->dt2_d2r & 0xdf) != (o2->dt2_d2r & 0xdf)) return 1;
		if((o1->tl & 0x7f) != (o2->tl & 0x7f)) return 1;
		if(o1->d1l_rr != o2->d1l_rr) return 1;
	}
	return 0;
}

int opm_operator_is_silent(struct opm_voice_operator *op) {
	return (op->ks_ar & 0x1f) < 1 || op->tl > 110; // approximation
}

int opm_voice_is_silent(struct opm_voice *v) {
	if(v->slot == 0) return 1;
	if(
		opm_operator_is_silent(&v->operators[0]) &&
		opm_operator_is_silent(&v->operators[1]) &&
		opm_operator_is_silent(&v->operators[2]) &&
		opm_operator_is_silent(&v->operators[3])
	) return 1;
	return 0;
}

void opm_voice_compute_md5_sum(struct opm_voice *v, uint8_t *digest) {
	struct md5_ctx ctx;
	md5_init_ctx(&ctx);
	uint8_t data[5 + 3 + 4 * 7] = {
		v->lfrq,
		v->amd & 0x7f,
		v->pmd & 0x7f,
		v->w & 0x03,
		v->ne_nfrq & 0x9f,
		v->rl_fb_con,
		v->pms_ams & 0x73,
		v->slot & 0x0f,
	};
	uint8_t *b = data + 5 + 3;
	struct opm_voice_operator *op = v->operators;
	for(int i = 0; i < 4; i++) {
		*(b++) = op->dt1_mul & 0x7f;
		*(b++) = op->tl & 0x7f;
		*(b++) = op->ks_ar & 0xdf;
		*(b++) = op->ams_d1r & 0x9f;
		*(b++) = op->dt2_d2r & 0xdf;
		*(b++) = op->d1l_rr;
		*(b++) = op->ws & 0x07;
		op++;
	}
	md5_process_bytes(data, sizeof(data), &ctx);
	md5_finish_ctx(&ctx, digest);
}

int opm_voice_load_opl_voice(struct opm_voice *opmv, struct opl_voice *oplv) {
	return 0;
}

int opm_voice_load_opn_voice(struct opm_voice *opmv, struct opn_voice *opnv) {
	/* per chip registers */
	opmv->lfrq = opnv->lfo << 4;
	opmv->amd = opmv->pmd = opmv->w = opmv->ne_nfrq = 0;

	/* per channel registers */
	opmv->rl_fb_con = opnv->fb_con & 0x3f;
	opmv->pms_ams = ((opnv->lr_ams_pms & 0x07) << 4) | ((opnv->lr_ams_pms >> 4) & 0x03);

	// /* slot mask */
	opmv->slot = opnv->slot;

	/* operators */
	for(int j = 0; j < 4; j++) {
		struct opn_voice_operator *nop = &opnv->operators[j];
		struct opm_voice_operator *mop = &opmv->operators[j];

		mop->dt1_mul = nop->dt_mul & 0x7f;
		mop->tl = nop->tl & 0x7f;
		mop->ks_ar = nop->ks_ar & 0xdf;
		mop->ams_d1r = nop->am_dr & 0x9f;
		mop->dt2_d2r = nop->sr & 0x1f;
		mop->d1l_rr = nop->sl_rr;
	}
	return 0;
}

int opm_pitch_to_kc_kf(float pitch, int clock) {
	const uint8_t opm_notes[12] = { 0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14 };
	float f = 3584 + 64 * 12 * log2(pitch * 3579545.0 / clock / 440.0);
	if(f < 0) f = 0;
	if(f > 64 * 8 * 12 - 1) f = 64 * 8 * 12 - 1;
	int octave = (int)f / 64 / 12;
	int kc = octave << 4 | opm_notes[((int)f / 64) % 12];
	int kf = (int)f % 64 << 2;
	return kc << 8 | kf;
}

float opm_kc_kf_to_pitch(uint8_t kc, uint8_t kf, int clock) {
	const uint8_t notes[16] = { 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 11, };
	int octave = kc >> 4;
	int note = notes[kc & 0x0f];
	int k = (octave * 12 + note) << 6 | (kf >> 2);
	return powf(2.0, (k - 3584) / 768.0) * (float)clock * 440.0 / 3579545.0;
}

