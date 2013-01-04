/*
	NEStopia / Linux
	Port by R. Belmont
	
	auxio.h - movie and state I/O definitions
*/

#ifndef AUXIO_H
#define AUXIO_H

void auxio_init(void);
void auxio_do_state_save(void);
void auxio_do_state_load(void);
void auxio_do_movie_save(void);
void auxio_do_movie_load(void);
void auxio_do_movie_stop(void);
void auxio_set_fds_bios(void);
void auxio_shutdown(void);
int auxio_load_archive(const char *filename, unsigned char **dataout, int *datasize, int *dataoffset, const char *filetoload = NULL, char *outname = NULL);
void auxio_load_db(void);

#endif
