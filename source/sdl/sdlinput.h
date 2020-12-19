#ifndef _SDLINPUT_H_
#define _SDLINPUT_H_

#define TOTALBUTTONS (NUMGAMEPADS*NUMBUTTONS)
#define DEADZONE (32768/3)

#include "core/api/NstApiInput.hpp"

typedef struct {
	SDL_Scancode u;
	SDL_Scancode d;
	SDL_Scancode l;
	SDL_Scancode r;
	SDL_Scancode select;
	SDL_Scancode start;
	SDL_Scancode a;
	SDL_Scancode b;
	SDL_Scancode ta;
	SDL_Scancode tb;
	
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
} gamepad_t;

typedef struct {
	// User Interface
	char *qsave1;
	char *qsave2;
	char *qload1;
	char *qload2;
	
	char *screenshot;
	
	char *fdsflip;
	char *fdsswitch;
	
	char *insertcoin1;
	char *insertcoin2;
	
	char *reset;
	
	char *ffspeed;
	char *rwstart;
	char *rwstop;
	
	char *fullscreen;
	char *filter;
	char *scalefactor;
	
	// Player 1
	char *kb_p1u;
	char *kb_p1d;
	char *kb_p1l;
	char *kb_p1r;
	char *kb_p1select;
	char *kb_p1start;
	char *kb_p1a;
	char *kb_p1b;
	char *kb_p1ta;
	char *kb_p1tb;
	
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
	
	// Player 2
	char *kb_p2u;
	char *kb_p2d;
	char *kb_p2l;
	char *kb_p2r;
	char *kb_p2select;
	char *kb_p2start;
	char *kb_p2a;
	char *kb_p2b;
	char *kb_p2ta;
	char *kb_p2tb;
	
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


void nstsdl_input_conf(int type, int pnum);
void nstsdl_input_conf_button(int pnum, int bnum);
void nstsdl_input_conf_defaults();
void nstsdl_input_conf_set(SDL_Event event, int type, int pnum, int counter);
void nstsdl_input_conf_read();
void nstsdl_input_conf_write();

void nstsdl_input_joysticks_detect();
void nstsdl_input_joysticks_close();

void nstsdl_input_match_joystick(Input::Controllers *controllers, SDL_Event event);
int nstsdl_input_checksign(int axisvalue);

void nstsdl_input_process(Input::Controllers *controllers, SDL_Event event);

char* nstsdl_input_translate_event(SDL_Event event);
SDL_Event nstsdl_input_translate_string(const char *string);

#endif
