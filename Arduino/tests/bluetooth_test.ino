/*
 * Set to 1 if you want to run this sketch. Other RUNNING macros should then be
 * set to 0 in order prevent multiple definitions of setup and loop.
 */
#define RUNNING 0
#if RUNNING

#include <CurieBLE.h>

#include "/../SolarTracking.h"

/*
 * Sketch to test Bluetooth connectivity from the Arduino 101 to any peripheral
 * devices. In this test, the value of the NW LDR sensor is read and outputted.
 * If a peripheral device is connected to the Arduino 101, then this device will
 * be able to read the nw_ldr_char and see how it periodically changes.
 *
 * AUTHOR:
 * Nikola Istvanic
 */

BLEPeripheral blep;
BLEService st = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

BLEUnsignedCharCharacteristic nw_ldr_char(
    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite | BLENotify);

BLEDescriptor nw_descriptor = BLEDescriptor("2901", "NW Reading Percentage");

u_int8_t nw = 0; // last NW LDR reading as a percentage

void update_ldr_reading(u_int16_t new_reading)
{
    u_int8_t curr_reading = map(new_reading, 0, 1023, 0, 100);

    /* Update Bluetooth characteristic value only when necessary */
    if (curr_reading != nw) {
        Serial.print("\n***\n*** Updating LDR reading in ");
        Serial.print((sensor) 0);
        Serial.print(" LDR to ");
        Serial.print(curr_reading);
        Serial.println("\n***");
        nw_ldr_char.setValue(curr_reading);
        nw = curr_reading;
    }
}

void setup()
{
    Serial.end();
    delay(100);
    Serial.begin(9600);

    /* Setup BLE */
    blep.setLocalName("Helios Panel");
    blep.setAdvertisedServiceUuid(st.uuid());
    blep.addAttribute(st);          
    blep.addAttribute(nw_ldr_char);
    blep.addAttribute(nw_descriptor);

    /* Initialize Bluetooth Characteristic value */
    nw_ldr_char.setValue(0);

    blep.begin();
    Serial.println("\n***\n*** Bluetooth system activated, awaiting peripheral "
        "connection\n***");
}

void loop()
{
    /* Listen for BLE central (non-server) to connect to */
    BLECentral central = blep.central();
    u_int16_t nw_current_reading = read_ldr((sensor) 0);

    /* If connected to a central device */
    if (central) {
        Serial.print(
            "\n***\n*** Bluetooth peripheral device connected. MAC address: ");
        Serial.print(central.address());
        Serial.println("\n***");

        /* While connected to a device, update its NW LDR measurement */
        while (central.connected()) {
            update_ldr_reading(nw_current_reading);
            delay(READ_DELAY);
        }
        /* Disconnected */
        Serial.print("\n***\n*** Disconnected from central device: ");
        Serial.print(central.address());
        Serial.println("\n***");
    } else {
        /* Run like normal if not connected, don't update characterisic value */
        Serial.print("\n***\n*** LDR reading in ");
        Serial.print((sensor) 0);
        Serial.print(": ");
        Serial.print(nw_current_reading);
        Serial.println("\n***");
        delay(READ_DELAY);
    }
}

#endif
