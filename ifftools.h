/* =============================================================================
 * PROGRAM:  CODE LIBRARY
 * FILENAME: ifftools.h
 *
 * DESCRIPTION:
 * This module provides functions to read IFF ILBM files and free the bitmaps
 * for ILBM files read.
 *
 * =============================================================================
 * COPYRIGHT:
 *
 * Copyright (c) 1996, 2004, Julian Olds
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   . Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   . Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * None.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * FreeBitmap : This function frees the bitmap for an ILBM file.
 * ReadIff    : This function reads an IFF ILBM file and returns the bitmap.
 *
 * =============================================================================
 */

#ifndef _IFF_TOOLS_H
#define _IFF_TOOLS_H

#include <graphics/gfx.h>


/* =============================================================================
 * FUNCTION: FreeBitmap
 *
 * DESCRIPTION:
 * This function frees a bitmap.
 *
 * PARAMETERS:
 *
 *   Bitmap : A pointer to the bitmap to be freed.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void FreeBitmap(struct BitMap *Bitmap);

/* =============================================================================
 * FUNCTION: ReadIff
 *
 * DESCRIPTION:
 * This function reads an IFF ILBM file from the specified filename and
 * returns a pointer to a bitmap of the image contained in the file.
 * The palette of the bitmap is returned in Palette.
 *
 * PARAMETERS:
 *
 *   FileName : The name of the IFF ILBM file to read.
 *
 *   Palette  : This must point to an array of ULONG big enough to store the
 *              entire palette of the bitmap.
 *              each entry in the array stroes a RGB triplet in the format
 *              0x00RRGGBB
 *
 * RETURN VALUE:
 *
 *   BitMap *: A pointer to the bitmap inthe file, or NULL if the read failed.
 */
struct BitMap *ReadIff(char *FileName, ULONG *Palette);


#endif