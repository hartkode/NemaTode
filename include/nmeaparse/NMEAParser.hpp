/*
 * NMEAParser.h
 *
 *  Created on: Aug 12, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "nmeaparse/Event.hpp"

// read class definition for info
#define NMEA_PARSER_MAX_BUFFER_SIZE 2000

namespace nmea {

class NMEAParser;

class NMEASentence {
	friend NMEAParser;

private:
	bool isvalid_;

public:
	std::string              text_;       // whole plaintext of the received command
	std::string              name_;       // name of the command
	std::vector<std::string> parameters_; // list of parameters from the command
	std::string              checksum_;
	bool                     checksumIsCalculated_;
	uint8_t                  parsedChecksum_;
	uint8_t                  calculatedChecksum_;

	enum MessageID { // These ID's are according to NMEA standard.
		Unknown = -1,
		GGA     = 0,
		GLL     = 1,
		GSA     = 2,
		GSV     = 3,
		RMC     = 4,
		VTG     = 5, // notice missing 6,7
		ZDA     = 8
	};

	NMEASentence();

	[[nodiscard]] bool checksumOK() const;
	[[nodiscard]] bool valid() const;
};

class NMEAParseError : public std::exception {
public:
	std::string  message_;
	NMEASentence nmea_;

	explicit NMEAParseError(std::string msg);
	NMEAParseError(std::string msg, NMEASentence n);

	[[nodiscard]] const char* what() const noexcept override;
};

class NMEAParser {
private:
	std::unordered_map<std::string, std::function<void(NMEASentence)>> eventTable_;
	std::string                                                        buffer_;
	bool                                                               fillingbuffer_;
	uint32_t                                                           maxbuffersize_; // limit the max size if no newline ever comes... Prevents huge buffer string internally

	void parseText(NMEASentence& nmea, std::string txt); // fills the given NMEA sentence with the results of parsing the string.

	void onInfo(NMEASentence& nmea, const std::string& txt) const;
	void onWarning(NMEASentence& nmea, const std::string& txt) const;
	void onError(NMEASentence& nmea, const std::string& txt) const;

public:
	NMEAParser();

	bool log_;

	Event<void(const NMEASentence&)> onSentence_;                                                                                             // called every time parser receives any NMEA sentence

	void                             setSentenceHandler(const std::string& cmdKey, const std::function<void(const NMEASentence&)>& handler); // one handler called for any named sentence where name is the "cmdKey"
	[[nodiscard]] std::string        getRegisteredSentenceHandlersCSV() const;                                                               // show a list of message names that currently have handlers.

	// Byte streaming functions
	void readByte(uint8_t byte);
	void readBuffer(uint8_t* ptr, uint32_t size);
	void readLine(const std::string& line);

	// This function expects the data to be a single line with an actual sentence in it, else it throws an error.
	void readSentence(std::string cmd); // called when parser receives a sentence from the byte stream. Can also be called by user to inject sentences.

	static uint8_t calculateChecksum(const std::string&); // returns checksum of string -- XOR
};

} // namespace nmea
