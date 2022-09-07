#include <AnalogRTCLibrary.h>

MAX31341 rtc(&Wire, MAX31341_I2C_ADDRESS);
MAX31341::reg_status_t g_stat;

// Pin Number that connects to MAX31341 interrupt pin
// Please update pin_interrupt depend on your target board connection
int pin_inta = 3;

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
    int ret;

    Serial.begin(115200);
    Serial.println("---------------------");
    Serial.println("ALARM use case example:");
    Serial.println("The sensor will be configured to generate periodic alarm.");
    Serial.println("Note: You may need to unplug and plug target board while switching between examples, ");
    Serial.println("to previous example configuration to be deleted.");
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

    ret = rtc.configure_inta_clkin_pin(MAX31341::CONFIGURE_PIN_AS_INTA);
    if (ret) {
        Serial.println("Configure inta failed!");
    }
    
    ret = rtc.set_alarm(MAX31341::ALARM2, &rtc_ctime, MAX31341::ALARM_PERIOD_EVERYMINUTE);
    if (ret) {
        Serial.println("Set alarm failed!");
    }

    ret = rtc.irq_enable(MAX31341::INTR_ID_ALARM2);
    if (ret) {
        Serial.println("IRQ enalbe failed!");
    }
    
    rtc.irq_clear_flag();
    print_time(); // print current time
}

void loop()  {

    //Serial.println("Tick");

    int pin_state = digitalRead(pin_inta);
    
    if (pin_state == LOW) {
        int ret;

        // reading status byte will clear interrupt flags
        ret = rtc.get_status(g_stat);
        if (ret) {
            Serial.println("Read status failed!");
            return;
        }

        if (g_stat.bits.a2f) { // is alarm1 flag set?
            print_time();
        }
    }
}
