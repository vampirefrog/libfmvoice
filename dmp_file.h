#ifndef DMP_FILE_H_
#define DMP_FILE_H_

#include <stdint.h>

#define DMP_FILE_GENESIS 1

struct dmp_file_operator {
	uint8_t
		mult, tl,
		ar, dr, sl, rr, am,
		ksr, dt, d2r, ssg;
};

struct dmp_file {
	uint8_t version, mode;
	uint8_t num_operators, lfo, fb, alg;
	uint8_t lfo2;
	struct dmp_file_operator operators[4];
};

void dmp_file_init(struct dmp_file *f);
int dmp_file_load(struct dmp_file *f, uint8_t *data, size_t data_len, int system);
int dmp_file_save(struct dmp_file *f, int (*write_fn)(void *buf, size_t len, void *data_ptr), void *data_ptr);
void dmp_file_dump(struct dmp_file *f);

#endif /* DMP_FILE_H_ */
