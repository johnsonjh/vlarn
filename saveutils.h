/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: saveutils.h
 *
 * DESCRIPTION:
 * This module contains utilities used in loading and saving games.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * FileSum : The current checksum for the file being written/read.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * sum    : Checksum calculation function
 * bwrite : Binary write with checksum update
 * bread  : Binary read with checksum update.
 *
 * =============================================================================
 */

#ifndef __SAVEUTILS_H
#define __SAVEUTILS_H

#include <stdio.h>

/*
 * This is the current checksum value for bread and bwrite.
 */
extern int FileSum;

/* =============================================================================
 * FUNCTION: sum
 *
 * DESCRIPTION:
 * Checksum calculation function.
 *
 * PARAMETERS:
 *
 *   data : A pointer to the data to be checksummed
 *
 *   n    : The number of bytes in Data to be checksummed
 *
 * RETURN VALUE:
 *
 *   The checksum of data.
 */
unsigned int sum(unsigned char *data, int n);

/* =============================================================================
 * FUNCTION: bwrite
 *
 * DESCRIPTION:
 * Binary write function with checksum update.
 * Writes the binary data to the specified file and updates the FileSum for
 * the data written.
 *
 * PARAMETERS:
 *
 *   fp  : A pointer to the file being written
 *
 *   buf : A pointer to the buffer to write.
 *
 *   num : The number of characters in buf to write.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void bwrite(FILE *fp, char *buf, long num);

/* =============================================================================
 * FUNCTION: bread
 *
 * DESCRIPTION:
 * Binary read function with checksum update.
 * Read the binary data from the specified file and updates the FileSum for
 * the data read.
 *
 * PARAMETERS:
 *
 *   fp  : A pointer to the file being read
 *
 *   buf : A pointer to the buffer to store the read data.
 *
 *   num : The number of characters be read into buf.
 *
 * RETURN VALUE:
 *
 *   None.
 */
void bread(FILE *fp, char *buf, long num);

#endif
