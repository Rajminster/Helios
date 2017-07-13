#include "SolarTracking.h"

/*
 * Variable for keeping track of how many times in a row all LDRs measure a
 * value below a certain threshold. Once this exceeds the LOW_READ macro in
 * SolarTracking.h, this library will assume that it is sunset and will turn
 * around the pane and will put the Arduino 101 into a low power sleep state
 */
u_int8_t times_low = 0;

/*
 * Boolean flag which signals to the Arduino 101 that it should enter a brief
 * period of sleep whenever the loop method is run. This flag is set to true if
 * The Sun is never found in the search method and if the LDRs all measure a low
 * reading which this library interprets as The Sun setting.
 *
 * This flag is set to false if The Sun is found in the search method, if at
 * least one LDR has a high reading, or if the of the Bluetooth application
 * makes the device search for The Sun again and it is found
 */
bool sleeping;

void setup()
{
    Serial.end();
    delay(100);

    /* Setup baud rate, pins, interrupt handling, and initialize Servo angles */
    initialize();

    /* Search for The Sun, sleep if sun wasn't found; track otherwise */
    sleeping = search();
}

void loop()
{
    int16_t* ldr_readings = read_ldr_all();

    /* If at least one LDR has a significant reading, stop sleeping */
    if (ldr_readings[0] >= LOW_READ || ldr_readings[1] >= LOW_READ
        || ldr_readings[2] >= LOW_READ || ldr_readings[3] >= LOW_READ) {
        sleeping = false;
        times_low = 0;
    }

    /*
     * If low readings have been measured on all LDRs consistently conclude it's
     * night; face the opposite direction (presumably East) and put Arduino 101
     * to sleep. Only turn East if The Sun has been found at least once
     */
    if (!sleeping && times_low >= LOW_TIMES) {
        /* Only move motor once */
        turn_east(); // face direction of sunrise
        sleeping = true;
    }

    /* If The Sun wasn't found or it's night time, only sleep */
    if (sleeping) {
        sleep(); // Arduino 101 is now sleeping briefly
    } else {
        /* If all sensors have low readings, increment times_low */
        if (ldr_readings[0] < LOW_READ && ldr_readings[1] < LOW_READ
            && ldr_readings[2] < LOW_READ && ldr_readings[3] < LOW_READ) {
            times_low++;
        } else {
            track();
        }
        delay(READ_DELAY); // only delay if not sleeping
    }
}
