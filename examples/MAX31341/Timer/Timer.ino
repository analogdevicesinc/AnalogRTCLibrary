#include <AnalogRTCLibrary.h>

MAX31341 rtc(&Wire, MAX31341_I2C_ADDRESS);
MAX31341::reg_status_t g_stat;

int pin_inta = 3;// interrupt pins that connects to MAX31341
struct tm rtc_ctime;

void print_time(void) {
    char buf[40];

    int ret = rtc.get_time(&rtc_ctime);
    if (ret) {
        Serial.println("get_time failed!");
        return;
    }
    
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &rtc_ctime);
    Serial.println(buf);
}

void setup() {

    Serial.begin(115200);
    Serial.println("---------------------");
    Serial.println("RTC timer use case example:");
    Serial.println(" ");

    pinMode(pin_inta, INPUT_PULLUP);

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
    
    int ret = rtc.set_time(&rtc_ctime);
    if (ret) {
        Serial.println("Set time failed!");
    }

    rtc.timer_stop();
    rtc.timer_init(16, true, MAX31341::TIMER_FREQ_16HZ);
    rtc.irq_enable(MAX31341::INTR_ID_TIMER);
    rtc.timer_start();
}

void loop() {

    int pin_state = digitalRead(pin_inta);
    
    if (pin_state == LOW) {
        int ret;

        // reading status byte will clear interrupt flags
        ret = rtc.get_status(g_stat);
        if (ret) {
            Serial.println("Read status failed!");
            return;
        }

        if (g_stat.bits.tif) { // is timer flag set?
            print_time();
        }
    }
}
