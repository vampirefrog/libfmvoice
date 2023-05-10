AR=ar
CC=gcc
CFLAGS=-Wall -O2
LDFLAGS=-lz -lm
PROGS=dmpdump dx21dump fb01dump insdump opmdump tfidump y12dump bnkdump

.PHONY: all

all: $(PROGS)

dmpdump: dmpdump.o dmp_file.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
dx21dump: dx21dump.o syx_dx21.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
fb01dump: fb01dump.o syx_fb01.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
insdump: insdump.o ins_file.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
opmdump: opmdump.o opm_file.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
tfidump: tfidump.o tfi_file.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
y12dump: y12dump.o y12_file.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@
bnkdump: bnkdump.o bnk_file.o tools.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.a *.d $(PROGS)
