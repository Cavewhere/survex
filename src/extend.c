/* > extend.c
 * Produce an extended elevation
 * Copyright (C) 1995,1996,1997,1998 Olly Betts
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "useful.h"
#include "filename.h"
#include "message.h"
#include "filelist.h"
#include "img.h"

typedef struct POINT {
   float x, y, z;
   char *label;
   struct POINT *next;
} point;

typedef struct LEG {
   point *fr, *to;
   struct LEG *next;
   int fDone;
} leg;

static point headpoint = {0, 0, 0, NULL, NULL};
static leg headleg = {NULL, NULL, NULL, 1};

static float zMax = -99999999;
static point *start = NULL;

static img *pimg;

static void do_stn(point *, float);

static point *
find_point(float x, float y, float z)
{
   point *p;
   for (p = headpoint.next; p != NULL; p = p->next)
      if (x == p->x && y == p->y && z == p->z) return p;

   p = osmalloc(ossizeof(point));
   p->x = x;
   p->y = y;
   p->z = z;
   p->label = "<none>";
   if (z > zMax) {
      zMax = z;
      start = p;
   }
   p->next = headpoint.next;
   headpoint.next = p;
   return p;
}

static void
add_leg(point *fr, point *to)
{
   leg *p;
   p = osmalloc(ossizeof(leg));
   p->fr = fr;
   p->to = to;
   p->next = headleg.next;
   p->fDone = 0;
   headleg.next = p;
}

static void
add_label(point *p, const char *label)
{
   p->label = osstrdup(label);
}

int
main(int argc, char **argv)
{
   const char *fnmData = argv[1], *fnmOutput;
   char szDesc[256];
   float x, y, z;
   char sz[256];
   int result;
   point *fr, *to;
   fr = NULL;

   ReadErrorFile(argv[0]);

   if (argc < 2 || argc > 3) {
      fprintf(stderr, "Syntax: extend <input .3d file> [<output .3d file>]\n");
      exit(1);
   }

   fnmOutput = (argc == 2 ? AddExt("extend", EXT_SVX_3D) : argv[2]);

   /* try to open image file, and check it has correct header */
   pimg = img_open(fnmData, szDesc, NULL);
   if (pimg == NULL) fatalerror(img_error(), fnmData);

   putnl();
   puts(msg(/*Reading in data - please wait...*/105));

   do {
      result = img_read_datum(pimg, sz, &x, &y, &z);
      switch (result) {
      case img_BAD:
         puts(msg(/*Bad 3d image file '%s'*/106));
         break;
      case img_MOVE:
         fr = find_point(x, y, z);
         break;
      case img_LINE:
       	 if (!fr) {
       	    printf("img_LINE before any img_MOVE\n");
       	    exit(1);
       	 }
         to = find_point(x, y, z);
         add_leg(fr, to);
         fr = to;
         break;
      case img_CROSS:
         /* Use labels to position crosses too - newer format .3d files
          * don't have crosses in */
         break;
      case img_LABEL:
         fr = find_point(x, y, z);
         add_label(fr, sz);
         break;
      }
   } while (result != img_BAD && result != img_STOP);

   img_close(pimg);

   strcat(szDesc, " (extended)");
   pimg = img_open_write(fnmOutput, szDesc, fFalse);/* text file for now*/

   do_stn(start, 0.0f); /* only does highest connected component currently */
   img_close(pimg);
   
   return EXIT_SUCCESS;
}

static void
do_stn(point *p, float X)
{
   leg *l, *lp;
   float dX;
   img_write_datum(pimg, img_LABEL, p->label, X, 0, p->z);
   l = &headleg;
   while (lp = l, (l = l->next) != NULL) {
      if (!l->fDone) { /* otherwise we extend loops multiple times */
         if (l->to == p) {
            lp->next = l->next; /* unlink FIXME ought to free memory too */
            dX = (float)sqrt(sqrd(l->fr->x - l->to->x) + 
			     sqrd(l->fr->y - l->to->y));
            img_write_datum(pimg, img_MOVE, NULL, X + dX, 0, l->fr->z);
            img_write_datum(pimg, img_LINE, NULL, X, 0, l->to->z);
            l->fDone = 1;
            do_stn(l->fr, X + dX);
         } else if (l->fr == p) {
            lp->next = l->next; /* unlink FIXME ought to free memory too */
            dX = (float)sqrt(sqrd(l->fr->x - l->to->x) +
			     sqrd(l->fr->y - l->to->y));
            img_write_datum(pimg, img_MOVE, NULL, X, 0, l->fr->z);
            img_write_datum(pimg, img_LINE, NULL, X + dX, 0, l->to->z);
            l->fDone = 1;
            do_stn(l->to, X + dX);
         }
      }
   }
}
