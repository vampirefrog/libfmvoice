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
