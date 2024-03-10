/*
 * GPSFix.h
 *
 *  Created on: Jul 23, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

namespace nmea {

struct GPSSatellite;
class GPSAlmanac;
class GPSFix;
class GPSService;

// =========================== GPS SATELLITE =====================================

struct GPSSatellite {
	// satellite data
	double   snr{};       // 0-99 dB
	uint32_t prn{};       // id - 0-32
	double   elevation{}; // 0-90 deg
	double   azimuth{};   // 0-359 deg

	[[nodiscard]] std::string toString() const;

	explicit operator std::string() const;
};

// =========================== GPS ALMANAC =====================================

class GPSAlmanac {
	friend GPSService;

private:
	uint32_t visibleSize{};
	uint32_t lastPage{};
	uint32_t totalPages{};
	uint32_t processedPages{};

	void clear(); // will remove all information from the satellites
	void updateSatellite(GPSSatellite sat);

public:
	// mapped by prn
	std::vector<GPSSatellite> satellites;

	[[nodiscard]] double averageSNR() const;
	[[nodiscard]] double minSNR() const;
	[[nodiscard]] double maxSNR() const;

	[[nodiscard]] double percentComplete() const;
};

// =========================== GPS TIMESTAMP =====================================

// UTC time
class GPSTimestamp {
private:
	static std::string monthName(uint32_t index);

public:
	GPSTimestamp();

	int32_t hour;
	int32_t min;
	double  sec;

	int32_t month;
	int32_t day;
	int32_t year;

	// Values collected directly from the GPS
	double  rawTime;
	int32_t rawDate;

	[[nodiscard]] time_t getTime() const;

	// Set directly from the NMEA time stamp
	// hhmmss.sss
	void setTime(double raw_ts);

	// Set directly from the NMEA date stamp
	// ddmmyy
	void setDate(int32_t raw_date);

	[[nodiscard]] std::string toString() const;
};

// =========================== GPS FIX =====================================

class GPSFix {
	friend GPSService;

private:
	bool haslock;
	bool setlock(bool locked); // returns true if lock status **changed***, false otherwise.

public:
	GPSFix();

	GPSAlmanac   almanac;
	GPSTimestamp timestamp;

	char    status;  // Status: A=active, V=void (not locked)
	uint8_t type;    // Type: 1=none, 2=2d, 3=3d
	uint8_t quality; // Quality:
	                 //    0 = invalid
	                 //    1 = GPS fix (SPS)
	                 //    2 = DGPS fix
	                 //    3 = PPS fix
	                 //    4 = Real Time Kinematic (RTK)
	                 //    5 = Float RTK
	                 //    6 = estimated (dead reckoning) (2.3 feature)

	double dilution;           // Combination of Vertical & Horizontal
	double horizontalDilution; // Horizontal dilution of precision, initialized to 100, best =1, worst = >20
	double verticalDilution;   // Vertical is less accurate

	double  altitude;    // meters
	double  latitude;    // degrees N
	double  longitude;   // degrees E
	double  speed;       // km/h
	double  travelAngle; // degrees true north (0-360)
	int32_t trackingSatellites;
	int32_t visibleSatellites;

	[[nodiscard]] bool locked() const;

	[[nodiscard]] double horizontalAccuracy() const;
	[[nodiscard]] double verticalAccuracy() const;
	[[nodiscard]] bool   hasEstimate() const;

	[[nodiscard]] std::chrono::seconds timeSinceLastUpdate() const; // Returns seconds difference from last timestamp and right now.

	[[nodiscard]] std::string toString() const;

	explicit operator std::string() const;

	static std::string travelAngleToCompassDirection(double deg, bool abbrev = false);
};

} // namespace nmea
