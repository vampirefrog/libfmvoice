#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "dmp_file.h"
#include "tools.h"

void dmp_file_init(struct dmp_file *f) {
	memset(f, 0, sizeof(*f));
}

int dmp_file_load(struct dmp_file *f, uint8_t *data, size_t data_len) {
	memset(f, 0, sizeof(struct dmp_file));

	if(data[0] < 9) {
		fprintf(stderr, "Unsupported version 0x%02x (%d)\n", data[0], data[0]);
		return -1;
	}

	if(data_len < 51) {
		fprintf(stderr, "File too short %lu < 51\n", (unsigned long)data_len);
		return -1;
	}

	f->system = DMP_SYSTEM_YM2612_OPN;
	f->version = data[0];
	if (data[0] <= 10) {
		f->mode = data[1];
	} else {
		f->system = data[1];
		f->mode = data[2];
	}

	if (f->system != DMP_SYSTEM_YM2612_OPN && f->system != DMP_SYSTEM_YM2151_OPM) {
		fprintf(stderr, "Unsupported system %d\n", f->system);
		return -1;
	}

	if(f->mode == 1) {
		uint8_t *p = data + 2;
		if (f->version < 10) {
			f->num_operators = *p++ == 0 ? 2 : 4;
		} else {
			f->num_operators = 4;
		}
		f->lfo = *p++;
		f->fb = *p++;
		f->alg = *p++;
		if(f->version >= 10) {
			f->lfo2 = *p++;
		} else {
			f->lfo2 = 0;
		}

		for(int i = 0; i < f->num_operators; i++) {
			f->operators[i].mult = *p++;
			f->operators[i].tl   = *p++;
			f->operators[i].ar   = *p++;
			f->operators[i].dr   = *p++;
			f->operators[i].sl   = *p++;
			f->operators[i].rr   = *p++;
			f->operators[i].am   = *p++;
			if(f->version >= 10) {
				f->operators[i].ksr = *p++;
				f->operators[i].dt  = *p++;
				f->operators[i].d2r = *p++;
				f->operators[i].ssg = *p++;
			}
		}
	} else {
		fprintf(stderr, "Unsupported mode %d\n", f->mode);
		return -1;
	}

	return 0;
}

int dmp_file_save(struct dmp_file *f, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	uint8_t buf[50] = { 0 };
	uint8_t* p = buf;
	*p++ = 0x0a; // version 10
	*p++ = 1; // instrument type, 1 = FM
	*p++ = f->lfo;
	*p++ = f->fb;
	*p++ = f->alg;
	*p++ = f->lfo2;
	for(int i = 0; i < f->num_operators; i++) {
		*p++ = f->operators[i].mult;
		*p++ = f->operators[i].tl;
		*p++ = f->operators[i].ar;
		*p++ = f->operators[i].dr;
		*p++ = f->operators[i].sl;
		*p++ = f->operators[i].rr;
		*p++ = f->operators[i].am;
		*p++ = f->operators[i].ksr;
		*p++ = f->operators[i].dt;
		*p++ = f->operators[i].d2r;
		*p++ = f->operators[i].ssg;
	}
	return write_fn(buf, 50, data_ptr);
}

#ifdef HAVE_STDIO
void dmp_file_dump(struct dmp_file *dmp) {
	printf("version=%d mode=%d\n", dmp->version, dmp->mode);
	if(dmp->mode != 1) return;
	// operators, lfo, fb, alg, lfo2
	printf("num_operators=%d lfo+%d fb=%d alg=%d lfo2=%d\n", dmp->num_operators, dmp->lfo, dmp->fb, dmp->alg, dmp->lfo2);
	// operators
	printf("OP MUL AR DR SL RR AM KSR DT D2R SSG\n");
	for(int i = 0; i < 4; i++) {
		printf(
			"%d %d %d %d %d %d %d %d %d %d %d\n",
			dmp->operators[i].mult,
			dmp->operators[i].tl,
			dmp->operators[i].ar,
			dmp->operators[i].dr,
			dmp->operators[i].sl,
			dmp->operators[i].rr,
			dmp->operators[i].am,
			dmp->operators[i].ksr,
			dmp->operators[i].dt,
			dmp->operators[i].d2r,
			dmp->operators[i].ssg
		);
	}
}
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"

static int load(void *data, int data_len, struct fm_voice_bank  *bank) {
	struct dmp_file f;
	int r = dmp_file_load(&f, (uint8_t*)data, data_len);
	if(r) return r;
	struct opn_voice *voice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!voice) return -1;
	voice->slot = 0x0f;
	voice->fb_con = (f.fb & 0x07) << 3 | (f.alg & 0x07);
	voice->lr_ams_pms = (f.lfo & 0x07) << 4 | (f.lfo2 & 0x03);
	for(int i = 0; i < f.num_operators; i++) {
		struct opn_voice_operator *op = &voice->operators[i];
		struct dmp_file_operator *fop = &f.operators[i];
		op->dt_mul = (fop->dt & 0x07) << 3 | (fop->mult & 0x07);
		op->tl = fop->tl & 0x7f;
		op->ks_ar = fop->ksr << 6 | (fop->ar & 0x1f);
		op->am_dr = fop->dr & 0x1f;
		op->sr = fop->d2r;
		op->sl_rr = fop->sl << 4 | (fop->rr & 0x0f);
		op->ssg_eg = fop->ssg;
	}
	return 0;
}

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	if(bank->num_opn_voices <= pos->opn) return -1;
	struct dmp_file f;
	dmp_file_init(&f);
	struct opn_voice* voice = &bank->opn_voices[pos->opn];
	if (!voice) return -1;

	f.fb = opn_voice_get_fb(voice);
	f.alg = opn_voice_get_con(voice);
	f.lfo = opn_voice_get_ams(voice);
	f.lfo2 = opn_voice_get_pms(voice);
	for(int i = 0; i < 4; i++) {
		struct dmp_file_operator *fop = &f.operators[i];
		fop->dt = opn_voice_get_operator_dt(voice, i);
		fop->mult = opn_voice_get_operator_mul(voice, i);
		fop->tl = opn_voice_get_operator_tl(voice, i);
		fop->ksr = opn_voice_get_operator_ks(voice, i);
		fop->ar = opn_voice_get_operator_ar(voice, i);
		fop->dr = opn_voice_get_operator_dr(voice, i);
		fop->am = opn_voice_get_operator_am(voice, i);
		fop->d2r = opn_voice_get_operator_sr(voice, i);
		fop->sl = opn_voice_get_operator_sl(voice, i);
		fop->rr = opn_voice_get_operator_rr(voice, i);
		fop->ssg = opn_voice_get_operator_ssg_eg(voice, i);
	}
	pos->opn++;
	return dmp_file_save(&f, write_fn, data_ptr);
}

struct loader dmp_file_loader = {
	.load = load,
	.save = save,
	.name = "DMP",
	.description = "DefleMask Preset Format",
	.file_ext = "dmp",
	.max_opl_voices = 0,
	.max_opm_voices = 0,
	.max_opn_voices = 1,
};
#endif
