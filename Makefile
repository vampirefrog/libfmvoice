AR?=ar
CC?=gcc
CFLAGS?=-Wall -O2 -DHAVE_STDIO -DENABLE_LOADERS
LDFLAGS?=-lz -lm
PROGS:=fmbankdump dmpdump dx21dump fb01dump insdump opmdump tfidump y12dump bnkdump sbidump

.PHONY: all

all: libfmvoice.a $(PROGS)

libfmvoice.a: op3_file.o opm_file.o bnk_file.o ins_file.o sbi_file.o tfi_file.o y12_file.o syx_dx21.o syx_fb01.o dmp_file.o md5.o opl_voice.o opm_voice.o opn_voice.o fm_voice_bank.o loader.o
	$(AR) cr $@ $^

fmbankdump: fmbankdump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
dmpdump: dmpdump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
dx21dump: dx21dump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
fb01dump: fb01dump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
insdump: insdump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
opmdump: opmdump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
tfidump: tfidump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
y12dump: y12dump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
bnkdump: bnkdump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
sbidump: sbidump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@
op3dump: op3dump.o tools.o libfmvoice.a
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.a *.d libfmvoice.a $(PROGS)
