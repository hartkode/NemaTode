/*
 * NMEACommand.h
 *
 *  Created on: Sep 8, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#pragma once

#include <string>

#include "nmeaparse/NMEAParser.hpp"

namespace nmea {

class NMEACommand {
public:
	std::string message_;
	std::string name_;
	char        checksum_;

	explicit NMEACommand(const std::string& name);
	virtual ~NMEACommand();

	virtual std::string toString();
	std::string         addChecksum(const std::string& str);
};

class NMEACommandSerialConfiguration : public NMEACommand {
public:
	/*
	// $PSRF100,0,9600,8,1,0*0C

	Table 2-4 Set Serial Port Data Format
	Name		Example		Unit Description
	Message ID	$PSRF100	PSRF100 protocol header
	Protocol	0 			0=SiRF binary, 1=NMEA
	Baud 		9600 		1200, 2400, 4800, 9600, 19200, 38400, 57600, and 115200
	DataBits	8 			8,71

	StopBits	1 			0,1		1. SiRF protocol is only valid for 8 data bits, 1stop bit, and no parity.
	Parity		0 			0=None, 1=Odd, 2=Even
	Checksum	*0C
	<CR> <LF> End of message termination
	*/
	int32_t baud_{ 4800 };  // 4800, 9600, 19200, 38400
	int32_t databits_{ 8 }; // 7, 8 Databits
	int32_t stopbits_{ 1 }; // 0, 1 Stopbits
	int32_t parity_{ 0 };   // 0=none, 1=odd, 2=even Parity

	NMEACommandSerialConfiguration()
	    : NMEACommand("PSRF100"){};

	std::string toString() override;
};

class NMEACommandQueryRate : public NMEACommand {
public:
	// data fields that will be stringed.

	//  $PSRF103,00,01,00,01*25
	/*
	* Table 2-9 Query/Rate Control Data Format
	Name		Example		Unit Description
	Message ID	$PSRF103	PSRF103 protocol header
	Msg			00 			See Table 2-10
	Mode		01 			0=SetRate, 1=Query
	Rate		00 			sec Output—off=0, max=255
	CksumEnable 01 			0=Disable Checksum, 1=Enable Checksum
	Checksum	*25
	<CR> <LF> End of message termination
	*/
	/*
	* Table 2-10 Messages
	Value Description
	0 GGA
	1 GLL
	2 GSA
	3 GSV
	4 RMC
	5 VTG
	6 MSS (If internal beacon is supported)
	7 Not defined
	8 ZDA (if 1PPS output is supported)
	9 Not defined
	*/

	enum QueryRateMode {
		SETRATE = 0,
		QUERY   = 1
	};

	NMEASentence::MessageID messageID_{ NMEASentence::Unknown };
	QueryRateMode           mode_{ QueryRateMode::SETRATE };
	int                     rate_{ 0 };
	int                     checksumEnable_{ 1 };

	NMEACommandQueryRate()
	    : NMEACommand("PSRF103"){};

	std::string toString() override;
};

} // namespace nmea
