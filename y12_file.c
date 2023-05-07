#include <string.h>
#include <stdio.h>

#include "y12_file.h"
#include "tools.h"

int y12_file_load(struct y12_file *f, uint8_t *data, size_t data_len) {
	if(data_len < 0x80) return -1;

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
