#include <stdio.h>
#include <string.h>
#include "ins_file.h"

void ins_file_init(struct ins_file *f) {
	memset(f, 0, sizeof(*f));
}

int ins_file_load(struct ins_file *f, uint8_t *data, size_t data_len) {
	if(data_len < 6 || strncmp((char *)data, "MVSI", 4)) return -1;

	f->version = data[4];

	f->name = 0;
	uint8_t *p = data + 5;
	int i;
	f->name_len = f->data_offset = 0;
	for(i = 5; i < data_len; i++) {
		if(*p++ > 0) continue;
		f->name = strdup((char *)data + 5);
		f->data_offset = i + 1;
		f->name_len = i - 5;
		break;
	}
	if(!f->name) return -1;
	if(data_len < i + 25) return -1;

	for(int i = 0; i < 4; i++) {
		f->operators[i].mul_dt = p[0x00 + i];
		f->operators[i].tl     = p[0x04 + i];
		f->operators[i].rs_ar  = p[0x08 + i];
		f->operators[i].dr     = p[0x0c + i];
		f->operators[i].sr     = p[0x10 + i];
		f->operators[i].sl_rr  = p[0x14 + i];
	}
	f->fb_alg = p[0x18];

	return 0;
}

int ins_file_save(struct ins_file *f, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	int r;
	r = write_fn("MVSI", 4, data_ptr);
	if(r != 4) return -1;
	uint8_t buf[6*4];
	buf[0] = 49; // version
	r = write_fn(buf, 1, data_ptr);
	if(r != 1) return -2;
	r = write_fn(f->name, strlen(f->name) + 1, data_ptr);
	if(r != strlen(f->name) + 1) return -3;
	uint8_t *p = buf;
	for(int i = 0; i < 4; i++) *(p++) = f->operators[i].mul_dt;
	for(int i = 0; i < 4; i++) *(p++) = f->operators[i].tl;
	for(int i = 0; i < 4; i++) *(p++) = f->operators[i].rs_ar;
	for(int i = 0; i < 4; i++) *(p++) = f->operators[i].dr;
	for(int i = 0; i < 4; i++) *(p++) = f->operators[i].sr;
	for(int i = 0; i < 4; i++) *(p++) = f->operators[i].sl_rr;
	r = write_fn(buf, 6*4, data_ptr);
	if(r != 6*4) return -1;
	buf[0] = f->fb_alg;
	r = write_fn(buf, 1, data_ptr);
	if(r != 1) return -2;
	return 0;
}

#ifdef HAVE_STDIO
void ins_file_dump(struct ins_file *f) {
	fwrite(f->name, 1, f->name_len, stdout);
	printf("\nname_len=%d version=%d data_offset=%d\n", f->name_len, f->version, f->data_offset);
	printf("fb=%d alg=%d\n", f->fb_alg >> 3 & 0x07, f->fb_alg & 0x07);
	printf("OP MUL DT TL RS AR DR SR SL RR\n");
	for(int i = 0; i < 4; i++) {
		struct ins_file_operator *op = f->operators + i;
		printf(
			"%2d"
			"  %2d"
			"  %d"
			" %2d"
			" %2d"
			" %2d"
			" %2d"
			" %2d"
			" %2d"
			" %2d"
			"\n",
			i,
			op->mul_dt & 0x0f,
			op->mul_dt >> 4 & 0x07,
			op->tl & 0x7f,
			op->rs_ar >> 6,
			op->rs_ar & 0x1f,
			op->dr,
			op->sr,
			op->sl_rr >> 4,
			op->sl_rr & 0x0f
		);
	}
}
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"
#include "tools.h"

static int load(void *data, int data_len, struct fm_voice_bank *bank) {
	struct ins_file f;
	int r = ins_file_load(&f, data, data_len);
	if(r) return r;
	struct opn_voice *voice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!voice) return -1;
	memcpy(voice->name, f.name, MIN(f.name_len, 256));
	voice->lfo = 0;
	voice->slot = 0x0f;
	voice->fb_con = f.fb_alg;
	voice->lr_ams_pms = 3 << 6;
	for(int i = 0; i < 4; i++) {
		struct opn_voice_operator *op = &voice->operators[i];
		struct ins_file_operator *fop = &f.operators[i];
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

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	if(bank->num_opn_voices <= pos->opn) return -1;
	struct ins_file ins;
	ins.name = strdup(bank->opn_voices[pos->opn].name);
	if(!ins.name) return -1;
	ins.name_len = strlen(bank->opn_voices[pos->opn].name);
	ins.fb_alg = bank->opn_voices[pos->opn].fb_con;
	for(int i = 0; i < 4; i++) {
		ins.operators[i].mul_dt = bank->opn_voices[pos->opn].operators[i].dt_mul;
		ins.operators[i].tl = opn_voice_get_operator_tl(bank->opn_voices + pos->opn, i);
		ins.operators[i].rs_ar = bank->opn_voices[pos->opn].operators[i].ks_ar;
		ins.operators[i].dr = bank->opn_voices[pos->opn].operators[i].am_dr;
		ins.operators[i].sr = opn_voice_get_operator_sr(bank->opn_voices + pos->opn, i);
		ins.operators[i].sl_rr = bank->opn_voices[pos->opn].operators[i].sl_rr;
	}
	pos->opn++;
	return ins_file_save(&ins, write_fn, data_ptr);
}

struct loader ins_file_loader = {
	.load = load,
	.save = save,
	.name = "INS",
	.description = "AdLib Instrument Format",
	.file_ext = "ins",
	.max_opl_voices = 1,
	.max_opm_voices = 0,
	.max_opn_voices = 0,
};
#endif
