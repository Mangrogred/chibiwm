/* This code has parts of code from:
 * https://github.com/lslvr/1wm 
 * https://git.suckless.org/dwm
 * https://github.com/mackstann/tinywm		
 * https://jichu4n.com/posts/how-x-window-managers-work-and-how-to-write-one-part-i/
 * Thanks to everyone
*/

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

#define max(a, b) 		((a) > (b) ? (a) : (b))
#define stk(s)			XKeysymToKeycode(d, XStringToKeysym(s))
#define on(ev, x)		if (e.type == ev) { x; } 
#define keys(k, mod, _)	XGrabKey(d, stk(k), MODMASK | mod, r, 1, 1, 1);
#define button(b, mod)  XGrabButton(d, b, mod, r, 0, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
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

int xerror(Display *d, XErrorEvent *e);
void configurerequest(XEvent *e);
void buttonpress(XEvent *e);
void motionnotify(XEvent *e);
void maprequest(XEvent *e);
void unmapnotify(XEvent *e);

void setfocus(Window w);
void switch_ws(Display *d, int new_ws);
void move_to_ws(Display *d, Window win, int new_ws);

int main(void)
{
	d = XOpenDisplay(0); 
	r = DefaultRootWindow(d); 
	XSelectInput(d, r, SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask);
	TBL(keys);
	button(AnyButton, MODMASK);
	button(1, 0);
	XSetErrorHandler(xerror); 

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
	XMoveResizeWindow(d, ev->window, 0, 0, ev->width, ev->height);
}

void buttonpress(XEvent *e) {
	XButtonPressedEvent *ev = &e->xbutton;
	setfocus(ev->subwindow);
	if (ev->subwindow != None && ev->state & MODMASK) { 
		XRaiseWindow(d, ev->subwindow);
		XSetInputFocus(d, ev->subwindow, RevertToParent, CurrentTime);
		XGetWindowAttributes(d, ev->subwindow, &attr);
		if (!attr.override_redirect) 
			start = *ev;
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

void setfocus(Window w) {
	if (w == None || w == r) return;
	if (focused != None && focused != w)
		XSetWindowBorder(d, focused, BORDER_INACTIVE_COLOR);
	focused = w;
	XSetInputFocus(d, focused, RevertToParent, CurrentTime);
	XSetWindowBorder(d, focused, BORDER_ACTIVE_COLOR);
}

void switch_ws(Display *d, int new_ws) {
	for (int i = 0; i < ws_count[current_ws]; i++)
		XUnmapWindow(d, ws[current_ws][i]);
	current_ws = new_ws;
	for (int i = 0; i < ws_count[current_ws]; i++)
		XMapWindow(d, ws[current_ws][i]);
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
}


/* TODO:
 * 1. add tiling
 * 2. add status bar
 * 3. add logs to xerror
 * 4. cheese
*/
