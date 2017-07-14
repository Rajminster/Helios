/*
 * Solar Tracking Library.
 *
 * This library provides methods for interacting with an Intel Arduino 101,
 * solar panels, LDRs, and Servo motors with the intent of finding and tracking
 * the motion of The Sun in order to generate the most amount of power through
 * the solar panels used.
 *
 * Library may also apply to Arduino Uno; however, the Arduino sketch
 * tracker.ino only applies to the Arduino 101 as it uses the 101's built in
 * Bluetooth connectivity.
 *
 * This library is used whenever communication between the Arduino 101 and LDRs
 * is required. This library allows for control of Servo motor angle with or
 * without appropriate signals from one of four LDRs.
 *
 * For more information on the Arduino 101 board, refer to:
 * https://software.intel.com/en-us/iot/hardware/curie/dev-kit
 *
 * In order to track The Sun, four LDR sensors are attached to a board in a
 * quadrant formation. These sensors are all separated from one another by small
 * walls which form a cross/plus sign. If The Sun isn't in the direct path of
 * the cross's center, then at least one of the sensors will get a significantly
 * higher or lower reading than the others as it may be shaded by this small
 * wall which serves as a signal to pan and/or tilt the entire pane containing
 * all LDRs and solar panels. This process ensures that all solar panels receive
 * as much direct sunlight as possible.
 *
 * If two LDRs in the row labeled as North receive more sunlight than two in the
 * Southern row, then the pane must tilt such that the Servo motor controlling
 * the tilt increases in angle; if the Southern row receives more, then the pane
 * must tilt at a decreasing angle.
 *
 * This library allows the pane containing the solar panels and LDR sensors to
 * track The Sun in 360 degrees of rotational motion and 180 degrees of tilting
 * motion.
 *
 * Motion of the pane is controlled in two dimensions through two different
 * Servo motors. The entire pane containing the LDRs and solar panels is rotated
 * (or panned) through a 360 degree Servo motor which is used in
 * SolarTracker.cpp as pan. The pane itself where the LDRs are is tilted by
 * another Servo motor known as tilt. This motor can only go from 0 degrees to
 * 180 degrees. Since pan has a range of values greater than 255, its angle is
 * represented by a unsigned short; tilt's angle is represented by an unsigned
 * char.
 *
 * Whenever all four of the device's LDRs measure a consistently low reading or
 * if The Sun wasn't found on its initial search, the device will enter a deep
 * sleep mode each time the Arduino loop method executes. This deep sleep will
 * last for SLEEP_DURATION (see below) milliseconds. In order to break this deep
 * sleep cycle either at least one LDR must measure a significant reading or a
 * user needs to initiate a search using the external application.
 *
 * CREATED:
 * 2017-06-24
 *
 * AUTHOR:
 * Nikola Istvanic
 */
#ifndef SolarTracking_h
#define SolarTracking_h

/* INCLUDE */

#include <Servo.h>

/* DEFINES */
#define SEARCH_TOL 500 // higher tolerance for initially finding The Sun
#define TRACK_DIFF  50 // reading difference tolerance when tracking The Sun
#define LOW_READ    40 // LDR reading which is considered low
#define LOW_TIMES   10 // assume night when all LDRs read low this many times
#define NUM_LDRS     4 // number of LDR sensors
#define NUM_LOOP     2 // number of loops to make when searching for The Sun

#define READ_DELAY  1000 // millisecond delay between LDR readings
#define WRITE_DELAY 2000 // millisecond delay for Servo motor movement
#define SLEEP_DELAY 3000 // millisecond duration for sleep

/* ENUM */
/*
 * Enum for LDRs which are attached to the pane. When facing the pane while the
 * tilt angle is less than 90 degrees, the top left LDR is specified as NW, the
 * top right lDR is NE, the bottom left is SW, and the bottom right is SE.
 *
 * NOTE: this library assumes that analog pins 0, 1, 2, and 3 are connected to
 * LDRs NW, NE, SW, and SE, respectfully.
 */
enum sensor { NW, NE, SW, SE };

/* TYPEDEFS */
typedef unsigned char u_int8_t; // 8-bit unsigned integer [0 - 255]
typedef signed short int16_t; // 16-bit signed integer [-32768 - 32767]
typedef unsigned short u_int16_t; // 16-bit unsigned integer [0 - 65535]

/* CONSTANTS */
const u_int16_t PAN_INIT = 0; // pan Servo motor initial angle
const u_int16_t PAN_MAX = 360; // pan Servo motor upper bound
const u_int8_t TILT_INIT = 45; // tilt Servo motor initial angle
const u_int8_t TILT_MAX = 180; // tilt Servo motor upper bound

/* GLOBAL VARIABLES */
/*
 * Servo motor for panning the entire panel system, must be able to pan
 * in 360 degrees
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

/* Current angle for the pane Servo motor */
u_int8_t tilt_angle;

/* PROTOTYPES */
/*
 * Setup all necessary variables to their appropriate starting values as this
 * method is called first on upload/Arduino 101 boot.
 *
 * Set baud rate as 9600.
 *
 * Attach pan and tilt Servo motor pins to their board values; this is done in
 * this library because the Servo motor variables must be defined in
 * SolarTracking.cpp.
 *
 * After the write digital pins have been attached, write the correct starting
 * angles for both motors. For the tilt Servo motor, this angle will be 0
 * degrees (meaning the pane with the LDRs and solar panels will be
 * perpendicular to the ground); for the pan Servo motor, this angle will also
 * be 0 degrees
 *
 * Wait for WRITE_DELAY milliseconds for both motors to go to their written
 * values.
 */
void initialize();

/*
 * Puts the Arduino 101 into a low power sleep mode.
 *
 * The Arduino 101 enters a sleep mode where significantly less power is used.
 * The device only goes to sleep for a finite amount of time which is defined
 * in SolarTracking.h. This duration, SLEEP_DURATION, is the amount of
 * milliseconds the device will sleep for.
 *
 * During normal execution, if the Arduino 101 needs to go to sleep, for every
 * call to loop, the device will check if it needs to exit this deep sleep. If
 * so, it will continue tracking The Sun; otherwise the device will enter this
 * deep sleep mode for SLEEP_DURATION and will continue to do so as long as the
 * device needs to be in sleep mode.
 */
void sleep();

/*
 * Makes the pane containing the LDRs and solar panels face what this library
 * thinks is the direction of sunrise, East.
 *
 * Whenever the device is facing sunset and it captures the last rays of
 * sunlight, the LDRs will all measure a low reading after a certain period of
 * time. This library sees these conditions as reason enough to turn the device
 * around in order to face the approximate direction of sunrise.
 *
 * The angle which the device will now face is the current angle + 180 degrees
 * mod 361 so the value will be from 0 to 360. This new angle should make the
 * device face the opposite direction which (if the assumption of sunset was
 * correct) should be the general direction of sunrise.
 */
void turn_east();

/*
 * Read a single LDR sensor and return that value as a signed short int.
 *
 * This method takes in an enum which is declared above and returns the value of
 * an analogRead of that analog pin. The value of the sensor read is printed as
 * well.
 */
int16_t read_ldr(sensor ldr);

/*
 * Reads all LDR sensors, outputs, and returns int16_t pointer with these
 * values.
 *
 * The signed short pointer will be in the following format:
 *     Index 0: NW LDR reading
 *     Index 1: NE LDR reading
 *     Index 2: SW LDR reading
 *     Index 3: SE LDR reading
 *
 * where NW, NE, SW, and SE are the labeled cardinal directions on the pane near
 * each LDR.
 */
int16_t* read_ldr_all();

/*
 * Read values from the NW and SW labeled LDRs, read values from the NE and SE
 * LDRs, take average of each, subtract Eastern average from Western average,
 * and return.
 *
 * If this value is between a -tolerance and +tolerance, then there isn't a
 * large enough disparity between Western and Eastern LDR readings to cause the
 * pane to move.
 *
 * If this value is below -tolerance, then there is more sunlight on the Eastern
 * labeled LDRs; if it's greater than +tolerance, then the Western labeled LDRs
 * are receiving more light.
 */
int16_t _get_dh();

/*
 * Read values from the NW and NE labeled LRDs, read values from the SW and SE
 * labeled LDRs, take average of each, subtract Southern average from Northern
 * average, and return.
 *
 * If this value is between a -tolerance and +tolerance, then there is no need
 * to change the angle of pane tilt.
 *
 * If this value is below -tolerance, then there is more sunlight on the
 * Southern labeled LDRs, so the tilt angle is decreased; if the value is
 * greater than +tolerance, then the Northern labeled LDRs are receiving more
 * sunlight, so the tilt angle is increased.
 */
int16_t _get_dv();

/*
 * Search for The Sun or a significantly intense source of light, starting at
 * the initial angle for the pan Servo motor, rotating clockwise.
 *
 * Tolerance for this method is much higher in order to be sure that The Sun is
 * what's being found.
 *
 * The pan Servo motor will start at degree 0, begin to move clockwise, and
 * search for a point where there is a difference in either Northern labeled
 * LDRs and Southern labeled LDRs or Western labeled LDRs and Eastern labeled
 * LDRs which has magnitude greater than SEARCH_TOL. Once this point is reached,
 * the device switches to tracking The Sun with more precise pan and tilt Servo
 * motor movement.
 *
 * If the device makes NUM_LOOP full loops without finding a significantly
 * bright source of light, the device will be placed in low power sleep mode.
 *
 * If a power source is found, then this method will return false. This return
 * value is used in the sketch as the value for a flag which determines if the
 * device should be sleeping or actively tracking The Sun. If a power source is
 * not found, then this method will return true to indicate that the device
 * should go immediately to sleep mode in the sketch loop method.
 */
bool search();

/*
 * Once a power source (presumably The Sun) has been found, this method ensures
 * the cluster of LDRs directly faces it for the duration of the day.
 *
 * For example, let The Sun be in the direct path of the LDR cluster and let the
 * angle of pane tilt be less than or equal to 90 degrees. If it were to move
 * say West and the Eastern labeled LDRs measure more light than the Western
 * labeled LDRs as a result. Then the device would have to rotate
 * counterclockwise. If the Western labeled LDRs receive more sunlight, then the
 * device would have to rotate clockwise.
 *
 * If the Northern labeled LDRs receive more sunlight, then the angle of pane
 * tilt should increase; otherwise it should decrease.
 *
 * When tilting or rotating the device, the tilt_pane or rotate_pane methods
 * (see below) are used. They only change the angle of rotation or tilt by one
 * degree at a time because this small change in angle is all that is needed
 * when tracking The Sun.
 *
 * Tracking first handles rotating the device before handling tilting pane
 * because certain solar positions may not require a tilt at first, but after
 * rotating the device then a tilt is required. Because of this, panning the
 * device first is required.
 */
void track();

/*
 * Rotates the pane containing the LDR cluster and solar panels either clockwise
 * or counterclockwise, depending on how the parameter supplied compares to the
 * variable pan_angle.
 *
 * In the search method, the pan_angle is changed by 5 degrees clockwise in
 * order to rotate the entire device quickly to check if this angle a
 * significant light source can be found.
 *
 * In the track method by contrast, the pan_angle variable +/- 1 is supplied as
 * the parameter for this method. This will rotate the device by one degree in
 * order to only minutely change its orientation. This one degree is all that is
 * necessary as The Sun's position in the sky does not move rather quickly with
 * respect to the area the LDRs face.
 */
void pan_pane(u_int16_t angle);

/*
 * Updates the angle of tilt for the pane containing the LDRs and solar panels
 * by either increasing or decreasing this angle.
 *
 * This method is called in the search and track methods in order to tilt the
 * pane either up or down to point more directly toward The Sun. In track, the
 * change in angle is always 1 because only a small change in pane tilt is
 * needed as The Sun's position in the sky changes slowly over time.
 */
void tilt_pane(u_int8_t angle);

#endif