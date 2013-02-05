
#ifndef __FIXSTRING_H__
#define __FIXSTRING_H__

#include <string>

/**
 * fixString takes a string an do various transformation on it.
 * It is assumed that the string is encoded in latin1. If it is encoded
 * in utf8 it blows up.
 *
 * 1. Check that each character is between 32 and 126 inclusive.
 * 2. Transform Å->AA, å->aa, Æ->AE, æ->ae, ø->oe and Ø->OE, .
 * 3. Invalid characters NOT recognized in 2. is removed.
 * 4. Transform each character to upper case.
 */

std::string
fixString( const std::string &s );



#endif
