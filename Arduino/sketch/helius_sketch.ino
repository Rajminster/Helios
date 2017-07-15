#include <CurieBLE.h>

#include "SolarTracking.h"

BLEPeripheral blep;
BLEService st = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

/* BLE LDR Characteristics. Read and get notified when value changes */
BLECharacteristic nw_ldr_char("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLERead
    | BLENotify, 2);
BLECharacteristic ne_ldr_char("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLERead
    | BLENotify, 2);
BLECharacteristic sw_ldr_char("6E400004-B5A3-F393-E0A9-E50E24DCCA9E", BLERead
    | BLENotify, 2);
BLECharacteristic se_ldr_char("6E400005-B5A3-F393-E0A9-E50E24DCCA9E", BLERead
    | BLENotify, 2);
// BLECharacteristic energy_char("6E400006-B5A3-F393-E0A9-E50E24DCCA9E", BLERead
//    | BLENotify, 2);

BLECharacteristic characteristics[4] = { nw_ldr_char, ne_ldr_char, sw_ldr_char,
    se_ldr_char };

BLEDescriptor nw_descriptor = BLEDescriptor("2901", "NW Labeled LDR");
BLEDescriptor ne_descriptor = BLEDescriptor("2901", "NE Labeled LDR");
BLEDescriptor sw_descriptor = BLEDescriptor("2901", "SW Labeled LDR");
BLEDescriptor se_descriptor = BLEDescriptor("2901", "SE Labeled LDR");
//BLEDescriptor energy_descriptor = BLEDescriptor("2901", "Energy Generated");

// u_int16 energy = 0; // last energy reading
int16_t nw = 0; // last NW LDR reading
int16_t ne = 0; // last NE LDR reading
int16_t sw = 0; // last SW LDR reading
int16_t se = 0; // last SE LDR reading

int16_t old_readings[NUM_LDR] = { nw, ne, sw, se };

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
bool off = false;

/*
 * Method which will take the above defined variables and update their values
 * every DELAY milliseconds. If there is a change in value, any peripheral
 * device connected to this Service via Bluetooth will be notified.
 */
void update_ldr_readings(std::vector<int16_t> readings)
{
    int i;

    /* Check for changes in LDR readings */
    for (i = 0; i < NUM_LDR; i++) {
        /* Change in LDR measurement */
        u_int16_t curr_reading = map(readings[i], 0, 1023, 0, 100);
        if (curr_reading != old_readings[i]) {
            Serial.print("\n***\n*** Updating LDR reading in ");
            Serial.print((sensor) i);
            Serial.print(" LDR to ");
            Serial.print(curr_reading);
            Serial.println("\n***");
            const unsigned char new_LDR_reading[2] = {
                0, (unsigned char) curr_reading
            };
            /* update the current measurement characteristic */
            characteristics[i].setValue(new_LDR_reading, 2);
            old_readings[i] = curr_reading;
        }
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
        /* TODO
        if (on_button_pressed) {
            off = false;
        }
        */
    } else {
        std::vector<int16_t> ldr_all = read_ldr_all();
    
        /* If at least one LDR has a significant reading, stop sleeping */
        if (ldr_all[0] >= LOW_READ || ldr_all[1] >= LOW_READ
            || ldr_all[2] >= LOW_READ || ldr_all[3] >= LOW_READ) {
            sleeping = false;
            times_low = 0;
        }
    
        /* Check if the button for initiating a search has been pressed */
        if (bluetooth) {
            update_ldr_readings(ldr_all);
            /* TODO
            if (button_pressed) {
                sleep = search();
            }
            */
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
            delay(READ_DELAY); // only delay if not sleeping
        }
    }
}

void setup()
{
    /* Setup Bluetooth connectivity after The Sun has been found */
    blep.setLocalName("Helius Panel");
    blep.setAdvertisedServiceUuid(st.uuid()); // add the service UUID
    blep.addAttribute(st);          // add the BLE service
    blep.addAttribute(nw_ldr_char); // add the LDR Characteristics for all LDRs
    blep.addAttribute(nw_descriptor);
    blep.addAttribute(ne_descriptor);
    blep.addAttribute(sw_descriptor);
    blep.addAttribute(se_descriptor);
    blep.addAttribute(ne_ldr_char);
    blep.addAttribute(sw_ldr_char);
    blep.addAttribute(se_ldr_char);
    // blep.addAttribute(energy_Char);

    /* Initialize Bluetooth Characteristic values */
    const unsigned char nw_char_array[2] = { 0, (unsigned char) 0 };
    const unsigned char ne_char_array[2] = { 0, (unsigned char) 0 };
    const unsigned char sw_char_array[2] = { 0, (unsigned char) 0 };
    const unsigned char se_char_array[2] = { 0, (unsigned char) 0 };
    // const unsigned char energy_char_array[2] = { 0, (unsigned char) 0 };

    nw_ldr_char.setValue(nw_char_array, 2);
    ne_ldr_char.setValue(ne_char_array, 2);
    sw_ldr_char.setValue(sw_char_array, 2);
    se_ldr_char.setValue(se_char_array, 2);
    // energyChar.setValue(energy_char_array, 2);

    /* Setup baud rate, pins, interrupt handling, and initialize Servo angles */
    initialize();

    /* Search for The Sun, sleep if sun wasn't found; track otherwise */
    sleeping = search();

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