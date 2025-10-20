#include <Arduino.h>

#include <HardwareSerial.h>

#include <SPI.h>
#include <SD.h>

#include <TinyGPS.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 16(rx) and 17(tx). The reciever TX is hooked up to the GPS RX.

https://randomnerdtutorials.com/esp32-uart-communication-serial-arduino/
UART2	GPIO 17	GPIO 16

*/

/*
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)


*/

#define SW_SERIAL_RX_PIN 16
#define SW_SERIAL_TX_PIN 17
#define GPS_BAUD 9600

#define SD_CHIP_SELECT_PIN 10

#define STOP_PIN 8

TinyGPS gps;
HardwareSerial ss(2);

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

static constexpr size_t SD_CARD_BUFFER_SIZE = 128;
uint8_t sd_card_buffer[SD_CARD_BUFFER_SIZE];
size_t sd_card_buffer_idx = 0;

File myFile;

bool is_stopped = true;

void write_data_sd(uint8_t byte)
{
    // if the file opened okay, write to it:
    if (!is_stopped)
    {
        sd_card_buffer[sd_card_buffer_idx++] = byte;
        if (sd_card_buffer_idx >= SD_CARD_BUFFER_SIZE)
        {
            myFile.write(sd_card_buffer, SD_CARD_BUFFER_SIZE);
            sd_card_buffer_idx = 0;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(STOP_PIN, INPUT);

    Serial.print("Initializing SD card...");

    if (!SD.begin(SD_CHIP_SELECT_PIN))
    {
        Serial.println("initialization failed!");
    }
    else
    {
        Serial.println("initialization done.");
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        myFile = SD.open("test.txt", FILE_WRITE);
        if (!myFile)
        {
            // if the file didn't open, print an error:
            Serial.println("error opening test.txt");
        }
        is_stopped = false;
    }

    Serial.print("Testing TinyGPS library v. ");
    Serial.println(TinyGPS::library_version());
    Serial.println("by Mikal Hart");
    Serial.println();
    Serial.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
    Serial.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail");
    Serial.println("-------------------------------------------------------------------------------------------------------------------------------------");

    ss.begin(GPS_BAUD, SERIAL_8N1, SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
}

void loop()
{
    if (!is_stopped && digitalRead(STOP_PIN) && myFile)
    {
        Serial.println("Logging Stopped.");
        // close the file:
        myFile.close();
        is_stopped = true;
    }

    float flat, flon;
    unsigned long age, chars = 0;
    unsigned short sentences = 0, failed = 0;
    static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;

    print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
    print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
    gps.f_get_position(&flat, &flon, &age);
    print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
    print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
    print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
    print_date(gps);
    print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
    print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
    print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
    print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
    print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
    print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
    print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

    gps.stats(&chars, &sentences, &failed);
    print_int(chars, 0xFFFFFFFF, 6);
    print_int(sentences, 0xFFFFFFFF, 10);
    print_int(failed, 0xFFFFFFFF, 9);
    Serial.println();

    smartdelay(1000);
}

static void smartdelay(unsigned long ms)
{
    unsigned long start = millis();
    do
    {
        while (ss.available())
        {
            uint8_t byte = ss.read();
            write_data_sd(byte);
            gps.encode(byte);
        }
    } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
    if (val == invalid)
    {
        while (len-- > 1)
            Serial.print('*');
        Serial.print(' ');
    }
    else
    {
        Serial.print(val, prec);
        int vi = abs((int)val);
        int flen = prec + (val < 0.0 ? 2 : 1); // . and -
        flen += vi >= 1000 ? 4 : vi >= 100 ? 3
                             : vi >= 10    ? 2
                                           : 1;
        for (int i = flen; i < len; ++i)
            Serial.print(' ');
    }
    smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
    char sz[32];
    if (val == invalid)
        strcpy(sz, "*******");
    else
        sprintf(sz, "%ld", val);
    sz[len] = 0;
    for (int i = strlen(sz); i < len; ++i)
        sz[i] = ' ';
    if (len > 0)
        sz[len - 1] = ' ';
    Serial.print(sz);
    smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
    int year;
    byte month, day, hour, minute, second, hundredths;
    unsigned long age;
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    if (age == TinyGPS::GPS_INVALID_AGE)
        Serial.print("********** ******** ");
    else
    {
        char sz[32];
        sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
                month, day, year, hour, minute, second);
        Serial.print(sz);
    }
    print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
    smartdelay(0);
}

static void print_str(const char *str, int len)
{
    int slen = strlen(str);
    for (int i = 0; i < len; ++i)
        Serial.print(i < slen ? str[i] : ' ');
    smartdelay(0);
}
