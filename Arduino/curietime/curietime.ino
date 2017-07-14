#include <CurieTime.h>
void setup() {
  // put your setup code here, to run once:
  Serial.end();
  delay(100);
  Serial.begin(9600);
  setTime(int hour, int minute, int second, int day, int month, int year)
  Serial.println(now());
  Serial.println(year());
  Serial.println(month());
  Serial.println(day());
  Serial.println(hour());
  Serial.println(minute());
  Serial.println(second());
}

void loop() {
  // put your main code here, to run repeatedly:

}
