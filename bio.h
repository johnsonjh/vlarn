/* =============================================================================
 * PROGRAM:  CODE LIBRARY
 * FILENAME: bio.h
 *
 * DESCRIPTION:
 *
 * Buffered I/O package.
 *
 * Allows multiple small reads and writes to be handled efficiently.
 * Probably not so good for large reads/writes.
 * Excellent for reading IFF pictures though.
 *
 * Duplicates the Amiga DOS functions and are used in exactly the same
 * way as their counterparts as described in the Amiga technical reference
 * Manual (Libraries and devices).
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
 * None
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * BOpen  : Open a buffered I/O file.
 * BClose : Close a buffered I/O file.
 * BRead  : Read from a buffered I/O file.
 * BSeek  : Seek a position in a buffered I/O file.
 * BWrite : Write to a buffered I/O file.
 *
 * =============================================================================
 */

#ifndef __BIO_H
#define __BIO_H

#include <libraries/dos.h>
#include <libraries/dosextens.h>

/*
** NOTE : The buffer is used for either input out output at any one time.
**        Any read after a write, or write after a read will cause the
**        buffer to be flushed.
**        The seek operation will also cause the buffer to be flushed.
*/

struct BFile {
  BPTR file;                   /* Pointer to a DOS file handle         */
  UBYTE *buffer;               /* Pointer to Input/Output buffer       */
  long file_offset;            /* Current position in buffered file    */
  long buffer_size;            /* Number of bytes read into the buffer */
  long in_buffer_count;        /* Position in the buffer for input     */
  long out_buffer_count;       /* Position in the buffer for output    */
};

/* =============================================================================
 * FUNCTION: BOpen
 *
 * DESCRIPTION:
 * This function opens a file for buffered I/O reading and/or writing.
 *
 * PARAMETERS:
 *
 *   name       : The name of the file to open.
 *
 *   accessMode : The access mode for the file.
 *
 * RETURN VALUE:
 *
 *   BFile *: A pointer the the buffered I/O file opened, or NULL if the
 *            open failed.
 */
struct BFile *BOpen(char *name, long accessMode);

/* =============================================================================
 * FUNCTION: BClose
 *
 * DESCRIPTION:
 * This function closes a buffered I/O file.
 *
 * PARAMETERS:
 *
 *   file : A pointer to the file to close.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void BClose(struct BFile *file);

/* =============================================================================
 * FUNCTION: BRead
 *
 * DESCRIPTION:
 * This function reads 'length' bytes from 'file' into the buffer 'buff'.
 *
 * PARAMETERS:
 *
 *   file    : A pointer to the buffered I/O file handle of the file to read.
 *
 *   buff    : A pointer to the buffer to hold the data read.
 *
 *   length  : The number of bytes to read.
 *
 * RETURN VALUE:
 *
 *   long: The number of bytes actually read.
 */
long BRead(struct BFile *file, UBYTE *buff, long length);

/* =============================================================================
 * FUNCTION: BSeek
 *
 * DESCRIPTION:
 * This function seeks a position in a bufferd I/O file.
 *
 * PARAMETERS:
 *
 *   file     : A pointer to the buffered I/O file handle of the file to seek.
 *
 *   position : The position in the file to seek.
 *
 *   mod      : The seek mode.
 *
 * RETURN VALUE:
 *
 *   long: The new file position.
 */
long BSeek(struct BFile *file, long position, long mod);

/* =============================================================================
 * FUNCTION: BWrite
 *
 * DESCRIPTION:
 * This function write 'length' bytes from 'buff' to a buffered I/O file.
 *
 * PARAMETERS:
 *
 *   file   : A pointer to the bufferd I/O file handle of the file to write.
 *
 *   buff   : A pointer to the buffer containing the data to be written.
 *
 *   length : The number of bytes to write.
 *
 * RETURN VALUE:
 *
 *   long: The number of bytes actually written to the file.
 */
long BWrite(struct BFile *file, UBYTE *buff, long length);

#endif