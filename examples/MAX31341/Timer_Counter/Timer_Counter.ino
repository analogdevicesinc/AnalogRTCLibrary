#include <AnalogRTCLibrary.h>

MAX31341 rtc(&Wire, MAX31341_I2C_ADDRESS);

void setup() {
    int ret;

    Serial.begin(115200);
    Serial.println("---------------------");
    Serial.println("MAX31341 timer use case example:");
    Serial.println("MAX31341 timer module will be configured then timer count value will be display");
    Serial.println(" ");

    rtc.begin();
        
    MAX31341::timer_freq_t freq = MAX31341::TIMER_FREQ_16HZ;
    uint8_t value = 0xFF;

    ret = rtc.timer_init(value, true, freq);
    if (ret) {
        Serial.println("Timer init failed!");
    }

    ret = rtc.timer_start();
    if (ret) {
        Serial.println("Timer start failed!");
    }
}

void loop()  {
    static uint8_t last_count = 0;
    uint8_t current_count = 0;
     
    rtc.timer_get(current_count);
    if (current_count != last_count) {
        last_count = current_count;
        Serial.print("TMR Counter: ");
        Serial.println(current_count, HEX);    
    }

}
