/* =============================================================================
 * PROGRAM:  ularn
 * FILENAME: getopt.h
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

#ifndef __GETOPT_H
#define __GETOPT_H

/*
 * For communication from `getopt' to the caller.
 * When `getopt' finds an option that takes an argument,
 * the argument value is returned here.
 * Also, when `ordering' is RETURN_IN_ORDER,
 * each non-option ARGV-element is returned here.
 */
extern char *optarg;

/*
 * Index in ARGV of the next element to be scanned.
 * This is used for communication to and from the caller
 * and for communication between successive calls to `getopt'.
 *
 * On entry to `getopt', zero means this is the first call; initialize.
 *
 * When `getopt' returns EOF, this is the index of the first of the
 * non-option elements that the caller should itself scan.
 *
 * Otherwise, `optind' communicates from one call to the next
 * how much of ARGV has been scanned so far.
 */
extern int optind;

/*
 * Callers store zero here to inhibit the error message
 * for unrecognized options.
 */
extern int opterr;

/* =============================================================================
 * FUNCTION: getopt
 *
 * DESCRIPTION:
 * Scan elements of ARGV (whose length is ARGC) for option characters
 * given in OPTSTRING.
 *
 * If an element of ARGV starts with '-', and is not exactly "-" or "--",
 * then it is an option element.  The characters of this element
 * (aside from the initial '-') are option characters.  If `getopt'
 * is called repeatedly, it returns successively each of the option characters
 * from each of the option elements.
 *
 * If `getopt' finds another option character, it returns that character,
 * updating `optind' and `nextchar' so that the next call to `getopt' can
 * resume the scan with the following option character or ARGV-element.
 *
 * If there are no more option characters, `getopt' returns `EOF'.
 * Then `optind' is the index in ARGV of the first ARGV-element
 * that is not an option.  (The ARGV-elements have been permuted
 * so that those that are not options now come last.)
 *
 * OPTSTRING is a string containing the legitimate option characters.
 * If an option character is seen that is not listed in OPTSTRING,
 * return '?' after printing an error message.  If you set `opterr' to
 * zero, the error message is suppressed but we still return '?'.
 *
 * If a char in OPTSTRING is followed by a colon, that means it wants an arg,
 * so the following text in the same ARGV-element, or the text of the following
 * ARGV-element, is returned in `optarg'.  Two colons mean an option that
 * wants an optional arg; if there is text in the current ARGV-element,
 * it is returned in `optarg', otherwise `optarg' is set to zero.
 *
 * If OPTSTRING starts with `-' or `+', it requests different methods of
 * handling the non-option ARGV-elements.
 * See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.
 *
 * Long-named options begin with `+' instead of `-'.
 * Their names may be abbreviated as long as the abbreviation is unique
 * or is an exact match for some defined option.  If they have an
 * argument, it follows the option name in the same ARGV-element, separated
 * from the option name by a `=', or else the in next ARGV-element.
 * When `getopt' finds a long-named option, it returns 0 if that option's
 * `flag' field is nonzero, the value of the option's `val' field
 * otherwise.
 *
 * PARAMETERS:
 *
 *   argc      : The argument count
 *
 *   argv      : The arguments array
 *
 *   optstring : The options characters allowed.
 *
 * RETURN VALUE:
 *
 *   See description.
 */
int ugetopt(int argc, char **argv, const char *optstring);

#endif
