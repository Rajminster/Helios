/*
 * Set to 1 if you want to run this sketch. Other RUNNING macros should then be
 * set to 0 in order prevent multiple definitions of setup and loop.
 */
#define RUNNING 0
#if RUNNING

#include <CurieTime.h>

/*
 * Test sketch to output Unix time. Time would have been used to be a second
 * test in the conditional which determines if the devices goes to sleep.
 * CurieTime was not used because it was unreliable in outputting the current
 * time and only gave Unix time near its beginning whenever the device was
 * booted.
 *
 * AUTHOR:
 * Rishi Raj
 */
void setup() {
    Serial.end();
    delay(100);
    Serial.begin(9600);
    Serial.println(now());
    Serial.println(year());
    Serial.println(month());
    Serial.println(day());
    Serial.println(hour());
    Serial.println(minute());
    Serial.println(second());
}

void loop() {}

#endif
