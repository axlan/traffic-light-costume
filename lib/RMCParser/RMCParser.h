
// https://receiverhelp.trimble.com/alloy-gnss/en-us/NMEA-0183messages_RMC.html
// https://receiverhelp.trimble.com/alloy-gnss/en-us/NMEA-0183messages_CommonMessageElements.html
// $GPRMC,182331.00,A,3750.81870,N,12216.72288,W,0.092,,191025,,,A*6B
// Latitude and Longitude 	Latitude is represented as ddmm.mmmm and longitude is represented as dddmm.mmmm, where:
// dd or ddd is degrees and mm.mmmm is minutes and decimal fractions of minutes


#pragma pack(1)
struct RMCEntry {
    float lat = 0;
    float lon = 0;
    float vel = 0;
};
#pragma pack()

class RMCParser
{
public:
    RMCEntry last_parsed;

    /**
     * Process a byte looking for RMC NMEA messages.
     *
     * return `true` if the lat, lon, and velocity have been parsed out of a new RMC message.
     */
    bool HandleByte(char byte);

private:
    char parse_buffer[16] = {0};

    int buffer_idx = -1;
    int comma_count = -1;
};
