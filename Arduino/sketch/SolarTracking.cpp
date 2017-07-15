#include <Arduino.h>
#include <Power.h>
#include <stdlib.h>
#include <wsrc.h>

#include "SolarTracking.h"

void initialize()
{
    /* Setup baud rate as 9600 */
    Serial.end();
    delay(100);
    Serial.begin(9600);
    Serial.println("\n***\n*** Initializing pin connections\n***");

    /* Setup digital pins for Servo motors */
    pan.attach(9);
    tilt.attach(10);

    /* Move motors to their starting positions */
    Serial.println("\n***\n*** Motors approaching initial positions\n***");
    pan_pane(PAN_INIT);
    tilt_pane(TILT_INIT); // 45 degrees to prevent pane from shading all LDRs

    /* Wait for motors to finish moving */
    delay(WRITE_DELAY);
}

void sleep(u_int32_t duration)
{
    Serial.println("\n***\n*** Shutting down power systems\n***");

    /* Go into a deepSleep for duration milliseconds */
    PM.deepSleep(duration);

    /* Arduino 101 is awake whenever it reaches here */
    Serial.println("\n***\n*** Main power online\n***");
}

void turn_east()
{
    /* Turn pane around to face direction of sunrise */
    Serial.println("\n***\n*** No power detected, moving panels East\n***");
    pan_pane(pan_angle + 180);
    tilt_pane(TILT_INIT);
    delay(WRITE_DELAY); // wait for motors to reach their destinations
}

int16_t read_ldr(sensor ldr)
{
    int16_t reading = analogRead(ldr);
    delay(1); // delay to ensure read is complete
    Serial.print("\n***\n*** Reading from ");
    Serial.print(ldr);
    Serial.print(" LDR: ");
    Serial.print(reading);
    Serial.println("\n***");
    return reading;
}

int16_t* read_ldr_all()
{
    int i;
    static int16_t ret[NUM_LDR];

    for (i = 0; i < NUM_LDR; i++) {
        ret[i] = read_ldr((sensor) i);
    }
    return ret;
}

int16_t _get_dh()
{
    /* Average Western labeled LDRs and Eastern labeled LDRs */
    int16_t west_avg = (read_ldr(NW) + read_ldr(SW)) / 2;
    int16_t east_avg = (read_ldr(NE) + read_ldr(SE)) / 2;

    /*
     * Find difference in Western and Eastern readings to determine if pan
     * Servo motor should change angle
     */
    return west_avg - east_avg;
}

int16_t _get_dv()
{
    /* Average Northern labeled LDRs and Southern labeled LDRs */
    int16_t north_avg = (read_ldr(NW) + read_ldr(NE)) / 2;
    int16_t south_avg = (read_ldr(SW) + read_ldr(SE)) / 2;

    /*
     * Find difference in Northern and Southern readings to determine if tilt
     * Servo motor should change angle
     */
    return north_avg - south_avg;
}

bool search()
{
    int i;
    int16_t* readings;
    u_int16_t temp_angle = 0; // temp to know when a full rotation has occurred
    /* Make sure pane is tilted at a 45 degree */
    if (tilt_angle != TILT_INIT) {
        tilt_pane(TILT_INIT);
        delay(WRITE_DELAY);
    }

    /* Perform NUM_LOOP loops to search for a significant light source */
    Serial.println("\n***\n*** Initiating search for power source\n***");
    for (i = 0; i < NUM_LOOP; i++) {
        /* Rotate the device fully once */
        while (temp_angle <= PAN_MAX) {
            readings = read_ldr_all();
            Serial.print("\n***\n*** On search ");
            Serial.print(i);
            if (readings[0] > SEARCH_TOL || readings[1] > SEARCH_TOL
                || readings[2] > SEARCH_TOL || readings[3] > SEARCH_TOL) {
                /* Significant light intensity found; don't sleep */
                Serial.println("\n*** Power source detected. Initiating light "
                    "tracking protocol\n***");
                return false;
            }
            Serial.println("\n*** No significant power source detected\n***");
            pan_pane(pan_angle + 5);
            delay(WRITE_DELAY);
            temp_angle += 5;
        }
        temp_angle = 0;
    }
    /*
     * Sun not found; begin sleeping until Sun appears in front of the LDR pane
     * or a user wakes up the device using the application
     */
    Serial.println("\n***\n*** Error: no power detected. Initiating low power "
        "protocol\n***");
    return true;
}

void track()
{
    /* Get changes in readings across horizontal and vertical LDR axes */
    int16_t dh = _get_dh();
    int16_t dv = _get_dv();

    /*
     * Handle horizontal readings first to prevent any 90 degree tilt angle edge
     * cases
     */

    /*
     * Check if the horizontal reading difference exceeds the track tolerance.
     * This would mean the pan Servo motor should rotate either clockwise or
     * counterclockwise, depending on the sign of dh, in order to more directly
     * face The Sun
     */
    if (abs(dh) > TRACK_DIFF) {
        /* Side labeled as West has more light */
        if (dh > 0) {
            /*
             * If the LDRs labeled as Western receive more light, then depending
             * on the angle of tilt the device should move either clockwise or
             * counterclockwise. If the angle is less than 90 degrees, then the
             * device should move clockwise (increase in rotational angle). If
             * the tilt angle is greater than 90 degrees (West and East will be
             * switched) then move in the opposite direction, counterclockwise
             */
            pan_pane(pan_angle + (tilt_angle <= 90 ? 1 : -1));
        } else {
            /*
             * LDRs labeled as East receive more light. In this case, depending
             * on the angle of pane tilt, the device should either rotate
             * clockwise or counterclockwise. If the tilt angle is less than or
             * equal to 90 degrees, then the device should rotate
             * counterclockwise; otherwise it should rotate clockwise
             */
            pan_pane(pan_angle + (tilt_angle <= 90 ? -1 : 1));
        }
    }
    /*
     * Check if the vertical reading difference exceeds the track tolerance.
     * This would mean the pane Servo motor should angle toward the sky or
     * toward the ground, depending on the sign of dv, in order to more directly
     * face The Sun
     */
    if (abs(dv) > TRACK_DIFF) {
        if (dv > 0) {
            /* LDRs labeled as Northern receive more light */
            tilt_pane(tilt_angle + 1); // increase tilt angle
        } else {
            /* LDRs labeled as Southern receive more light */
            tilt_pane(tilt_angle - 1); // decrease tilt angle
        }
    }
}

void pan_pane(u_int16_t angle)
{
    /* Make sure pan_angle is within the range 0 to 360 */
    pan_angle = angle % (PAN_MAX + 1);
    pan.write(pan_angle);
}

void tilt_pane(u_int8_t angle)
{
    /* Never tilt the pane past 180 degrees as hardware is in the way */
    if (angle <= TILT_MAX) {
        tilt_angle = angle;
        tilt.write(tilt_angle);
    }
}
