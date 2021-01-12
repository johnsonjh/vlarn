/* =============================================================================
 * PROGRAM:  CODE LIBRARY
 * FILENAME: ifftools.c
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

#include <stdio.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <clib/exec_protos.h>

#include "bio.h"
#include "ifftools.h"

/* =======================================================================
** Definitions
*/

#define MAKE_ID(a,b,c,d)\
 ( ((long) (a)<<24) + ((long) (b)<<16) + ((long) (c)<<8) + ((long) (d)) )

#define  FORM MAKE_ID('F','O','R','M')
#define  ILBM MAKE_ID('I','L','B','M')
#define  BMHD MAKE_ID('B','M','H','D')
#define  CMAP MAKE_ID('C','M','A','P')
#define  GRAB MAKE_ID('G','R','A','B')
#define  DEST MAKE_ID('D','E','S','T')
#define  SPRT MAKE_ID('S','P','R','T')
#define  CAMG MAKE_ID('C','A','M','G')
#define  CRNG MAKE_ID('C','R','N','G')
#define  CCRT MAKE_ID('C','C','R','T')
#define  BODY MAKE_ID('B','O','D','Y')

#define mskNone                 0
#define mskHasMask              1
#define mskHasTransparentColour 2
#define mskLasso                3

#define cmpNone                 0
#define cmpByteRun1             1


typedef struct {
  UWORD w,h;
  WORD  x,y;
  UBYTE nPlanes;
  UBYTE masking;
  UBYTE compression;
  UBYTE pad1;
  UWORD transparentColour;
  UBYTE xAspect,yAspect;
  WORD pageWidth,pageHeight;
} BitMapHeader;


typedef struct {
  long ckID;
  long ckSize;
} ChunkHeader;


/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: FreeBitmap
 */
void FreeBitmap(struct BitMap *Bitmap)
{
  int Plane;
  int PlaneSize;

  if (Bitmap != NULL)
  {
    PlaneSize = Bitmap->BytesPerRow * Bitmap->Rows;

    for (Plane = 0 ; Plane < Bitmap->Depth ; Plane++)
    {
      if (Bitmap->Planes[Plane] != NULL)
      {
        FreeMem(Bitmap->Planes[Plane], PlaneSize);
        Bitmap->Planes[Plane] = NULL;
      }
    }

    FreeMem(Bitmap, sizeof(struct BitMap));
  }
}

/* =============================================================================
 * FUNCTION: ReadIff
 */
struct BitMap *ReadIff(char *fname, ULONG *palette)
{
  struct BitMap *bitmap;      /* pointer to the bitmap to be read       */
  ChunkHeader  ckhd;          /* chunk header                           */
  BitMapHeader bmhd;          /* bitmap header                          */

  struct BFile *file;         /* pointer to buffered file to be read    */

  PLANEPTR planes[8];         /* quick access to bitplane pointers      */
  PLANEPTR plane_ptr;         /* pointer to the row to be read          */

  BYTE  inbyte1;
  BYTE  inbyte2;
  UBYTE byte3[3];             /* colour map data buffer (rgb triplet    */
  long  byte4;                /* 4 byte ID                              */
  short j;
  short bytes_per_row;        /* number of bytes in each row of the pic */
  short bytes_this_row;       /* bytes read into the current row so far */
  short plane_no;             /* bit plane currently being read         */
  short count;                /* number of bytes to place in bitmap     */
  long  size;                 /* number of bytes read in from file      */
  long  row_offset;           /* offset for the start of this row       */
  long  current_row;          /* current row being read                 */
  long  plane_size;           /* bit plane size                         */
  long  i;

  file = BOpen(fname, MODE_OLDFILE);
  if (file == NULL)
  {
    printf("Can't open file %s.\n",fname);
    return(NULL);
  }

  size = BRead(file, (UBYTE *) &ckhd, (long) sizeof(ChunkHeader));
  if (ckhd.ckID != FORM)
  {
    printf("Not a form!!\n", ckhd.ckID);
    BClose(file);
    return (NULL);
  }

  size = BRead(file, (UBYTE *) &byte4, 4L);
  if (byte4 != ILBM)
  {
    printf("Not an ILBM form!!\n");
    BClose(file);
    return(NULL);
  }

  bitmap = (struct BitMap *)
    AllocMem((long) sizeof(struct BitMap), MEMF_CHIP | MEMF_CLEAR);


  while (size != 0L)
  {
    /* Read in a chunk header */

    size = BRead(file, (UBYTE *) &ckhd, (long) sizeof(ChunkHeader));

    switch (ckhd.ckID)
    {
      case BMHD:
      {
        size = BRead(file, (UBYTE *) &bmhd, (long) sizeof(BitMapHeader));
        break;
      }

      case BODY:
      {
        bytes_per_row = ((bmhd.w+15)>>4)<<1;
        plane_size = ((long) bytes_per_row) * ((long) bmhd.h);

        bitmap->BytesPerRow = bytes_per_row;
        bitmap->Rows = bmhd.h;
        bitmap->Depth = bmhd.nPlanes;

        for(j = 0 ; j < bmhd.nPlanes ; j++)
        {
          bitmap->Planes[j] = (PLANEPTR)
            AllocMem(plane_size, MEMF_CHIP | MEMF_CLEAR);

          planes[j] = bitmap->Planes[j];
        }

        row_offset = 0L;
        for (current_row = 0L;current_row < ((long) bmhd.h);current_row++)
        {
          for(plane_no = 0 ; plane_no < bmhd.nPlanes ; plane_no++)
          {
            plane_ptr = planes[plane_no] + row_offset;

            if (bmhd.compression == cmpByteRun1)
            {
              bytes_this_row = 0;
              while (bytes_this_row < bytes_per_row)
              {
                size = BRead(file, &inbyte1, 1L);

                /*
                ** If the byte 'N' read is greater not negative then
                ** take the next N bytes verbatim.
                */
                if (inbyte1 >= 0)
                {
                  count = inbyte1+1;
                  size = BRead(file,
                               (plane_ptr + bytes_this_row),
                               (long) count);
                  bytes_this_row += count;
                }
                else
                {
                  if (inbyte1 != -128)
                  {
                    count = 1-inbyte1;
                    size = BRead(file, &inbyte2, 1L);
                    for(j = 0 ; j < count ; j++)
                    {
                      plane_ptr[bytes_this_row] = inbyte2;
                      bytes_this_row++;
                    }
                  }
                }
              }
            }
            else
            {
              size = BRead(file, plane_ptr,(long) bytes_per_row);
            }
          }

          row_offset += (long) bytes_per_row;
        }

        while (size != 0L)
          size = BRead(file, &inbyte1, 1L);

        break;
      }

      case CMAP:
      {

        for (j = 0L, i = 0L ; i < ckhd.ckSize ; j++, i += 3L)
        {
          size = BRead(file, byte3, 3L);
          palette[j] = (byte3[0]<<16) + (byte3[1]<<8) + byte3[2];
        }

        if ((ckhd.ckSize % 2L) == 1L)
          size = BSeek(file, 1L, OFFSET_CURRENT);

        break;
      }

      default :
      {
        size = BSeek(file, (long) ckhd.ckSize, OFFSET_CURRENT);
        break;
      }
    }
  }

  BClose(file);

  return (bitmap);
}

