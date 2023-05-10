#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bnk_file.h"

int bnk_file_load(struct bnk_file *f, uint8_t *data, size_t data_len) {
	f->ver_major = data[0];
	f->ver_minor = data[1];

	if(data[2] != 'A' || data[3] != 'D' || data[4] != 'L' || data[5] != 'I' || data[6] != 'B' || data[7] != '-')
		return -1;

	f->num_used = data[8] | data[9] << 8;
	f->num_instruments = data[10] | data[11] << 8;
	uint32_t name_offset = data[12] | data[13] << 8 | data[14] << 16 | data[15] << 24;
	if(name_offset >= data_len) return -1;
	uint32_t data_offset = data[16] | data[17] << 8 | data[18] << 16 | data[19] << 24;
	if(data_offset >= data_len) return -1;
	if(28 + f->num_instruments * 12 + f->num_instruments * 30 > data_len) return -1;

	f->names = malloc(f->num_instruments * sizeof(struct bnk_file_name));
	uint8_t *name_bytes = &data[name_offset];
	for(int i = 0; i < f->num_instruments; i++) {
		f->names[i].index = name_bytes[0] | name_bytes[1] << 8;
		f->names[i].flags = name_bytes[2];
		memcpy(f->names[i].name, name_bytes + 3, 9);
		name_bytes += 12;
	}
	f->instruments = malloc(f->num_instruments * sizeof(struct bnk_file_instrument));
	uint8_t *inst_bytes = &data[data_offset];
	for(int i = 0; i < f->num_instruments; i++) {
		struct bnk_file_instrument *inst = f->instruments + i;
		inst->percussive = *inst_bytes++;
		inst->voice_num = *inst_bytes++;
		for(int j = 0; j < 2; j++) {
			inst->operators[j].ksl = *inst_bytes++;
			inst->operators[j].mul = *inst_bytes++;
			inst->operators[j].fb = *inst_bytes++;
			inst->operators[j].ar = *inst_bytes++;
			inst->operators[j].sl = *inst_bytes++;
			inst->operators[j].eg = *inst_bytes++;
			inst->operators[j].dr = *inst_bytes++;
			inst->operators[j].rr = *inst_bytes++;
			inst->operators[j].tl = *inst_bytes++;
			inst->operators[j].am = *inst_bytes++;
			inst->operators[j].vib = *inst_bytes++;
			inst->operators[j].ksr = *inst_bytes++;
			inst->operators[j].con = *inst_bytes++;
		}
		inst->operators[0].wave_sel = *inst_bytes++;
		inst->operators[1].wave_sel = *inst_bytes++;
	}

	return 0;
}

void bnk_file_dump(struct bnk_file *f) {
	printf("version=%d.%d num_used=%d num_instruments=%d\n", f->ver_major, f->ver_minor, f->num_used, f->num_instruments);

	for(int i = 0; i < f->num_instruments; i++) {
		printf("Name %d: index=%d flags=0x%02x name=\"%.8s\"\n", i, f->names[i].index, f->names[i].flags, f->names[i].name);
		if((f->names[i].flags & 1) == 0) continue;

		struct bnk_file_instrument *inst = f->instruments + i;
		printf("Inst. %d: percussive=%d voice_num=%d con=%d fb=%d\n", i, inst->percussive, inst->voice_num, inst->operators[0].con, inst->operators[0].fb);
		printf("OP KSL MUL AR SL EG DR RR TL AM VIB KSR WAVE\n");
		for(int j = 0; j < 2; j++) {
			struct bnk_file_operator *op = inst->operators + j;
			printf("%2d  %2d  %2d %2d %2d %2d %2d %2d %2d %2d  %2d  %2d    %d\n", j, op->ksl, op->mul, op->ar, op->sl, op->eg, op->dr, op->rr, op->tl, op->am, op->vib, op->ksr, op->wave_sel);
			if(inst->percussive && inst->voice_num != 6) break;
		}
	}
}
