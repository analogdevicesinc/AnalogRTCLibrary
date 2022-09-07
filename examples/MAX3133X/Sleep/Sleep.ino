#include <AnalogRTCLibrary.h>

MAX31334 *rtc;

int INTAb = PIN2;
int INTBb = PIN3;
volatile bool interrupt_occured = false;

const char *days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",  "Sunday"};

void set_test_time(tm * testTime)
{
    /*Date: 31/12/2022 Day: Saturday
      Time: 23:59:45*/
    testTime->tm_sec = 45;
    testTime->tm_min = 59;
    testTime->tm_hour = 23;
    testTime->tm_mday = 31;
    testTime->tm_mon = 11;
    testTime->tm_year = 122;
    testTime->tm_wday = 5;
    testTime->tm_yday = 0;
    testTime->tm_isdst = 0;
}

void print_time(tm *rtc_ctime, uint16_t *sub_sec = NULL) {
    Serial.print("Date: ");
    Serial.print(rtc_ctime->tm_mday);
    Serial.print("/");
    Serial.print(rtc_ctime->tm_mon + 1);
    Serial.print("/");
    Serial.print(rtc_ctime->tm_year + 1900);
    Serial.print(" Day: ");
    Serial.println(days[rtc_ctime->tm_wday]);

    Serial.print("Time: ");
    Serial.print(rtc_ctime->tm_hour);
    Serial.print(":");
    Serial.print(rtc_ctime->tm_min);
    Serial.print(":");
    if(sub_sec == NULL) {
        Serial.println(rtc_ctime->tm_sec);
    } else {
        Serial.print(rtc_ctime->tm_sec);
        Serial.print(".");
        Serial.println(*sub_sec);
    }
}

void rtc_interrupt_handler() {
    interrupt_occured = true;
}

void setup() {
    max3133x_status_reg_t status_reg;
    tm rtc_ctime;

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(INTAb, INPUT);
    pinMode(INTBb, INPUT);

    Serial.begin(9600);
    Serial.println("MAX3133x RTC Sleep Example");

    Wire.setClock(400000);

    rtc = new MAX31334(&Wire);
    
    if(rtc->begin()) {
        Serial.println("Error while rtc begin!");
        return;
    }
    
    set_test_time(&rtc_ctime);
    if (rtc->set_time(&rtc_ctime)) {
        Serial.println("Error while set time!");
        return;
    } else {
        Serial.println("Set Time");
    }

    // Disable Clock in/out to configure pins as interrupt.
    if (rtc->clkout_disable()) {
        Serial.println("Error while disable CLKOUT!");
        return;
    } else {
        Serial.println("CLKOUT Disable");
    }

    if (rtc->get_time(&rtc_ctime)) {
        Serial.println("Error while getting time!");
        return;
    } else {
        print_time(&rtc_ctime);
    }

    if (rtc->interrupt_enable(TIE|DIE)) {
        Serial.println("Error while setting interrupt!");
        return;
    } else {
        Serial.println("Interrupt Enable");
    }

    if (rtc->timer_init(85, true, MAX3133X::TIMER_FREQ_16HZ)) {
        Serial.println("Error while Timer Init!");
        return;
    } else {
        Serial.println("Timer Init");
    }

    if (rtc->wakeup_enable(TWE|DIE)) {
        Serial.println("Error while wakeup enable!");
        return;
    } else {
        Serial.println("Wakeup Enable");
    }

    if (rtc->sleep_enter()) {
        Serial.println("Error while Sleep Enter");
        return;
    } else {
        Serial.println("Sleep Enter");
    }
    
    delay(1000);

    if (rtc->sleep_exit()) {
        Serial.println("Error while Sleep Exit");
        return;
    } else {
        Serial.println("Sleep Exit");
    }

    if (rtc->get_status_reg(&status_reg)) {
        Serial.println("Error while get status register!");
        return;
    } else {
        Serial.print("Register :                      0x");Serial.println(status_reg.raw);
        Serial.print("Alarm1 is                       ");Serial.println(status_reg.bits.a1f);
        Serial.print("Alarm2 is                       ");Serial.println(status_reg.bits.a2f);
        Serial.print("Timer interrupt flag is         ");Serial.println(status_reg.bits.tif);
        Serial.print("Digital (DIN) interrupt flag is ");Serial.println(status_reg.bits.dif);
        Serial.print("VBAT Low flag is                ");Serial.println(status_reg.bits.vbatlow);
        Serial.print("Power Fail flag is              ");Serial.println(status_reg.bits.pfail);
        Serial.print("Oscillator stop flag is         ");Serial.println(status_reg.bits.osf);

        (status_reg.bits.psdect == 1) ? Serial.println("Part is running on VBAT") : Serial.println("Part is running on VCC");
    }

    if (rtc->timer_start()) {
        Serial.println("Error while Timer Start!");
        return;
    } else {
        Serial.println("Timer Start");
    }

    if (rtc->sleep_enter()) {
        Serial.println("Error while Sleep Enter");
        return;
    } else {
        Serial.println("Sleep Enter");
    }

    attachInterrupt(digitalPinToInterrupt(INTAb), rtc_interrupt_handler, FALLING);
    attachInterrupt(digitalPinToInterrupt(INTBb), rtc_interrupt_handler, FALLING);
}

void loop(){
    tm rtc_ctime;
    uint16_t subsec;
    max3133x_int_en_reg_t   int_en_reg;
    max3133x_status_reg_t   status_reg;

    if (interrupt_occured) {
        interrupt_occured = false;
        Serial.println("Interrupt! ");

        Serial.print("INTAb: ");
        Serial.print(digitalRead(INTAb));
        Serial.print(" ,INTBb: ");
        Serial.println(digitalRead(INTBb));

        if (rtc->get_status_reg(&status_reg))
            Serial.println("Read status register failed!");

        if (rtc->get_interrupt_reg(&int_en_reg)) 
            Serial.println("Read interrupt enable register failed!");

        if ((status_reg.bits.a1f) && (int_en_reg.bits.a1ie)) {
            Serial.println("Alarm1 Interrupt! ");
        } else if ((status_reg.bits.a2f) && (int_en_reg.bits.a2ie)) {
            Serial.println("Alarm2 Interrupt! ");
        } else if ((status_reg.bits.tif) && (int_en_reg.bits.tie)) {
            Serial.println("Timer Interrupt! ");
        } else if ((status_reg.bits.dif) && (int_en_reg.bits.die)) {
            Serial.println("DIN Interrupt! ");
        } else {
            Serial.println("Other Interrupt! ");
            Serial.print("status_reg.raw: ");Serial.println(status_reg.raw);
            Serial.print("int_en_reg.raw: ");Serial.println(int_en_reg.raw);
        } 

        if (rtc->get_time(&rtc_ctime, &subsec))
            Serial.println("Error while getting time!");
        else
            print_time(&rtc_ctime, &subsec);
    }

    if (rtc->get_sleep_state())
        Serial.println("In sleep state");
    else
        Serial.println("Not in sleep state");

    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}