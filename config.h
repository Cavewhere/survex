/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

#ifndef CONFIG_H
#define CONFIG_H

/* Copyright Message for Aven */
#define AVEN_COPYRIGHT_MSG "Copyright (C) 1999-2003,2005,2006 Mark R. Shinwell"

/* Copyright Message for Aven in UTF-8 */
#define AVEN_COPYRIGHT_MSG_UTF8 "Copyright © 1999-2003,2005,2006 Mark R. Shinwell"

/* For old systems which don't define it themselves */
/* #undef CLOCKS_PER_SEC */

/* Version number of package (comma-separated) */
#define COMMAVERSION 1,2,15

/* Copyright Message */
#define COPYRIGHT_MSG "Copyright (C) 1990-2014 Olly Betts"

/* Copyright Message in UTF-8 */
#define COPYRIGHT_MSG_UTF8 "Copyright © 1990-2014 Olly Betts"

/* Location of platform independent support files */
#define DATADIR "/usr/local/share"

/* Define to 1 if you have the `avcodec_encode_video2' function. */
/* #undef HAVE_AVCODEC_ENCODE_VIDEO2 */

/* Define to 1 if you have the `avcodec_free_frame' function. */
/* #undef HAVE_AVCODEC_FREE_FRAME */

/* Define to 1 if you have the `avcodec_open2' function. */
/* #undef HAVE_AVCODEC_OPEN2 */

/* Define to 1 if you have the `avformat_new_stream' function. */
/* #undef HAVE_AVFORMAT_NEW_STREAM */

/* Define to 1 if you have the `avformat_write_header' function. */
/* #undef HAVE_AVFORMAT_WRITE_HEADER */

/* Define to 1 if you have the `avio_close' function. */
/* #undef HAVE_AVIO_CLOSE */

/* Define to 1 if you have the `avio_open' function. */
/* #undef HAVE_AVIO_OPEN */

/* Define to 1 if you have the `av_frame_alloc' function. */
/* #undef HAVE_AV_FRAME_ALLOC */

/* Define to 1 if you have the `av_frame_free' function. */
/* #undef HAVE_AV_FRAME_FREE */

/* Define to 1 if you have the `av_guess_format' function. */
/* #undef HAVE_AV_GUESS_FORMAT */

/* Define to 1 if you have the declaration of `AVMEDIA_TYPE_VIDEO', and to 0
   if you don't. */
/* #undef HAVE_DECL_AVMEDIA_TYPE_VIDEO */

/* Define to 1 if you have the declaration of `AV_CODEC_ID_NONE', and to 0 if
   you don't. */
/* #undef HAVE_DECL_AV_CODEC_ID_NONE */

/* Define to 1 if you have the declaration of `AV_PIX_FMT_YUV420P', and to 0
   if you don't. */
/* #undef HAVE_DECL_AV_PIX_FMT_YUV420P */

/* Define to 1 if you have the declaration of `lround', and to 0 if you don't.
   */
#define HAVE_DECL_LROUND 1

/* Define to 1 if you have the `difftime' function. */
#define HAVE_DIFFTIME 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the `getc_unlocked' function. */
//#define HAVE_GETC_UNLOCKED 1

/* Define to 1 if you have the `getpwuid' function. */
#define HAVE_GETPWUID 1

/* Define to 1 if you have the <GL/glext.h> header file. */
/* #undef HAVE_GL_GLEXT_H */

/* Define to 1 if you have the <GL/gl.h> header file. */
/* #undef HAVE_GL_GL_H */

/* Define to 1 if you have the `hypot' function. */
#define HAVE_HYPOT 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libavformat/avformat.h> header file. */
/* #undef HAVE_LIBAVFORMAT_AVFORMAT_H */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `lround' function. */
//#define HAVE_LROUND 1

/* Define to 1 if you have the `lstat' function. */
//#define HAVE_LSTAT 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
#define HAVE_MMAP 1

/* Define to 1 if you have the <OpenGL/glext.h> header file. */
/* #undef HAVE_OPENGL_GLEXT_H */

/* Define to 1 if you have the <OpenGL/gl.h> header file. */
#define HAVE_OPENGL_GL_H 1

/* Define to 1 if you have the `popen' function. */
#define HAVE_POPEN 1

/* Define to 1 if you have the `putc_unlocked' function. */
//#define HAVE_PUTC_UNLOCKED 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the <setjmp.h> header file. */
#define HAVE_SETJMP_H 1

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
//#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Name of package */
#define PACKAGE "survex"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://trac.survex.com/"

/* Define to the full name of this package. */
#define PACKAGE_NAME "survex"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "survex 1.2.15"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "survex"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.2.15"

/* Name of package (capitalised) */
#define PRETTYPACKAGE "Survex"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* The size of `wxChar', as computed by sizeof. */
#define SIZEOF_WXCHAR 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Version number of package */
#define VERSION "1.2.15"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* strcasecmp define for systems which call it something else */
/* #undef strcasecmp */

/* Use getc_unlocked() and putc_unlocked() where available, since they are
 * faster, and we don't multithread file accesses.
 */

#ifdef HAVE_GETC_UNLOCKED
# define GETC(F) getc_unlocked(F)
#else
# define GETC(F) getc(F)
#endif
#ifdef HAVE_PUTC_UNLOCKED
# define PUTC(C, F) putc_unlocked(C, F)
#else
# define PUTC(C, F) putc(C, F)
#endif

/* portable case insensitive string compare */
#if defined(strcasecmp) || defined(HAVE_STRCASECMP)
# define my_strcasecmp strcasecmp
#else
#include <ctype.h>
static int my_strcasecmp(const char *s1, const char *s2) {
   unsigned char c1, c2;
   do {
      c1 = *s1++;
      c2 = *s2++;
   } while (c1 && toupper(c1) == toupper(c2));
   /* now calculate real difference */
   return c1 - c2;
}
#endif
#endif
