#include <Arduino.h>
#include <PZEM004Tv30.h>

#define NUM_PZEMS 6

PZEM004Tv30 pzems[] = {
    PZEM004Tv30(Serial1, 14, 27, 0x01),
    PZEM004Tv30(Serial1, 14, 27, 0x02),
    PZEM004Tv30(Serial1, 14, 27, 0x03),
    PZEM004Tv30(Serial2, 16, 17, 0x04),
    PZEM004Tv30(Serial2, 16, 17, 0x05),
    PZEM004Tv30(Serial2, 16, 17, 0x06)   
};

String names[] = {"PZEM1", "PZEM2", "PZEM3", "PZEM4", "PZEM5", "PZEM6"};

void setup() {
    Serial.begin(115200);
    while (!Serial)
        continue;

    Serial.println("PZEM Monitor Starting...");
    Serial.println("========================");
}

void loop() {
    for(int i = 0; i < NUM_PZEMS; i++) {
        Serial.println("========================");
        Serial.print(names[i]);
        Serial.print(" - Address: 0x");
        Serial.println(pzems[i].getAddress(), HEX);
        Serial.println("------------------------");

        float voltage   = pzems[i].voltage();
        delay(100);
        float current   = pzems[i].current();
        delay(100);
        float power     = pzems[i].power();
        delay(100);
        float energy    = pzems[i].energy();
        delay(100);
        float frequency = pzems[i].frequency();
        delay(100);
        float pf        = pzems[i].pf();
        delay(100);

        if(isnan(voltage))        { Serial.println("Error reading voltage");   }
        else if(isnan(current))   { Serial.println("Error reading current");   }
        else if(isnan(power))     { Serial.println("Error reading power");     }
        else if(isnan(energy))    { Serial.println("Error reading energy");    }
        else if(isnan(frequency)) { Serial.println("Error reading frequency"); }
        else if(isnan(pf))        { Serial.println("Error reading PF");        }
        else {
            Serial.print("Voltage:   "); Serial.print(voltage);      Serial.println(" V");
            Serial.print("Current:   "); Serial.print(current);      Serial.println(" A");
            Serial.print("Power:     "); Serial.print(power);        Serial.println(" W");
            Serial.print("Energy:    "); Serial.print(energy, 3);    Serial.println(" kWh");
            Serial.print("Frequency: "); Serial.print(frequency, 1); Serial.println(" Hz");
            Serial.print("PF:        "); Serial.println(pf);
        }

        Serial.println("------------------------");
        delay(500);
    }

    Serial.println();
    delay(2000);
}