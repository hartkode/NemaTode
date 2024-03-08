/*
 * NumberConversion.h
 *
 *  Created on: Aug 14, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#pragma once

#include <cstdint>
#include <exception>
#include <sstream>
#include <string>
#include <utility>

namespace nmea {

class NumberConversionError : public std::exception {
public:
	const std::string message;
	explicit NumberConversionError(std::string msg)
	    : message(std::move(msg)){};

	~NumberConversionError() override = default;

	[[nodiscard]] const char* what() const noexcept override
	{
		return message.c_str();
	}
};

double  parseDouble(const std::string& str);
int64_t parseInt(const std::string& str, int radix = 10);

// void NumberConversion_test();

} // namespace nmea
