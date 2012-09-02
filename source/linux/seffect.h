#ifndef _SEFFECT_H_
#define _SEFFECT_H_

void seffect_init(LinuxNst::Settings *settings);
int seffect_surround_lite_process(short *data, int length);
void seffect_ex_process(long *pBuf,long samples);

#endif
