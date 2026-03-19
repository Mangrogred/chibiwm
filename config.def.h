// Mod1Mask - Alt
// Mod2Mask - Numlock
// Mod3Mask - Shift
// Mod4Mask - Super (aka win button)
// Mod5Mask - Shift
#define MODMASK Mod4Mask

#define BORDER_SIZE 4
#define BORDER_INACTIVE_COLOR 0x444444
#define BORDER_ACTIVE_COLOR 0x4444AA

#define WORKSPACES 6 

//example:			x("button", MaskForAdditionalButton or 0,		function in c)
#define TBL(x)  	x("q", 0, 			XKillClient(d, e.xkey.subwindow))	\
					x("q", ShiftMask, 	running = false)					\
					x("e", 0, 			system("xterm &"))					\
					x("d", 0, 			system("rofi -show run &")) 		\
					x("1", 0, 			switch_ws(d, 0))					\
					x("2", 0, 			switch_ws(d, 1))					\
					x("3", 0, 			switch_ws(d, 2))					\
					x("4", 0, 			switch_ws(d, 3))					\
					x("5", 0, 			switch_ws(d, 4))					\
					x("6", 0, 			switch_ws(d, 5))					\
					x("1", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 0)) \
					x("2", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 1)) \
					x("3", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 2)) \
					x("4", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 3)) \
					x("5", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 4)) \
					x("6", ShiftMask, 	move_to_ws(d, e.xkey.subwindow, 5)) \

