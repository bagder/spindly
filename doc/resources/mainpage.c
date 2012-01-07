/*! \mainpage libspdy
 * \section intro_sec Introduction
 * Libspdy is in early work in progress, but if you like living on the
 * bleeding edge you can check out the git repo:
 *  - git clone http://libspdy.org/git/ libspdy
 *
 * Libspdy is a C implementation of Googles SPDY protocol. Its main goals are:
 *  - Completely implementing the SPDY protocol standard.
 *  - Being lightweight: As few dependencies as possible. (Right now: zlib (and gzip coming up), as well as Check for the unit tests.)
 *  - High testcoverage: Every function is tested.
 *   - Unittests using Check
 *   - Coverage check using LCOV
 *  - Compiler independent:
 *   - Tested with gcc and clang
 *  - Platform independent:
 *   - Tested on Linux, Mac OS X and Windows 7.
 *  - Good documentation: The API documentation itself and the code should be as documented as possible - without cluttering the source.
 *
 * If you want to learn more about SPDY, check out the official site by Google: http://www.chromium.org/spdy/
 * Standard implemented by the latest version: http://www.chromium.org/spdy/spdy-protocol/spdy-protocol-draft2
 *
 *  Author: Thomas Roth <code@stacksmashing.net>
 */
