/* This code has parts of code from:
 * https://github.com/lslvr/1wm 
 * https://git.suckless.org/dwm
 * https://github.com/mackstann/tinywm		
 * https://jichu4n.com/posts/how-x-window-managers-work-and-how-to-write-one-part-i/
 * Thanks to everyone
*/

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"

#define max(a, b) 		((a) > (b) ? (a) : (b))
#define stk(s)			XKeysymToKeycode(d, XStringToKeysym(s))
#define on(ev, x)		if (e.type == ev) { x; } 
#define keys(k, mod, _)	XGrabKey(d, stk(k), MODMASK | mod, r, 1, 1, 1);
#define map(k, mod, x)	if (e.xkey.keycode == stk(k) && (e.xkey.state & ~Mod2Mask) == (MODMASK | mod)) { x; }

bool running = true;
Window ws[WORKSPACES][256];
int ws_count[WORKSPACES] = {0};
int current_ws = 0;
Display *d;
XWindowAttributes attr;
XButtonEvent start;
XEvent e;
Window r;
Window focused = None;
Window bar;
Pixmap bar_buf;
GC gc;
XftFont *font;

int xerror(Display *d, XErrorEvent *e);
void configurerequest(XEvent *e);
void buttonpress(XEvent *e);
void motionnotify(XEvent *e);
void maprequest(XEvent *e);
void unmapnotify(XEvent *e);

void create_bar(void);
void draw_bar(void);
void setfocus(Window w);
void switch_ws(Display *d, int new_ws);
void move_to_ws(Display *d, Window win, int new_ws);

int main(void)
{
	d = XOpenDisplay(0); 
	r = DefaultRootWindow(d); 
	XSelectInput(d, r, SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask);
	TBL(keys);
	XGrabButton(d, 1, MODMASK, r, 0, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(d, 3, MODMASK, r, 0, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(d, 1, 0, r, 0, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
	XSetErrorHandler(xerror); 
	create_bar();
	draw_bar();

	while (running && !XNextEvent (d, &e)) {
		on(ConfigureRequest,	configurerequest(&e))
		on(ButtonPress,			buttonpress(&e))
		on(EnterNotify,			setfocus(e.xcrossing.window))
		on(MotionNotify,		motionnotify(&e))
        on(MapRequest,			maprequest(&e))
		on(UnmapNotify,			unmapnotify(&e))
		on(ButtonRelease,		start.subwindow = None)
		on(KeyPress,			TBL(map))
    }
	return 0;
}

void configurerequest(XEvent *e) {
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XMoveResizeWindow(d, ev->window, 0, BAR_SIZE, ev->width, ev->height); // TODO: ADD TILING
}

void buttonpress(XEvent *e) {
	XButtonPressedEvent *ev = &e->xbutton;
	if (ev->subwindow != None) {
		setfocus(ev->subwindow);
		XRaiseWindow(d, ev->subwindow);
		if (ev->subwindow != None && ev->state & MODMASK) { 
			XSetInputFocus(d, ev->subwindow, RevertToParent, CurrentTime);
			XGetWindowAttributes(d, ev->subwindow, &attr);
			if (!attr.override_redirect) 
				start = *ev;
		}
	}
	XAllowEvents(d, ReplayPointer, CurrentTime);
}

void motionnotify(XEvent *e) {
	XButtonPressedEvent *ev = &e->xbutton;
	if (start.subwindow != None) {
		int xdiff = ev->x_root - start.x_root;
		int ydiff = ev->y_root - start.y_root;
		XMoveResizeWindow(d, start.subwindow,
			attr.x + (start.button==1 ? xdiff : 0),
			attr.y + (start.button==1 ? ydiff : 0),
			max(1, attr.width + (start.button==3 ? xdiff : 0)),
			max(1, attr.height + (start.button==3 ? ydiff : 0)));
	}
}

void maprequest(XEvent *e) {
	XMapRequestEvent *ev = &e->xmaprequest;
	XGetWindowAttributes(d, ev->window, &attr);
	if (!attr.override_redirect) {
		bool found = false;
		for (int w = 0; w < WORKSPACES; w++)
			for (int i = 0; i < ws_count[w]; i++)
				if (ws[w][i] == ev->window) found = true;
		if (!found)
			ws[current_ws][ws_count[current_ws]++] = ev->window;
		XSelectInput(d, ev->window, EnterWindowMask);
		XSetWindowBorderWidth(d, ev->window, BORDER_SIZE);
		XSetWindowBorder(d, ev->window, BORDER_INACTIVE_COLOR);
	}
	XMapWindow(d, ev->window);
	setfocus(ev->window);
}

void unmapnotify(XEvent *e) {
	XUnmapEvent *ev = &e->xunmap;
	if (ev->send_event) {
		for (int w = 0; w < WORKSPACES; w++) {
			for (int i = 0; i < ws_count[w]; i++) {
				if (ws[w][i] == ev->window) {
					ws[w][i] = ws[w][--ws_count[w]];
					break;
				}
			}
		}
	}
}

int xerror(Display *d, XErrorEvent *e) {
	return 0; 
}

void create_bar(void) {
	bar = XCreateSimpleWindow(d, r, 0, 0, DisplayWidth(d, 0), BAR_SIZE, 0, 0, BAR_BACKGROUND_COLOR);

	XSetWindowAttributes wa;
	wa.override_redirect = True;

	XChangeWindowAttributes(d, bar, CWOverrideRedirect, &wa);
	XSelectInput(d, bar, ExposureMask);
	XMapWindow(d, bar);
	XSetWindowBorderWidth(d, bar, 0);

	bar_buf = XCreatePixmap(d, r, DisplayWidth(d, 0), BAR_SIZE, DefaultDepth(d, 0));
	gc = XCreateGC(d, bar, 0, NULL);
	font = XftFontOpenName(d, DefaultScreen(d), BAR_FONT);
}

void draw_bar(void) {
	XSetForeground(d, gc, BAR_BACKGROUND_COLOR);
	XFillRectangle(d, bar_buf, gc, 0, 0, DisplayWidth(d, 0), BAR_SIZE);

	XftDraw *xdraw = XftDrawCreate(d, bar_buf, DefaultVisual(d, 0), DefaultColormap(d, 0));

	XftColor col;
	for (int i = 0; i < WORKSPACES; i++) {
		XftColorAllocName(d, DefaultVisual(d, 0), DefaultColormap(d, 0),
			i == current_ws ? BAR_ACTIVE_WS_COLOR : BAR_INACTIVE_WS_COLOR, &col);
		char label[2];
		snprintf(label, sizeof(label), "%d", i + 1);
		XftDrawStringUtf8(xdraw, &col, font, 8 + i * (BAR_FONT_SIZE * 2), (BAR_SIZE / 2) + (BAR_FONT_SIZE / 2), (FcChar8*)label, 1);
		XftColorFree(d, DefaultVisual(d, 0), DefaultColormap(d, 0), &col);
	}

	XCopyArea(d, bar_buf, bar, gc, 0, 0, DisplayWidth(d, 0), BAR_SIZE, 0, 0);
	XFlush(d);
}

void setfocus(Window w) {
	if (w == None || w == r) return;
	if (focused != None && focused != w)
		XSetWindowBorder(d, focused, BORDER_INACTIVE_COLOR);
	focused = w;
	XSetInputFocus(d, focused, RevertToParent, CurrentTime);
	XSetWindowBorder(d, focused, BORDER_ACTIVE_COLOR);
}

void switch_ws(Display *d, int new_ws) {
	if (new_ws == current_ws) return;
	for (int i = 0; i < ws_count[current_ws]; i++)
		XUnmapWindow(d, ws[current_ws][i]);
	current_ws = new_ws;
	for (int i = 0; i < ws_count[current_ws]; i++)
		XMapWindow(d, ws[current_ws][i]);
	draw_bar();
	focused = None;
}

void move_to_ws(Display *d, Window win, int new_ws) {
	if (new_ws == current_ws || win == None) return;
	for (int i = 0; i < ws_count[current_ws]; i++) {
		if (ws[current_ws][i] == win) {
			ws[current_ws][i] = ws[current_ws][--ws_count[current_ws]];
			break;
		}
	}
	ws[new_ws][ws_count[new_ws]++] = win;
	XUnmapWindow(d, win);
	focused = None;
}


/* TODO:
 * 1. add tiling
 * 2. add status bar
 * 3. add logs to xerror
 * 4. cheese
*/
