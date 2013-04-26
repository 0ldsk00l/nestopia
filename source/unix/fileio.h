/*
	NEStopia / Linux
	Port by R. Belmont
	
	fileio.h - movie and state I/O definitions
*/

#ifndef AUXIO_H
#define AUXIO_H

void fileio_init(void);
void fileio_do_state_save(void);
void fileio_do_state_load(void);
void fileio_do_movie_save(void);
void fileio_do_movie_load(void);
void fileio_do_movie_stop(void);
void fileio_set_fds_bios(void);
void fileio_shutdown(void);
int fileio_load_archive(const char *filename, unsigned char **dataout, int *datasize, int *dataoffset, const char *filetoload = NULL, char *outname = NULL);
void fileio_load_db(void);

#endif
