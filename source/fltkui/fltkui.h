#ifndef MAIN_H
#define MAIN_H

#define UI_MBARHEIGHT 24
#define UI_SPACING 24
#define UI_ELEMHEIGHT 25
#define UI_ELEMWIDTH 160
#define UI_DIAL_LG 100
#define UI_DIAL_SM 40

class NstWindow : public Fl_Double_Window {
private:
	int handle(int e);

public:
	NstWindow(int w, int h, const char* t = 0) : Fl_Double_Window(w, h, t) { }
	virtual ~NstWindow() { }
};

class NstGlArea : public Fl_Gl_Window {
private:
	void draw() { nst_ogl_render(); }
	int handle(int e);

public:
	NstGlArea(int x, int y, int w, int h, const char *l = 0) : Fl_Gl_Window(x, y, w, h, l) {
		box(FL_DOWN_FRAME);
	}
};

extern Fl_Color NstGreen;
extern Fl_Color NstPurple;
extern Fl_Color NstRed;
extern Fl_Color NstBlueGrey;
extern Fl_Color NstLightGrey;

void fltkui_resize();
void fltkui_fullscreen(Fl_Widget* w, void* userdata);

#endif
