#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "dmp_file.h"
#include "tools.h"

int dmp_file_load(struct dmp_file *f, uint8_t *data, size_t data_len, int system) {
	memset(f, 0, sizeof(struct dmp_file));

	if(data[0] != 9) {
		fprintf(stderr, "Unsupported version 0x%02x (%d)\n", data[0], data[0]);
		return -1;
	}

	if(data_len < 51) {
		fprintf(stderr, "File too short %lu < 51\n", (unsigned long)data_len);
		return -1;
	}

	f->version = data[0];
	f->mode = data[1];

	if(f->mode == 1) {
		uint8_t *p = data + 2;
		f->num_operators = *p++ == 0 ? 2 : 4;
		f->lfo = *p++;
		f->fb = *p++;
		f->alg = *p++;
		if(system == DMP_FILE_GENESIS) {
			f->lfo2 = *p++;
		}

		for(int i = 0; i < f->num_operators; i++) {
			f->operators[i].mult = *p++;
			f->operators[i].tl   = *p++;
			f->operators[i].ar   = *p++;
			f->operators[i].dr   = *p++;
			f->operators[i].sl   = *p++;
			f->operators[i].rr   = *p++;
			f->operators[i].am   = *p++;
			if(system == DMP_FILE_GENESIS) {
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
	return -1;
}

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	return -1;
}

struct loader dmp_file_loader = {
	.load = load,
	.save = save,
	.name = "DMP",
	.description = "DefleMask Preset Format",
	.file_ext = "dmp",
	.max_opl_voices = 1,
	.max_opm_voices = 0,
	.max_opn_voices = 0,
};
#endif
