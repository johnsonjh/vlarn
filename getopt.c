/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: getopt.c
 *
 * DESCRIPTION:
 * Command line options processing functions.
 *
 * =============================================================================
 * EXPORTED VARIABLES
 *
 * optarg - This is set to the argument associated with any option found by
 *          getopt that takes an argument.
 *
 * optind - Index in argv of the next element to be scanned.
 *
 * opterr - Caller sets to 0 to inhibit error status messages.
 *
 * =============================================================================
 * EXPORTED FUNCTIONS
 *
 * getopt - Arguments processing function.
 *
 * =============================================================================
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <mem.h>

#include "getopt.h"

/* This version of `getopt' appears to the caller like standard Unix `getopt'
   but it behaves differently for the user, since it allows the user
   to intersperse the options with the other arguments.

   As `getopt' works, it permutes the elements of `argv' so that,
   when it is done, all the options precede everything else.  Thus
   all application programs are extended to handle flexible argument order.

   Setting the environment variable _POSIX_OPTION_ORDER disables permutation.
   Then the behavior is completely standard.

   GNU application programs can use a third alternative mode in which
   they can distinguish the relative order of options and other arguments.  */

/* =============================================================================
 * Exported variables
 */

char *optarg = 0;

int optind = 0;

int opterr = 1;

/* =============================================================================
 * Local variables
 */

/*
 * The next char to be scanned in the option-element
 * in which the last option character we returned was found.
 * This allows us to pick up the scan where we left off.
 *
 * If this is zero, or a null string, it means resume the scan
 * by advancing to the next ARGV-element.
 */
static char *nextchar;


/* Describe how to deal with options that follow non-option ARGV-elements.

   If the caller did not specify anything,
   the default is REQUIRE_ORDER if the environment variable
   _POSIX_OPTION_ORDER is defined, PERMUTE otherwise.

   REQUIRE_ORDER means don't recognize them as options;
   stop option processing when the first non-option is seen.
   This is what Unix does.
   This mode of operation is selected by either setting the environment
   variable _POSIX_OPTION_ORDER, or using `+' as the first character
   of the list of option characters.

   PERMUTE is the default.  We permute the contents of ARGV as we scan,
   so that eventually all the non-options are at the end.  This allows options
   to be given in any order, even with programs that were not written to
   expect this.

   RETURN_IN_ORDER is an option available to programs that were written
   to expect options and other ARGV-elements in any order and that care about
   the ordering of the two.  We describe each non-option ARGV-element
   as if it were the argument of an option with character code 1.
   Using `-' as the first character of the list of option characters
   selects this mode of operation.

   The special argument `--' forces an end of option-scanning regardless
   of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
   `--' can cause `getopt' to return EOF with `optind' != ARGC.  */

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

/* Describe the long-named options requested by the application.
   _GETOPT_LONG_OPTIONS is a vector of `struct option' terminated by an
   element containing a name which is zero.
   The field `has_arg' is 1 if the option takes an argument,
   2 if it takes an optional argument.  */

struct option
{
  char *name;
  int has_arg;
  int *flag;
  int val;
};

const struct option *_getopt_long_options;

int _getopt_long_only = 0;

/* Index in _GETOPT_LONG_OPTIONS of the long-named option actually found.
   Only valid when a long-named option was found. */

int option_index;

/* Handle permutation of arguments.  */

/* Describe the part of ARGV that contains non-options that have
   been skipped.  `first_nonopt' is the index in ARGV of the first of them;
   `last_nonopt' is the index after the last of them.  */

static int first_nonopt;
static int last_nonopt;

/* =============================================================================
 * Local functions
 */

/* Exchange two adjacent subsequences of ARGV.
   One subsequence is elements [first_nonopt,last_nonopt)
   which contains all the non-options that have been skipped so far.
   The other is elements [last_nonopt,optind), which contains all
   the options processed since those non-options were skipped.

   `first_nonopt' and `last_nonopt' are relocated so that they describe
   the new indices of the non-options in ARGV after they are moved.  */

static void exchange (char **argv)
{
  int nonopts_size = (last_nonopt - first_nonopt) * sizeof (char *);
  char **temp = (char **) malloc (nonopts_size);

  /* Interchange the two blocks of data in ARGV.  */
  memcpy (&argv[first_nonopt], temp, nonopts_size);
  memcpy (&argv[last_nonopt], &argv[first_nonopt],
   (optind - last_nonopt) * sizeof (char *));
  memcpy (temp, &argv[first_nonopt + optind - last_nonopt], nonopts_size);

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;

  free(temp);
}

/* =============================================================================
 * Exported functions
 */

/* =============================================================================
 * FUNCTION: ugetopt
 */
int ugetopt (int argc, char **argv, const char *optstring)
{
  optarg = 0;

  /* Initialize the internal data when the first call is made.
     Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.
  */

  if (optind == 0)
  {
    first_nonopt = last_nonopt = optind = 1;

    nextchar = 0;

    /* Determine how to handle the ordering of options and nonoptions.  */

    if (optstring[0] == '-')
    {
      ordering = RETURN_IN_ORDER;
      ++optstring;
    }
    else if (optstring[0] == '+')
    {
      ordering = REQUIRE_ORDER;
      ++optstring;
    }
    else if (getenv ("_POSIX_OPTION_ORDER") != 0)
      ordering = REQUIRE_ORDER;
    else
      ordering = PERMUTE;
  }

  if (nextchar == 0 || *nextchar == 0)
  {
    if (ordering == PERMUTE)
    {
      /* If we have just processed some options following some non-options,
         exchange them so that the options come first.  */

      if (first_nonopt != last_nonopt && last_nonopt != optind)
        exchange (argv);
      else if (last_nonopt != optind)
        first_nonopt = optind;

      /* Now skip any additional non-options
         and extend the range of non-options previously skipped.  */

      while ((optind < argc) &&
             ((argv[optind][0] != '-') || (argv[optind][1] == 0)) &&
             ((_getopt_long_options == 0)||
              (argv[optind][0] != '+') ||
              (argv[optind][1] == 0)))
      {
        optind++;
      }

      last_nonopt = optind;
    }

    /* Special ARGV-element `--' means premature end of options.
       Skip it like a null option,
       then exchange with previous non-options as if it were an option,
       then skip everything else like a non-option.
    */

    if (optind != argc && !strcmp (argv[optind], "--"))
    {
      optind++;

      if (first_nonopt != last_nonopt && last_nonopt != optind)
        exchange (argv);
      else if (first_nonopt == last_nonopt)
        first_nonopt = optind;

      last_nonopt = argc;

      optind = argc;
    }

    /* If we have done all the ARGV-elements, stop the scan
       and back over any non-options that we skipped and permuted.
    */

    if (optind == argc)
    {
      /* Set the next-arg-index to point at the non-options
         that we previously skipped, so the caller will digest them.
      */
      if (first_nonopt != last_nonopt)
        optind = first_nonopt;

      return EOF;
    }

    /* If we have come to a non-option and did not permute it,
       either stop the scan or describe it to the caller and pass it by.
    */

    if (((argv[optind][0] != '-') || (argv[optind][1] == 0)) &&
        ((_getopt_long_options == 0) ||
         (argv[optind][0] != '+') ||
         (argv[optind][1] == 0)))
    {
      if (ordering == REQUIRE_ORDER)
        return EOF;

      optarg = argv[optind++];
      return 1;
    }

    /* We have found another option-ARGV-element.
       Start decoding its characters.
    */

    nextchar = argv[optind] + 1;
  }

  if ((_getopt_long_options != 0) &&
      ((argv[optind][0] == '+') ||
       (_getopt_long_only && (argv[optind][0] == '-'))))
  {
    const struct option *p;
    char *s = nextchar;
    int exact = 0;
    int ambig = 0;
    const struct option *pfound = 0;
    int indfound;

    while (*s && (*s != '=')) s++;

    /* Test all options for either exact match or abbreviated matches.  */
    for (p = _getopt_long_options, option_index = 0;
         p->name;
         p++, option_index++)
    {
      if (!strncmp (p->name, nextchar, s - nextchar))
      {
        if (s - nextchar == strlen (p->name))
        {
          /* Exact match found.  */
          pfound = p;
          indfound = option_index;
          exact = 1;
          break;
        }
        else if (pfound == 0)
        {
          /* First nonexact match found.  */
          pfound = p;
          indfound = option_index;
        }
        else
          /* Second nonexact match found.  */
          ambig = 1;
      }
    }

    if (ambig && !exact)
    {
      fprintf (stderr,
               "%s: option `%s' is ambiguous\n",
               argv[0], argv[optind]);

      nextchar += strlen (nextchar);
      optind++;
      return '?';
    }

    if (pfound != 0)
    {
      option_index = indfound;
      optind++;
      if (*s)
      {
        if (pfound->has_arg > 0)
          optarg = s + 1;
        else
        {
          fprintf (stderr,
                   "%s: option `%c%s' doesn't allow an argument\n",
                   argv[0], argv[optind - 1][0], pfound->name);
          nextchar += strlen (nextchar);
          return '?';
        }
      }
      else if (pfound->has_arg == 1)
      {
        if (optind < argc)
          optarg = argv[optind++];
        else
        {
          fprintf (stderr,
                   "%s: option `%s' requires an argument\n",
                   argv[0], argv[optind - 1]);
          nextchar += strlen (nextchar);
          return '?';
        }
      }
      nextchar += strlen (nextchar);
      if (pfound->flag)
      {
        *(pfound->flag) = pfound->val;
        return 0;
      }
      return pfound->val;
    }

    /* Can't find it as a long option.  If this is getopt_long_only,
       and the option starts with '-' and is a valid short
       option, then interpret it as a short option.  Otherwise it's
       an error.
    */
    if ((_getopt_long_only == 0) ||
        (argv[optind][0] == '+') ||
        (strchr (optstring, *nextchar) == 0))
    {
      if (opterr != 0)
      {
        fprintf (stderr,
                 "%s: unrecognized option `%c%s'\n",
                 argv[0], argv[optind][0], nextchar);
      }

      nextchar += strlen (nextchar);
      optind++;
      return '?';
    }
  }

  /* Look at and handle the next option-character.  */

  {
    char c = *nextchar++;
    const char *temp = strchr (optstring, c);

    /* Increment `optind' when we start to process its last character.  */
    if (*nextchar == 0)
      optind++;

    if (temp == 0 || c == ':')
    {
      if (opterr != 0)
      {
        if (c < 040 || c >= 0177)
          fprintf (stderr,
                   "%s: unrecognized option, character code 0%o\n",
                   argv[0], c);
        else
          fprintf (stderr,
                   "%s: unrecognized option `-%c'\n",
                   argv[0], c);
      }
      return '?';
    }

    if (temp[1] == ':')
    {
      if (temp[2] == ':')
      {
        /* This is an option that accepts an argument optionally.  */
        if (*nextchar != 0)
        {
          optarg = nextchar;
          optind++;
        }
        else
          optarg = 0;

        nextchar = 0;
      }
      else
      {
        /* This is an option that requires an argument.  */
        if (*nextchar != 0)
        {
          optarg = nextchar;
          /* If we end this ARGV-element by taking the rest as an arg,
             we must advance to the next element now.
          */
          optind++;
        }
        else if (optind == argc)
        {
          if (opterr != 0)
          {
            fprintf (stderr,
                     "%s: option `-%c' requires an argument\n",
                     argv[0], c);
          }

          c = '?';
        }
        else
        {
          /* We already incremented `optind' once;
             increment it again when taking next ARGV-elt as argument.  */
          optarg = argv[optind++];
        }

        nextchar = 0;
      }
    }
    return c;
  }
}
