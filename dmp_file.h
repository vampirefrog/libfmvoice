#ifndef DMP_FILE_H_
#define DMP_FILE_H_

#include <stdint.h>

#define DMP_SYSTEM_YM2612_OPN		0x02
#define DMP_SYSTEM_YM2151_OPM		0x08

struct dmp_file_operator {
	uint8_t
		mult, tl,
		ar, dr, sl, rr, am,
		ksr, dt, d2r, ssg;
};

struct dmp_file {
	uint8_t version, mode, system;
	uint8_t num_operators, lfo, fb, alg;
	uint8_t lfo2;
	struct dmp_file_operator operators[4];
};

void dmp_file_init(struct dmp_file *f);
int dmp_file_load(struct dmp_file *f, uint8_t *data, size_t data_len);
int dmp_file_save(struct dmp_file *f, int (*write_fn)(void *buf, size_t len, void *data_ptr), void *data_ptr);
void dmp_file_dump(struct dmp_file *f);

#endif /* DMP_FILE_H_ */
