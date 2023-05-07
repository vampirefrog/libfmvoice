#ifndef OPM_FILE_H_
#define OPM_FILE_H_

#include <stdint.h>
#include <stdlib.h>

#define OPM_FILE_MAX_NAME 256

struct opm_file_operator {
	uint8_t ar, d1r, d2r, rr, d1l, tl, ks, mul, dt1, dt2, ame;
};

struct opm_file_voice {
	int number;
	char name[OPM_FILE_MAX_NAME];
	uint8_t lfo_lfrq, lfo_amd, lfo_pmd, lfo_wf, nfrq;
	uint8_t ch_pan, ch_fl, ch_con, ch_ams, ch_pms, ch_slot, ch_ne;
	struct opm_file_operator operators[4];
};

struct opm_file {
	struct opm_file_voice *voices;
	int num_voices;
};

void opm_file_init(struct opm_file *f);
int opm_file_load(struct opm_file *f, uint8_t *data, size_t data_len);
int opm_file_save(struct opm_file *f, size_t (*write_fn)(void *buf, size_t bufsize, void *data_ptr), int pad_to, void *data_ptr);
void opm_file_dump(struct opm_file *f);

#endif /* OPM_FILE_H_ */
