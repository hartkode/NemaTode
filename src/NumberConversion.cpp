/*
 * NumberConversion.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#include "nmeaparse/NumberConversion.hpp"

#include <cstdlib>

using namespace std;

namespace nmea {
// Note: both parseDouble and parseInt return 0 with "" input.

double
parseDouble(const string& str)
{
	char*  ptr   = nullptr;
	double value = ::strtod(str.c_str(), &ptr);

	if ( *ptr != '\0' ) {
		stringstream strm;
		strm << "NumberConversionError: parseDouble() error in argument \"" << str << "\", '"
		   << *ptr << "' is not a number.";
		throw NumberConversionError(strm.str());
	}

	return value;
}

int64_t
parseInt(const string& str, int radix)
{
	char*   ptr   = nullptr;
	int64_t value = ::strtoll(str.c_str(), &ptr, radix);

	if ( *ptr != '\0' ) {
		stringstream strm;
		strm << "NumberConversionError: parseInt() error in argument \"" << str << "\", '"
		   << *ptr << "' is not a number.";
		throw NumberConversionError(strm.str());
	}

	return value;
}

} // namespace nmea

/*
#include <iostream>
void NumberConversion_test(){
    string s;
    float f;
    long long k;

    try{
        s = "-1.345";
        f = parseDouble(s);
        cout << s << ": " << f << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }


    try{
        s = "-1.23e-2";
        f = parseDouble(s);
        cout << s << ": " << f << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }


    try{
        s = "";
        f = parseDouble(s);
        cout << s << ": " << f << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }



    try{
        // -- fails, ok
        s = "asd";
        f = parseDouble(s);
        cout << s << ": " << f << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }

    try{
        s = "-1234.123";
        k = parseInt(s);
        cout << s << ": " << k << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }



    try{
        s = "01234";
        k = parseInt(s);
        cout << s << ": " << k << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }


    try{
        // -- converts to 0
        s = "";
        k = parseInt(s);
        cout << s << ": " << k << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }


    try{
        // -- fails, ok
        s = "asd";
        k = parseInt(s);
        cout << s << ": " << k << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }

    try{
        // -- fails, ok
        s = "-16";
        k = parseInt(s);
        cout << s << ": " << k << endl;
    }
    catch(NumberConversionError& ex){
        cout << ex.message << endl;
    }

 }
 */
