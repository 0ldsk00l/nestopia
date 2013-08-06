#ifndef _VIDEO_H_
#define _VIDEO_H_

#define OVERSCAN_LEFT 0
#define OVERSCAN_RIGHT 0
#define OVERSCAN_BOTTOM 8
#define OVERSCAN_TOP 8

int powerOfTwo(const int value);
void opengl_init_structures();
void opengl_cleanup();
void opengl_blit();

long Linux_LockScreen(void*& ptr);
void Linux_UnlockScreen(void*);

#endif
