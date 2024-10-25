#include <AnalogRTCLibrary.h>

MAX31328 rtc(&Wire, MAX3128_I2C_ADDRESS);

// Pin Number that connects to MAX31328 interrupt pin
// Please update pin_inta depend on your target board connection
int pin_inta = 2;

char time_char_buffer[40];
struct tm rtc_ctime;

void print_time(void) {
  
    rtc.get_time(&rtc_ctime);

    strftime(time_char_buffer, sizeof(time_char_buffer), "%Y-%m-%d %H:%M:%S", &rtc_ctime);
    Serial.println(time_char_buffer);
}

void setup() {
    int ret;

    Serial.begin(115200);
    Serial.println("---------------------");
    Serial.println("MAX31328 ALARM2 use case example:");
    Serial.println("MAX31328 will be configured to generate periodic alarm.");
    Serial.println("Note: You may need to unplug and plug target board while switching between examples, ");
    Serial.println("to previous sensor configuration to be deleted.");
    Serial.println(" ");

    // Set alarm pin as input
    pinMode(pin_inta, INPUT);
    
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

    ret = rtc.set_alarm(MAX31328::ALARM2, &rtc_ctime, MAX31328::ALARM_PERIOD_EVERYMINUTE);
    if (ret) {
        Serial.println("Set alarm failed!");
    }

    ret = rtc.irq_enable(MAX31328::INTR_ID_ALARM2);
    if (ret) {
        Serial.println("IRQ enable failed!");
    }

    // clear all irq flags on startup
    rtc.irq_clear_flag();

    print_time(); // print current time
}

void loop()  {

    //Serial.println("Tick");

    int pin_state = digitalRead(pin_inta);
    
    if (pin_state == LOW) {
        print_time();
        rtc.irq_clear_flag(MAX31328::INTR_ID_ALARM2);
    }
}
