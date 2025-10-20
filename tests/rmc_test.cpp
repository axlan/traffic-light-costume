#include <iostream>
#include <string>
#include <cmath>

#include "RMCParser.h"

// Feed a full RMC sentence character-by-character to the parser and return whether it reported success
bool feed_sentence(RMCParser &p, const std::string &s)
{
    bool saw = false;
    for (char c : s)
    {
        if (p.HandleByte(c))
            saw = true;
    }
    return saw;
}

int main()
{
    RMCParser p;

    // A sample RMC sentence from the header comments
    std::string s = "$GPRMC,182331.00,A,3750.81870,N,12216.72288,W,0.092,,191025,,,A*6B\r\n";

    bool ok = feed_sentence(p, s);
    if (!ok)
    {
        std::cerr << "Parser did not report a complete RMC\n";
        return 2;
    }

    // Check approximate parsed values
    // Expected lat: 37 deg + 50.81870 minutes -> 37 + 50.81870/60
    double expected_lat = 37.0 + 50.81870 / 60.0;
    double expected_lon = -(122.0 + 16.72288 / 60.0);
    double expected_vel = 0.092;

    auto nearly = [](double a, double b)
    { return fabs(a - b) < 1e-4; };

    bool pass = nearly(p.last_parsed.lat, expected_lat) && nearly(p.last_parsed.lon, expected_lon) && nearly(p.last_parsed.vel, expected_vel);

    if (!pass)
    {
        std::cerr << "Parsed values differ:\n";
        std::cerr << "lat got=" << p.last_parsed.lat << " exp=" << expected_lat << "\n";
        std::cerr << "lon got=" << p.last_parsed.lon << " exp=" << expected_lon << "\n";
        std::cerr << "vel got=" << p.last_parsed.vel << " exp=" << expected_vel << "\n";
        return 3;
    }

    std::cout << "RMCParser test passed\n";
    return 0;
}
