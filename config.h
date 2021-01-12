/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: config.h
 *
 * DESCRIPTION:
 * This file contains any system specific configuration information for ularn.
 * As this port is Windows only, most of this stuff is redundant, and has
 * been deleted
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * None
 *
 * =============================================================================
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Uncomment exactly one of the following lines to
 * select the version to build.
 *
 * UNIX_TTY   - ASCII version for unix/linux etc
 * UNIX_X11   - X11 graphical version for unix/linux etc
 * WINDOWS    - Graphical version for Windows 32 (Win9X, 2K, XP etc).
 * W32_TTY    - ASCII version for Windows 32 (Win9X, 2K, XP etc).
 * AMIGA_WIN  - Amiga (AGA only) graphical version
 */

//#define UNIX_TTY
#define UNIX_X11
//#define WINDOWS
//#define W32_TTY
//#define AMIGA_WIN

/*
 * Now set the O/S flag based on the version selected.
 */

#ifdef UNIX_X11
#define UNIX
#endif

#ifdef UNIX_TTY
#define UNIX
#endif

/*
 * LIBDIR:
 * This symbol indicates where the data files will reside.
 */
#define LIBDIR "lib"

#endif
