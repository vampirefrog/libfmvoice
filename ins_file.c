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
		if(*p++ == 0) {
			f->name = (char *)data + 5;
			f->data_offset = i + 1;
			f->name_len = i - 5;
			break;
		}
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
			"  %d"
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
