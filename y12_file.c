#include <string.h>
#include <stdio.h>

#include "y12_file.h"

void y12_file_init(struct y12_file *f) {
	memset(f, 0, sizeof(*f));
}

int y12_file_load(struct y12_file *f, uint8_t *data, size_t data_len) {
	if(data_len != 0x80) return -1;

	uint8_t *p = data;
	for(int i = 0; i < 4; i++) {
		f->operators[i].mul_dt = *p++;
		f->operators[i].tl = *p++;
		f->operators[i].ar_rs = *p++;
		f->operators[i].dr_am = *p++;
		f->operators[i].sr = *p++;
		f->operators[i].rr_sl = *p++;
		f->operators[i].ssg = *p++;
		p += 9;
	}
	f->alg = *p++;
	f->fb = *p++;
	p += 14;
	memcpy(f->name, p, 16);
	f->name_len = strnlen(f->name, 16);
	p += 16;
	memcpy(f->dumper, p, 16);
	f->dumper_len = strnlen(f->dumper, 16);
	p += 16;
	memcpy(f->game, p, 16);
	f->game_len = strnlen(f->game, 16);

	return 0;
}

int y12_file_save(struct y12_file *f, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	uint8_t buf[128] = { 0 };
	uint8_t* p = buf;
	for(int i = 0; i < 4; i++) {
		*p++ = f->operators[i].mul_dt;
		*p++ = f->operators[i].tl;
		*p++ = f->operators[i].ar_rs;
		*p++ = f->operators[i].dr_am;
		*p++ = f->operators[i].sr;
		*p++ = f->operators[i].rr_sl;
		*p++ = f->operators[i].ssg;
		p += 9;
	}
	*p++ = f->alg;
	*p++ = f->fb;
	p += 14;
	memcpy(p, f->name, 16);
	p += 16;
	memcpy(p, f->dumper, 16);
	p += 16;
	memcpy(p, f->game, 16);
	bc += 16;
	return write_fn(buf, 128, data_ptr);
}

#ifdef HAVE_STDIO
static char safechar(char c) { return c >= 0x20 && c < 0x7f ? c : '.'; }

void y12_file_dump(struct y12_file *f) {
	printf("alg=%d fb=%d\n", f->alg, f->fb);
	printf("name=");
	for(int i = 0; i < f->name_len && i < 16; i++) {
		putchar(safechar(f->name[i]));
	}
	printf("\n");
	printf("dumper=");
	for(int i = 0; i < f->dumper_len && i < 16; i++) {
		putchar(safechar(f->dumper[i]));
	}
	printf("\n");
	printf("game=");
	for(int i = 0; i < f->game_len && i < 16; i++) {
		putchar(safechar(f->game[i]));
	}
	printf("\n");
	printf("OP MUL DT TL AR RS DR AM SR RR SL SSG\n");
	for(int i = 0; i < 4; i++) {
		struct y12_file_operator *op = f->operators + i;
		printf(
			"%d  %d %d %d %d %d %d %d %d %d %d %d\n",
			i,
			op->mul_dt & 0x0f,
			op->mul_dt >> 4 & 0x07,
			op->tl & 0x7f,
			op->ar_rs >> 6,
			op->ar_rs & 0x1f,
			op->dr_am & 0x1f,
			op->dr_am >> 7,
			op->sr,
			op->rr_sl >> 4,
			op->rr_sl & 0x0f,
			op->ssg
		);
	}
}
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"
#include "tools.h"

static int load(void *data, int data_len, struct fm_voice_bank  *bank) {
	struct y12_file f;
	int r = y12_file_load(&f, data, data_len);
	if(r) return r;
	struct opn_voice *voice = fm_voice_bank_reserve_opn_voices(bank, 1);
	if(!voice) return -1;
	snprintf(voice->name, sizeof(voice->name), "%.*s", MIN(f.name_len, 16), f.name);
	snprintf(voice->dumper, sizeof(voice->dumper), "%.*s", MIN(f.dumper_len, 16), f.dumper);
	snprintf(voice->game, sizeof(voice->game), "%.*s", MIN(f.game_len, 16), f.game);
	voice->fb_con = (f.fb & 0x07) << 3 | (f.alg & 0x07);
	for(int i = 0; i < 4; i++) {
		struct opn_voice_operator *op = &voice->operators[i];
		struct y12_file_operator *fop = &f.operators[i];
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

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	if(bank->num_opn_voices <= pos->opn) return -1;
	struct y12_file f;
	y12_file_init(&f);
	struct opn_voice* voice = &bank->opn_voices[pos->opn];
	if (!voice) return -1;
	memcpy(f.name, voice->name, 15);
	memcpy(f.dumper, voice->dumper, 15);
	memcpy(f.game, voice->game, 15);
	f.name_len = strlen(f.name);
	f.dumper_len = strlen(f.dumper);
	f.game_len = strlen(f.game);
	f.fb = opn_voice_get_fb(voice);
	f.alg = opn_voice_get_con(voice);
	for(int i = 0; i < 4; i++) {
		f.operators[i].mul_dt = voice->operators[i].dt_mul;
		f.operators[i].tl = opn_voice_get_operator_tl(bank->opn_voices + pos->opn, i);
		f.operators[i].ar_rs = voice->operators[i].ks_ar;
		f.operators[i].dr_am = voice->operators[i].am_dr;
		f.operators[i].sr = opn_voice_get_operator_sr(bank->opn_voices + pos->opn, i);
		f.operators[i].rr_sl = voice->operators[i].sl_rr;
	}
	pos->opn++;
	return y12_file_save(&f, write_fn, data_ptr);
}

struct loader y12_file_loader = {
	.load = load,
	.save = save,
	.name = "Y12",
	.description = "Y12",
	.file_ext = "y12",
	.max_opl_voices = 0,
	.max_opm_voices = 0,
	.max_opn_voices = 1,
};
#endif
