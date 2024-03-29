/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2021 R. Danbrook
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <cstdio>
#include <cstring>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Output.H>

#include "nstcommon.h"
#include "config.h"
#include "audio.h"
#include "video.h"
#include "input.h"

#include "fltkui.h"
#include "fltkui_config.h"

static const char *icfg_labels[10] = {
	"Press Key For: Up",
	"Press Key For: Down",
	"Press Key For: Left",
	"Press Key For: Right",
	"Press Key For: Select",
	"Press Key For: Start",
	"Press Key For: A",
	"Press Key For: B",
	"Press Key For: Turbo A",
	"Press Key For: Turbo B"
};

NstInputConfWindow *icfg;

static Fl_Dial *dial_vall, *dial_vsq1, *dial_vsq2, *dial_vtri, *dial_vnoise, *dial_vdpcm,
	*dial_vfds, *dial_vmmc5, *dial_vvrc6, *dial_vvrc7, *dial_vn163, *dial_vs5b;

extern inputsettings_t inputconf;

static void cb_filter(Fl_Widget *w, long) {
	conf.video_filter = ((Fl_Choice*)w)->value();
	fltkui_resize();
}

static void cb_scale(Fl_Widget *w, long) {
	conf.video_scale_factor = ((Fl_Choice*)w)->value() + 1;
	fltkui_resize();
}

static void cb_ntscmode(Fl_Widget *w, long) {
	conf.video_ntsc_mode = ((Fl_Choice*)w)->value();
	fltkui_resize();
}

static void cb_xbrrounding(Fl_Widget *w, long) {
	conf.video_xbr_corner_rounding = ((Fl_Choice*)w)->value();
	video_init();
	video_toggle_filterupdate();
}

static void cb_palettemode(Fl_Widget *w, long) {
	conf.video_palette_mode = ((Fl_Choice*)w)->value();
	video_init();
}

static void cb_decoder(Fl_Widget *w, long) {
	conf.video_decoder = ((Fl_Choice*)w)->value();
	video_init();
}

static void cb_brightness(Fl_Widget *w, long) {
	conf.video_brightness = ((Fl_Valuator*)w)->value();
	video_init();
}

static void cb_saturation(Fl_Widget *w, long) {
	conf.video_saturation = ((Fl_Valuator*)w)->value();
	video_init();
}

static void cb_contrast(Fl_Widget *w, long) {
	conf.video_contrast = ((Fl_Valuator*)w)->value();
	video_init();
}

static void cb_hue(Fl_Widget *w, long) {
	conf.video_hue = ((Fl_Valuator*)w)->value();
	video_init();
}

static void cb_xbrpixblend(Fl_Widget *w, long) {
	conf.video_xbr_pixel_blending = ((Fl_Check_Button*)w)->value();
	video_init();
	video_toggle_filterupdate();
}

static void cb_linearfilter(Fl_Widget *w, long) {
	conf.video_linear_filter = ((Fl_Check_Button*)w)->value();
	video_init();
}

static void cb_tvaspect(Fl_Widget *w, long) {
	conf.video_tv_aspect = ((Fl_Check_Button*)w)->value();
	fltkui_resize();
}

static void cb_unmask_overscan(Fl_Widget *w, long) {
	conf.video_unmask_overscan = ((Fl_Check_Button*)w)->value();
	fltkui_resize();
}

static void cb_unlimited_sprites(Fl_Widget *w, long) {
	conf.video_unlimited_sprites = ((Fl_Check_Button*)w)->value();
}

static void cb_samplerate(Fl_Widget *w, long) {
	switch (((Fl_Choice*)w)->value()) {
		case 0: conf.audio_sample_rate = 44100; break;
		case 1: conf.audio_sample_rate = 48000; break;
		case 2: conf.audio_sample_rate = 96000; break;
		default: conf.audio_sample_rate = 48000; break;
	}

	if (nst_playing()) {
		nst_pause();
		nst_play();
	}
}

static void cb_dials() {
	dial_vall->value(conf.audio_volume);
	dial_vsq1->value(conf.audio_vol_sq1);
	dial_vsq2->value(conf.audio_vol_sq2);
	dial_vtri->value(conf.audio_vol_tri);
	dial_vnoise->value(conf.audio_vol_noise);
	dial_vdpcm->value(conf.audio_vol_dpcm);
	dial_vfds->value(conf.audio_vol_fds);
	dial_vmmc5->value(conf.audio_vol_mmc5);
	dial_vvrc6->value(conf.audio_vol_vrc6);
	dial_vvrc7->value(conf.audio_vol_vrc7);
	dial_vn163->value(conf.audio_vol_n163);
	dial_vs5b->value(conf.audio_vol_s5b);
}

static void cb_volume(Fl_Widget *w, long adj) {
	Fl_Dial *dial = (Fl_Dial*)w;

	switch (adj) {
		case 0:
			conf.audio_volume = (int)dial->value();
			conf.audio_vol_sq1 = conf.audio_vol_sq2 = conf.audio_vol_tri = conf.audio_vol_noise =
			conf.audio_vol_dpcm = conf.audio_vol_fds = conf.audio_vol_mmc5 = conf.audio_vol_vrc6 =
			conf.audio_vol_vrc7 = conf.audio_vol_n163 = conf.audio_vol_s5b = conf.audio_volume;
			break;
		case 1: conf.audio_vol_sq1 = (int)dial->value(); break;
		case 2: conf.audio_vol_sq2 = (int)dial->value(); break;
		case 3: conf.audio_vol_tri = (int)dial->value(); break;
		case 4: conf.audio_vol_noise = (int)dial->value(); break;
		case 5: conf.audio_vol_dpcm = (int)dial->value(); break;
		case 6: conf.audio_vol_fds = (int)dial->value(); break;
		case 7: conf.audio_vol_mmc5 = (int)dial->value(); break;
		case 8: conf.audio_vol_vrc6 = (int)dial->value(); break;
		case 9: conf.audio_vol_vrc7 = (int)dial->value(); break;
		case 10: conf.audio_vol_n163 = (int)dial->value(); break;
		case 11: conf.audio_vol_s5b = (int)dial->value(); break;
	}

	audio_adj_volume();
	cb_dials();

}

static void cb_stereo(Fl_Widget *w, long) {
	conf.audio_stereo = ((Fl_Check_Button*)w)->value();

	if (nst_playing()) {
		nst_pause();
		nst_play();
	}
}

int NstInputConfWindow::handle(int e) {
	switch (e) {
		case FL_KEYUP:
			fltkui_input_conf_set(Fl::event_key(), player, btn);
			this->hide();
			this->set_non_modal();
			break;
	}

	return Fl_Double_Window::handle(e);
}

static void cb_icfg(Fl_Widget *w, long btn) {
	icfg->set_modal();
	icfg->btn = btn;
	icfg->text->label(icfg_labels[btn]);
	icfg->show();

	if (icfg->device == 1) {
		nstsdl_input_conf_button(icfg->player, btn);
		icfg->hide();
		icfg->set_non_modal();
	}
}

static void cb_player(Fl_Widget *w, long) {
	icfg->player = ((Fl_Choice*)w)->value();
}

static void cb_idevice(Fl_Widget *w, long) {
	icfg->device = ((Fl_Choice*)w)->value();
}

static void cb_turbopulse(Fl_Widget *w, long) {
	conf.timing_turbopulse = ((Fl_Valuator*)w)->value();
}

static void cb_default_system(Fl_Widget *w, long) {
	conf.misc_default_system = ((Fl_Choice*)w)->value();
}

static void cb_power_state(Fl_Widget *w, long) {
	conf.misc_power_state = ((Fl_Choice*)w)->value();
}

static void cb_ffspeed(Fl_Widget *w, long) {
	conf.timing_ffspeed = ((Fl_Valuator*)w)->value();
}

static void cb_soft_patching(Fl_Widget *w, long) {
	conf.misc_soft_patching = ((Fl_Check_Button*)w)->value();
}

static void cb_genie_distortion(Fl_Widget *w, long) {
	conf.misc_genie_distortion = ((Fl_Check_Button*)w)->value();
}

static void cb_disable_cursor(Fl_Widget *w, long) {
	conf.misc_disable_cursor = ((Fl_Check_Button*)w)->value();
}

static void cb_disable_cursor_special(Fl_Widget *w, long) {
	conf.misc_disable_cursor_special = ((Fl_Check_Button*)w)->value();
}

static void cb_ok(Fl_Widget *w, long) {
	w->parent()->hide();
}

void NstConfWindow::populate() {
	Fl_Tabs *tabs = new Fl_Tabs(10, 5, 380, 360);

	Fl_Group *vtab = new Fl_Group(10, 30, 380, 360, "&Video");

	Fl_Choice *ch_filter = new Fl_Choice(20, 55, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Filter");
	ch_filter->align(FL_ALIGN_TOP_LEFT);
	ch_filter->add("None");
	ch_filter->add("NTSC");
	ch_filter->add("xBR");
	ch_filter->add("HqX");
	ch_filter->add("2XSaI");
	ch_filter->add("ScaleX");
	ch_filter->value(conf.video_filter);
	ch_filter->callback(cb_filter);

	Fl_Choice *ch_scale = new Fl_Choice(200, 55, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Scale Factor");
	ch_scale->align(FL_ALIGN_TOP_LEFT);
	ch_scale->add("1x");
	ch_scale->add("2x");
	ch_scale->add("3x");
	ch_scale->add("4x");
	ch_scale->add("5x");
	ch_scale->add("6x");
	ch_scale->add("7x");
	ch_scale->add("8x");
	ch_scale->value(conf.video_scale_factor - 1);
	ch_scale->callback(cb_scale);

	Fl_Choice *ch_ntscmode = new Fl_Choice(20, 105, UI_ELEMWIDTH, UI_ELEMHEIGHT, "NTSC Mode");
	ch_ntscmode->align(FL_ALIGN_TOP_LEFT);
	ch_ntscmode->add("Composite");
	ch_ntscmode->add("S-Video");
	ch_ntscmode->add("RGB");
	ch_ntscmode->add("Monochrome");
	ch_ntscmode->add("Custom");
	ch_ntscmode->value(conf.video_ntsc_mode);
	ch_ntscmode->callback(cb_ntscmode);

	Fl_Choice *ch_xbrrounding = new Fl_Choice(200, 105, UI_ELEMWIDTH, UI_ELEMHEIGHT, "xBR Corner Rounding");
	ch_xbrrounding->align(FL_ALIGN_TOP_LEFT);
	ch_xbrrounding->add("None");
	ch_xbrrounding->add("Some");
	ch_xbrrounding->add("All");
	ch_xbrrounding->value(conf.video_xbr_corner_rounding);
	ch_xbrrounding->callback(cb_xbrrounding);

	Fl_Choice *ch_palettemode = new Fl_Choice(20, 155, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Palette Mode");
	ch_palettemode->align(FL_ALIGN_TOP_LEFT);
	ch_palettemode->add("YUV");
	ch_palettemode->add("RGB");
	ch_palettemode->add("Custom");
	ch_palettemode->value(conf.video_palette_mode);
	ch_palettemode->callback(cb_palettemode);

	Fl_Choice *ch_decoder = new Fl_Choice(200, 155, UI_ELEMWIDTH, UI_ELEMHEIGHT, "YUV Decoder");
	ch_decoder->align(FL_ALIGN_TOP_LEFT);
	ch_decoder->add("Consumer");
	ch_decoder->add("Canonical");
	ch_decoder->add("Alternative");
	ch_decoder->value(conf.video_decoder);
	ch_decoder->callback(cb_decoder);

	Fl_Hor_Value_Slider *sld_brightness = new Fl_Hor_Value_Slider(20, 210, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Brightness");
	sld_brightness->align(FL_ALIGN_TOP_LEFT);
	sld_brightness->bounds(-100, 100);
	sld_brightness->box(FL_FLAT_BOX);
	sld_brightness->callback(cb_brightness);
	sld_brightness->step(1);
	sld_brightness->selection_color(NstGreen);
	sld_brightness->type(FL_HOR_NICE_SLIDER);
	sld_brightness->value(conf.video_brightness);

	Fl_Hor_Value_Slider *sld_saturation = new Fl_Hor_Value_Slider(20, 250, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Saturation");
	sld_saturation->align(FL_ALIGN_TOP_LEFT);
	sld_saturation->bounds(-100, 100);
	sld_saturation->box(FL_FLAT_BOX);
	sld_saturation->callback(cb_saturation);
	sld_saturation->step(1);
	sld_saturation->selection_color(NstGreen);
	sld_saturation->type(FL_HOR_NICE_SLIDER);
	sld_saturation->value(conf.video_saturation);

	Fl_Hor_Value_Slider *sld_contrast = new Fl_Hor_Value_Slider(20, 290, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Contrast");
	sld_contrast->align(FL_ALIGN_TOP_LEFT);
	sld_contrast->bounds(-100, 100);
	sld_contrast->box(FL_FLAT_BOX);
	sld_contrast->callback(cb_contrast);
	sld_contrast->step(1);
	sld_contrast->selection_color(NstGreen);
	sld_contrast->type(FL_HOR_NICE_SLIDER);
	sld_contrast->value(conf.video_contrast);

	Fl_Hor_Value_Slider *sld_hue = new Fl_Hor_Value_Slider(20, 330, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Hue");
	sld_hue->align(FL_ALIGN_TOP_LEFT);
	sld_hue->bounds(-45, 45);
	sld_hue->box(FL_FLAT_BOX);
	sld_hue->callback(cb_hue);
	sld_hue->step(1);
	sld_hue->selection_color(NstGreen);
	sld_hue->type(FL_HOR_NICE_SLIDER);
	sld_hue->value(conf.video_hue);

	Fl_Check_Button *chk_xbrpixblend = new Fl_Check_Button(200, 210, UI_ELEMWIDTH, UI_ELEMHEIGHT, "xBR Pixel Blending");
	chk_xbrpixblend->value(conf.video_xbr_pixel_blending);
	chk_xbrpixblend->callback(cb_xbrpixblend);

	Fl_Check_Button *chk_linearfilter = new Fl_Check_Button(200, 235, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Linear Filter");
	chk_linearfilter->value(conf.video_linear_filter);
	chk_linearfilter->callback(cb_linearfilter);

	Fl_Check_Button *chk_tvaspect = new Fl_Check_Button(200, 260, UI_ELEMWIDTH, UI_ELEMHEIGHT, "TV Aspect Ratio");
	chk_tvaspect->value(conf.video_tv_aspect);
	chk_tvaspect->callback(cb_tvaspect);

	Fl_Check_Button *chk_unmask_overscan = new Fl_Check_Button(200, 285, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Unmask Overscan");
	chk_unmask_overscan->value(conf.video_unmask_overscan);
	chk_unmask_overscan->callback(cb_unmask_overscan);

	Fl_Check_Button *chk_unlimited_sprites = new Fl_Check_Button(200, 310, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Unlimited Sprites");
	chk_unlimited_sprites->value(conf.video_unlimited_sprites);
	chk_unlimited_sprites->callback(cb_unlimited_sprites);

	vtab->end();

	Fl_Group *atab = new Fl_Group(10, 30, 380, 360, "&Audio");

	Fl_Choice *ch_samplerate = new Fl_Choice(20, 55, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Sample Rate");
	ch_samplerate->align(FL_ALIGN_TOP_LEFT);
	ch_samplerate->add("44100Hz");
	ch_samplerate->add("48000Hz");
	ch_samplerate->add("96000Hz");
	switch (conf.audio_sample_rate) {
		case 44100: ch_samplerate->value(0); break;
		case 48000: ch_samplerate->value(1); break;
		case 96000: ch_samplerate->value(2); break;
		default: ch_samplerate->value(1); break;
	}
	ch_samplerate->callback(cb_samplerate);

	Fl_Check_Button *chk_stereo = new Fl_Check_Button(200, 55, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Stereo");
	chk_stereo->value(conf.audio_stereo);
	chk_stereo->callback(cb_stereo);

	dial_vall = new Fl_Dial(20, 100, UI_DIAL_LG, UI_DIAL_LG, "All");
	dial_vall->bounds(0, 100);
	dial_vall->step(1);
	dial_vall->color(NstPurple);
	dial_vall->selection_color(NstGreen);
	dial_vall->callback(cb_volume, 0);
	dial_vall->value(conf.audio_volume);

	dial_vsq1 = new Fl_Dial(130, 115, UI_DIAL_SM, UI_DIAL_SM, "SQ1");
	dial_vsq1->bounds(0, 100);
	dial_vsq1->step(1);
	dial_vsq1->color(NstGreen);
	dial_vsq1->selection_color(NstPurple);
	dial_vsq1->callback(cb_volume, 1);
	dial_vsq1->value(conf.audio_vol_sq1);

	dial_vsq2 = new Fl_Dial(180, 115, UI_DIAL_SM, UI_DIAL_SM, "SQ2");
	dial_vsq2->bounds(0, 100);
	dial_vsq2->step(1);
	dial_vsq2->color(NstGreen);
	dial_vsq2->selection_color(NstPurple);
	dial_vsq2->callback(cb_volume, 2);
	dial_vsq2->value(conf.audio_vol_sq2);

	dial_vtri = new Fl_Dial(230, 115, UI_DIAL_SM, UI_DIAL_SM, "TRI");
	dial_vtri->bounds(0, 100);
	dial_vtri->step(1);
	dial_vtri->color(NstGreen);
	dial_vtri->selection_color(NstPurple);
	dial_vtri->callback(cb_volume, 3);
	dial_vtri->value(conf.audio_vol_tri);

	dial_vnoise = new Fl_Dial(280, 115, UI_DIAL_SM, UI_DIAL_SM, "NOISE");
	dial_vnoise->bounds(0, 100);
	dial_vnoise->step(1);
	dial_vnoise->color(NstGreen);
	dial_vnoise->selection_color(NstPurple);
	dial_vnoise->callback(cb_volume, 4);
	dial_vnoise->value(conf.audio_vol_noise);

	dial_vdpcm = new Fl_Dial(330, 115, UI_DIAL_SM, UI_DIAL_SM, "DPCM");
	dial_vdpcm->bounds(0, 100);
	dial_vdpcm->step(1);
	dial_vdpcm->color(NstGreen);
	dial_vdpcm->selection_color(NstPurple);
	dial_vdpcm->callback(cb_volume, 5);
	dial_vdpcm->value(conf.audio_vol_dpcm);

	dial_vfds = new Fl_Dial(80, 225, UI_DIAL_SM, UI_DIAL_SM, "FDS");
	dial_vfds->bounds(0, 100);
	dial_vfds->step(1);
	dial_vfds->color(NstGreen);
	dial_vfds->selection_color(NstPurple);
	dial_vfds->callback(cb_volume, 6);
	dial_vfds->value(conf.audio_vol_fds);

	dial_vmmc5 = new Fl_Dial(130, 225, UI_DIAL_SM, UI_DIAL_SM, "MMC5");
	dial_vmmc5->bounds(0, 100);
	dial_vmmc5->step(1);
	dial_vmmc5->color(NstGreen);
	dial_vmmc5->selection_color(NstPurple);
	dial_vmmc5->callback(cb_volume, 7);
	dial_vmmc5->value(conf.audio_vol_mmc5);

	dial_vvrc6 = new Fl_Dial(180, 225, UI_DIAL_SM, UI_DIAL_SM, "VRC6");
	dial_vvrc6->bounds(0, 100);
	dial_vvrc6->step(1);
	dial_vvrc6->color(NstGreen);
	dial_vvrc6->selection_color(NstPurple);
	dial_vvrc6->callback(cb_volume, 8);
	dial_vvrc6->value(conf.audio_vol_vrc6);

	dial_vvrc7 = new Fl_Dial(230, 225, UI_DIAL_SM, UI_DIAL_SM, "VRC7");
	dial_vvrc7->bounds(0, 100);
	dial_vvrc7->step(1);
	dial_vvrc7->color(NstGreen);
	dial_vvrc7->selection_color(NstPurple);
	dial_vvrc7->callback(cb_volume, 9);
	dial_vvrc7->value(conf.audio_vol_vrc7);

	dial_vn163 = new Fl_Dial(280, 225, UI_DIAL_SM, UI_DIAL_SM, "N163");
	dial_vn163->bounds(0, 100);
	dial_vn163->step(1);
	dial_vn163->color(NstGreen);
	dial_vn163->selection_color(NstPurple);
	dial_vn163->callback(cb_volume, 10);
	dial_vn163->value(conf.audio_vol_n163);

	dial_vs5b = new Fl_Dial(330, 225, UI_DIAL_SM, UI_DIAL_SM, "S5B");
	dial_vs5b->bounds(0, 100);
	dial_vs5b->step(1);
	dial_vs5b->color(NstGreen);
	dial_vs5b->selection_color(NstPurple);
	dial_vs5b->callback(cb_volume, 11);
	dial_vs5b->value(conf.audio_vol_s5b);

	atab->end();

	Fl_Group *itab = new Fl_Group(10, 30, 380, 360, "&Input");

	Fl_Button *btn_icfg_u = new Fl_Button(70, 55, 30, UI_ELEMHEIGHT, "U");
	btn_icfg_u->callback(cb_icfg, 0);
	btn_icfg_u->color(NstBlueGrey);
	btn_icfg_u->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_d = new Fl_Button(70, 115, 30, UI_ELEMHEIGHT, "D");
	btn_icfg_d->callback(cb_icfg, 1);
	btn_icfg_d->color(NstBlueGrey);
	btn_icfg_d->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_l = new Fl_Button(30, 85, 30, UI_ELEMHEIGHT, "L");
	btn_icfg_l->callback(cb_icfg, 2);
	btn_icfg_l->color(NstBlueGrey);
	btn_icfg_l->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_r = new Fl_Button(110, 85, 30, UI_ELEMHEIGHT, "R");
	btn_icfg_r->callback(cb_icfg, 3);
	btn_icfg_r->color(NstBlueGrey);
	btn_icfg_r->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_slct = new Fl_Button(150, 85, 60, UI_ELEMHEIGHT, "Select");
	btn_icfg_slct->callback(cb_icfg, 4);
	btn_icfg_slct->color(NstGreen);
	btn_icfg_slct->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_strt = new Fl_Button(220, 85, 60, UI_ELEMHEIGHT, "Start");
	btn_icfg_strt->callback(cb_icfg, 5);
	btn_icfg_strt->color(NstGreen);
	btn_icfg_strt->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_a = new Fl_Button(330, 100, 30, UI_ELEMHEIGHT, "A");
	btn_icfg_a->callback(cb_icfg, 6);
	btn_icfg_a->color(NstRed);
	btn_icfg_a->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_b = new Fl_Button(290, 100, 30, UI_ELEMHEIGHT, "B");
	btn_icfg_b->callback(cb_icfg, 7);
	btn_icfg_b->color(NstRed);
	btn_icfg_b->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_ta = new Fl_Button(330, 65, 30, UI_ELEMHEIGHT, "TA");
	btn_icfg_ta->callback(cb_icfg, 8);
	btn_icfg_ta->color(NstRed);
	btn_icfg_ta->labelcolor(NstLightGrey);

	Fl_Button *btn_icfg_tb = new Fl_Button(290, 65, 30, UI_ELEMHEIGHT, "TB");
	btn_icfg_tb->callback(cb_icfg, 9);
	btn_icfg_tb->color(NstRed);
	btn_icfg_tb->labelcolor(NstLightGrey);

	icfg = new NstInputConfWindow(110, 55, 170, UI_ELEMHEIGHT, "Input Config");
	icfg->color(NstPurple);
	icfg->hide();
	icfg->text = new Fl_Box(0, 0, 0, UI_ELEMHEIGHT);
	icfg->text->align(FL_ALIGN_RIGHT);
	icfg->text->labelcolor(NstLightGrey);
	icfg->player = icfg->btn = icfg->device = 0;
	icfg->end();

	Fl_Choice *ch_player = new Fl_Choice(20, 180, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Player");
	ch_player->align(FL_ALIGN_TOP_LEFT);
	ch_player->add("Player 1");
	ch_player->add("Player 2");
	ch_player->value(0);
	ch_player->callback(cb_player);

	Fl_Choice *ch_idevice = new Fl_Choice(20, 230, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Input Device");
	ch_idevice->align(FL_ALIGN_TOP_LEFT);
	ch_idevice->add("Keyboard");
	ch_idevice->add("Joystick");
	ch_idevice->value(0);
	ch_idevice->callback(cb_idevice);

	Fl_Hor_Value_Slider *sld_turbopulse = new Fl_Hor_Value_Slider(200, 180, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Turbo Pulse");
	sld_turbopulse->align(FL_ALIGN_TOP_LEFT);
	sld_turbopulse->bounds(2, 9);
	sld_turbopulse->box(FL_FLAT_BOX);
	sld_turbopulse->callback(cb_turbopulse);
	sld_turbopulse->step(1);
	sld_turbopulse->selection_color(NstGreen);
	sld_turbopulse->type(FL_HOR_NICE_SLIDER);
	sld_turbopulse->value(conf.timing_turbopulse);

	itab->end();

	Fl_Group *mtab = new Fl_Group(10, 30, 380, 360, "&Misc");

	Fl_Choice *ch_default_system = new Fl_Choice(20, 55, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Default System");
	ch_default_system->align(FL_ALIGN_TOP_LEFT);
	ch_default_system->add("Auto");
	ch_default_system->add("NTSC");
	ch_default_system->add("PAL");
	ch_default_system->add("Famicom");
	ch_default_system->add("Dendy");
	ch_default_system->value(conf.misc_default_system);
	ch_default_system->callback(cb_default_system);

	Fl_Choice *ch_power_state = new Fl_Choice(20, 105, UI_ELEMWIDTH, UI_ELEMHEIGHT, "RAM Power-on State");
	ch_power_state->align(FL_ALIGN_TOP_LEFT);
	ch_power_state->add("0x00");
	ch_power_state->add("0xFF");
	ch_power_state->add("Random");
	ch_power_state->value(conf.misc_power_state);
	ch_power_state->callback(cb_power_state);

	Fl_Hor_Value_Slider *sld_ffspeed = new Fl_Hor_Value_Slider(20, 160, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Fast-Forward Speed");
	sld_ffspeed->align(FL_ALIGN_TOP_LEFT);
	sld_ffspeed->bounds(1, 8);
	sld_ffspeed->box(FL_FLAT_BOX);
	sld_ffspeed->callback(cb_ffspeed);
	sld_ffspeed->step(1);
	sld_ffspeed->selection_color(NstGreen);
	sld_ffspeed->type(FL_HOR_NICE_SLIDER);
	sld_ffspeed->value(conf.timing_ffspeed);

	Fl_Check_Button *chk_soft_patching = new Fl_Check_Button(200, 55, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Auto Soft Patching");
	chk_soft_patching->value(conf.misc_soft_patching);
	chk_soft_patching->callback(cb_soft_patching);

	Fl_Check_Button *chk_genie_distortion = new Fl_Check_Button(200, 80, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Genie Sound Distortion");
	chk_genie_distortion->value(conf.misc_genie_distortion);
	chk_genie_distortion->callback(cb_genie_distortion);

	Fl_Check_Button *chk_disable_cursor = new Fl_Check_Button(200, 105, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Disable Cursor");
	chk_disable_cursor->value(conf.misc_disable_cursor);
	chk_disable_cursor->callback(cb_disable_cursor);

	Fl_Check_Button *chk_disable_cursor_special = new Fl_Check_Button(200, 130, UI_ELEMWIDTH, UI_ELEMHEIGHT, "Disable Special Cursor");
	chk_disable_cursor_special->value(conf.misc_disable_cursor_special);
	chk_disable_cursor_special->callback(cb_disable_cursor_special);

	mtab->end();

	tabs->end();

	Fl_Button *btn_ok = new Fl_Button(350, 370, 40, UI_ELEMHEIGHT, "&OK");
	btn_ok->callback(cb_ok, 0);
	this->end();
}
