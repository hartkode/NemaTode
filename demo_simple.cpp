/*
 *
 *  See the license file included with this source.
 */

#include <fstream>
#include <iomanip>
#include <iostream>

#include "nmeaparse/nmea.hpp"

using namespace std;
using namespace nmea;

int
main(int argc, char** argv)
{
	// Fill with your NMEA bytes... make sure it ends with \n
	char bytestream[] = "\n";

	// Create a GPS service that will keep track of the fix data.
	NMEAParser parser;
	GPSService gps(parser);
	parser.log_ = false;

	cout << "Fix  Sats  Sig\t\tSpeed    Dir  Lat         , Lon           Accuracy" << endl;
	// Handle any changes to the GPS Fix... This is called whenever it's updated.
	gps.onUpdate += [&gps]() {
		cout << (gps.fix_.locked() ? "[*] " : "[ ] ") << setw(2) << setfill(' ') << gps.fix_.trackingSatellites_ << "/" << setw(2) << setfill(' ') << gps.fix_.visibleSatellites_ << " ";
		cout << fixed << setprecision(2) << setw(5) << setfill(' ') << gps.fix_.almanac_.averageSNR() << " dB   ";
		cout << fixed << setprecision(2) << setw(6) << setfill(' ') << gps.fix_.speed_ << " km/h [" << GPSFix::travelAngleToCompassDirection(gps.fix_.travelAngle_, true) << "]  ";
		cout << fixed << setprecision(6) << gps.fix_.latitude_ << "\xF8 "
		                                                        "N, "
		     << gps.fix_.longitude_ << "\xF8 "
		                             "E"
		     << "  ";
		cout << "+/- " << setprecision(1) << gps.fix_.horizontalAccuracy() << "m  ";
		cout << endl;
	};

	// -- STREAM THE DATA  ---

	// From a buffer in memory...
	parser.readBuffer((uint8_t*) bytestream, sizeof(bytestream));

	// -- OR --
	// From a device byte stream...
	// gps.parser.readByte(byte_from_device);

	// -- OR --
	// From a file
	string   line;
	ifstream file("nmea_log.txt");
	while ( getline(file, line) ) {
		try {
			parser.readLine(line);
		}
		catch ( NMEAParseError& e ) {
			cout << e.message_ << endl
			     << endl;
			// You can keep feeding data to the gps service...
			// The previous data is ignored and the parser is reset.
		}
	}

	// Show the final fix information
	cout << gps.fix_.toString() << endl;

	cin.ignore();

	return 0;
}
