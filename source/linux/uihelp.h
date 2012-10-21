#ifndef _UIHELP_H_
#define _UIHELP_H_

void UIHelp_Init(int argc, char *argv[], LinuxNst::Settings *settings, LinuxNst::CheatMgr *cheatmgr, int xres, int yres);
void UIHelp_Unload(void);
void UIHelp_NSFLoaded(void);
void UIHelp_GameLoaded(void);
GdkPixbuf *UIHelp_GetNSTIcon(void);

#endif

