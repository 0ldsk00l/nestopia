#ifndef _INPUT_H_
#define _INPUT_H_

#define NUMGAMEPADS 2
#define NUMBUTTONS 10

#include "core/api/NstApiInput.hpp"

using namespace Nes::Api;

typedef struct {
	unsigned char player;
	unsigned char nescode;
	unsigned char pressed;
	unsigned char turboa;
	unsigned char turbob;
} nesinput_t;

typedef struct {
	int p1a;
	int p1b;
	int p2a;
	int p2b;
} turbo_t;

void nst_input_init();

void nst_input_inject(Input::Controllers *controllers, nesinput_t input);
void nst_input_inject_mouse(Input::Controllers *controllers, int b, int s, int x, int y);

void nst_input_turbo_init();
void nst_input_turbo_pulse(Input::Controllers *controllers);

int nst_input_zapper_present();

int input_configure_item(int pnum, int bnum, int type);
#endif
