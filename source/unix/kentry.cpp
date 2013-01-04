/*
	NEStopia / Linux
	David B. Robins (nestopia@davidrobins.net)
	R. Belmont
	
	kentry.cpp - key code mapping
*/

#include "kentry.h"
#include "input.h"
#include "core/api/NstApiInput.hpp"

const KEntry keycodes[] =
{
	{ "ENTER", SDLK_RETURN },
	{ "LCTRL", SDLK_LCTRL },
	{ "RCTRL", SDLK_RCTRL },
	{ "LALT", SDLK_LALT },
	{ "RALT", SDLK_RALT },
	{ "SPACE", SDLK_SPACE },
	{ "LSHIFT", SDLK_LSHIFT },
	{ "RSHIFT", SDLK_RSHIFT },
	{ "TAB", SDLK_TAB },
	{ "UP", SDLK_UP },
	{ "DOWN", SDLK_DOWN },
	{ "LEFT", SDLK_LEFT },
	{ "RIGHT", SDLK_RIGHT },
	{ "KPENTER", SDLK_KP_ENTER },
	{ "KP0", SDLK_KP0 },
	{ "KP1", SDLK_KP1 },
	{ "KP2", SDLK_KP2 },
	{ "KP3", SDLK_KP3 },
	{ "KP4", SDLK_KP4 },
	{ "KP5", SDLK_KP5 },
	{ "KP6", SDLK_KP6 },
	{ "KP7", SDLK_KP7 },
	{ "KP8", SDLK_KP8 },
	{ "KP9", SDLK_KP9 },
	{ "KPDOT", SDLK_KP_PERIOD },
	{ "KPSLASH", SDLK_KP_DIVIDE },
	{ "KPSTAR", SDLK_KP_MULTIPLY },
	{ "KPMINUS", SDLK_KP_MINUS },
	{ "KPPLUS", SDLK_KP_PLUS },
	{ "ESCAPE", SDLK_ESCAPE },
	{ "BACKSPACE", SDLK_BACKSPACE },
	{ "F1", SDLK_F1 },
	{ "F2", SDLK_F2 },
	{ "F3", SDLK_F3 },
	{ "F4", SDLK_F4 },
	{ "F5", SDLK_F5 },
	{ "F6", SDLK_F6 },
	{ "F7", SDLK_F7 },
	{ "F8", SDLK_F8 },
	{ "F9", SDLK_F9 },
	{ "F10", SDLK_F10 },
	{ "F11", SDLK_F11 },
	{ "F12", SDLK_F12 },
	{ NULL, -1 }
};


#define DEFINE_META(x) { #x, x }
const KEntry metacodes[] =
{
	DEFINE_META(RESET),		// 0
	DEFINE_META(QSAVE1),		// 1
	DEFINE_META(QLOAD1),		// 2
	DEFINE_META(QSAVE2),		// 3
	DEFINE_META(QLOAD2),		// 4
	DEFINE_META(SAVE),		// 5
	DEFINE_META(LOAD),		// 6
	DEFINE_META(MSAVE),		// 7
	DEFINE_META(MLOAD),		// 8
	DEFINE_META(MSTOP),		// 9
	DEFINE_META(FLIP),		// 10
	DEFINE_META(FSCREEN),		// 11
	DEFINE_META(RBACK),		// 12
	DEFINE_META(RFORE),		// 13
	DEFINE_META(STOP),		// 14
	DEFINE_META(EXIT),		// 15
	DEFINE_META(COIN1),		// 16
	DEFINE_META(COIN2),		// 17
	{ NULL, -1 }
};
#undef DEFINE_META


#define DEFINE_CONTROL(string, code) { #string, Nes::Api::Input::Controllers::Pad::code }
const KEntry controlcodes[] =
{
	DEFINE_CONTROL(UP, UP),
	DEFINE_CONTROL(DN, DOWN),
	DEFINE_CONTROL(LT, LEFT),
	DEFINE_CONTROL(RT, RIGHT),
	DEFINE_CONTROL(A, A),
	DEFINE_CONTROL(B, B),
	DEFINE_CONTROL(START, START),
	DEFINE_CONTROL(SELECT, SELECT),
	{ NULL, -1 }
};
#undef DEFINE_CONTROL


// try to match str in kentry; returns -1 on failure
int kentry_find_str(const KEntry *kentry, const char *str)
{
	for (; kentry->code != -1 && strcmp(str, kentry->string); ++kentry);
	return kentry->code;
}


// try to match code in kentry; returns null on failure
const char *kentry_find_code(const KEntry *kentry, int code)
{
	for (; kentry->code != -1 && kentry->code != code; ++kentry);
	return kentry->string;
}


