	/*
	NEStopia / Linux
	David B. Robins (nestopia@davidrobins.net)
	
	kentry.h - key code mapping
*/

#ifndef KENTRY_H_
#define KENTRY_H_

#include "SDL.h"

typedef struct
{
	char *string;
	int code;
} KEntry;

extern const KEntry keycodes[];
extern const KEntry metacodes[];
extern const KEntry controlcodes[];

int kentry_find_str(const KEntry *kentry, const char *str);
const char *kentry_find_code(const KEntry *kentry, int code);

#endif // KENTRY_H_

