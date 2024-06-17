#ifdef HAVE_STDIO
#include <stdio.h>
#endif
#include <string.h>
#include <math.h>

#include "opn_voice.h"
#include "md5.h"

void opn_voice_init(struct opn_voice *v) {
	memset(v, 0, sizeof(*v));
}

#ifdef HAVE_STDIO
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
#endif

void opn_voice_normalize(struct opn_voice *v) {
	const uint8_t slot_masks[8] = { 0x08,0x08,0x08,0x08,0x0c,0x0e,0x0e,0x0f };

	// maximize volume
	uint8_t slot_mask = slot_masks[v->fb_con & 0x07];
	uint8_t min_tl = 127;

	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m) {
			if((v->operators[i].ks_ar & 0x1f) < 1 && v->operators[i].tl > 100)
				v->operators[i].tl = 127;
			if(v->operators[i].tl < min_tl)
				min_tl = v->operators[i].tl;
		}
	}

	for(int i = 0, m = 1; i < 4; i++, m <<= 1) {
		if(slot_mask & m) {
			v->operators[i].tl -= min_tl;
		}
	}
}

int opn_voice_compare(struct opn_voice *v1, struct opn_voice *v2) {
	if(v1->fb_con != v2->fb_con) return 1;
	for(int j = 0, m = 1; j < 4; j++, m <<= 1) {
		struct opn_voice_operator *o1 = &v1->operators[j];
		struct opn_voice_operator *o2 = &v2->operators[j];
		if((o1->dt_mul & 0x7f) != (o2->dt_mul & 0x7f)) return 1;
		if((o1->ks_ar & 0xdf) != (o2->ks_ar & 0xdf)) return 1;
		if((o1->am_dr & 0x9f) != (o2->am_dr & 0x9f)) return 1;
		if((o1->sr & 0x1f) != (o2->sr & 0x1f)) return 1;
		if((o1->tl & 0x7f) != (o2->tl & 0x7f)) return 1;
		if((o1->sl_rr) != (o2->sl_rr)) return 1;
	}
	return 0;
}

int opn_operator_is_silent(struct opn_voice_operator *op) {
	return (op->ks_ar & 0x1f) < 1 || op->tl > 110; // approximation
}

int opn_voice_is_silent(struct opn_voice *v) {
	if(v->slot == 0) return 1;
	if(
		opn_operator_is_silent(&v->operators[0]) &&
		opn_operator_is_silent(&v->operators[1]) &&
		opn_operator_is_silent(&v->operators[2]) &&
		opn_operator_is_silent(&v->operators[3])
	) return 1;
	return 0;
}

void opn_voice_compute_md5_sum(struct opn_voice *v, uint8_t *digest) {
	struct md5_ctx ctx;
	md5_init_ctx(&ctx);
	uint8_t data[1 + 3 + 4 * 7] = {
		v->lfo & 0x0f,
		v->slot & 0x0f,
		v->fb_con & 0x3f,
		v->lr_ams_pms & 0xf7,
	};
	uint8_t *b = data + 1 + 3;
	struct opn_voice_operator *op = v->operators;
	for(int i = 0; i < 4; i++) {
		*(b++) = op->dt_mul & 0x7f;
		*(b++) = op->tl & 0x7f;
		*(b++) = op->ks_ar & 0xdf;
		*(b++) = op->am_dr & 0x9f;
		*(b++) = op->sr & 0x0f;
		*(b++) = op->sl_rr;
		*(b++) = op->ssg_eg & 0x0f;
		op++;
	}
	md5_process_bytes(data, sizeof(data), &ctx);
	md5_finish_ctx(&ctx, digest);
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

int opn_pitch_to_block_fnum(float pitch, int clock) {
	int octave = (69 + 12 * log2(pitch / 440.0)) / 12 - 1;
	int fnum = (144 * pitch * (1 << 19) / clock) / (1 << (octave - 1));
	return octave << 11 | (fnum & 0x7ff);
}

float opn_block_fnum_to_pitch(uint8_t block_fnum2, uint8_t fnum1, int clock) {
	int block = block_fnum2 >> 3;
	int fnum = (block_fnum2 & 0x07) << 8 | fnum1;
	return (float)fnum * (float)clock * powf(2.0, block - 20.0) / 144.0;
}

int opnx_pitch_to_block_fnum(float pitch, int clock) {
	int octave = (69 + 12 * log2(pitch / 440.0)) / 12;
	int fnum = (144 * pitch * (1 << 20) / clock) / (1 << (octave - 1));
	return octave << 11 | (fnum & 0x7ff);
}

float opnx_block_fnum_to_pitch(uint8_t block_fnum2, uint8_t fnum1, int clock) {
	int block = block_fnum2 >> 3;
	int fnum = (block_fnum2 & 0x07) << 8 | fnum1;
	return (float)fnum * (float)clock * powf(2.0, block - 21.0) / 144.0;
}
