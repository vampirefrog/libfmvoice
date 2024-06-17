#include "opl_voice.h"

#include <string.h>
#include <math.h>
#ifdef HAVE_STDIO
#include <stdio.h>
#endif

#include "opm_voice.h"
#include "opn_voice.h"

void opl_voice_init(struct opl_voice *v) {
	memset(v, 0, sizeof(*v));
}

#ifdef HAVE_STDIO
void opl_voice_dump(struct opl_voice *v) {
	printf("name=%.256s\n", v->name);
	printf("4OP=%d percussion=%d\n", v->en_4op, v->perc_inst);
	for(int i = 0; i < 2; i++) {
		printf(
			"%d: ch=%c%c%c%c fb=%d c=%d  ",
			i,
			v->ch_fb_cnt[i] & 0x80 ? 'D' : '-',
			v->ch_fb_cnt[i] & 0x40 ? 'C' : '-',
			v->ch_fb_cnt[i] & 0x20 ? 'B' : '-',
			v->ch_fb_cnt[i] & 0x10 ? 'A' : '-',
			v->ch_fb_cnt[i] >> 1 & 0x07,
			v->ch_fb_cnt[i] & 0x01
		);
	}
	printf("\n");
	printf("DAM=%d DVB=%d RYT=%d BD=%d SD=%d TOM=%d TC=%d HH=%d\n",
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 7,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 6 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 5 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 4 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 3 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 2 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh >> 1 & 1,
		v->dam_dvb_ryt_bd_sd_tom_tc_hh & 1
	);
	printf("OP: AR DR SL RR TL MUL EG AM VIB KSR KSL WS\n");
	for(int i = 0; i < 4; i++) {
		if(!v->en_4op && i >= 2) break;
		struct opl_voice_operator *op = &v->operators[i];
		printf(
			"%2d: %2d %2d %2d %2d %2d %3d %2d %2d %3d %3d %3d %2d\n",
			i,
			op->ar_dr >> 4,
			op->ar_dr & 0x0f,
			op->sl_rr >> 4,
			op->sl_rr & 0x0f,
			op->ksl_tl & 0x3f,
			op->am_vib_eg_ksr_mul & 0x0f,
			op->am_vib_eg_ksr_mul >> 5 & 1,
			op->am_vib_eg_ksr_mul >> 7,
			op->am_vib_eg_ksr_mul >> 6 & 1,
			op->am_vib_eg_ksr_mul >> 4 & 1,
			op->ksl_tl >> 6,
			op->ws & 0x07
		);
	}
}
#endif

void opl_voice_normalize(struct opl_voice *v) {

}

int opl_voice_compare(struct opl_voice *v1, struct opl_voice *v2) {
	return -1;
}

int opl_operator_is_silent(struct opl_voice_operator *op) {
	return (op->ar_dr >> 4) < 1 || op->ksl_tl > 60; // approximation
}

int opl_voice_is_silent(struct opl_voice *v) {
	return 0;
}

int opl_voice_load_opm_voice(struct opl_voice *oplv, struct opm_voice *opmv) {
	return 0;
}

int opl_voice_load_opn_voice(struct opl_voice *oplv, struct opn_voice *opnv) {
	return 0;
}

int opl_pitch_to_block_fnum(float pitch, int clock) {
	uint8_t octave = (69 + 12 * log2(pitch / 440.0)) / 12 - 1;
	uint16_t fnum = (144 * pitch * (1 << 18) / clock) / (1 << (octave - 1));
	return octave << 11 | (fnum & 0x7ff);
}

float opl_block_fnum_to_pitch(uint8_t block_fnum2, uint8_t fnum1, int clock) {
	int block = block_fnum2 >> 3;
	int fnum = (block_fnum2 & 0x07) << 8 | fnum1;
	return (float)fnum * (float)clock * powf(2.0, block - 19.0) / 144.0;
}

