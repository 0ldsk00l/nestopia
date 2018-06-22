#ifndef _SAMPLES_H_
#define _SAMPLES_H_

#include "core/api/NstApiUser.hpp"

using namespace Nes::Api;

int nst_sample_load_file(const char* filepath);
int nst_sample_load_archive(const char* filename, const char* reqfile);
int nst_sample_unload_file();
void nst_sample_setcontent(User::File& file);
void nst_sample_load_samples(User::File& file, const char* sampgame);

#endif
