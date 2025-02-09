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

int sbi_file_save(struct sbi_file *f, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	uint8_t buf[52] = { 0 };
	int bc = 0;
	buf[bc++] = 'S';
	buf[bc++] = 'B';
	buf[bc++] = 'I';
	buf[bc++] = 0x1a;
	memcpy(&buf[bc], f->name, 32);
	bc += 32;
	buf[bc++] = f->am_vib_eg_ksr_mul[0];
	buf[bc++] = f->am_vib_eg_ksr_mul[1];
	buf[bc++] = f->ksl_tl[0];
	buf[bc++] = f->ksl_tl[1];
	buf[bc++] = f->ar_dr[0];
	buf[bc++] = f->ar_dr[1];
	buf[bc++] = f->sl_rr[0];
	buf[bc++] = f->sl_rr[1];
	buf[bc++] = f->ws[0];
	buf[bc++] = f->ws[1];
	buf[bc++] = f->fb_con;
	buf[bc++] = f->perc_voice;
	buf[bc++] = f->transpose;
	buf[bc++] = f->perc_pitch;
	return write_fn(buf, 52, data_ptr);
}

#ifdef HAVE_STDIO
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
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"

static int load(void *data, int data_len, struct fm_voice_bank  *bank) {
	struct sbi_file f;
	int r = sbi_file_load(&f, data, data_len);
	if(r) return r;
	struct opl_voice *voice = fm_voice_bank_reserve_opl_voices(bank, 1);
	if(!voice) return -1;
	memcpy(voice->name, f.name, 32);
	voice->en_4op = 0;
	voice->perc_inst = f.perc_voice;
	voice->ch_fb_cnt[0] = f.fb_con;
	voice->ch_fb_cnt[1] = 0;
	for(int i = 0; i < 2; i++) {
		struct opl_voice_operator *op = &voice->operators[i];
		op->am_vib_eg_ksr_mul = f.am_vib_eg_ksr_mul[i];
		op->ksl_tl = f.ksl_tl[i];
		op->ar_dr = f.ar_dr[i];
		op->sl_rr = f.sl_rr[i];
		op->ws = f.ws[i];
	}
	return 0;
}

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	if(bank->num_opl_voices <= pos->opl) return -1;
	struct sbi_file f;
	sbi_file_init(&f);
	struct opl_voice *voice = &bank->opl_voices[pos->opl];
	memcpy(f.name, voice->name, 32);
	f.perc_voice = voice->perc_inst;
	f.fb_con = voice->ch_fb_cnt[0];
	for(int i = 0; i < 2; i++) {
		struct opl_voice_operator *op = &voice->operators[i];
		f.am_vib_eg_ksr_mul[i] = op->am_vib_eg_ksr_mul;
		f.ksl_tl[i] = op->ksl_tl;
		f.ar_dr[i] = op->ar_dr;
		f.sl_rr[i] = op->sl_rr;
		f.ws[i] = op->ws;
	}
	pos->opl++;
	return sbi_file_save(&f, write_fn, data_ptr);
}

struct loader sbi_file_loader = {
	.load = load,
	.save = save,
	.name = "SBI",
	.description = "Sound Blaster Instrument",
	.file_ext = "sbi",
	.max_opl_voices = 1,
	.max_opm_voices = 0,
	.max_opn_voices = 0,
};
#endif
