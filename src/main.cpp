#include <Arduino.h>

#include <SoftwareSerial.h>

#include <SPI.h>
#include <SD.h>

#include <RMCParser.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 4(rx) and 3(tx). The reciever TX is hooked up to pin 4 and the reciever rx is on pin 3.
*/

/*
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
 */

#define SW_SERIAL_RX_PIN 4
#define SW_SERIAL_TX_PIN 3
#define GPS_BAUD 9600

#define SD_CHIP_SELECT_PIN 10

#define STOP_PIN 8

static SoftwareSerial gps_serial(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);

static RMCParser rmc_parser;

static bool is_stopped = true;

static File log_file;
static constexpr size_t SD_CARD_BUFFER_SIZE = 30;
static RMCEntry sd_card_buffer[SD_CARD_BUFFER_SIZE];
static size_t sd_card_buffer_idx = 0;

void WriteEntryToFile(const RMCEntry& entry)
{
    // if the file opened okay, write to it:
    if (!is_stopped)
    {
        sd_card_buffer[sd_card_buffer_idx++] = entry;
        if (sd_card_buffer_idx >= SD_CARD_BUFFER_SIZE)
        {
            log_file.write((const uint8_t*)sd_card_buffer, sizeof(sd_card_buffer));
            sd_card_buffer_idx = 0;
            Serial.println("!!");
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
        log_file = SD.open("log.bin", FILE_WRITE);
        if (!log_file)
        {
            // if the file didn't open, print an error:
            Serial.println("error opening log.bin");
        }
        is_stopped = false;
    }


    gps_serial.begin(GPS_BAUD);
}

void loop()
{
    if (!is_stopped && digitalRead(STOP_PIN) && log_file)
    {
        Serial.println("Logging Stopped.");
        // close the file:
        log_file.close();
        is_stopped = true;
    }


    while (gps_serial.available() > 0)
    {
        if (rmc_parser.HandleByte((char)gps_serial.read()))
        {
            Serial.println("!");
            WriteEntryToFile(rmc_parser.last_parsed);
            // Serial.println("======================");
            // Serial.println(rmc_parser.lat);
            // Serial.println(rmc_parser.lon);
            // Serial.println(rmc_parser.vel);
        }
    }
}
