/***************************************************************************************
* WAVSPLITTER                                                                          *
* Split .wav files and remove silence                                                  *
* usage: wavsplitter.exe (filename).wav                                                *
***************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE 0
#define TRUE 1
#define SILENCE_BUFFER 128
#define ERROR_NOFILE "usage: wavsplitter.exe (filename).wav\n"

/***************************************************************************************
* Structs                                                                              *
***************************************************************************************/
struct wav_header {
	unsigned int       rif_a; /* "RIFF" */
	unsigned int       rif_b; /* 4 + (8 + fmt_b) + (8 + dat_b) */
	unsigned int       rif_c; /* "WAVE" */
	unsigned int       fmt_a; /* "fmt " */
	unsigned int       fmt_b; /* 16 if PCM */
	unsigned short int fmt_c; /* PCM = 1 */
	unsigned short int fmt_d; /* Channels; Mono = 1, Stereo = 2, etc. */
	unsigned int       fmt_e; /* Sample rate */
	unsigned int       fmt_f; /* Bytes per second */
	unsigned short int fmt_g; /* Bytes per sample; fmt_d * (fmt_h / 8) */
	unsigned short int fmt_h; /* Bit depth */
	unsigned int       dat_a; /* "data" */
	unsigned int       dat_b; /* Number of bytes after header */
};

main(argc, argv)
int argc;
char *argv[];
{
	char s_byte[4];
	char filename[64];
	char *s_data;
	unsigned int fileindex;
	unsigned int i, j;
	unsigned int locked;
	unsigned int s_buff;
	unsigned int silent;
	struct wav_header h_in, h_out;
	FILE *fp_in, *fp_out;

	if (argc >= 2) {
		/* open up the original file for read access in byte mode */
		if ((fp_in = fopen(argv[1], "rb")) == NULL) {
			printf(ERROR_NOFILE);
			return(0);
		}
		/* read the wave header from the original file */
		fread(&h_in, sizeof(struct wav_header), 1, fp_in);
		/* allocate the buffer to hold the samples */
		s_data = malloc(h_in.dat_b);
		/* fill out the header for the output files */
		h_out.rif_a = h_in.rif_a;
		h_out.rif_b = 0;
		h_out.rif_c = h_in.rif_c;
		h_out.fmt_a = h_in.fmt_a;
		h_out.fmt_b = h_in.fmt_b;
		h_out.fmt_c = h_in.fmt_c;
		h_out.fmt_d = h_in.fmt_d;
		h_out.fmt_e = h_in.fmt_e;
		h_out.fmt_f = h_in.fmt_f;
		h_out.fmt_g = h_in.fmt_g;
		h_out.fmt_h = h_in.fmt_h;
		h_out.dat_a = h_in.dat_a;
		h_out.dat_b = 0;
		/* initialize values before entering the loop */
		locked = FALSE;
		s_buff = 0;
		/* open up the first output file */
		fileindex = 0;
		/* loop through and check every sample for silence */
		for (i = 0; i < h_in.dat_b; i += h_in.fmt_g) {
			silent = TRUE;
			/* check each byte in the sample */
			for (j = 0; j < h_in.fmt_g; j++) {
				/* copy the current byte from the file to an array */
				fread(&s_byte[j], sizeof(char), 1, fp_in);
				/* check for sound */
				if (s_byte[j] != 0) {
					locked = TRUE;
					silent = FALSE;
				}
			}
			if (locked) {
				if (h_out.dat_b == 0) {
					sprintf(filename, "output%04d.wav", fileindex);
					fp_out = fopen(filename, "wb");
				}
				memcpy(&s_data[h_out.dat_b], s_byte, h_in.fmt_g);
				h_out.dat_b += h_in.fmt_g;
				if (silent)
					s_buff += 1;
				else
					s_buff = 0;
				if (s_buff >= SILENCE_BUFFER) {
					h_out.dat_b -= (SILENCE_BUFFER * h_in.fmt_g);
					h_out.rif_b = 4 + (8 + h_out.fmt_b) + (8 + h_out.dat_b);
					fwrite(&h_out, sizeof(struct wav_header), 1, fp_out);
					fwrite(s_data, sizeof(char), h_out.dat_b, fp_out);
					fclose(fp_out);
					h_out.rif_b = 0;
					h_out.dat_b = 0;
					locked = FALSE;
					s_buff = 0;
					fileindex += 1;
				}
			}
		}
		fclose(fp_in);
		fclose(fp_out);
		free(s_data);
	}
	else {
		printf(ERROR_NOFILE);
	}
	return(0);
}
