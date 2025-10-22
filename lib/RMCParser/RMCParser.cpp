#include "RMCParser.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

// parse dddmm.mmmm or ddmm.mmmm
static float ParseDegrees(const char *float_str)
{
    double full_value = atof(float_str);
    double int_val = floor(full_value / 100.0);
    double minutes = full_value - int_val * 100.0;
    return int_val + minutes / 60.0;
}

bool RMCParser::HandleByte(char byte)
{
    // Simple state machine parsing comma-separated fields from NMEA RMC sentences.
    // Start of sentence
    if (byte == '$')
    {
        comma_count = -1;
        buffer_idx = 0;
        return false;
    }

    // if newline, reset too
    if (byte == '\n')
    {
        buffer_idx = -1;
        comma_count = -1;
        return false;
    }

    if (buffer_idx < 0)
    {
        return false;
    }

    if (comma_count < 0)
    {
        parse_buffer[buffer_idx++] = byte;

        if (buffer_idx == 5)
        {
            if (memcmp(parse_buffer, "GPRMC", 5) != 0)
            {
                buffer_idx = -1;
            }
            else
            {
                comma_count = 0;
                buffer_idx = 0;
            }
        }
        return false;
    }

    // When comma_count >= 0 we are collecting fields. Fields of interest are separated by ','.
    if (byte == ',')
    {
        if (comma_count >= 3)
        {
            if (buffer_idx == 0)
            {
                buffer_idx = -1;
                comma_count = -1;
                return false;
            }

            // terminate current field
            parse_buffer[buffer_idx] = '\0';

            // process field according to index
            // field indices: 0=msg id (already checked),1=time,2=status,3=lat,4=N/S,5=lon,6=E/W,7=speed
            if (comma_count == 3)
            {
                last_parsed.lat = ParseDegrees(parse_buffer);
            }
            else if (comma_count == 4)
            {
                if (parse_buffer[0] == 'S' || parse_buffer[0] == 's')
                    last_parsed.lat = -last_parsed.lat;
            }
            else if (comma_count == 5)
            {
                last_parsed.lon = ParseDegrees(parse_buffer);
            }
            else if (comma_count == 6)
            {
                if (parse_buffer[0] == 'W' || parse_buffer[0] == 'w')
                    last_parsed.lon = -last_parsed.lon;
            }
            else if (comma_count == 7)
            {
                last_parsed.vel = atof(parse_buffer);
            }
        }

        // next field
        comma_count++;
        buffer_idx = 0;

        // if we've parsed past speed field, consider sentence complete
        if (comma_count > 7)
        {
            // reset to wait for next sentence
            buffer_idx = -1;
            comma_count = -1;
            return true;
        }

        return false;
    }

    // regular character: append to buffer if space
    if (buffer_idx < (int)sizeof(parse_buffer) - 1)
    {
        parse_buffer[buffer_idx++] = byte;
        return false;
    }
    else
    {
        // overflow: abandon this sentence until next '$'
        buffer_idx = -1;
        comma_count = -1;
        return false;
    }
}
