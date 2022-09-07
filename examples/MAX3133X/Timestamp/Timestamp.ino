#include <AnalogRTCLibrary.h>

MAX31331 *rtc;
uint8_t ts_print_flag = 0;

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

void print_time(tm *rtc_ctime, uint16_t *sub_sec = NULL, bool ts = false) {
    Serial.print("Date: ");
    Serial.print(rtc_ctime->tm_mday);
    Serial.print("/");
    Serial.print(rtc_ctime->tm_mon + 1);
    Serial.print("/");
    
    if(!ts) {
        Serial.print(rtc_ctime->tm_year + 1900);
        Serial.print(" Day: ");
        Serial.println(days[rtc_ctime->tm_wday]);
    } else {
        Serial.println(rtc_ctime->tm_year + 1900);
    }

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

void setup() {
    tm rtc_ctime;
    uint16_t subsec;
    max3133x_status_reg_t status_reg;
    
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(9600);
    Serial.println("MAX3133x RTC Timestamp Example");
    
    Wire.setClock(400000);
    
    rtc = new MAX31331(&Wire);
    
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

    if (rtc->get_time(&rtc_ctime, &subsec)) {
        Serial.println("Error while getting time!");
        return;
    } else {
        print_time(&rtc_ctime, &subsec);
    }

    if (rtc->timestamp_registers_reset()) {
        Serial.println("Error while timestamp_registers_reset!");
        return;
    } else {
        Serial.println("Timestamp Registers Reset");
    }

    if (rtc->timestamp_record_enable(TSVLOW|TSPWM|TSDIN)) {
        Serial.println("Error while setting timestamps!");
        return;
    } else {
        Serial.println("Timestamp Record Enable for TSVLOW, TSPWM, TSDIN");
    }

    if (rtc->timestamp_function_enable()) {
        Serial.println("Error while timestamp_function_enable!");
        return;
    } else {
        Serial.println("Timestamp Enable");
    }

    if (rtc->trickle_charger_enable(MAX3133X::TRICKLE_CHARGER_3K, false)) {
        Serial.println("Error while trickle_charger_enable!");
        return;
    } else {
        Serial.println("Trickle Charger Enable");
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
}

void loop(){
    tm rtc_ctime;
    MAX3133X::timestamp_t timestamp;
    int ret;

    for (int ts_idx = 0; ts_idx < MAX3133X::NUM_OF_TS; ts_idx++) {
        ret = rtc->get_timestamp(ts_idx, &timestamp);
        if (!ret) {
            if (timestamp.ts_trigger) {
                if ((ts_print_flag & (1<<ts_idx)) == (1<<ts_idx)) //printed before.
                    continue;
                
                Serial.print("TS"); Serial.print(ts_idx); Serial.println(" Triggered");

                Serial.print("TS Num:"); Serial.print(timestamp.ts_num); Serial.println(" Triggered");
                if (timestamp.ts_trigger == MAX3133X::DINF) {
                    Serial.println("Triggered by DIN transition");
                } else if (timestamp.ts_trigger == MAX3133X::VCCF) {
                    Serial.println("Triggered by VBAT -> VCC switch");
                } else if (timestamp.ts_trigger == MAX3133X::VBATF) {
                    Serial.println("Triggered by VCC -> VBAT switch");
                } else if (timestamp.ts_trigger == MAX3133X::VLOWF) {
                    Serial.println("Triggered by VLOW detection");
                } else {
                    Serial.println("Undefined Trigger");
                    break;
                }

                print_time(&timestamp.ctime, &timestamp.sub_sec, true);
                ts_print_flag |= (1<<ts_idx);
            }
        } else {
            Serial.print("Error Reading TS. Return val: "); 
            Serial.print(ret); 
            Serial.print(" ts_idx:"); 
            Serial.println(ts_idx); 
        }
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(750);
}