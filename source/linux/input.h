#ifndef INPUT_H_
#define INPUT_H_

#define CRg(rg) (sizeof(rg) / sizeof(rg[0]))

#define DEADZONE (32768/3)


// input events
enum InputEvtT
{
	RESET = 0, 	// reset machine
	QSAVE1,		// quick save state
	QSAVE2,
	QLOAD1,		// quick load state
	QLOAD2,
	SAVE,	  	// normal save (prompt)
	LOAD,	  	// normal load (prompt)
	MSAVE,		// movie save (prompt)
	MLOAD,		// movie load (prompt)
	MSTOP,		// movie stop
	FLIP,	  	// flip disk
	FSCREEN,	// full screen toggle
	RBACK,		// rewind backward
	RFORE,		// rewind forward
	STOP,	  	// stop game
	EXIT,	  	// stop game and exit emulator
	COIN1,		// VS. coin 1
	COIN2,		// VS. coin 2
};


struct InputDefT
{
	SDL_Event evt;	// event to match

	int player;             // player action, -1 if not player-related
	int codeout;            // Nes::Api::Input::Controllers::Pad constant if player,
                                //  InputEvtT otherwise
};


InputDefT *parse_input_file();
void write_output_file(InputDefT *ctl_defs);
bool translate_event(char *linebuf, InputDefT *pcontrol, int &icontrol, int ccontrol);

#endif
