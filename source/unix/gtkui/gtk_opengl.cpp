/*******************************************************************************
 * Copyright (C) 2010, 2014 Carlos Pereira, Javier Cabezas
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gtk_opengl.h"

/**************************************************
 * GTK_OPENGL_QUERY: check if OpenGL is supported *
 **************************************************/

int gtk_opengl_query (void)
{
Display *xdisplay;

/*
 * 1) get default X display;
 * 2) check if OpenGL is supported;
 * 3) return the result as TRUE or FALSE.
 */

xdisplay = gdk_x11_get_default_xdisplay ();
if (xdisplay == NULL || glXQueryExtension (xdisplay,
NULL, NULL) == FALSE) return FALSE;

return TRUE;
}

/*****************************************************************
 * GTK_OPENGL_CREATE: create OpenGL context for GTK drawing area *
 *****************************************************************/

/*************************************************
 * QUESTION: to prevent memory leaks, do we      *
 * need to free something more on this function? *
 *************************************************/

GLXContext gtk_opengl_create (GtkWidget *area,
int *attributes, GLXContext context, int direct)
{
GdkVisual *visual;
GdkScreen *screen;
XVisualInfo *xvisual;
Display *xdisplay;
int xscreen;

/*
 * 1) disable GTK double buffer for drawing area;
 * 2) get default X display;
 * 3) get X and GDK screen;
 * 4) get X and GDK visual;
 * 5) set GDK visual
 */

gtk_widget_set_double_buffered (area, FALSE);
xdisplay = gdk_x11_get_default_xdisplay ();
xscreen = DefaultScreen (xdisplay);
screen = gdk_screen_get_default ();
xvisual = glXChooseVisual (xdisplay, xscreen, attributes);
visual = gdk_x11_screen_lookup_visual (screen, xvisual->visualid);
gtk_widget_set_visual (area, visual);

/*
 * 1) first OpenGL area (context = NULL): create suitable GL
 * context or return NULL
 * 2) other OpenGL areas (context != NULL): make sure the supplied
 * context exists at this stage! typically get it from a window
 * that exists now, instead of using the context of the first
 * window created, which may no longer exist at this stage!
 * 3) use direct rendering (TRUE) or rendering through X (FALSE)
 */

return glXCreateContext (xdisplay, xvisual, context, direct);
}

/***************************************************************
 * GTK_OPENGL_REMOVE: free OpenGL context for GTK drawing area *
 ***************************************************************/

/*************************************************
 * QUESTION: to prevent memory leaks, do we      *
 * need to free something more on this function? *
 *************************************************/

void gtk_opengl_remove (GtkWidget *area, GLXContext context)
{
GdkWindow *drawable;
GdkDisplay *display;
Display *xdisplay;

/*
 * 1) get drawable from the GTK drawing area;
 * 2) get X display from the drawable;
 */

drawable = gtk_widget_get_window (area);
display = gdk_window_get_display (drawable);
xdisplay = gdk_x11_display_get_xdisplay (display);

/*
 * 1) remove OpenGL context
 */

glXDestroyContext (xdisplay, context);
}

/**************************************************
 * GTK_OPENGL_CURRENT: set current OpenGL context *
 **************************************************/

int gtk_opengl_current (GtkWidget *area, GLXContext context)
{
GdkWindow *drawable;
GdkDisplay *display;
Display *xdisplay;
Window xid;

/*
 * 1) get drawable from the GTK drawing area;
 * 2) get X display and X window id from the drawable;
 * 3) use X display and X window id to connect with a
 * OpenGL context, to set the current OpenGL rendering context;
 * 4) return TRUE if successful, FALSE otherwise, in which
 * case the previous OpenGL rendering context remains active.
 */

drawable = gtk_widget_get_window (area);
display = gdk_window_get_display (drawable);
xdisplay = gdk_x11_display_get_xdisplay (display);
xid = gdk_x11_window_get_xid (drawable);

return glXMakeCurrent (xdisplay, xid, context);
}

/*******************************************************
 * GTK_OPENGL_SWAP: swap front and back OpenGL buffers *
 *******************************************************/

void gtk_opengl_swap (GtkWidget *area)
{
GdkWindow *drawable;
GdkDisplay *display;
Display *xdisplay;
Window xid;

/*
 * 1) get drawable from the GTK drawing area;
 * 2) get X display and X window id from the drawable;
 * 3) use X display and X window id to exchange front
 * and back OpenGL buffers.
 */

drawable = gtk_widget_get_window (area);
display = gdk_window_get_display (drawable);
xdisplay = gdk_x11_display_get_xdisplay (display);
xid = gdk_x11_window_get_xid (drawable);

glXSwapBuffers (xdisplay, xid);
}

/**********************************************
 * GTK_OPENGL_WAIT_GL: fflush OpenGL commands *
 **********************************************/

void gtk_opengl_wait_gl (void)
{
/*
 * 1) wait for completion of all OpenGL tasks before proceding
 */

glXWaitGL();
}

/****************************************
 * GTK_OPENGL_WAIT_X: fflush X commands *
 ****************************************/

void gtk_opengl_wait_x (void)
{
/*
 * 1) wait for completion of all X tasks before proceding
 */

glXWaitX();
}
