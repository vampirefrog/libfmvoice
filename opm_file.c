#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "opm_file.h"

void opm_file_init(struct opm_file *f) {
	memset(f, 0, sizeof(*f));
}

int opm_file_push_voice(struct opm_file *opm, struct opm_file_voice *v) {
	opm->num_voices++;
	opm->voices = realloc(opm->voices, opm->num_voices * sizeof(opm->voices[0]));
	if(!opm->voices) return -1;
	memcpy(&opm->voices[opm->num_voices - 1], v, sizeof(*v));
	return 0;
}

int opm_file_load(struct opm_file *opm, uint8_t *data, size_t data_len) {
	if(data_len < 30) return -1;

#define ALL_STATES \
	STATE(None) \
	STATE(Slash) \
	STATE(Comment) \
	STATE(InAt) \
	STATE(AfterAt) \
	STATE(InVoiceNum) \
	STATE(AfterVoiceNum) \
	STATE(InVoiceName) \
	STATE(InParamName) \
	STATE(AfterParamName) \
	STATE(InParamVal) \
	STATE(AfterParamVal)

	enum {
#define STATE(x) x,
ALL_STATES
#undef STATE
	} state = None;

#define CHAR_DOT(x) ((x) < 0x20 || (x) >= 0x7f ? '.' : (x))

#define MAX_PARAM_NAME 4
#define MAX_DIGITS 4
#define MAX_PARAMS 11
#define MAX_VOICE_NAME 256
#define APPEND_DIGIT { if(digitNum >= MAX_DIGITS) { fprintf(stderr, "Too many digits %d '%c' (%d) at line %d\n", digitNum, CHAR_DOT(c), c, line); return -1; } else { digits[digitNum++] = c; } }
#define APPEND_PARAM { if(paramNum >= MAX_PARAMS) { fprintf(stderr, "Too many parameters at line %d\n", line); return -1; } else { digits[digitNum] = 0; params[paramNum++] = atoi(digits); digitNum = 0; } }
#define APPEND_VOICE_NAME_CHAR { if(voiceNameLen >= MAX_VOICE_NAME) { fprintf(stderr, "Too many voice chars '%c' (%d) at line %d\n", CHAR_DOT(c), c, line); return -1; } else { voiceName[voiceNameLen++] = c; } }
#define PARSE_PARAM { \
	if(!strcmp(paramName, "LFO")) { \
		if(paramNum != 5) { \
			fprintf(stderr, "LFO parameters %d != 5\n", paramNum); \
			return -1; \
		} \
		opm->voices[opm->num_voices-1].lfo_lfrq = params[0]; \
		opm->voices[opm->num_voices-1].lfo_amd = params[1]; \
		opm->voices[opm->num_voices-1].lfo_pmd = params[2]; \
		opm->voices[opm->num_voices-1].lfo_wf = params[3]; \
		opm->voices[opm->num_voices-1].nfrq = params[4]; \
	} else if(!strcmp(paramName, "CH")) { \
		if(paramNum != 7) { \
			fprintf(stderr, "CH parameters %d != 7\n", paramNum); \
			return -1; \
		} \
		opm->voices[opm->num_voices-1].ch_pan = params[0]; \
		opm->voices[opm->num_voices-1].ch_fl = params[1]; \
		opm->voices[opm->num_voices-1].ch_con = params[2]; \
		opm->voices[opm->num_voices-1].ch_ams = params[3]; \
		opm->voices[opm->num_voices-1].ch_pms = params[4]; \
		opm->voices[opm->num_voices-1].ch_slot = params[5]; \
		opm->voices[opm->num_voices-1].ch_ne = params[6]; \
	} else if((paramName[0] == 'M' || paramName[0] == 'C') && (paramName[1] == '1' || paramName[1] == '2')) { \
		if(paramNum != 11) { \
			fprintf(stderr, "Operator %c%c parameters %d != 11\n", paramName[0], paramName[1], paramNum); \
			return -1; \
		} \
		int opNum = paramName[0] == 'M' ? (paramName[1] == '1' ? 0 : 1) : (paramName[1] == '1' ? 2 : 3); \
		opm->voices[opm->num_voices-1].operators[opNum].ar = params[0]; \
		opm->voices[opm->num_voices-1].operators[opNum].d1r = params[1]; \
		opm->voices[opm->num_voices-1].operators[opNum].d2r = params[2]; \
		opm->voices[opm->num_voices-1].operators[opNum].rr = params[3]; \
		opm->voices[opm->num_voices-1].operators[opNum].d1l = params[4]; \
		opm->voices[opm->num_voices-1].operators[opNum].tl = params[5]; \
		opm->voices[opm->num_voices-1].operators[opNum].ks = params[6]; \
		opm->voices[opm->num_voices-1].operators[opNum].mul = params[7]; \
		opm->voices[opm->num_voices-1].operators[opNum].dt1 = params[8]; \
		opm->voices[opm->num_voices-1].operators[opNum].dt2 = params[9]; \
		opm->voices[opm->num_voices-1].operators[opNum].ame = params[10]; \
	} \
}

	int line = 1;
	char paramName[MAX_PARAM_NAME + 1];
	int paramNameLen = 0;
	char voiceName[MAX_VOICE_NAME];
	int voiceNameLen = 0;
	char digits[MAX_DIGITS + 1];
	int digitNum = 0;
	int params[MAX_PARAMS + 1];
	int paramNum = 0;
	for(int i = 0; i < data_len; i++) {
		uint8_t c = data[i];
		//printf("line=%d c=%02x (%c) state=%d %s\n", line, c, c < 0x20 ? '.' : c, state, state_names[state]);

		if(c == '\r') continue;

		if(c == '\n') {
			line++;
		} else if(c == '/') {
			if(state != Comment)
				state = Slash;
		}

		switch(state) {
			case None:
				if(isspace(c)) {
					// ignore
				} else switch(c) {
					case '@':
						state = InAt;
						break;
					case 'L':
					case 'C':
					case 'M':
						state = InParamName;
						paramNameLen = 1;
						paramName[0] = c;
						break;
					default:
						fprintf(stderr, "Unexpected '%c' (%d) at start of line %d\n", CHAR_DOT(c), c, line);
						return -1;
				}
				break;
			case Slash:
				if(c == '/')
					state = Comment;
				else {
					fprintf(stderr, "Unexpected '%c' (%d) after '/' at line %d\n", CHAR_DOT(c), c, line);
					return -1;
				}
				break;
			case Comment:
				if(c == '\n')
					state = None;
				break;
			case InParamName:
				if(c == '\n') {
					fprintf(stderr, "Unexpected newline in parameter name at line %d\n", line-1);
					return -1;
				} else if(c == ':') {
					paramName[paramNameLen] = 0;
					state = AfterParamName;
				} else if(paramNameLen >= MAX_PARAM_NAME) {
					fprintf(stderr, "Unexpected '%c' (%d) in parameter name at line %d\n", CHAR_DOT(c), c, line);
					return -1;
				} else {
					paramName[paramNameLen++] = c;
				}
				break;
			case AfterParamName:
				if(isspace(c)) {
					// ignore
				} else if(isdigit(c)) {
					paramNum = 0;
					digits[0] = 0;
					digitNum = 0;
					APPEND_DIGIT;
					state = InParamVal;
				} else {
					fprintf(stderr, "Unexpected '%c' (%d) after parameter name at line %d\n", CHAR_DOT(c), c, line);
					return -1;
				}
				break;
			case InParamVal:
				if(c == '\n') {
					APPEND_PARAM;
					PARSE_PARAM;
					paramNum = 0;
					state = None;
				} else if(isspace(c)) {
					APPEND_PARAM;
					state = AfterParamVal;
				} else if(isdigit(c)) {
					APPEND_DIGIT;
				} else {
					fprintf(stderr, "Unexpected '%c' (%d) in parameter value at line %d\n", CHAR_DOT(c), c, line);
					return -1;
				}
				break;
			case AfterParamVal:
				if(c == '\n') {
					PARSE_PARAM;
					paramNum = 0;
					state = None;
				} else if(isspace(c)) {
					// do nothing
				} else if(isdigit(c)) {
					APPEND_DIGIT;
					state = InParamVal;
				}
				break;
			case InAt:
				if(isspace(c)) {
					// ignore
				} else if(c == ':') {

					state = AfterAt;
				}
				break;
			case AfterAt:
				if(isspace(c)) {
					//ignore
				} else if(isdigit(c)) {
					digitNum = 0;
					APPEND_DIGIT;
					state = InVoiceNum;
				} else {
					fprintf(stderr, "Unexpected '%c' (%d) after '@' at line %d\n", CHAR_DOT(c), c, line);
					return -1;
				}
				break;
			case InVoiceNum:
				if(isdigit(c)) {
					APPEND_DIGIT;
				} else {
					digits[digitNum] = 0;
					struct opm_file_voice v;
					v.number = atoi(digits);
					v.lfo_lfrq = 0;
					v.lfo_amd = 0;
					v.lfo_pmd = 0;
					v.lfo_wf = 0;
					v.nfrq = 0;
					v.ch_pan = 64;
					v.ch_fl = 7;
					v.ch_con = 4;
					v.ch_ams = 0;
					v.ch_pms = 0;
					v.ch_slot = 0x78;
					v.ch_ne = 0;
					for(int j = 0; j < 4; j++) {
						struct opm_file_operator *o = v.operators + j;
						o->ar = 0;
						o->d1r = 0;
						o->d2r = 0;
						o->rr = 0;
						o->d1l = 0;
						o->tl = 127;
						o->ks = 0;
						o->mul = 1;
						o->dt1 = 0;
						o->dt2 = 0;
						o->ame = 0;
					}
					if(opm_file_push_voice(opm, &v)) return -1;

					if(isspace(c)) {
						state = AfterVoiceNum;
					} else {
						APPEND_VOICE_NAME_CHAR;
						state = InVoiceName;
					}
				}
				break;
			case AfterVoiceNum:
				if(isspace(c)) {
					// ignore
				} else {
					APPEND_VOICE_NAME_CHAR;
					state = InVoiceName;
				}
				break;
			case InVoiceName:
				if(c == '\n') {
					voiceName[voiceNameLen] = 0;
					strncpy(opm->voices[opm->num_voices-1].name, voiceName, voiceNameLen);
					opm->voices[opm->num_voices-1].name[voiceNameLen] = 0;
					voiceNameLen = 0;
					state = None;
				} else {
					APPEND_VOICE_NAME_CHAR;
				}
				break;
		}
	}

	return 0;
}

static const char *opm_file_operator_name(int i) {
	const char *names[] = { "M1", "C1", "M2", "C2" };
	if(i > 3) return "??";
	return names[i];
}

int opm_file_save(struct opm_file *f, int (*write_fn)(void *buf, size_t bufsize, void *data_ptr), int pad_to, void *data_ptr) {
	uint8_t buf[256];

#define WRITEF(args...) { \
	int w = snprintf((char *)buf, sizeof(buf), args); \
	if(w < 0) return -1; \
	size_t wr = write_fn(buf, w, data_ptr); \
	if(wr < w) return -2; \
}

	WRITEF("//MiOPMdrv sound bank Paramer Ver2002.04.22\n");
	WRITEF("//@:[Num] [Name]\n");
	WRITEF("//LFO: LFRQ AMD PMD WF NFRQ\n");
	WRITEF("//CH: PAN FL CON AMS PMS SLOT NE\n");
	WRITEF("//[OPname]: AR D1R D2R RR D1L  TL KS MUL DT1 DT2 AMS-EN\n");

	for(int i = 0; i < f->num_voices; i++) {
		struct opm_file_voice *v = f->voices + i;
		WRITEF("\n");
		WRITEF("@:%d %s\n", i, v->name);
		WRITEF("LFO: %d %d %d %d %d\n", v->lfo_lfrq, v->lfo_amd, v->lfo_pmd, v->lfo_wf, v->nfrq);
		WRITEF("CH: %d %d %d %d %d %d %d\n", v->ch_pan, v->ch_fl, v->ch_con, v->ch_ams, v->ch_pms, v->ch_slot, v->ch_ne);
		for(int j = 0; j < 4; j++) {
			const int opmap[] = { 0, 2, 1, 3 };
			struct opm_file_operator *o = v->operators + opmap[j];
			WRITEF("%s: %2d  %2d  %2d %2d  %2d %3d %d  %2d %d %d %d\n", opm_file_operator_name(j), o->ar, o->d1r, o->d2r, o->rr, o->d1l, o->tl, o->ks, o->mul, o->dt1, o->dt2, o->ame);
		}
	}

	for(int i = f->num_voices; i < pad_to; i++) {
		WRITEF("\n");
		WRITEF("@:%d no Name\n", i);
		WRITEF("LFO: 0 0 0 0 0\n");
		WRITEF("CH: 64 0 0 0 0 64 0\n");
		WRITEF("M1: 31 0 0 4 0 0 0 1 0 0 0\n");
		WRITEF("C1: 31 0 0 4 0 0 0 1 0 0 0\n");
		WRITEF("M2: 31 0 0 4 0 0 0 1 0 0 0\n");
		WRITEF("C2: 31 0 0 4 0 0 0 1 0 0 0\n");
	}

#undef WRITEF
	return 0;
}

#ifdef HAVE_STDIO
void opm_file_dump(struct opm_file *f) {
	for(int i = 0; i < f->num_voices; i++) {
		struct opm_file_voice *v = f->voices + i;
		printf("%d number=%d name=%.*s\n", i, v->number, OPM_FILE_MAX_NAME, v->name);
		printf("LFO: lfrq=%d amd=%d pmd=%d wf=%d nfrq=%d\n", v->lfo_lfrq, v->lfo_amd, v->lfo_pmd, v->lfo_wf, v->nfrq);
		printf("CH: pan=%d fl=%d con=%d ams=%d pms=%d slot=%d ne=%d\n", v->ch_pan, v->ch_fl, v->ch_con, v->ch_ams, v->ch_pms, v->ch_slot, v->ch_ne);
		printf("OP: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AME\n");
		for(int j = 0; j < 4; j++) {
			struct opm_file_operator *o = v->operators + j;
			printf("%s: %2d  %2d  %2d %2d  %2d %2d %2d  %2d  %2d  %2d   %d\n", opm_file_operator_name(j), o->ar, o->d1r, o->d2r, o->rr, o->d1l, o->tl, o->ks, o->mul, o->dt1, o->dt2, o->ame);
		}
	}
}
#endif

#ifdef ENABLE_LOADERS
#include "loader.h"

static int load(void *data, int data_len, struct fm_voice_bank  *bank) {
	struct opm_file f;
	opm_file_init(&f);
	int r = opm_file_load(&f, data, data_len);
	if(r) return r;
	struct opm_voice *voice = fm_voice_bank_reserve_opm_voices(bank, f.num_voices);
	if(!voice) return -1;
	for(int i = 0; i < f.num_voices; i++) {
		struct opm_file_voice *v = &f.voices[i];
		memcpy(voice->name, v->name, 256);
		voice->lfrq = v->lfo_lfrq;
		voice->amd = v->lfo_amd & 0x7f;
		voice->pmd = v->lfo_pmd & 0x7f;
		voice->w = v->lfo_wf & 0x03;
		voice->ne_nfrq = v->ch_ne << 7 | (v->nfrq & 0x1f);
		voice->rl_fb_con = (v->ch_pan & 0xc0) | (v->ch_fl & 0x07) << 3 | (v->ch_con & 0x07);
		voice->pms_ams = (v->ch_pms & 0x07) << 4 | (v->ch_ams & 0x03);
		voice->slot = v->ch_slot >> 3;
		for(int j = 0; j < 4; j++) {
			struct opm_voice_operator *op = &voice->operators[j];
			struct opm_file_operator *fop = &v->operators[j];
			op->dt1_mul = fop->dt1 << 4 | fop->mul;
			op->tl = fop->tl;
			op->ks_ar = fop->ks << 6 | fop->ar;
			op->ams_d1r = fop->ame << 7 | fop->d1r;
			op->dt2_d2r = fop->dt2 << 6 | fop->d2r;
			op->d1l_rr = fop->d1l << 4 | fop->rr;
		}
		voice++;
	}
	return 0;
}

static int save(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	struct opm_file opm_file;
	opm_file_init(&opm_file);
	for(int i = pos->opm; i < bank->num_opm_voices; i++) {
		struct opm_voice *v = bank->opm_voices + i;
		struct opm_file_voice fv;
		memset(&fv, 0, sizeof(fv));
		fv.number = i;
		if(v->name && strlen(v->name) > 0) snprintf(fv.name, sizeof(fv.name), v->name);
		else snprintf(fv.name, sizeof(fv.name), "Instrument %d", i);
		fv.lfo_lfrq = v->lfrq;
		fv.lfo_amd = v->amd;
		fv.lfo_pmd = v->pmd;
		fv.lfo_wf = v->w;
		fv.nfrq = v->ne_nfrq;
		fv.ch_pan = 64;
		fv.ch_fl = v->rl_fb_con >> 3 & 0x07;
		fv.ch_con = v->rl_fb_con & 0x07;
		fv.ch_pms = v->pms_ams >> 4 & 0x07;
		fv.ch_ams = v->pms_ams & 0x03;
		fv.ch_slot = v->slot << 3;
		fv.ch_ne = 0;
		for(int j = 0; j < 4; j++) {
			const uint8_t dtmap[] = { 3, 4, 5, 6,  3, 2, 1, 0 };
			struct opm_file_operator *fop = &fv.operators[j];
			struct opm_voice_operator *op = &v->operators[j];
			fop->ar = op->ks_ar & 0x1f;
			fop->d1r = op->ams_d1r & 0x1f;
			fop->d2r = op->dt2_d2r & 0x1f;
			fop->rr = op->d1l_rr & 0x0f;
			fop->d1l = op->d1l_rr >> 4;
			fop->tl = op->tl & 0x7f;
			fop->ks = op->ks_ar >> 6;
			fop->mul = op->dt1_mul & 0x0f;
			// fop->dt1 = dtmap[op->dt1_mul >> 4 & 0x07];
			fop->dt1 = (op->dt1_mul >> 4) & 0x07;
			fop->dt2 = op->dt2_d2r >> 6;
			fop->ame = op->ams_d1r >> 7;
		}
		opm_file_push_voice(&opm_file, &fv);
		pos->opm++;
	}
	return opm_file_save(&opm_file, write_fn, 128, data_ptr);
}

struct loader opm_file_loader = {
	.load = load,
	.save = save,
	.name = "OPM",
	.description = "MiOPMdrv Sound Bank Parameter",
	.file_ext = "opm",
	.max_opl_voices = 0,
	.max_opm_voices = 128,
	.max_opn_voices = 0,
};
#endif
