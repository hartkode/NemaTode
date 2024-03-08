/*
 * GPSService.h
 *
 *  Created on: Aug 14, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#pragma once

#include <chrono>
#include <functional>
#include <nmeaparse/Event.hpp>
#include <nmeaparse/GPSFix.hpp>
#include <nmeaparse/NMEAParser.hpp>
#include <string>

namespace nmea {

class GPSService {
private:
	void read_PSRF150(const NMEASentence& nmea);
	void read_GPGGA(const NMEASentence& nmea);
	void read_GPGSA(const NMEASentence& nmea);
	void read_GPGSV(const NMEASentence& nmea);
	void read_GPRMC(const NMEASentence& nmea);
	void read_GPVTG(const NMEASentence& nmea);

public:
	GPSFix fix;

	explicit GPSService(NMEAParser& parser);
	virtual ~GPSService() = default;

	Event<void(bool)> onLockStateChanged; // user assignable handler, called whenever lock changes
	Event<void()>     onUpdate;           // user assignable handler, called whenever fix changes

	void attachToParser(NMEAParser& parser); // will attach to this parser's nmea sentence events
};

} // namespace nmea
