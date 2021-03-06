/*
 * Solar Tracking Library.
 *
 * This library provides methods for interacting with an Intel Arduino 101,
 * solar panels, LDRs, and Servo motors with the intent of finding and tracking
 * the motion of The Sun in order to generate the most amount of power through
 * the solar panels used.
 *
 * Depending on the size of the solar panels used, the amount of power generated
 * will vary. Using a DC current sensor, this amount of power can be measured
 * and displayed.
 *
 * This library may also apply to Arduino Uno; however, the Arduino sketch
 * helios_sketch.ino only applies to the Arduino 101 as it uses the 101's built
 * in Bluetooth connectivity.
 *
 * This library is used whenever communication between the Arduino 101 and LDRs
 * is required. This library allows for control of Servo motor angle with or
 * without appropriate signals from one of four LDRs as well as methods to read
 * values from any of the LDRs required: NorthWest, NorthEast, SouthWest, and
 * SouthEast.
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
 * track The Sun in 360 degrees of continuous, rotational motion and 180 degrees
 * of tilting motion.
 *
 * Motion of the pane is controlled in two dimensions through two 180 degree
 * Servo motors. The entire pane containing the LDRs and solar panels is rotated
 * (or panned) using a 180 degree Servo motor which is defined in
 * SolarTracking.cpp as pan. The pane itself where the LDRs are located is
 * tilted by another Servo motor known as tilt. This motor can only go from 0
 * degrees to 180 degrees. Values written to both Servo motors are the angle
 * which the motor should face which will range from 0 to 180 degrees. 360
 * panning motion is achieved as follows: if the angle of panning is between 0
 * and 180 degrees, write the angle as normal; if the angle is greater than 180
 * degrees, then the pane is flipped and 180 - the angle is written to the pan
 * motor. By flipping the pane and writing 180 - the angle, the pane containing
 * the LDRs will now face the opposite direction it used to and will point in
 * the direction whose angle would be greater than 180 degrees.
 *
 * Whenever all four of the device's LDRs measure a consistently low reading or
 * if The Sun wasn't found on its initial search, the device will enter a deep
 * sleep mode each time the Arduino loop method executes. Since the device is
 * considered on at this point, it will enter a deep sleep which will last for
 * SLEEP_ON (see below) milliseconds. In order to break this deep sleep cycle
 * either at least one LDR must measure a significant reading or a user needs to
 * initiate a search using the external application. The user can also use the
 * Bluetooth Android application to force the device to enter a deep sleep mode
 * which is considered turning off the device. When the device is off, it will
 * always enter a deep sleep mode for SLEEP_OFF milliseconds without looking at
 * the LDR readings; only the user can turn the device back on by using the
 * Android application at that point.
 *
 * CREATED:
 * 2017-06-24
 *
 * AUTHORS:
 * Nikola Istvanic
 * Dorairaj Rammohan
 */
#ifndef SolarTracking_h
#define SolarTracking_h

/* DEFINES */
#define SEARCH_TOL 500 // higher tolerance for initially finding The Sun
#define LOW_READ   350 // LDR reading which is considered low
#define TRACK_DIFF  50 // reading difference tolerance when tracking The Sun
#define LOW_TIMES   10 // assume night when all LDRs read low this many times
#define NUM_LDR      4 // number of LDR sensors
#define NUM_LOOP     2 // number of loops to make when searching for The Sun

#define DA             10 // amount of degrees to change in search method
#define READ_DELAY   1000 // millisecond delay between LDR readings
#define LARGE_ANGLE  1000 // millisecond delay for large Servo motor movement
#define SMALL_ANGLE   500 // millisecond delay for small Servo motor movement
#define SLEEP_ON     3000 // millisecond duration for sleep when device is on
#define SLEEP_OFF   10000 // millisecond duration for sleep when device is off

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
typedef unsigned int u_int32_t; // 32-bit unsigned integer [0 - 4294967295]

/* CONSTANTS */
const u_int8_t TILT_INIT = 135; // tilt Servo motor initial angle
const u_int8_t PAN_INIT = 0; // pan Servo motor initial angle
const u_int8_t TILT_MAX = 180; // max angle for tilt Servo motor
const u_int16_t PAN_MAX = 360; // max angle for pan Servo motor
const u_int8_t DC_PIN = 4; // analog pin for the DC sensor
const double VREF = 5.0; // reference voltage

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
 * angle for the tilt motor and ensure the pan motor is not moving. For the tilt
 * Servo motor, this angle will be 45 degrees; for the pan Servo motor, the
 * initial angle will be 0.
 */
void initialize();

/*
 * Puts the Arduino 101 into a low power, deep sleep mode for the duration
 * parameter milliseconds.
 *
 * During normal execution, if the Arduino 101 is on and needs to go to sleep,
 * for every call to loop, the device will check if it needs to exit this deep
 * sleep. If so, it will continue tracking The Sun; otherwise the device will
 * enter this deep sleep mode for duration milliseconds and will continue to do
 * so as long as the device needs to be in sleep mode.
 *
 * If the device is considered off, then for every call to loop, the device will
 * immediately go to deep sleep and check if the user wants the device to turn
 * on. This means the device won't consider readings from the LDR but just what
 * the user wants. When off, the device will sleep for a significantly longer
 * duration of time.
 */
void sleep(u_int32_t duration);

/*
 * Makes the pane containing the LDRs and solar panels face what this library
 * thinks is the direction of sunrise, East.
 *
 * Whenever the device is facing sunset and it captures the last rays of
 * sunlight, the LDRs will all measure a low reading after a certain period of
 * time. This library sees these conditions as reason enough to turn the device
 * around in order to face the approximate direction of sunrise.
 */
void turn_east();

/*
 * Read a single LDR sensor and return that value as a signed 16-bit int. The
 * return value must be signed because signed arithmetic is performed on these
 * the LDR readings in the _get_dh and _get_dv methods.
 *
 * This method takes in an enum which is declared above and returns the value of
 * an analogRead of that analog pin. The value of the sensor read is printed as
 * well.
 */
int16_t read_ldr(sensor ldr);

/*
 * Reads all LDR sensors, outputs, and returns int16_t array with these values.
 *
 * The array will be in the following format:
 *     Index 0: NW LDR reading
 *     Index 1: NE LDR reading
 *     Index 2: SW LDR reading
 *     Index 3: SE LDR reading
 * where NW, NE, SW, and SE are the labeled cardinal directions on the pane near
 * each LDR.
 */
int16_t* read_ldr_all();

/*
 * Read the value from the DC current sensor to obtain the amount of power
 * generated by this device.
 *
 * Return and print the power obtained by using equations for power, current,
 * and voltage using the reading the DC current sensor.
 */
double read_power();

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
 * Search for The Sun or a significantly intense source of light by panning
 * the device NUM_LOOP times and looking for a significantly high LDR reading.
 *
 * Tolerance for this method is much higher in order to be sure that The Sun is
 * what's being found.
 *
 * A temporary angle which represents the direction the device will face will
 * start at degree 0, and as the pan Servo begins to move clockwise. It will
 * search for a point where at least one of the LDR readings is significantly
 * high (> SEARCH_TOL). Once this point is reached, the device will begin to
 * track this light source.
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
 * When tilting or rotating the device, the tilt_pane or pan_pane methods
 * (see below) are used. A low value is used for the pan rotational movement if
 * required, and the tilt_angle degree is changed minutely because this small
 * change in angle is all that is needed when tracking The Sun.
 *
 * Tracking first handles rotating the device before handling tilting pane
 * because certain solar positions may not require a tilt at first, but after
 * rotating the device then a tilt is required. Because of this, panning the
 * device first is required.
 */
void track();

/*
 * Updates the angle of the pan Servo motor for the entire pane.
 *
 * When combined with the 180 degree tilt Servo motor, this device can achieve
 * full 360 degree panning motion. With 360 degree motion, this device can
 * successfully track The Sun on whatever path it takes.
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
