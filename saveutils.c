/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: saveutils.c
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

#include <stdio.h>
#include <errno.h>

#include "ularn_win.h"
#include "scores.h"
#include "saveutils.h"

/* =============================================================================
 * Local variables
 */

 /* The number of characters written */
static int w = 0;

/* The number of characters read */
static int r = 0;

/* =============================================================================
 * Exported variables
 */

int FileSum;

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: sum
 */
unsigned int sum(unsigned char *data, int n)
{
  unsigned int sum;
  int c, nb;

  sum = nb = 0;
  while (nb++ < n)
  {
    c = *data++;
    if (sum & 01)
      sum = (sum >> 1) + 0x8000;
    else
      sum >>= 1;
    sum += c;
    sum &= 0xFFFF;
  }
  return(sum);
}

/* =============================================================================
 * FUNCTION: bwrite
 */
void bwrite(FILE *fp, char *buf, long num)
{
  int nwrote;
  static int ncalls = 0;

  ncalls++;
  nwrote = fwrite(buf, 1, num, fp);

  w += nwrote;

  if (nwrote != num)
  {
    Printf("Error writing to save file\n");
    Printf("wrote %d, wanted %d\n", nwrote, num);
    //    Printf("errno = %d\t[%s]\n", errno, sys_errlist[errno]);
    Printf("    Wrote %d bytes so far\n", w);
    Printf("        Call: %d\n", ncalls);

    nap(4000);
    died(DIED_POST_MORTEM_DEATH, 0);
  }

  FileSum += sum((unsigned char *)buf, num);
}

/* =============================================================================
 * FUNCTION: bread
 */
void bread(FILE *fp, char *buf, long num)
{
  int nread;
  static int ncalls = 0;

  ncalls++;
  nread = fread(buf, 1, num, fp);

  r += nread;

  if (nread != num)
  {
    Printf("Error reading from save file\n");
    Printf("  Got %d, wanted %d bytes\n", nread, num);
    //    Printf("errno = %d\t[%s]\n",errno,sys_errlist[errno]);
    Printf("    Read %d bytes so far\n", w);
    Printf("        Call: %d\n", ncalls);

    nap(4000);
    died(DIED_POST_MORTEM_DEATH, 0);
  }

  FileSum += sum((unsigned char *)buf, num);
}

