#ifndef _CHEATS_H_
#define _CHEATS_H_

namespace LinuxNst
{
	class CheatMgr
	{
	public:
		CheatMgr();
		~CheatMgr();

		// bring up the cheat manager GUI
		void ShowManager();
 		
		// load enabled cheats into the NEStopia core
		void Enable();

		// unload cheats (both our private list and the core)
		void Unload();

		// add a code to the internal list
		void AddCode(Nes::Api::Cheats::Code &code, bool use_enable = false, bool enable = true, char *description = NULL);

	private:
	};
};

void cheats_init();
void cheats_unload(void);

#endif
