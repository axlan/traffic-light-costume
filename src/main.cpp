#include <Arduino.h>

#include <SoftwareSerial.h>

#include <SPI.h>
#include <SD.h>

#include <RMCParser.h>
#include <TrafficLightLogic.h>

/*
   9600-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
   The reciever TX is hooked up to pin 4 and the reciever rx is on pin 3.
*/

/*
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 */

#define SW_SERIAL_RX_PIN 4
#define SW_SERIAL_TX_PIN 3
#define GPS_BAUD 9600

#define RED_LED_PIN 5
#define GREEN_LED_PIN 7
#define YELLOW_LED_PIN 6

#define SD_CHIP_SELECT_PIN 10

static SoftwareSerial gps_serial(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);

static RMCParser rmc_parser;
static TrafficLight light_ctrl;

static bool is_logging_engabled = false;
static File log_file;
static char log_filename[16];
static constexpr size_t SD_CARD_BUFFER_SIZE = 30;
static RMCEntry sd_card_buffer[SD_CARD_BUFFER_SIZE];
static size_t sd_card_buffer_idx = 0;

void WriteEntryToFile(const RMCEntry &entry)
{
    // if the file opened okay, write to it:
    if (is_logging_engabled)
    {
        sd_card_buffer[sd_card_buffer_idx++] = entry;
        if (sd_card_buffer_idx >= SD_CARD_BUFFER_SIZE)
        {
            log_file = SD.open(log_filename, FILE_WRITE);
            log_file.write((const uint8_t *)sd_card_buffer, sizeof(sd_card_buffer));
            log_file.close();
            sd_card_buffer_idx = 0;
            Serial.println("W");
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);

    Serial.print("Initializing SD card...");

    if (!SD.begin(SD_CHIP_SELECT_PIN))
    {
        Serial.println("initialization failed!");
    }
    else
    {
        Serial.println("initialization done.");
        // Determine file name based on the number of files in the SD root directory.
        size_t file_count = 0;
        File root = SD.open("/");
        if (root)
        {
            File entry = root.openNextFile();
            while (entry)
            {
                file_count++;
                entry.close();
                entry = root.openNextFile();
            }
            root.close();
        }

        // Create filename like "log000.bin", "log001.bin", ...
        snprintf(log_filename, sizeof(log_filename), "log%03u.bin", (unsigned)file_count);
        Serial.print("Using log file: ");
        Serial.println(log_filename);

        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        log_file = SD.open(log_filename, FILE_WRITE);
        if (!log_file)
        {
            // if the file didn't open, print an error:
            Serial.println("error opening file.");
        }
        else
        {
            is_logging_engabled = true;
            log_file.close();
        }
    }

    gps_serial.begin(GPS_BAUD);
}

void loop()
{
    while (gps_serial.available() > 0)
    {
        if (rmc_parser.HandleByte((char)gps_serial.read()))
        {
            Serial.println("===");
            Serial.println(rmc_parser.last_parsed.vel);
            LightMode mode = light_ctrl.Update(rmc_parser.last_parsed.vel);
            switch (mode)
            {
            case LightMode::GREEN:
                digitalWrite(RED_LED_PIN, LOW);
                digitalWrite(YELLOW_LED_PIN, LOW);
                digitalWrite(GREEN_LED_PIN, HIGH);
                Serial.println('G');
                break;
            case LightMode::YELLOW:
                digitalWrite(RED_LED_PIN, LOW);
                digitalWrite(YELLOW_LED_PIN, HIGH);
                digitalWrite(GREEN_LED_PIN, LOW);
                Serial.println('Y');
                break;
            case LightMode::RED:
                digitalWrite(RED_LED_PIN, HIGH);
                digitalWrite(YELLOW_LED_PIN, LOW);
                digitalWrite(GREEN_LED_PIN, LOW);
                Serial.println('R');
                break;
            }

            WriteEntryToFile(rmc_parser.last_parsed);
        }
    }
}
