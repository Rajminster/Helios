/*
 * Set to 1 if you want to run this sketch. Other RUNNING macros should then be
 * set to 0 in order prevent multiple definitions of setup and loop.
 */
#define RUNNING 0
#if RUNNING

#include "/../SolarTracking.h"

/*
 * Simple sketch to run the solar tracking algorithm. This sketch is similar to
 * /../sketch/helios_sketch except this sketch has no Bluetooth capabilities.
 * This means that this sketch can be run on an Arduino Uno.
 *
 * The basic order of this sketch is that setup will initialize the Serial
 * console, attach the Servo motor pins to the correct digital pins, and move
 * both motors to their initial angles (0 for the pan motor, and 45 for the tilt
 * motor).
 *
 * Then setup will call the method search (see SolarTracking.cpp) and set the
 * return value of that method to a boolean flag which determines if the device
 * should go to sleep to save power. The flag will be true if the device was
 * unable to find a significant light source; otherwise it will be false to
 * indicate not to go to sleep for every call to loop.
 *
 * Now the loop method will be called. Here first all LDR sensors are read.
 * The values of all sensors will be checked to see if one reads above a small
 * value (see LOW_READ in SolarTracking.h). If this is the case, then the sleep
 * flag will be set to false (in case the device was sleeping, it would stop 
 * this endless sleep cycle).
 * 
 * If the device is not sleeping and all LDR readings are below LOW_READ for
 * LOW_TIMES (see SolarTracking.h) times, then the device will assume that it is
 * night. It will move the pane containing the solar panel and LDRs to face what
 * the library considers the general direction of East. This is because if the
 * assumption of night was correct, facing East will be the general direction
 * of sunrise. When facing East, the device will be checking the LDR values if
 * any one LDR measures above LOW_READING as previously described. If this does
 * not occur, then the device will go to sleep for SLEEP_ON (see
 * SolarTracking.h) milliseconds.
 *
 * If the device is not sleeping and all LDR readings are above READ_LOW, then
 * loop will check if all LDRs are measuring below LOW_READ. If so, the variable
 * num_times (see below) will be incremented; otherwise the track method (see
 * SolarTracking.cpp) will be called.
 *
 * This entire process of running loop (not sleeping) will take READ_DELAY
 * milliseconds; otherwise will take READ_DELAY + SLEEP_ON milliseconds.
 *
 * AUTHOR:
 * Nikola Istvanic
 */

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
        sleep(SLEEP_ON); // Arduino 101 now sleeping for SLEEP_ON milliseconds
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

#endif
