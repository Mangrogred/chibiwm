// Mod1Mask - Alt
// Mod2Mask - Numlock
// Mod3Mask - Shift
// Mod4Mask - Super (aka win button)
// Mod5Mask - Shift
#define MODMASK Mod4Mask

#define BORDER_SIZE 2
#define BORDER_COLOR 0x333333

#define WORKSPACES 4 

//example:			x("button", MaskForAdditionalButton or 0,		function in c)
#define TBL(x)  	x("n", 0, 			XCirculateSubwindowsUp(d, r); XSetInputFocus(d, e.xkey.window, 2, 0)) \
                	x("q", 0, 			XKillClient(d, e.xkey.subwindow)) \
					x("q", ShiftMask, 	running = false) \
                	x("e", 0, 			system("xterm &")) \
                	x("d", 0, 			system("dmenu &")) \
					x("1", 0, 			switch_ws(d, 0)) \
					x("2", 0, 			switch_ws(d, 1)) \
					x("3", 0, 			switch_ws(d, 2)) \
					x("4", 0, 			switch_ws(d, 3)) \
					x("1", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 0)) \
					x("2", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 1)) \
					x("3", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 2)) \
					x("4", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 3)) 

