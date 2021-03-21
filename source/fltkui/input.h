#ifndef _INPUT_H_
#define _INPUT_H_

#define NUMGAMEPADS 2
#define NUMBUTTONS 10
#define TOTALBUTTONS (NUMGAMEPADS*NUMBUTTONS)
#define DEADZONE (32768/3)

#include "core/api/NstApiInput.hpp"

typedef struct {
	int u;
	int d;
	int l;
	int r;
	int select;
	int start;
	int a;
	int b;
	int ta;
	int tb;

	SDL_Event ju;
	SDL_Event jd;
	SDL_Event jl;
	SDL_Event jr;
	SDL_Event jselect;
	SDL_Event jstart;
	SDL_Event ja;
	SDL_Event jb;
	SDL_Event jta;
	SDL_Event jtb;
	SDL_Event rwstart;
	SDL_Event rwstop;
	SDL_Event softreset;
	SDL_Event hardreset;
} gamepad_t;

typedef struct {
	// User Interface
	int qsave1;
	int qsave2;
	int qload1;
	int qload2;

	int screenshot;

	int fdsflip;
	int fdsswitch;

	int insertcoin1;
	int insertcoin2;

	int reset;

	int ffspeed;
	int rwstart;
	int rwstop;

	int fullscreen;

	// Player 1
	int kb_p1u;
	int kb_p1d;
	int kb_p1l;
	int kb_p1r;
	int kb_p1select;
	int kb_p1start;
	int kb_p1a;
	int kb_p1b;
	int kb_p1ta;
	int kb_p1tb;

	char *js_p1u;
	char *js_p1d;
	char *js_p1l;
	char *js_p1r;
	char *js_p1select;
	char *js_p1start;
	char *js_p1a;
	char *js_p1b;
	char *js_p1ta;
	char *js_p1tb;

	char *js_rwstart;
	char *js_rwstop;

	char *js_softreset;
	char *js_hardreset;

	// Player 2
	int kb_p2u;
	int kb_p2d;
	int kb_p2l;
	int kb_p2r;
	int kb_p2select;
	int kb_p2start;
	int kb_p2a;
	int kb_p2b;
	int kb_p2ta;
	int kb_p2tb;

	char *js_p2u;
	char *js_p2d;
	char *js_p2l;
	char *js_p2r;
	char *js_p2select;
	char *js_p2start;
	char *js_p2a;
	char *js_p2b;
	char *js_p2ta;
	char *js_p2tb;
} inputsettings_t;

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

void nstsdl_input_conf(int type, int pnum);
void nstsdl_input_conf_button(int pnum, int bnum);
void nstsdl_input_conf_defaults();
void nstsdl_input_conf_set(SDL_Event event, int pnum, int bnum);
void nstsdl_input_conf_read();
void nstsdl_input_conf_write();

void nstsdl_input_joysticks_detect();
void nstsdl_input_joysticks_close();

void nstsdl_input_match_joystick(Input::Controllers *controllers, SDL_Event event);
int nstsdl_input_checksign(int axisvalue);

void nstsdl_input_process(Input::Controllers *controllers, SDL_Event event);

char* nstsdl_input_translate_event(SDL_Event event);
SDL_Event nstsdl_input_translate_string(const char *string);

void fltkui_input_conf_set(int kval, int pnum, int bnum);
void fltkui_input_process_key(int e);
#endif
