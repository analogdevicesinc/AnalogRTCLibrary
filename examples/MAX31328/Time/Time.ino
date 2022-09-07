#include <AnalogRTCLibrary.h>

MAX31328 rtc(&Wire, MAX3128_I2C_ADDRESS);

char time_char_buffer[40];
struct tm rtc_ctime;

void setup() {
    int ret;

    Serial.begin(115200);
    Serial.println("---------------------");
    Serial.println("Set get rtc use case example:");
    Serial.println("RTC will be set to specific value then it will be read every second");
    Serial.println(" ");

    rtc.begin();
        
    rtc_ctime.tm_year = 121; // years since 1900
    rtc_ctime.tm_mon  = 10;  // 0-11
    rtc_ctime.tm_mday = 24;  // 1-31
    rtc_ctime.tm_hour = 15;  // 0-23
    rtc_ctime.tm_min  = 10;  // 0-59
    rtc_ctime.tm_sec  = 0;   // 0-61
    //
    rtc_ctime.tm_yday  = 0;  // 0-365
    rtc_ctime.tm_wday  = 0;  // 0-6
    rtc_ctime.tm_isdst = 0;  // Daylight saving flag
    
    ret = rtc.set_time(&rtc_ctime);
    if (ret) {
        Serial.println("Set time failed!");
    }
}

void loop()  {
    int ret;

    delay(1000); // wait a little

    ret = rtc.get_time(&rtc_ctime);
    if (ret) {
        Serial.println("get_time failed!");
    } else {
        strftime(time_char_buffer, 40, "%F %T", &rtc_ctime);
        Serial.println(time_char_buffer);
    }
}
