/* osalloc.h
 * Function prototypes for OS dep. malloc &c - funcs in error.c
 * Copyright (C) 1996,1997,2001 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef OSALLOC_H /* only include once */
#define OSALLOC_H

/* define TOMBSTONES to enable tombstones on malloc blocks
 * for bounds checking */
/*#define TOMBSTONES 1*/

#include <stdlib.h>

#include "osdepend.h"

/* OSSIZE_T is to osmalloc, etc what size_t is to malloc, etc */
#ifdef HAVE_FAR_POINTERS
# include "alloc.h"
# define osfree(p) farfree((p))
# define xosmalloc(s) farmalloc((s))
# define xosrealloc(p, s) farrealloc((p), (s))
# define OSSIZE_T long
#else
# ifndef TOMBSTONES
#  define osfree(p) free((p))
#  define xosmalloc(s) malloc((s))
#  define xosrealloc(p, s) realloc((p), (s))
# else
void osfree(void *p);
/* ick: */
#  define xosmalloc(s) osmalloc((s))
#  define xosrealloc(p, s) osrealloc((p), (s))
# endif
# define OSSIZE_T size_t
#endif

/* NB No extra () around X as sizeof((char*)) doesn't work */
#define ossizeof(X) ((OSSIZE_T)sizeof(X))
/* C++ like malloc thingy -- call osnew(<type>) eg. osnew(point) */
#define osnew(T) (T*)osmalloc(ossizeof(T))

void FAR *osmalloc(OSSIZE_T);
void FAR *osrealloc(void *, OSSIZE_T);
char FAR *osstrdup(const char *str);

#endif
