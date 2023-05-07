FM voice loading, saving and conversion library
===============================================

File Formats
------------

| Format | Type | Op | Max voices |
|-------:|:----:|:--:|:----------:|
| [INS format](https://vgmrips.net/wiki/INS_File_Format) | OPN | 4 | 1 |
| [DMP format](https://vgmrips.net/wiki/DMP_File_Format) | OPN | 4 | 1 |
| [OPM format](https://vgmrips.net/wiki/OPM_File_Format) | OPM | 4 | * |
| [TFI format](https://vgmrips.net/wiki/TFI_File_Format) | OPN | 4 | 1 |
| [Y12 format](https://vgmrips.net/wiki/Y12_File_Format) | OPN | 4 | 1 |

OPM files
---------

Reading:

```C
uint8_t *data; // Have your data in this buffer
size_t data_len; // with this length
struct opm_file opm;
int r = opm_file_load(&opm, data, data_len);
if(r != OPM_FILE_SUCCESS) {
	fprintf(stderr, "Could not load file: %s (%d)\n", opm_file_error_string(r), r);
	exit(1);
}
```

Writing:
```C
size_t write_fn(void *buf, size_t bufsize, void *data_ptr) {
	FILE *f = (FILE *)data_ptr;
	return fwrite(buf, 1, bufsize, f);
}

struct opm_file opm;
opm_file_init(&opm);

FILE *f = fopen("out.opm", "w");
int opm_file_save(&opm, write_fn, f);
fclose(f);
```
