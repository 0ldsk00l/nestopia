#ifndef _FLTKUI_CHEATS_H_
#define _FLTKUI_CHEATS_H_

class NstChtWindow : public Fl_Double_Window {

public:
	NstChtWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) { }
	virtual ~NstChtWindow() { }

	void refresh();
	void populate();
};

#endif
