#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "op3_file.h"

void op3_file_init(struct op3_file *f) {
	memset(f, 0, sizeof(*f));
}

int op3_file_load(struct op3_file *f, uint8_t *data, size_t data_len) {
	if(data_len < 32) return -1;

	if(strncmp("Junglevision Patch File\x1A\0\0\0\0\0\0\0\0", (char *)data, 32)) return -1;

	f->count_melodic = data[32] | data[33] << 8;
	f->count_percussive = data[34] | data[35] << 8;
	f->start_melodic = data[36] | data[37] << 8;
	f->start_percussive = data[38] | data[39] << 8;
	uint8_t *ptr = data + 40;
#define READ_OP(o) \
	inst->op[o].ave_kvm = *ptr++; \
	inst->op[o].ksl_tl = *ptr++; \
	inst->op[o].ar_dr = *ptr++; \
	inst->op[o].sl_rr = *ptr++; \
	inst->op[o].ws = *ptr++;

#define READ_SET(type) \
	for(int i = 0; i < f->count_##type; i++) { \
		struct op3_file_instrument *inst = &f->type[i]; \
		inst->en_4op = *ptr++; \
		inst->percnotenum = *ptr++; \
		READ_OP(0) \
		inst->fb_con12 = *ptr++; \
		READ_OP(1) \
		READ_OP(2) \
		inst->fb_con34 = *ptr++; \
		READ_OP(3) \
	}

	READ_SET(melodic)
	READ_SET(percussive)

	return 0;
}

int op3_file_save(struct op3_file *f, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	uint8_t buf[8192] = { 0 };
	strcpy((char*)&buf[0], "Junglevision Patch File\x1A");
	uint8_t* p = buf;
	*p++ = f->count_melodic & 0xff;
	*p++ = f->count_melodic >> 8;
	*p++ = f->count_percussive & 0xff;
	*p++ = f->count_percussive >> 8;
	*p++ = f->start_melodic & 0xff;
	*p++ = f->start_melodic >> 8;
	*p++ = f->start_percussive & 0xff;
	*p++ = f->start_percussive >> 8;

#define WRITE_OP(o) \
	*p++ = inst->op[o].ave_kvm; \
	*p++ = inst->op[o].ksl_tl; \
	*p++ = inst->op[o].ar_dr; \
	*p++ = inst->op[o].sl_rr; \
	*p++ = inst->op[o].ws;

#define WRITE_SET(type) \
	for(int i = 0; i < f->count_##type; i++) { \
		struct op3_file_instrument *inst = &f->type[i]; \
		*p++ = inst->en_4op; \
		*p++ = inst->percnotenum; \
		WRITE_OP(0) \
		*p++ = inst->fb_con12; \
		WRITE_OP(1) \
		WRITE_OP(2) \
		*p++ = inst->fb_con34; \
		WRITE_OP(3) \
	}

	WRITE_SET(melodic)
	WRITE_SET(percussive)
	return 0;
}
#ifdef HAVE_STDIO
static void op3_dump_instrument(struct op3_file_instrument *inst) {
	printf("en_4op=%d percnotenum=%d\n", inst->en_4op, inst->percnotenum);
	printf("OP AM VIB EG KSR MUL KSL TL AR DR SL RR WS\n");
	for(int j = 0; j < 4; j++) {
		if(j >= 2 && !inst->en_4op) break;
		struct op3_file_instrument_op *op = inst->op + j;
		printf(
			"%2d  %d   %d   %d  %d  %2d  %2d %2d %2d %2d %2d %2d  %d\n",
			j,
			op->ave_kvm >> 7,
			op->ave_kvm >> 6 & 1,
			op->ave_kvm >> 5 & 1,
			op->ave_kvm >> 4 & 1,
			op->ave_kvm & 0x0f,
			op->ksl_tl >> 6,
			op->ksl_tl & 0x3f,
			op->ar_dr >> 4,
			op->ar_dr & 0x0f,
			op->sl_rr >> 4,
			op->sl_rr & 0x0f,
			op->ws & 0x07
		);
	}
	if(inst->en_4op)
		printf("fb12=%d con12=%d fb34=%d con34=%d\n", inst->fb_con12 >> 1 & 0x07, inst->fb_con12 & 0x01, inst->fb_con34 >> 1 & 0x07, inst->fb_con34 & 0x01);
	else
		printf("fb12=%d con12=%d\n", inst->fb_con12 >> 1 & 0x07, inst->fb_con12 & 0x01);
}

void op3_file_dump(struct op3_file *f) {
	printf("count_melodic=%d count_percussive=%d start_melodic=%d start_percussive=%d\n", f->count_melodic, f->count_percussive, f->start_melodic, f->start_percussive);
	for(int i = 0; i < f->count_melodic; i++) {
		printf("melodic %d: ", i);
		op3_dump_instrument(&f->melodic[i]);
	}
	for(int i = 0; i < f->count_percussive; i++) {
		printf("percussive %d: ", i);
		op3_dump_instrument(&f->percussive[i]);
	}
}
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"

static int opl_voice_from_op3_file_instrument(struct opl_voice *v, struct op3_file_instrument *inst) {
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

static int op3_file_instrument_from_opl_voice(struct op3_file_instrument *inst, struct opl_voice *v) {
	inst->en_4op = v->en_4op;
	inst->percnotenum = v->perc_inst;
	inst->fb_con12 = v->ch_fb_cnt[0] & ~0xf0;
	inst->fb_con34 = v->ch_fb_cnt[1] & ~0xf0;
	for(int i = 0; i < 4; i++) {
		struct opl_voice_operator *op = &v->operators[i];
		struct op3_file_instrument_op *op3 = &inst->op[i];
		op3->ave_kvm = op->am_vib_eg_ksr_mul;
		op3->ksl_tl = op->ksl_tl;
		op3->ar_dr = op->ar_dr;
		op3->sl_rr = op->sl_rr;
		op3->ws = op->ws;
	}
	return 0;
}

static int load(void *data, int data_len, struct fm_voice_bank *bank) {
	struct op3_file f;
	int r = op3_file_load(&f, data, data_len);
	if(r) return r;
	struct opl_voice *voice = fm_voice_bank_reserve_opl_voices(bank, f.count_melodic + f.count_percussive);
	if(!voice) return -1;
	for(int i = 0; i < f.count_melodic; i++) {
		opl_voice_from_op3_file_instrument(voice, &f.melodic[i]);
		voice++;
	}
	for(int i = 0; i < f.count_percussive; i++) {
		opl_voice_from_op3_file_instrument(voice, &f.percussive[i]);
		voice++;
	}
	return 0;
}

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	if(bank->num_opl_voices <= pos->opl) return -1;
	struct op3_file f;
	op3_file_init(&f);
	struct opl_voice *voice = &bank->opl_voices[pos->opl];
	f.count_melodic = bank->num_opl_voices;
	f.count_percussive = 0;
	f.start_melodic = 0;
	f.start_percussive = 0;
	for(int i = 0; i < f.count_melodic; i++) {
		op3_file_instrument_from_opl_voice(&f.melodic[i], voice);
		voice++;
	}
	pos->opl++;
	return op3_file_save(&f, write_fn, data_ptr);
}

struct loader op3_file_loader = {
	.load = load,
	.save = save,
	.name = "OP3",
	.description = "OP3",
	.file_ext = "op3",
	.max_opl_voices = 1,
	.max_opm_voices = 0,
	.max_opn_voices = 0,
};
#endif
