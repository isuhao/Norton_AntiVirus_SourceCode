//************************************************************************
//
// $Header:   S:/GRENLEAF/VCS/_MATCH.CPv   1.0   12 Mar 1998 12:14:20   DCHI  $
//
// Description:
//  Greenleaf ArchiveLib 2.0
//
//************************************************************************
// $Log:   S:/GRENLEAF/VCS/_MATCH.CPv  $
// 
//    Rev 1.0   12 Mar 1998 12:14:20   DCHI
// Initial revision.
// 
//************************************************************************

//
// _MATCH.CPP
//
//  Source file for ArchiveLib 2.0
//
//  Copyright (c) Greenleaf Software, Inc. 1994-1996
//  All Rights Reserved
//
// CONTENTS
//
//  is_pattern()
//  is_valid_pattern()
//  matche()
//  matche_after_star()
//  match()
//  main()  (For testing, w/conditional compile)
//
// DESCRIPTION
//
//  Some nice code written by J. Kercheval, in the public domain.
//  This code provides us with the pattern matching functions used
//  by the ALName functions.  It all seems to work without any trouble
//  at all.
//
//  I tried to change this file as little as possible.  I modified the
//  name of the file, and removed BOOLEAN, TRUE, and FALSE from the
//  header file.  Other than that, it is just as I found it.
//
// REVISION HISTORY
//
//  May 22, 1994  1.0A     : First release
//
//  January 9, 1995  1.01A : Exported functions so this could be called
//                           from a DLL.  Don't know how safe that will be!
//
//  February 14, 1996  2.0A : New release w/no substantial changes
//
#include "arclib.h"
#if !defined( AL_IBM )
#pragma hdrstop
#endif

/*
 EPSHeader

   File: match.c
   Author: J. Kercheval
   Created: Sat, 01/05/1991  22:21:49
*/

/*
 EPSRevision History

   J. Kercheval  Wed, 02/20/1991  22:29:01  Released to Public Domain
   J. Kercheval  Fri, 02/22/1991  15:29:01  fix '\' bugs (two :( of them)
   J. Kercheval  Sun, 03/10/1991  19:31:29  add error return to matche()
   J. Kercheval  Sun, 03/10/1991  20:11:11  add is_valid_pattern code
   J. Kercheval  Sun, 03/10/1991  20:37:11  beef up main()
   J. Kercheval  Tue, 03/12/1991  22:25:10  Released as V1.1 to Public Domain
   J. Kercheval  Thu, 03/14/1991  22:22:25  remove '\' for DOS file parsing
   J. Kercheval  Mon, 05/13/1991  21:49:05  ifdef full match code
   J. Kercheval  Mon, 01/06/1992  21:31:44  add match character defines
*/

/*
 * Wildcard Pattern Matching
 */

#include "arclib.h"
#include "_match.h"
//
// The next five lines used to be in the header file
//
#ifndef BOOLEAN
#define BOOLEAN int
#define TRUE 1
#define FALSE 0
#endif

/* character defines */
#define MATCH_CHAR_SINGLE               '?'
#define MATCH_CHAR_KLEENE_CLOSURE       '*'
#define MATCH_CHAR_RANGE_OPEN           '['
#define MATCH_CHAR_RANGE                '-'
#define MATCH_CHAR_RANGE_CLOSE          ']'
#define MATCH_CHAR_LITERAL              '\\'
#define MATCH_CHAR_NULL                 '\0'
#define MATCH_CHAR_CARAT_NEGATE         '^'
#define MATCH_CHAR_EXCLAMATION_NEGATE   '!'

/* forward function prototypes */
int matche_after_star( register const char *pattern, register char *text);
int fast_match_after_star(register char *pattern, register char *text);


/*----------------------------------------------------------------------------
 *
 * Return TRUE if PATTERN has any special wildcard characters
 *
 ---------------------------------------------------------------------------*/

AL_LINKAGE BOOLEAN AL_FUNCTION
is_pattern( const char *p )  /* Tag internal function */
{
    while (*p) {
        switch (*p++) {
                case MATCH_CHAR_SINGLE:
                case MATCH_CHAR_KLEENE_CLOSURE:
                case MATCH_CHAR_RANGE_OPEN:

#ifndef FILE_MATCH
                case MATCH_CHAR_LITERAL:
#endif

                return TRUE;
        }
    }
    return FALSE;
}


/*----------------------------------------------------------------------------
 *
 * Return TRUE if PATTERN has is a well formed regular expression according
 * to the above syntax
 *
 * error_type is a return code based on the type of pattern error.  Zero is
 * returned in error_type if the pattern is a valid one.  error_type return
 * values are as follows:
 *
 *   PATTERN_VALID - pattern is well formed

#ifndef FILE_MATCH
 *   PATTERN_ESC   - pattern has invalid escape ('\' at end of pattern)
#endif

 *   PATTERN_RANGE - [..] construct has a no end range in a '-' pair (ie [a-])
 *   PATTERN_CLOSE - [..] construct has no end bracket (ie [abc-g )
 *   PATTERN_EMPTY - [..] construct is empty (ie [])
 *
 ---------------------------------------------------------------------------*/

AL_LINKAGE BOOLEAN AL_FUNCTION
is_valid_pattern( const char *p, int *error_type )  /* Tag internal function */
{

    /* init error_type */
    *error_type = PATTERN_VALID;

    /* loop through pattern to EOS */
    while (*p) {

        /* determine pattern type */
        switch (*p) {

#ifndef FILE_MATCH
                /* check literal escape, it cannot be at end of pattern */
            case MATCH_CHAR_LITERAL:
                if (!*++p) {
                    *error_type = PATTERN_ESC;
                    return FALSE;
                }
                p++;
                break;
#endif

                /* the [..] construct must be well formed */
            case MATCH_CHAR_RANGE_OPEN:
                p++;

                /* if the next character is ']' then bad pattern */
                if (*p == MATCH_CHAR_RANGE_CLOSE) {
                    *error_type = PATTERN_EMPTY;
                    return FALSE;
                }

                /* if end of pattern here then bad pattern */
                if (!*p) {
                    *error_type = PATTERN_CLOSE;
                    return FALSE;
                }

                /* loop to end of [..] construct */
                while (*p != MATCH_CHAR_RANGE_CLOSE) {

                    /* check for literal escape */
                    if (*p == MATCH_CHAR_LITERAL) {
                        p++;

                        /* if end of pattern here then bad pattern */
                        if (!*p++) {
                            *error_type = PATTERN_ESC;
                            return FALSE;
                        }
                    }
                    else
                        p++;

                    /* if end of pattern here then bad pattern */
                    if (!*p) {
                        *error_type = PATTERN_CLOSE;
                        return FALSE;
                    }

                    /* if this a range */
                    if (*p == MATCH_CHAR_RANGE) {

                        /* we must have an end of range */
                        if (!*++p || *p == MATCH_CHAR_RANGE_CLOSE) {
                            *error_type = PATTERN_RANGE;
                            return FALSE;
                        }
                        else {

                            /* check for literal escape */
                            if (*p == MATCH_CHAR_LITERAL)
                                p++;

                            /* if end of pattern here then bad pattern */
                            if (!*p++) {
                                *error_type = PATTERN_ESC;
                                return FALSE;
                            }
                        }
                    }
                }
                break;

                /* all other characters are valid pattern elements */
            case MATCH_CHAR_KLEENE_CLOSURE:
            case MATCH_CHAR_SINGLE:
            default:            /* "normal" character */
                p++;
                break;
        }
    }

    return TRUE;
}


/*----------------------------------------------------------------------------
 *
 * Match the pattern PATTERN against the string TEXT;
 *
 * returns MATCH_VALID if pattern matches, or an errorcode as follows
 * otherwise:
 *
 *           MATCH_PATTERN  - bad pattern

#ifndef FILE_MATCH
 *           MATCH_LITERAL  - match failure on literal mismatch
#endif

 *           MATCH_RANGE    - match failure on [..] construct
 *           MATCH_ABORT    - premature end of text string
 *           MATCH_END      - premature end of pattern string
 *           MATCH_VALID    - valid match
 *
 *
 * A match means the entire string TEXT is used up in matching.
 *
 * In the pattern string:
 *      `*' matches any sequence of characters (zero or more)
 *      `?' matches any character
 *      [SET] matches any character in the specified set,
 *      [!SET] or [^SET] matches any character not in the specified set.
 *      \ is allowed within a set to escape a character like ']' or '-'
 *
 * A set is composed of characters or ranges; a range looks like character
 * hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the minimal set of
 * characters allowed in the [..] pattern construct.  Other characters are
 * allowed (ie. 8 bit characters) if your system will support them.
 *
 * To suppress the special syntactic significance of any of `[]*?!^-\', and
 * match the character exactly, precede it with a `\'.
 *
 ---------------------------------------------------------------------------*/

AL_LINKAGE int AL_FUNCTION
matche( const char *p, char *t)  /* Tag internal function */
{
    register char range_start, range_end;       /* start and end in range */

    BOOLEAN invert;             /* is this [..] or [!..] */
    BOOLEAN member_match;       /* have I matched the [..] construct? */
    BOOLEAN loop;               /* should I terminate? */

    for (; *p; p++, t++) {

        /* if this is the end of the text then this is the end of the match */
        if (!*t) {
            return (*p == MATCH_CHAR_KLEENE_CLOSURE &&
                    *++p == MATCH_CHAR_NULL) ?
                MATCH_VALID : MATCH_ABORT;
        }

        /* determine and react to pattern type */
        switch (*p) {

                /* single any character match */
            case MATCH_CHAR_SINGLE:
                break;

                /* multiple any character match */
            case MATCH_CHAR_KLEENE_CLOSURE:
                return matche_after_star(p, t);

                /* [..] construct, single member/exclusion character match */
            case MATCH_CHAR_RANGE_OPEN:{

                    /* move to beginning of range */
                    p++;

                    /* check if this is a member match or exclusion match */
                    invert = FALSE;
                    if (*p == MATCH_CHAR_EXCLAMATION_NEGATE ||
                        *p == MATCH_CHAR_CARAT_NEGATE) {
                        invert = TRUE;
                        p++;
                    }

                    /* if closing bracket here or at range start then we have
                     * a malformed pattern */
                    if (*p == MATCH_CHAR_RANGE_CLOSE) {
                        return MATCH_PATTERN;
                    }

                    member_match = FALSE;
                    loop = TRUE;

                    while (loop != FALSE ) {

                        /* if end of construct then loop is done */
                        if (*p == MATCH_CHAR_RANGE_CLOSE) {
                            loop = FALSE;
                            continue;
                        }

                        /* matching a '!', '^', '-', '\' or a ']' */
                        if (*p == MATCH_CHAR_LITERAL) {
                            range_start = range_end = *++p;
                        }
                        else {
                            range_start = range_end = *p;
                        }

                        /* if end of pattern then bad pattern (Missing ']') */
                        if (!*p)
                            return MATCH_PATTERN;

                        /* check for range bar */
                        if (*++p == MATCH_CHAR_RANGE) {

                            /* get the range end */
                            range_end = *++p;

                            /* if end of pattern or construct then bad
                             * pattern */
                            if (range_end == MATCH_CHAR_NULL ||
                                range_end == MATCH_CHAR_RANGE_CLOSE)
                                return MATCH_PATTERN;

                            /* special character range end */
                            if (range_end == MATCH_CHAR_LITERAL) {
                                range_end = *++p;

                                /* if end of text then we have a bad pattern */
                                if (!range_end)
                                    return MATCH_PATTERN;
                            }

                            /* move just beyond this range */
                            p++;
                        }

                        /* if the text character is in range then match
                         * found. make sure the range letters have the proper
                         * relationship to one another before comparison */
                        if (range_start < range_end) {
                            if (*t >= range_start && *t <= range_end) {
                                member_match = TRUE;
                                loop = FALSE;
                            }
                        }
                        else {
                            if (*t >= range_end && *t <= range_start) {
                                member_match = TRUE;
                                loop = FALSE;
                            }
                        }
                    }

                    /* if there was a match in an exclusion set then no match */
                    /* if there was no match in a member set then no match */
                    if ((invert && member_match) ||
                        !(invert || member_match))
                        return MATCH_RANGE;

                    /* if this is not an exclusion then skip the rest of the
                     * [...] construct that already matched. */
                    if (member_match) {
                        while (*p != MATCH_CHAR_RANGE_CLOSE) {

                            /* bad pattern (Missing MATCH_CHAR_RANGE_CLOSE) */
                            if (!*p)
                                return MATCH_PATTERN;

                            /* skip exact match */
                            if (*p == MATCH_CHAR_LITERAL) {
                                p++;

                                /* if end of text then we have a bad pattern */
                                if (!*p)
                                    return MATCH_PATTERN;
                            }

                            /* move to next pattern char */
                            p++;
                        }
                    }

                    break;
                }

#ifndef FILE_MATCH
                /* next character is quoted and must match exactly */
            case MATCH_CHAR_LITERAL:

                /* move pattern pointer to quoted char and fall through */
                p++;

                /* if end of text then we have a bad pattern */
                if (!*p)
                    return MATCH_PATTERN;
#endif

                /* must match this character exactly */
            default:
                if (*p != *t)
                    return MATCH_LITERAL;
        }
    }

    /* if end of text not reached then the pattern fails */
    if (*t)
        return MATCH_END;
    else
        return MATCH_VALID;
}


/*----------------------------------------------------------------------------
 *
 * recursively call matche() with final segment of PATTERN and of TEXT.
 *
 ---------------------------------------------------------------------------*/

int
matche_after_star( register const char *p, register char *t)  /* Tag internal function */
{
    register int match = 0;
    register nextp;

    /* pass over existing ? and * in pattern */
    while (*p == MATCH_CHAR_SINGLE ||
           *p == MATCH_CHAR_KLEENE_CLOSURE) {

        /* take one char for each ? and + */
        if (*p == MATCH_CHAR_SINGLE) {

            /* if end of text then no match */
            if (!*t++) {
                return MATCH_ABORT;
            }
        }

        /* move to next char in pattern */
        p++;
    }

    /* if end of pattern we have matched regardless of text left */
    if (!*p) {
        return MATCH_VALID;
    }

    /* get the next character to match which must be a literal or '[' */
    nextp = *p;

#ifndef FILE_MATCH
    if (nextp == MATCH_CHAR_LITERAL) {
        nextp = p[1];

        /* if end of text then we have a bad pattern */
        if (!nextp)
            return MATCH_PATTERN;
    }
#endif

    /* Continue until we run out of text or definite result seen */
    do {

        /* a precondition for matching is that the next character in the
         * pattern match the next character in the text or that the next
         * pattern char is the beginning of a range.  Increment text pointer
         * as we go here */
        if (nextp == *t || nextp == MATCH_CHAR_RANGE_OPEN) {
            match = matche(p, t);
        }

        /* if the end of text is reached then no match */
        if (!*t++)
            match = MATCH_ABORT;

    } while (match != MATCH_VALID &&
             match != MATCH_ABORT &&
             match != MATCH_PATTERN);

    /* return result */
    return match;
}


/*----------------------------------------------------------------------------
 *
 * match() is a shell to matche() to return only BOOLEAN values.
 *
 ---------------------------------------------------------------------------*/

AL_LINKAGE BOOLEAN AL_FUNCTION
match( char *p, char *t)  /* Tag internal function */
{
    int error_type;

    error_type = matche(p, t);
    return (error_type == MATCH_VALID) ? TRUE : FALSE;
}


#ifdef TEST

/*
    * This test main expects as first arg the pattern and as second arg
    * the match string.  Output is yaeh or nay on match.  If nay on
    * match then the error code is parsed and written.
*/

#include <stdio.h>

int main(int argc, char *argv[])
{
    int error;
    int is_valid_error;

    if (argc != 3) {
        printf("Usage:  MATCH Pattern Text\n");
    }
    else {
        printf("Pattern: %s\n", argv[1]);
        printf("Text   : %s\n", argv[2]);

        if ( !is_pattern(argv[1])) {
            printf("    First Argument Is Not A Pattern\n");
        }
        else {

#ifdef FILE_MATCH
            match(argv[1], argv[2]) ? printf("TRUE") : printf("FALSE");
#endif

            error = matche(argv[1], argv[2]);
            is_valid_pattern(argv[1], &is_valid_error);

            switch (error) {
                case MATCH_VALID:
                    printf("    Match Successful");
                    if (is_valid_error != PATTERN_VALID)
                        printf(" -- is_valid_pattern() is complaining\n");
                    else
                        printf("\n");
                    break;

#ifndef FILE_MATCH
                case MATCH_LITERAL:
                    printf("    Match Failed on Literal\n");
                    break;
#endif

                case MATCH_RANGE:
                    printf("    Match Failed on [..]\n");
                    break;
                case MATCH_ABORT:
                    printf("    Match Failed on Early Text Termination\n");
                    break;
                case MATCH_END:
                    printf("    Match Failed on Early Pattern Termination\n");
                    break;
                case MATCH_PATTERN:
                    switch (is_valid_error) {
                        case PATTERN_VALID:
                            printf("    Internal Disagreement On Pattern\n");
                            break;

#ifndef FILE_MATCH
                        case PATTERN_ESC:
                            printf("    Literal Escape at End of Pattern\n");
                            break;
#endif

                        case PATTERN_RANGE:
                            printf("    No End of Range in [..] Construct\n");
                            break;
                        case PATTERN_CLOSE:
                            printf("    [..] Construct is Open\n");
                            break;
                        case PATTERN_EMPTY:
                            printf("    [..] Construct is Empty\n");
                            break;
                        default:
                            printf("    Internal Error in is_valid_pattern()\n");
                    }
                    break;
                default:
                    printf("    Internal Error in matche()\n");
                    break;
            }
        }

    }
    return (0);
}

#endif
