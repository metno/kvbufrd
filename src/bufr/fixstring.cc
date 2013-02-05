#include <iostream>
#include <iomanip>
#include "fixstring.h"
using namespace std;

/**
 * fixString takes a string an do various transformation on it.
 *
 * 1. Check that each character is between 32 and 126 inclusive.
 * 2. Transform Å->AA, å->aa, Æ->AE, æ->ae, ø->oe and Ø->OE, .
 * 3. Invalid characters NOT recognized in 2. is removed.
 * 4. Transform each character to upper case.
 */


/**FIXME: this code is mostly broken, but it work most of the time :-).*/

std::string
fixString( const std::string &s ){
	string res;
	unsigned char c;

	//cerr << "String: '" << s << "'" << endl;

	for( string::size_type i=0; i<s.length(); ++i ) {
		c = s[i];

		//cerr << " " << hex << (int)c;
		if( c == 0xC3 ) { //Assume utf8
			c=s[++i];
			switch( c ) {
			case 0x86: res += "AE"; break; //Æ
			case 0xA6: res += "ae"; break; //æ
			case 0x98: res += "OE"; break; //Ø
			case 0xB8: res += "oe"; break; //ø
			case 0x85: res += "AA"; break; //Å
			case 0xA5: res += "aa"; break; //å
			default:
				break;
			}
		} else if( c<32 || c > 126 ) { //Assume latin 1
			switch( c ) {
			case 0xC6: res += "AE"; break; //Æ
 			case 0xE6: res += "ae"; break; //æ
 			case 0xD8: res += "OE"; break; //Ø
 			case 0xF8: res += "oe"; break; //ø
 			case 0xC5: res += "AA"; break; //Å
 			case 0xE5: res += "aa"; break; //å
 			default:
 				break;
			}
		} else {
			res += c;
		}
	}

	for( string::size_type i=0; i<res.length(); ++i ) {
		res[i] = toupper( res[i] );
	}

	//cerr << endl;
	return res;
}


