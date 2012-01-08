#ifndef SPINDLY_H
#define SPINDLY_H 1

#define SPINDLY_COPYRIGHT "2011-2012 The spindly project and its contributors"

/* The stringified version. This may have -DEV appended for non-released
   versions. */
#define SPINDLY_VERSION                             "0.1.0-DEV"

/* The numeric version number is also available "in parts" by using these
   defines: */
#define SPINDLY_VERSION_MAJOR                       0
#define SPINDLY_VERSION_MINOR                       1
#define SPINDLY_VERSION_PATCH                       0

/* This is the numeric version of the spindly version number, meant for easier
   parsing and comparions by programs. The SPINDLY_VERSION_NUM define will
   always follow this syntax:

         0xXXYYZZ

   Where XX, YY and ZZ are the main version, release and patch numbers in
   hexadecimal (using 8 bits each). All three numbers are always represented
   using two digits.  1.2 would appear as "0x010200" while version 9.11.7
   appears as "0x090b07".

   This 6-digit (24 bits) hexadecimal number does not show pre-release number,
   and it is always a greater number in a more recent release. It makes
   comparisons with greater than and less than work.
*/
#define SPINDLY_VERSION_NUM                         0x000100

/*
 * This is the date and time when the full source package was created. The
 * timestamp is not stored in the source code repo, as the timestamp is
 * properly set in the tarballs by the maketgz script.
 *
 * The format of the date should follow this template:
 *
 * "Mon Feb 12 11:35:33 UTC 2007"
 */
#define SPINDLY_TIMESTAMP "DEV"

#endif /* SPINDLY_H */
