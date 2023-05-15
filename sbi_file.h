#ifndef SBI_FILE_H_
#define SBI_FILE_H_

#include <stdint.h>
#include <stdlib.h>

// https://moddingwiki.shikadi.net/wiki/SBI_Format
struct sbi_file {
	uint8_t name[32];
	uint8_t am_vib_eg_ksr_mul[2];
	uint8_t ksl_tl[2];
	uint8_t ar_dr[2];
	uint8_t sl_rr[2];
	uint8_t ws[2];
	uint8_t fb_con;
	uint8_t perc_voice;
	int8_t transpose;
	uint8_t perc_pitch;
};

void sbi_file_init(struct sbi_file *f);
int sbi_file_load(struct sbi_file *sbi, uint8_t *data, size_t data_len);
int sbi_file_save(struct sbi_file *f, size_t (*write_fn)(void *buf, size_t bufsize, void *data_ptr), void *data_ptr);
void sbi_file_dump(struct sbi_file *f);

#endif /* SBI_FILE_H_ */
