/* > xrot.h
 * header for caverot plot & translate routines for Unix + X
 * Copyright (C) 1997 Olly Betts
 */

#if 0
# include "grx20.h"
# define cleardevice() GrClearContext(0)
/* !HACK! 15 and 0 in next line are colours */
# define outtextxy(X, Y, S) GrTextXY((X), (Y), (S), 15, 0)
# define set_tcolour(X) /* !!HACK!! */
# define set_gcolour(X) /* !!HACK!! */
# define text_xy(X, Y, S) outtextxy(12 + (X) * 12, 12 + (Y) * 12, S)
# define Y_UP
#endif

typedef int coord;  /* data type used after data is read in */

#define COORD_MAX   INT_MAX
#define COORD_MIN   INT_MIN

#define FIXED_PT_POS     16 /* position of fixed decimal point in word */

#define RED_3D           (9)  /* colours for 3d view effect */
#define GREEN_3D         (8)
#define BLUE_3D          (6)

#define RETURN_KEY      (13)  /* key definitions */
#define ESCAPE_KEY      (27)
/*#if 0*/
#define DELETE_KEY   (0x153)  /* enhanced DOS key codes (+ 0x100) */
#define COPYEND_KEY  (0x14f)  /* to distinguish them from normal key codes */
#define CURSOR_LEFT  (0x14b)
#define CURSOR_RIGHT (0x14d)
#define CURSOR_DOWN  (0x150)
#define CURSOR_UP    (0x148)
/*#endif*/

#define NO_FUNC_PTRS
#ifdef NO_FUNC_PTRS

/* really plebby version (needed if fn pointers won't fit in a coord) */
# define MOVE (coord)1
# define DRAW (coord)2
# define STOP (coord)0

#elif defined(HAVE_SETJMP)

/* for speed, store function ptrs in table */

# define MOVE (coord)(moveto)
# define DRAW (coord)(lineto)
# define STOP (coord)(stop)

/* prototype so STOP macro will work */
extern void stop(int X, int Y);

#else

/* for speed, store function ptrs in table */

# define MOVE (coord)(moveto)
# define DRAW (coord)(lineto)
# define STOP (coord)NULL

#endif
