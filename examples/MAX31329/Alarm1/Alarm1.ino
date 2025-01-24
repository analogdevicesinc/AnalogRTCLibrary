#include <AnalogRTCLibrary.h>

MAX31329 rtc(&Wire, MAX31329_I2C_ADDRESS);
MAX31329::reg_status_t g_stat;

// Pin Number that connects to MAX31343 interrupt pin
// Please update pin_interrupt depend on your target board connection
int pin_interrupt = 2;

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
    Serial.println("MAX31329 ALARM1 use case example:");
    Serial.println("The sensor will be configured to generate periodic alarm.");
    Serial.println("Note: You may need to unplug and plug target board while switching between examples, ");
    Serial.println("to previous example configuration to be deleted.");
    Serial.println(" ");

    // Set alarm pin as input
    pinMode(pin_interrupt, INPUT);
    
    rtc.begin();
    
    rtc_ctime.tm_year = 124; // years since 1900
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
    
    ret = rtc.set_alarm(MAX31329::ALARM1, &rtc_ctime, MAX31329::ALARM_PERIOD_EVERYMINUTE);
    if (ret) {
        Serial.println("Set alarm failed!");
    }

    ret = rtc.irq_enable(MAX31329::INTR_ID_ALARM1);
    if (ret) {
        Serial.println("IRQ enalbe failed!");
    }
    
    rtc.irq_clear_flag();
    print_time(); // print current time

    // uint8_t cfg2;
    // rtc.read_register(MAX31329_R_STATUS, (uint8_t *)&cfg2, 8);
    // Serial.print("STATUS Register = ");
    // Serial.println(cfg2,HEX);
    
    // rtc.read_register(MAX31329_R_INT_EN, (uint8_t *)&cfg2, 8);
    // Serial.print("INT_EN Register = ");
    // Serial.println(cfg2,HEX);
    
    // ret = rtc.read_register(MAX31329_R_CFG2, (uint8_t *)&cfg2, 8);
    // Serial.print("Config 2 Register =");
    // Serial.println(cfg2, HEX);

    // rtc.read_register(MAX31329_R_ALM1_SEC, (uint8_t *)&cfg2, 8);
    // Serial.print("Alarm 1 Second Register = ");
    // Serial.println(cfg2,HEX);
}

void loop()  {
    int pin_state = digitalRead(pin_interrupt);

    if (pin_state == LOW) {
        int ret;
        // reading status byte will clear interrupt flags
        ret = rtc.get_status(g_stat);
        if (ret) {
            Serial.println("Read status failed!");
            return;
        }

        if (g_stat.bits.a1f) { // is alarm1 flag set?
            print_time();
        }
    }
}
