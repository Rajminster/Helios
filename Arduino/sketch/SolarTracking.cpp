#include <Arduino.h>
#include <Power.h>
#include <Servo.h>
#include <stdlib.h>
#include <wsrc.h>

#include "SolarTracking.h"

/*
 * Servo motor for panning the entire panel system. This motor will move in 180
 * degrees
 */
Servo pan;

/* Current angle for the pan Servo motor */
u_int16_t pan_angle;

/*
 * Servo motor for tilting the entire pane where the panels and LDRs are
 * located. This motor will only move from an angle of 0 degrees (where the pane
 * would be perpendicular to the ground) to 180 degrees (where the pane would be
 * perpendicular to the ground, facing the opposite direction)
 */
Servo tilt;

/* Current angle for the tilt Servo motor */
u_int8_t tilt_angle;

void initialize()
{
    /* Setup baud rate as 9600 */
    Serial.end();
    delay(100);
    Serial.begin(9600);
    Serial.println("\n***\n*** Initializing pin connections\n***");

    /* Setup digital pins for Servo motors */
    pan.attach(12);
    tilt.attach(13);

    /* Move motors to their starting positions */
    Serial.println("\n***\n*** Motors approaching initial positions\n***");
    pan_pane(PAN_INIT); // make sure motor is at 0 degrees
    tilt_pane(TILT_INIT); // 45 degrees to prevent pane from shading all LDRs
    delay(LARGE_ANGLE); // wait for tilt to finish moving
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
    tilt_pane(TILT_INIT);
    pan_pane(pan_angle + 180);
    delay(LARGE_ANGLE);
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

double read_power()
{
    /*
     * Power reading
     * current = ADC value * 5V/1023 (for Arduino) / Rs * Rl
     * => the converted sensor value is Vout * 1k Ohms, and the Load resistor
     * is 10k ohms
     */
    double sensorVal = analogRead(DC_PIN) * VREF / 1023;
    double current = sensorVal / 100;
    double v = sensorVal / 1000; // voltage = sensor value / 1k Ohms
    Serial.print("\n***\n*** Power generated: ");
    Serial.print(current * v);
    Serial.println(" Watts\n***");
    return current * v; // Power = V * I
}

int16_t _get_dh()
{
    /* Average Western labeled LDRs and Eastern labeled LDRs */
    int16_t west_avg = (read_ldr(NW) + read_ldr(SW)) / 2;
    int16_t east_avg = (read_ldr(NE) + read_ldr(SE)) / 2;

    /*
     * Find difference in Western and Eastern readings to determine if pan Servo
     * motor should change angle
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
        delay(LARGE_ANGLE);
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
            pan_pane(pan_angle + DA);
            temp_angle += DA;
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
     * cases.
     *
     * Check if the horizontal reading difference exceeds the track tolerance.
     * This would mean the pan Servo motor should rotate either clockwise or
     * counterclockwise, depending on the sign of dh, in order to more directly
     * face The Sun
     */
    if (abs(dh) > TRACK_DIFF) {
        if (dh > 0) {
            /*
             * If the LDRs labeled as Western receive more light, then depending
             * on the angle of tilt the device should move either clockwise or
             * counterclockwise. If the angle is less than 90 degrees, then the
             * device should move clockwise. If the tilt angle is greater than
             * 90 degrees (West and East will be switched) then move in the
             * opposite direction, counterclockwise
             */
            pan_pane(tilt_angle <= 90 ? pan_angle + 1 : pan_angle - 1);
        } else {
            /*
             * LDRs labeled as East receive more light. In this case, depending
             * on the angle of pane tilt, the device should either rotate
             * clockwise or counterclockwise. If the tilt angle is less than or
             * equal to 90 degrees, then the device should rotate
             * counterclockwise; otherwise it should rotate clockwise
             */
            pan_pane(tilt_angle <= 90 ? pan_angle - 1 : pan_angle + 1);
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
            tilt_pane(tilt_angle - 1); // decrease tilt angle
        } else {
            /* LDRs labeled as Southern receive more light */
            tilt_pane(tilt_angle + 1); // increase tilt angle
        }
    }
}

void pan_pane(u_int16_t angle)
{
    /* Make sure angle is within the range 0 to 360 */
    angle %= PAN_MAX + 1;

    if (angle > TILT_MAX) {
        /*
         * Emulate 360 degree motion by flipping the pane and writing to pan
         * 180 - angle to make the pane face what would be 180 to 360 degrees
         */
        if (pan_angle <= TILT_MAX) {
            /* Flip pane once */
            tilt_pane(TILT_MAX - tilt_angle);
            pan.write(angle - TILT_MAX);
            delay(LARGE_ANGLE); // give time to flip pane and sweep pan motor
        } else {
            pan.write(angle - TILT_MAX);
            delay(SMALL_ANGLE);
        }
    } else {
        if (pan_angle > TILT_MAX) {
            /* Flip pane back to normal if needed */
            tilt_pane(TILT_MAX - tilt_angle);
            pan.write(angle);
            delay(LARGE_ANGLE);
        } else {
            pan.write(angle);
            delay(SMALL_ANGLE);
        }
    }
    pan_angle = angle;
}

void tilt_pane(u_int8_t angle)
{
    /* Never tilt the pane past 180 degrees as hardware is in the way */
    if (angle <= TILT_MAX) {
        tilt_angle = angle;
        tilt.write(tilt_angle);
    }
}
