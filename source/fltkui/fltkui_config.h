#ifndef _FLTKUI_CONFIG_H_
#define _FLTKUI_CONFIG_H_

class NstConfWindow : public Fl_Double_Window {
private:
	bool icfg_running;

public:
	NstConfWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) { }
	virtual ~NstConfWindow() { }

	void populate();
};

class NstInputConfWindow : public Fl_Double_Window {
private:
	int handle(int e);

public:
	NstInputConfWindow(int x, int y, int w, int h, const char* t) : Fl_Double_Window(x, y, w, h, t) {
		box(FL_DOWN_BOX);
	}
	virtual ~NstInputConfWindow() { }

	Fl_Box *text;
	int btn;
	int player;
	int device; // Keyboard or Joystick
};

#endif
