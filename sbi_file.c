#include <string.h>
#include <stdio.h>

#include "sbi_file.h"
#include "tools.h"

void sbi_file_init(struct sbi_file *f) {
	memset(f, 0, sizeof(*f));
}

int sbi_file_load(struct sbi_file *f, uint8_t *data, size_t data_len) {
	if(data_len < 47 || data_len > 52) return -1;

	if(data[0] != 'S' || data[1] != 'B' || data[2] != 'I' || (data[3] != 0x1a && data[3] != 0x1d)) return -1;

	memcpy(f->name, &data[4], 32);

	uint8_t *b = data + 36;
	f->am_vib_eg_ksr_mul[0] = *b++;
	f->am_vib_eg_ksr_mul[1] = *b++;
	f->ksl_tl[0] = *b++;
	f->ksl_tl[1] = *b++;
	f->ar_dr[0] = *b++;
	f->ar_dr[1] = *b++;
	f->sl_rr[0] = *b++;
	f->sl_rr[1] = *b++;
	f->ws[0] = *b++;
	f->ws[1] = *b++;
	f->fb_con = *b++;
	f->perc_voice = *b++;
	f->transpose = *b++;
	f->perc_pitch = *b++;
	return 0;
}

void sbi_file_dump(struct sbi_file *f) {
	printf("name=%.32s\n", f->name);
	printf("fb=%d con=%d perc_voice=%d transpose=%d perc_pitch=%d\n", f->fb_con >> 1 & 0x07, f->fb_con & 0x01, f->perc_voice, f->transpose, f->perc_pitch);
	printf(" OP AM VIB EG KSR MUL KSL TL AR DR SL RR WS\n");
	for(int i = 0; i < 2; i++) {
		printf("%s %d    %d  %d   %d  %2d   %d %2d %2d %2d %2d %2d  %d\n",
			i ? "CAR" : "MOD",
			f->am_vib_eg_ksr_mul[i] >> 7,
			f->am_vib_eg_ksr_mul[i] >> 6 & 1,
			f->am_vib_eg_ksr_mul[i] >> 5 & 1,
			f->am_vib_eg_ksr_mul[i] >> 4 & 1,
			f->am_vib_eg_ksr_mul[i] & 0x0f,
			f->ksl_tl[i] >> 6,
			f->ksl_tl[i] & 0x3f,
			f->ar_dr[i] >> 4,
			f->ar_dr[i] & 0x0f,
			f->sl_rr[i] >> 4,
			f->sl_rr[i] & 0x0f,
			f->ws[i]
		);
	}
}
