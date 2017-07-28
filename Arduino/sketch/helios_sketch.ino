#include <CurieBLE.h>

#include "/../SolarTracking.h"

#define SEC_LOW         1 // where to begin the denominator for average
#define SEC_HIGH    86400 // number of seconds in a day
#define SEC_IN_HOUR  3600 // number of seconds in an hour

/*
 * Sketch to run the solar tracking algorithm and connect to any peripheral
 * devices via Bluetooth Low Energy (BLE).
 *
 * This sketch contains code which setups and creates the BLE service and
 * necessary characteristics for the LDR readings, the total energy generated,
 * a signal which controls initiating a search, and a signal which controls
 * the power of the device (on or off).
 *
 * Whenever this sketch is initially uploaded and run, the first action that
 * occurs setting up the necessary BLE variables so that the device can connect
 * to any peripheral devices. After this, initialization of pins, Serial
 * console, and moving Sero motors to their initial positions occurs. After this
 * the device will initiate a search for The Sun. If the search method (see
 * below) finds a significant light source, a flag which controls whether or not
 * the device sleeps will be set to false. If the search method performs
 * NUM_LOOP (see SolarTracking.h) 360 degree loops, it could not find The Sun,
 * so the flag which controls sleep will be set to true.
 *
 * The device now enters the loop method.
 *
 * At any point in this sketch's execution of the loop method (if a peripheral
 * device is  connected), the user of the Android application can set the power
 * characteristic (see below) to false which would then set the device's state
 * to one where it only sleeps for SLEEP_OFF (see SolarTracking.h) milliseconds.
 * The only way to exit this longer sleep loop is for the user to hit the power
 * button in the application again. This will set the power characteristic and
 * the off flag (see below) to false.
 *
 * Also at any point in the sketch's execution of loop (if not off and a
 * peripheral device is connected), the user of the Android application can hit
 * the search button which will force the device to initiate a search. This will
 * set the sleep flag to the return value of the search method.
 *
 * During normal execution of loop, first the device checks for any peripheral
 * BLE devices to connect to. If a device is connected, then loop will call the
 * run method (see below) which will run the solar tracking algorithm like
 * normal and will update the LDR and power readings to the application;
 * otherwise loop will call the run method but will not update any of the BLE
 * characteristics as this is not required. If a peripheral device is connected,
 * the first thing run will do will be to check if its state is off which will
 * force it to sleep for SLEEP_ON milliseconds. If the peripheral device
 * requests that the device be turned on, then run will acknowledge this request
 * by setting the characteristic to false and setting the power flag to false.
 *
 * If the device is not off, then run will read all LDR values and check if any
 * one sensor has a reading above LOW_READ (see SolarTracking.h). If this is the
 * case, then the sleeping flag will be set to false. If this method was called
 * with the bluetooth parameter as true (meaning it's being called while a
 * peripheral device is connected), the method will update the LDR and the
 * energy generated values seen on the application if necessary.
 *
 * The only difference in calling run with true or false as its parameter is
 * that the all BLE values won't be updated if the parameter is false.
 *
 * Next, run will check if the number of times all LDR sensors read below
 * LOW_READ is greater than or equal to LOW_TIMES. This would mean the all LDR
 * sensors read a significantly low reading LOW_TIMES in a row; the software
 * written here will assume that because of this, it is fair to assume that the
 * device is trying to received solar energy at or later than sunset. As a
 * result, the device will pan the entire pane 180 degrees clockwise. If the
 * assumption was correct this direction will be the general direction of East
 * which would be the general direction of sunrise. The device will now sleep
 * for every call to loop for SLEEP_OFF milliseconds until one of the LDRs
 * measures greater than or equal to LOW_READ.
 *
 * If the flag for sleeping is true, the device will go to sleep; if it's false,
 * then run will check if all LDRs are reading below LOW_READ. If so, the
 * variable times_low (see below) will be incremented; otherwise the track
 * method (see SolarTracking.cpp) will be called. Next avg_power (see below)
 * will be incremented by whatever the DC current sensor is reading, the number
 * of total readings for the average (see num_readings below) will be
 * incremented by one, and the device will delay by READ_DELAY (see
 * SolarTracking.h) milliseconds.
 *
 * This entire process of running loop (not sleeping, not off) will take
 * READ_DELAY milliseconds, READ_DELAY + SLEEP_ON milliseconds when sleeping,
 * and READ_DELAY + SLEEP_OFF if off.
 *
 * AUTHOR:
 * Nikola Istvanic
 */

BLEPeripheral blep;
BLEService st = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

/* BLE LDR Characteristics. Read and get notified when value changes */
BLEUnsignedCharCharacteristic nw_ldr_char(
    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);
BLEUnsignedCharCharacteristic ne_ldr_char(
    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);
BLEUnsignedCharCharacteristic sw_ldr_char(
    "6E400004-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);
BLEUnsignedCharCharacteristic se_ldr_char(
    "6E400005-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);
BLEDoubleCharacteristic energy_char(
    "6E400006-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);
BLEUnsignedCharCharacteristic power_char(
    "6E400007-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);
BLEUnsignedCharCharacteristic search_char(
    "6E400008-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);

BLEUnsignedCharCharacteristic characteristics[NUM_LDR] = {
    nw_ldr_char, ne_ldr_char, sw_ldr_char, se_ldr_char };

BLEDescriptor nw_descriptor = BLEDescriptor("2901", "NW LDR Percentage");
BLEDescriptor ne_descriptor = BLEDescriptor("2901", "NE LDR Percentage");
BLEDescriptor sw_descriptor = BLEDescriptor("2901", "SW LDR Percentage");
BLEDescriptor se_descriptor = BLEDescriptor("2901", "SE LDR Percentage");
BLEDescriptor energy_descriptor = BLEDescriptor(
    "2901", "Average Energy Reading");
BLEDescriptor power_descriptor = BLEDescriptor("2901", "Power signal");
BLEDescriptor search_descriptor = BLEDescriptor("2901", "Search signal");

u_int8_t nw = 0; // last NW LDR percentage reading
u_int8_t ne = 0; // last NE LDR percentage reading
u_int8_t sw = 0; // last SW LDR percentage reading
u_int8_t se = 0; // last SE LDR percentage reading
double energy = 0; // last energy average reading
u_int8_t old_readings[NUM_LDR] = { nw, ne, sw, se };

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

/*
 * If the user of the Android application wants to turn off the device, it
 * should stay off regardless of whether or not the LDRs measure a significantly
 * high reading. The only way to turn the device back on is for the user to do
 * so using the Android application.
 *
 * When the device is on, a high enough reading from an LDR sensor will cause
 * it to break from its sleep state; whenever it is off, however, the device
 * will ignore the LDR readings and stay sleeping until the user gives the
 * signal to wake up.
 */
bool off;

/*
 * Running average of the amount of Watts measured by the DC current sensor per
 * second of the device running. This is measured whether or not the device is
 * connected to a Bluetooth device.
 */
double avg_power;

/*
 * Number of readings performed by the Arduino of the DC current sensor. This
 * value will range from 1 to 86400, the number of seconds in a day. This value
 * will increase as soon as the device measures the DC current sensor; it is
 * used as the denominator of the running average avg_power (see above).
 */
u_int32_t num_readings;

/*
 * Method which will take the above defined variables and update their values
 * every READ_DELAY milliseconds. If there is a change in value, any peripheral
 * device connected to this Service via Bluetooth will be notified.
 */
void update_readings(int16_t* readings)
{
    int i;
    u_int8_t curr_reading;

    /* First update LDR values if necessary */
    for (i = 0; i < NUM_LDR; i++) {
        /* Convert raw reading to percentage */
        curr_reading = map(readings[i], 0, 1023, 0, 100);
        if (curr_reading != old_readings[i]) {
            Serial.print("\n***\n*** Updating LDR reading in ");
            Serial.print((sensor) i);
            Serial.print(" LDR to ");
            Serial.print(curr_reading);
            Serial.println("\n***");
            characteristics[i].setValue(curr_reading);
            old_readings[i] = curr_reading;
        }
    }

    /* Update energy (in Watt hours) reading if necessary every 30 seconds */
    if (!(num_readings % 30)
        && energy != VREF * avg_power / num_readings / SEC_IN_HOUR) {
        Serial.print("\n***\n*** Updating energy reading to ");
        energy = VREF * avg_power / num_readings / SEC_IN_HOUR;
        Serial.print(energy);
        Serial.println("\n***");
        energy_char.setValue(energy);
    }
}

/*
 * Method which controls when to track and when to put the Arduino 101 to sleep.
 *
 * First this method reads all LDR values. If this method is being executed
 * within the Bluetooth connected while loop, then it will update the Bluetooth
 * reported values if necessary.
 *
 * Whether or not this is being called within the Bluetooth block, the method
 * will then check if at least one LDR reading is greater than a certain
 * threshold to determine whether or not the device should exit its sleep state.
 *
 * If the number of times all LDRs consistently read low values exceeds
 * LOW_TIMES, then this program assumes it is now night time and turns the pane
 * containing the LDRs and solar panels around to what is assumed to be East,
 * and the Arduino 101 will now sleep for every call of this method.
 *
 * If the Arduino 101 is not in its sleep mode, then this method will check if
 * all readings are below the aforementioned low threshold. If so, the number of
 * times this has occurred is incremented; otherwise the pane is adjusted where
 * necessary to more closely face the direct path of The Sun.
 */
void run(bool bluetooth)
{
    /* If off, don't look at LDR readings; stay off until user allows it */
    if (off) {
        sleep(SLEEP_OFF);
        /* Change power status */
        if (power_char.value()) {
            power_char.setValue(0);
            off = false;
        }
    } else {
        int16_t* ldr_all = read_ldr_all();

        /* If at least one LDR has a significant reading, stop sleeping */
        if (ldr_all[0] >= LOW_READ || ldr_all[1] >= LOW_READ
            || ldr_all[2] >= LOW_READ || ldr_all[3] >= LOW_READ) {
            sleeping = false;
            times_low = 0;
        }

        /* Check if button for power or initiating a search were pressed */
        if (bluetooth) {
            update_readings(ldr_all);
            if (search_char.value()) {
                search_char.setValue(0);
                sleeping = search();
            }
            /* Change power status */
            if (power_char.value()) {
                power_char.setValue(0);
                off = true;
            }
        }

        /*
         * If low readings have been measured on all LDRs consistently conclude
         * it's night; face the opposite direction (presumably East) and put
         * Arduino 101 to sleep. Only turn East if The Sun has been found at
         * least once
         */
        if (!sleeping && times_low >= LOW_TIMES) {
            /* Move motor only once */
            turn_east(); // face direction of sunrise
            sleeping = true;
        }

        /* If The Sun wasn't found or it's night time, only sleep */
        if (sleeping) {
            sleep(SLEEP_ON); // Arduino 101 is now sleeping briefly
        } else {
            /* If all sensors have low readings, increment times_low */
            if (ldr_all[0] < LOW_READ && ldr_all[1] < LOW_READ
                && ldr_all[2] < LOW_READ && ldr_all[3] < LOW_READ) {
                times_low++;
            } else {
                track();
            }
            /* Update average energy reading */
            avg_power += read_power();
            num_readings =
                num_readings >= SEC_HIGH ? SEC_LOW : num_readings + 1;
            delay(READ_DELAY); // only delay if not sleeping
        }
    }
}

void setup()
{
    /* Setup Bluetooth connectivity after The Sun has been found */
    blep.setLocalName("Helios Panel");
    blep.setAdvertisedServiceUuid(st.uuid()); // add the service UUID
    blep.addAttribute(st);          // add the BLE service
    blep.addAttribute(nw_ldr_char); // add the LDR Characteristics for all LDRs
    blep.addAttribute(nw_descriptor);
    blep.addAttribute(ne_descriptor);
    blep.addAttribute(sw_descriptor);
    blep.addAttribute(se_descriptor);
    blep.addAttribute(energy_descriptor);
    blep.addAttribute(power_descriptor);
    blep.addAttribute(search_descriptor);
    blep.addAttribute(ne_ldr_char);
    blep.addAttribute(sw_ldr_char);
    blep.addAttribute(se_ldr_char);
    blep.addAttribute(energy_char);
    blep.addAttribute(power_char);
    blep.addAttribute(search_char);

    /* Initialize Bluetooth Characteristic values */
    nw_ldr_char.setValue(0);
    ne_ldr_char.setValue(0);
    sw_ldr_char.setValue(0);
    se_ldr_char.setValue(0);
    energy_char.setValue(0);
    power_char.setValue(0);
    search_char.setValue(0);

    /* Setup baud rate, pins, interrupt handling, and initialize Servo angles */
    initialize();

    /* Search for The Sun, sleep if sun wasn't found; track otherwise */
    off = false;
    sleeping = search();
    num_readings = SEC_LOW;

    blep.begin();
    Serial.println("\n***\n*** Bluetooth system activated, awaiting peripheral "
        "connection\n***");
}

void loop()
{
    /* Listen for BLE central (non-server) to connect to */
    BLECentral central = blep.central();

    /* If connected to a central device */
    if (central) {
        Serial.print("\n***\n*** Bluetooth peripheral device connected. MAC "
            "address: ");
        Serial.print(central.address());
        Serial.println("\n***");

        /* While connected to a device, update its LDR measurements */
        while (central.connected()) {
            run(true);
        }
        /* Disconnected */
        Serial.print("\n***\n*** Disconnected from central device: ");
        Serial.print(central.address());
        Serial.println("\n***");
    } else {
        /* Run like normal if not connected but don't update Bluetooth values */
        run(false);
    }
}
