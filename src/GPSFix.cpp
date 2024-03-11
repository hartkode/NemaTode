/*
 * GPSFix.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#include "nmeaparse/GPSFix.hpp"

#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;
using namespace std::chrono;

using namespace nmea;

// ===========================================================
// ======================== GPS SATELLITE ====================
// ===========================================================

string
GPSSatellite::toString() const
{
	ostringstream strm;

	strm << "[PRN: " << setw(3) << setfill(' ') << prn_ << " "
	     << "  SNR: " << setw(3) << setfill(' ') << snr_ << " dB  "
	     << "  Azimuth: " << setw(3) << setfill(' ') << azimuth_ << " deg "
	     << "  Elevation: " << setw(3) << setfill(' ') << elevation_ << " deg  "
	     << "]";

	return strm.str();
}

GPSSatellite::operator string() const
{
	return toString();
}

// =========================================================
// ======================== GPS ALMANAC ====================
// =========================================================

void
GPSAlmanac::clear()
{
	lastPage       = 0;
	totalPages     = 0;
	processedPages = 0;
	visibleSize    = 0;
	satellites_.clear();
}

void
GPSAlmanac::updateSatellite(const GPSSatellite& sat)
{
	if ( satellites_.size() > visibleSize ) { // we missed the new almanac start page, start over.
		clear();
	}

	satellites_.emplace_back(sat);
}

double
GPSAlmanac::percentComplete() const
{
	if ( totalPages == 0 ) {
		return 0.0;
	}

	return ((double) processedPages) / ((double) totalPages) * 100.0;
}

double
GPSAlmanac::averageSNR() const
{
	double avg      = 0;
	double relevant = 0;

	for ( const auto& satellite: satellites_ ) {
		if ( satellite.snr_ > 0 ) {
			relevant += 1.0;
		}
	}

	for ( const auto& satellite: satellites_ ) {
		if ( satellite.snr_ > 0 ) {
			avg += satellite.snr_ / relevant;
		}
	}

	return avg;
}

double
GPSAlmanac::minSNR() const
{
	double min = 9999999;
	if ( satellites_.empty() ) {
		return 0;
	}
	int32_t num_over_zero = 0;
	for ( const auto& satellite: satellites_ ) {
		if ( satellite.snr_ > 0 ) {
			num_over_zero++;
			if ( satellite.snr_ < min ) {
				min = satellite.snr_;
			}
		}
	}
	if ( num_over_zero == 0 ) {
		return 0;
	}
	return min;
}

double
GPSAlmanac::maxSNR() const
{
	double max = 0;
	for ( const auto& satellite: satellites_ ) {
		if ( satellite.snr_ > 0 ) {
			if ( satellite.snr_ > max ) {
				max = satellite.snr_;
			}
		}
	}
	return max;
}

// ===========================================================
// ======================== GPS TIMESTAMP ====================
// ===========================================================

// indexed from 1!
string
GPSTimestamp::monthName(uint32_t index)
{
	if ( index < 1 || index > 12 ) {
		stringstream strm;
		strm << "[month:" << index << "]";
		return strm.str();
	}

	static const array<string, 12> names = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	return names.at(index - 1);
};

// Returns seconds since Jan 1, 1970. Classic Epoch time.
time_t
GPSTimestamp::getTime() const
{
	struct tm time {};

	time.tm_year = year_ - 1900; // This is year-1900, so 112 = 2012
	time.tm_mon  = month_;       // month from 0:Jan
	time.tm_mday = day_;
	time.tm_hour = hour_;
	time.tm_min  = min_;
	time.tm_sec  = (int) sec_;

	return mktime(&time);
}

void
GPSTimestamp::setTime(double raw_ts)
{
	rawTime_ = raw_ts;

	hour_ = (int32_t) trunc(raw_ts / 10000.0);
	min_  = (int32_t) trunc((raw_ts - hour_ * 10000) / 100.0);
	sec_  = raw_ts - min_ * 100 - hour_ * 10000;
}

// ddmmyy
void
GPSTimestamp::setDate(int32_t raw_date)
{
	rawDate_ = raw_date;
	// If uninitialized, use posix time.
	if ( rawDate_ == 0 ) {
		month_ = 1;
		day_   = 1;
		year_  = 1970;
	}
	else {
		day_   = (int32_t) trunc(raw_date / 10000.0);
		month_ = (int32_t) trunc((raw_date - 10000 * day_) / 100.0);
		year_  = raw_date - 10000 * day_ - 100 * month_ + 2000;
	}
}

string
GPSTimestamp::toString() const
{
	stringstream strm;
	strm << hour_ << "h " << min_ << "m " << sec_ << "s"
	     << "  " << GPSTimestamp::monthName(month_) << " " << day_ << " " << year_;
	return strm.str();
};

// =====================================================
// ======================== GPS FIX ====================
// =====================================================

// Returns the duration since the Host has received information
seconds
GPSFix::timeSinceLastUpdate() const
{
	time_t    now = time(NULL);
	struct tm time {};

	time.tm_hour = timestamp_.hour_;
	time.tm_min  = timestamp_.min_;
	time.tm_sec  = (int) timestamp_.sec_;
	time.tm_year = timestamp_.year_ - 1900;
	time.tm_mon  = timestamp_.month_ - 1;
	time.tm_mday = timestamp_.day_;

	time_t then = mktime(&time);
	auto   secs = (uint64_t) difftime(now, then);
	return seconds(secs);
}

bool
GPSFix::hasEstimate() const
{
	return (latitude_ != 0 && longitude_ != 0) || (quality_ == 6);
}

bool
GPSFix::setlock(bool locked)
{
	if ( haslock != locked ) {
		haslock = locked;
		return true;
	}
	return false;
}

bool
GPSFix::locked() const
{
	return haslock;
}

// Returns meters
double
GPSFix::horizontalAccuracy() const
{
	// horizontal 2drms 95% = 4.0  -- from GPS CHIP datasheets
	return 4.0 * horizontalDilution_;
}

// Returns meters
double
GPSFix::verticalAccuracy() const
{
	// Vertical 2drms 95% = 6.0  -- from GPS CHIP datasheets
	return 6.0 * verticalDilution_;
}

// Takes a degree travel heading (0-360') and returns the name
string
GPSFix::travelAngleToCompassDirection(double deg, bool abbrev)
{
	// normalize, just in case
	int32_t c = (int32_t) round(deg / 360.0 * 8.0);
	int32_t r = c % 8;
	if ( r < 0 ) {
		r = 8 + r;
	}

	if ( abbrev ) {
		static const array<string, 9> dirs = {
			"N",
			"NE",
			"E",
			"SE",
			"S",
			"SW",
			"W",
			"NW",
			"N"
		};
		return dirs.at(r);
	}
	else {
		static const array<string, 9> dirs = {
			"North",
			"North East",
			"East",
			"South East",
			"South",
			"South West",
			"West",
			"North West",
			"North"
		};
		return dirs.at(r);
	}
};

string
fixStatusToString(char status)
{
	switch ( status ) {
	case 'A':
		return "Active";
	case 'V':
		return "Void";
	default:
		return "Unknown";
	}
}

string
fixTypeToString(uint8_t type)
{
	switch ( type ) {
	case 1:
		return "None";
	case 2:
		return "2D";
	case 3:
		return "3D";
	default:
		return "Unknown";
	}
}

string
fixQualityToString(uint8_t quality)
{
	switch ( quality ) {
	case 0:
		return "Invalid";
	case 1:
		return "Standard";
	case 2:
		return "DGPS";
	case 3:
		return "PPS fix";
	case 4:
		return "Real Time Kinetic";
	case 5:
		return "Real Time Kinetic (float)";
	case 6:
		return "Estimate";
	default:
		return "Unknown";
	}
}

string
GPSFix::toString() const
{
	stringstream       strm;
	ios_base::fmtflags oldflags = strm.flags();

	strm << "========================== GPS FIX ================================" << endl
	     << " Status: \t\t" << ((haslock) ? "LOCK!" : "SEARCHING...") << endl
	     << " Satellites: \t\t" << trackingSatellites_ << " (tracking) of " << visibleSatellites_ << " (visible)" << endl
	     << " < Fix Details >" << endl
	     << "   Age:                " << timeSinceLastUpdate().count() << " s" << endl
	     << "   Timestamp:          " << timestamp_.toString() << "   UTC   \n\t\t\t(raw: " << timestamp_.rawTime_ << " time, " << timestamp_.rawDate_ << " date)" << endl
	     << "   Raw Status:         " << status_ << "  (" << fixStatusToString(status_) << ")" << endl
	     << "   Type:               " << (int) type_ << "  (" << fixTypeToString(type_) << ")" << endl
	     << "   Quality:            " << (int) quality_ << "  (" << fixQualityToString(quality_) << ")" << endl
	     << "   Lat/Lon (N,E):      " << setprecision(6) << fixed << latitude_ << "' N, " << longitude_ << "' E" << endl;

	strm.flags(oldflags); // reset

	strm << "   DOP (P,H,V):        " << dilution_ << ",   " << horizontalDilution_ << ",   " << verticalDilution_ << endl
	     << "   Accuracy(H,V):      " << horizontalAccuracy() << " m,   " << verticalAccuracy() << " m" << endl;

	strm << "   Altitude:           " << altitude_ << " m" << endl
	     << "   Speed:              " << speed_ << " km/h" << endl
	     << "   Travel Dir:         " << travelAngle_ << " deg  [" << travelAngleToCompassDirection(travelAngle_) << "]" << endl
	     << "   SNR:                avg: " << almanac_.averageSNR() << " dB   [min: " << almanac_.minSNR() << " dB,  max:" << almanac_.maxSNR() << " dB]" << endl;

	strm << " < Almanac (" << almanac_.percentComplete() << "%) >" << endl;
	if ( almanac_.satellites_.empty() ) {
		strm << " > No satellite info in almanac." << endl;
	}
	for ( size_t i = 0; i < almanac_.satellites_.size(); i++ ) {
		strm << "   [" << setw(2) << setfill(' ') << (i + 1) << "]   " << almanac_.satellites_[i].toString() << endl;
	}

	return strm.str();
}

GPSFix::operator string() const
{
	return toString();
}
