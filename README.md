# Helios

Named after the Greek god of The Sun, Helios is the combination of a solar tracking C++ library, an Arduino 101 sketch, and an Android application which serves to track The Sun's movement so that it's always directly facing the device on which the software is running.

## Hardware

In order to setup a device ideal for the software written in this repository, the following hardware components should be assembled:
- An Intel Arduino 101
- Four Light Dependent Resistor (LDR) sensors
- Four 100 Ω resistors
- A solar panel (5'' x 6'')
- Two standard Servo motors
- Analog DC current sensor

Use the following wiring diagram for assembling the hardware and correctly hooking up all components:

## Software

The software in this repository is organized into a solar tracking library, an Arduino sketch, and an Android application.

The library *SolarTracking* defines constants and methods which move the Servo motors to search for the relative position of The Sun. Once found, The Sun will be finely tracked using a method which determines which direction the pane containing the LDRs and solar panel should turn to next as The Sun moves across the sky.

The Arduino sketch *helios_sketch* sets up many of the Bluetooth functionality as well as a few methods which utilize those defined in the library. One built-in feature of the sketch is if the LDRs measure a defined low value consistently for a certain number of measurements in a row, the device will then assume that it is sunset and that the amount of power drawn at this point would be lower than is useful. The pane is then shifted to face East, the direction of sunrise while it sleeps in a deep sleep to conserve on power usage over the course of the night. If at any point during this sleep one of the LDRs measures a significant reading, the device will initiate a search.

The Android application both displays the percentages of the LDR readings as well as the amount of power generated. The application allows the user to turn the device on and off in addition to prompt it to initiate a search for The Sun. Turning the device in essence forces the device to go to a long deep sleep. This is different to the deep sleep mentioned in the description of the Arduino sketch in that in this sleep, all LDR readings are disregarded. The only way to wake up from this sleep would be for the user to turn the device back on. In addition to controlling the device's power state, the user can also initiate a search at any point in the device's lifetime.

The following libraries are required for this software:
- [Arduino101Power](https://github.com/bigdinotech/Arduino101Power)
-- For use of the method **deepSleep** to put the Arduino 101 in a deep sleep when the user turns it off
- [Servo](https://github.com/arduino-libraries/Servo)
-- To control the Servo motors' movement using the **attach** and **write** methods
- [CurieBLE](https://github.com/01org/corelibs-arduino101/tree/master/libraries/CurieBLE)
-- For all things Bluetooth Low Energy (BLE)
- [Android Bluetooth Low Energy](https://developer.android.com/guide/topics/connectivity/bluetooth-le.html)
-- For control of Android interaction with the Arduino 101
- [RxAndroidBle](https://github.com/Polidea/RxAndroidBle)
-- For control of application connectivity to the Arduino 101
