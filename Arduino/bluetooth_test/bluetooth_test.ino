#include <CurieBLE.h>

#include "SolarTracking.h"

BLEPeripheral blep;
BLEService st = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

BLECharacteristic nw_ldr_char("6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
                              BLERead | BLENotify, 2);

BLEDescriptor nw_descriptor = BLEDescriptor("2901", "NW Labeled LDR");

int16_t nw = 0; // last NW LDR reading as a percentage

void update_ldr_readings(int16_t new_reading)
{
  u_int16_t curr_reading = map(new_reading, 0, 1023, 0, 100);
  if (curr_reading != nw) {
    Serial.print("\n***\n*** Updating LDR reading in NW LDR to ");
    Serial.print(curr_reading);
    Serial.println("\n***");
    const unsigned char new_LDR_reading[2] = {
      0, (unsigned char) curr_reading
    };
    /* update the current measurement characteristic */
    nw_ldr_char.setValue(new_LDR_reading, 2);
    nw = curr_reading;
  }
}

void setup()
{
  Serial.end();
  delay(100);
  Serial.begin(9600);
  /* Setup Bluetooth connectivity after The Sun has been found */
  blep.setLocalName("Solar Tracker");
  blep.setAdvertisedServiceUuid(st.uuid()); // add the service UUID
  blep.addAttribute(st);          // add the BLE service
  blep.addAttribute(nw_ldr_char); // add the LDR Characteristics for all LDRs
  blep.addAttribute(nw_descriptor);

  /* Initialize Bluetooth Characteristic values */
  const unsigned char nw_char_array[2] = { 0, (unsigned char) 0 };

  nw_ldr_char.setValue(nw_char_array, 2);

  blep.begin();
  Serial.println(
    "\n***\n*** Bluetooth system activated, awaiting peripheral connection\n***");

}

void loop()
{
  /* Listen for BLE central (non-server) to connect to */
  BLECentral central = blep.central();

  u_int16_t nw_curr_reading = SolarTracking::read_ldr((sensor) 0);

  /* If connected to a central device */
  if (central) {
    Serial.print(
      "\n***\n*** Bluetooth peripheral device connected. MAC address: ");
    Serial.print(central.address());
    Serial.println("\n***");

    /* While connected to a device, update its LDR measurements */
    while (central.connected()) {
      Serial.print("READING IS ");
      Serial.println(nw_curr_reading);
      update_ldr_readings(nw_curr_reading);
      delay(1000);
    }
    /* Disconnected */
    Serial.print("\n***\n*** Disconnected from central device: ");
    Serial.print(central.address());
    Serial.println("\n***");
  } else {
    /* Run like normal if not connected but don't update Bluetooth values */
    Serial.print("READING IS ");
    Serial.println(nw_curr_reading);
    delay(1000);
  }
}
