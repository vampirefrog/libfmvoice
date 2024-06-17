#ifdef HAVE_STDIO
#include <stdio.h>
#endif
#include <string.h>
#include <stdlib.h>

#include "bnk_file.h"
#include "opl_voice.h"

void bnk_file_init(struct bnk_file *f) {
	memset(f, 0, sizeof(*f));
}

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

#ifdef HAVE_STDIO
void bnk_file_dump(struct bnk_file *f) {
	printf("version=%d.%d num_used=%d num_instruments=%d\n", f->ver_major, f->ver_minor, f->num_used, f->num_instruments);

	for(int i = 0; i < f->num_instruments; i++) {
		printf("Name %d: index=%d flags=0x%02x name=\"%.8s\"\n", i, f->names[i].index, f->names[i].flags, f->names[i].name);
		if((f->names[i].flags & 1) == 0) continue;

		if(f->names[i].index >= f->num_instruments) continue;
		struct bnk_file_instrument *inst = f->instruments + f->names[i].index;
		printf("Inst. %d: percussive=%d voice_num=%d con=%d fb=%d\n", i, inst->percussive, inst->voice_num, inst->operators[0].con, inst->operators[0].fb);
		printf("OP KSL MUL AR SL EG DR RR TL AM VIB KSR WAVE\n");
		for(int j = 0; j < 2; j++) {
			struct bnk_file_operator *op = inst->operators + j;
			printf("%2d  %2d  %2d %2d %2d %2d %2d %2d %2d %2d  %2d  %2d    %d\n", j, op->ksl, op->mul, op->ar, op->sl, op->eg, op->dr, op->rr, op->tl, op->am, op->vib, op->ksr, op->wave_sel);
			if(inst->percussive && inst->voice_num != 6) break;
		}
	}
}
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"

static int load(void *data, int data_len, struct fm_voice_bank *bank) {
	struct bnk_file f;
	int r = bnk_file_load(&f, data, data_len);
	if(r) return r;
	struct opl_voice *voices = fm_voice_bank_reserve_opl_voices(bank, f.num_used);
	if(!voices) return -1;
	struct opl_voice *voice = voices;
	for(int i = 0, u = 0; i < f.num_instruments; i++) {
		struct bnk_file_name *n = &f.names[i];
		if((n->flags & 1) == 0) continue;
		struct bnk_file_instrument *inst = &f.instruments[n->index];
		memcpy(voice->name, n->name, 8);
		voice->en_4op = 0;
		voice->perc_inst = inst->percussive;
		voice->ch_fb_cnt[0] = (inst->operators[0].fb & 0x07) << 1 | (inst->operators[0].con & 1);
		voice->ch_fb_cnt[1] = 0;
		voice->dam_dvb_ryt_bd_sd_tom_tc_hh = 0;
		for(int j = 0; j < 2; j++) {
			struct opl_voice_operator *op = &voice->operators[j];
			struct bnk_file_operator *fop = &inst->operators[j];
			op->am_vib_eg_ksr_mul = (fop->am & 1) << 7 | (fop->vib & 1) << 6 | (fop->eg & 1) << 5 | (fop->ksr & 1) << 4 | (fop->mul & 0x0f);
			op->ksl_tl = (fop->ksl & 0x03) << 6 | (fop->tl & 0x3f);
			op->ar_dr = (fop->ar & 0x0f) << 4 | (fop->dr & 0x0f);
			op->sl_rr = (fop->sl & 0x0f) << 4 | (fop->rr & 0x0f);
			op->ws = fop->wave_sel & 0x03;
		}
		voice++;
		u++;
		if(u >= f.num_used) break;
	}

	return 0;
}

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	struct bnk_file bnk;
	bnk_file_init(&bnk);

	bnk.num_instruments = bnk.num_used = bank->num_opl_voices;

	int new_num_instruments = bank->num_opl_voices;
	if(new_num_instruments >= 65536)
		new_num_instruments = 65536;
	int processed = new_num_instruments - bank->num_opl_voices;
	bnk.instruments = realloc(bnk.instruments, bnk.num_instruments * sizeof(struct bnk_file_instrument));
	bnk.names = realloc(bnk.names, bnk.num_instruments * sizeof(struct bnk_file_name));
	for(int i = bnk.num_instruments, j = pos->opl; i < processed; i++, j++) {
		bnk.num_instruments++;
		bnk.num_used++;
		strncpy(bnk.names[i].name, bank->opl_voices[j].name, sizeof(bnk.names[i].name) - 1);
		bnk.names[i].index = i; // TODO: not sure what this index field is really
		bnk.names[i].flags = 0;
		bnk.instruments[i].percussive = 0; // TODO: add a percussive flag to opl_voice?
		bnk.instruments[i].voice_num = 0; // TODO: what is this?
		for(int o = 0; o < 2; o++) {
			bnk.instruments[i].operators[o].ksl = opl_voice_operator_get_ksl(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].mul = opl_voice_operator_get_mul(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].fb = opl_voice_get_fb(bank->opl_voices + j);
			bnk.instruments[i].operators[o].ar = opl_voice_operator_get_ar(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].sl = opl_voice_operator_get_sl(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].eg = opl_voice_operator_get_eg_typ(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].dr = opl_voice_operator_get_dr(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].rr = opl_voice_operator_get_rr(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].tl = opl_voice_operator_get_tl(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].am = opl_voice_operator_get_am(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].vib = opl_voice_operator_get_vib(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].ksr = opl_voice_operator_get_ksr(bank->opl_voices + j, o);
			bnk.instruments[i].operators[o].con = opl_voice_get_cnt(bank->opl_voices + j);
			bnk.instruments[i].operators[o].wave_sel = opl_voice_operator_get_ws(bank->opl_voices + j, o);
		}
		pos->opl++;
	}

	return 0;
}

struct loader bnk_file_loader = {
	.load = load,
	.save = save,
	.name = "BNK",
	.description = "AdLib Instrument Bank Format",
	.file_ext = "bnk",
	.max_opl_voices = 65536,
	.max_opm_voices = 0,
	.max_opn_voices = 0,
};
#endif
