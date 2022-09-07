#include <AnalogRTCLibrary.h>

MAX31343 rtc(&Wire, MAX31343_I2C_ADDRESS);

#define MAX_LOOP_COUNTER  250

void setup() {
    int ret;

    Serial.begin(115200);
    Serial.println("---------------------");
    Serial.println("RTC temperature use case example:");
    Serial.println("Temperature will be read periodically");
    Serial.println(" ");

    rtc.begin();
}

void loop()  {
    int ret;

    delay(500); // wait a little

    ret = rtc.start_temp_conversion();
    if (ret) {
        Serial.println("Start temperature converstion failed!");
        return;
    }

    int counter = MAX_LOOP_COUNTER; // max wait time
    do {
        delay(1);
        if (rtc.is_temp_ready() == 0) { // returns 0 on success
            break; // means converstion done, so break
        } 
    } while(--counter);

    if (counter == 0) {
        Serial.println("Timeout occur!");
    } else {
        float temp = 0;
        ret = rtc.get_temp(temp);
        if (ret) {
            Serial.println("Temperature read failed!");
        } else {
            Serial.print("Conversion time: ");
            Serial.print( MAX_LOOP_COUNTER - counter );
            Serial.print(" ms");

            Serial.print("   ");
            Serial.print("Temperature: ");
            Serial.print(temp, 2);
            Serial.println(" Celsius");
        }
    }
}
