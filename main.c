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

#define stk(s)      XKeysymToKeycode(d, XStringToKeysym(s))
#define on(ev, ...)   if (e.type == ev) { __VA_ARGS__; } // if statements works poor with {x}, so {__VA_ARGS__;} is better in this situation (but lsp of nvim is fucking crying abt if() inside on())
#define keys(k, mod, _)  XGrabKey(d, stk(k), MODMASK | mod, r, 1, 1, 1);
#define map(k, mod, x)   if (e.xkey.keycode == stk(k) && (e.xkey.state & ~Mod2Mask) == (MODMASK | mod)) { x; }
#define max(a, b) ((a) > (b) ? (a) : (b))

bool running = true;
Window ws[WORKSPACES][256];
int ws_count[WORKSPACES] = {0};
int current_ws = 0;

void switch_ws(Display *d, int new_ws);
void move_to_ws(Display *d, Window win, int new_ws);
int xerror(Display *d, XErrorEvent *e);

int main(void)
{
	Display *d = XOpenDisplay(0); Window r = DefaultRootWindow(d); XEvent e; XWindowAttributes attr; XButtonEvent start;
	XSelectInput(d, r, SubstructureRedirectMask | SubstructureNotifyMask);
	TBL(keys);
	XGrabButton(d, AnyButton, MODMASK, r, 0, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XSetErrorHandler(xerror); // fix crashes

	while (running && !XNextEvent (d, &e)) {
		on(ConfigureRequest, XMoveResizeWindow(d, e.xconfigurerequest.window, 0, 0, e.xconfigurerequest.width, e.xconfigurerequest.height))
		on(ButtonPress,
			if (e.xbutton.subwindow != None) { 
				XRaiseWindow(d, e.xbutton.subwindow);
				XSetInputFocus(d, e.xbutton.subwindow, RevertToParent, CurrentTime);
				XGetWindowAttributes(d, e.xbutton.subwindow, &attr);
				if (!attr.override_redirect) 
					start = e.xbutton;
			}
			XAllowEvents(d, ReplayPointer, CurrentTime);
		)
		on(MotionNotify,
			if (start.subwindow != None) {
				int xdiff = e.xbutton.x_root - start.x_root;
				int ydiff = e.xbutton.y_root - start.y_root;
				XMoveResizeWindow(d, start.subwindow,
					attr.x + (start.button==1 ? xdiff : 0),
					attr.y + (start.button==1 ? ydiff : 0),
					max(1, attr.width + (start.button==3 ? xdiff : 0)),
					max(1, attr.height + (start.button==3 ? ydiff : 0)));
			}
		)
        on(MapRequest, 
				XGetWindowAttributes(d, e.xmaprequest.window, &attr);
				if (!attr.override_redirect) {
					bool found = false;
					for (int w = 0; w < WORKSPACES; w++)
						for (int i = 0; i < ws_count[w]; i++)
							if (ws[w][i] == e.xmaprequest.window) found = true;
					if (!found)
						ws[current_ws][ws_count[current_ws]++] = e.xmaprequest.window;
					XSelectInput(d, e.xmaprequest.window, EnterWindowMask);
					XSetWindowBorderWidth(d, e.xmaprequest.window, BORDER_SIZE);
					XSetWindowBorder(d, e.xmaprequest.window, BORDER_COLOR);
				}
				XMapWindow(d, e.xmaprequest.window);
		)
		on(UnmapNotify,
			if (e.xunmap.send_event) {
				for (int w = 0; w < WORKSPACES; w++) {
					for (int i = 0; i < ws_count[w]; i++) {
						if (ws[w][i] == e.xunmap.window) {
							ws[w][i] = ws[w][--ws_count[w]];
							break;
						}
					}
				}
			}
		)
		on(ButtonRelease, start.subwindow = None)
		on(KeyPress, TBL(map))
    }
	return 0;
}

int xerror(Display *d, XErrorEvent *e) {
	return 0; 
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
 * 1. add spawn function and maybe more like in dwm
 * 2. add tiling
 * 3. add status bar
 * 4. add logs to xerror
 * 5. cheese
*/
