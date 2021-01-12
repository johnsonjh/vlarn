/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: header.h
 *
 * DESCRIPTION:
 * This file contains the macro definitions for macros commonly used in ularn.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * None.
 *
 * =============================================================================
 */

#ifndef __HEADER_H
#define __HEADER_H

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifdef FTIMER
#  include <sys/timeb.h>
#endif /* FTIMER */

#ifdef ITIMER
#  include SYSTIME
#endif /* ITIMER */

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef UNIX
#include <unistd.h>
#endif

#ifdef WINDOWS
#include <io.h>
#endif

/*
 *  ------------------- macros --------------------
 */

#ifdef DRAND48
# define srand srand48
# define rand lrand48
#else
# ifdef RANDOM
#   define srand srandom
#   define rand random
# endif
#endif /* RANDOM */

// Generate a random number between 1 and x
#define rnd(x)  ((int)(rand() % (x)) + 1)
#define rndl(x)  ((long)(rand() % (x)) + 1)
// Generate a random number between 0 and x-1
#define rund(x) ((int)(rand() % (x)))
#define rundl(x) ((long)(rand() % (x)))

/* macros for miscellaneous data conversion */
#ifndef min
#define min(x,y) ((int)((x)>(y))?(y):(x))
#endif

#ifndef max
#define max(x,y) ((int)((x)>(y))?(x):(y))
#endif

#ifndef abs
#define abs(x) (((x) < 0) ? -(x) : (x))
#endif

/* Macro for adding plural to item descriptions */
#define plural(x) ((x==1) ? "" : "s")


#define ESC '\033'

#endif
