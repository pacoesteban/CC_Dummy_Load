#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
int interval = 2000;
unsigned long k_currentMillis = 0;
unsigned long k_previousMillis = 0;
int k_interval = 250;
int dacChipSelectPin = 9;
int adcChipSelectPin = 8;
int current = 0;
int currentUpLimit = 4095;
long power = 0;
long powerUpLimit = 120000;
int sensorPin = A0;
char menu = '1';
bool draw = true;
bool load = false;
char keyPressed = 'z';
float ampsIn = 0;
float voltsIn = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

/* Menu and print functions */
void printWelcome()
{
    lcd.setCursor(0,0);
    lcd.print("CC Dummy Load");
    lcd.setCursor(0,1);
    lcd.print(".....");
}

void drawMenu(int menu_section, bool setMenu = false)
{
    if (draw) {
        lcd.clear();
        String sectionTitle;
        char line2[17];
        switch (menu_section) {
            case '1':
                if (setMenu) {
                    sectionTitle = "Set Current";
                    sprintf(line2, "        mA");
                } else {
                    sectionTitle = "Constant Current";
                    sprintf(line2, "%4d  mA", current);
                }
                break;
            case '2':
                if (setMenu) {
                    sectionTitle = "Set Power";
                    sprintf(line2, "        mW");
                } else {
                    sectionTitle = "Constant Power";
                    sprintf(line2, "%5d  mW", power);
                }
                break;
            default:
                Serial.print("do nothing");
        }
        lcd.setCursor(0,0);
        lcd.print(sectionTitle);
        lcd.setCursor(5,1);
        lcd.print(line2);
    }
}

void setValueMenu(char menu_section)
{
        lcd.clear();
        lcd.cursor();
        lcd.blink();
        draw = true;
        drawMenu(menu, true);
        switch (menu_section) {
            case '1':
                lcd.setCursor(4,1);
                current = setValue(4);
                break;
            case '2':
                lcd.setCursor(3,1);
                power = setValue(3);
                break;
            default:
                lcd.setCursor(0,0);
                lcd.print("No mode !");
                menu = 1;
        }
        lcd.noCursor();
        lcd.noBlink();
        drawMenu(menu);
}

void drawOnLoad()
{

    char line1[16];
    char line2[16];
    float c_power;
    float say_power;

    ampsIn = (readAdc(1) * 10.0);
    voltsIn = (readAdc(0) * 8.0);

        
    if ((ampsIn > 0) && (ampsIn < 0.700)) {
        ampsIn += 0.070;
    } else if (ampsIn < 1.900) {
        ampsIn += 0.060;
    } else if (ampsIn < 3.000) {
        ampsIn += 0.050;
    } else {
        ampsIn += 0.040;
    }

    /* Serial.print("Amps: "); */
    /* Serial.println(ampsIn, 4); */
    /* Serial.print("Volts: "); */
    /* Serial.println(voltsIn, 4); */

    lcd.clear();
    lcd.setCursor(0,0);

    switch (menu) {
        case '1':
            sprintf(line1, "CC %4dmA %4dmA", current, round(ampsIn * 1000.000));
            lcd.print(line1);
            /* lcd.print(ampsIn); */
            lcd.setCursor(0,1);
            lcd.print("ON ");
            if (voltsIn < 1) {
                lcd.print(round(voltsIn*1000));
                lcd.print("mV ");
            } else {
                lcd.print(voltsIn, 2);
                lcd.print("V ");
            }
            c_power = voltsIn * ampsIn;
            if (c_power < 1) {
                lcd.print(round(c_power*1000));
                lcd.print("mW");
            } else {
                lcd.print(c_power, 2);
                lcd.print("W");
            }
            break;
        case '2':
            c_power = voltsIn * ampsIn;
            lcd.print("CP ");
            if (power < 1000) {
                lcd.print(power);
                lcd.print("mW ");
            } else {
                lcd.print(power/1000.000, 2);
                lcd.print("W ");
            }
            if (c_power < 1) {
                lcd.print(round(c_power*1000));
                lcd.print("mW");
            } else {
                lcd.print(c_power, 2);
                lcd.print("W");
            }
            lcd.setCursor(0,1);
            lcd.print("ON ");
            if (voltsIn < 1) {
                lcd.print(round(voltsIn*1000));
                lcd.print("mV ");
            } else {
                lcd.print(voltsIn, 2);
                lcd.print("V ");
            }
            sprintf(line2, "%4dmA", round(ampsIn * 1000.000));
            lcd.print(line2);
            break;
        default:
            Serial.println("foo");
    }
    
}
/* //// Menu and print functions */

/* Action functions */
void startCC()
{
    if (current > currentUpLimit) {
        current = currentUpLimit;
    }
    if (current > 0) {
        setDac(current);
    }

}

void startCP()
{
    if (power > powerUpLimit) {
        power = powerUpLimit;
    }
    if (power > 0) {
        updatePower();
    }
}

void updatePower()
{
    int target_mA;
    target_mA = round((float)power / voltsIn);
    if (target_mA > currentUpLimit) {
        target_mA = currentUpLimit;
    }
    if (target_mA > 0) {
        setDac(target_mA);
    }
}


int setValue(int pos)
{
    String inString;
    char newchar;
    newchar = readKeypad();
    while (newchar != '#') {
        if (isDigit(newchar)) {
            lcd.setCursor(pos,1);
            lcd.print(newchar);

            inString += (char)newchar;
            pos++;
        }
        newchar = readKeypad();
    }
    return inString.toInt();
}
/* //// Action functions */

/* Read Value functions */
char readKeypad()
{
    int caracter;
    k_currentMillis = millis();
    if ((k_currentMillis - k_previousMillis) > k_interval) {
        k_previousMillis = k_currentMillis;
        int sensorValue = analogRead(sensorPin);

        if (sensorValue > 920) {
           caracter = '1';
        } else if ((sensorValue > 905) && (sensorValue < 915)) {
            caracter = '2';
        } else if ((sensorValue > 890) && (sensorValue < 900)) {
            caracter = '3';
        } else if ((sensorValue > 848) && (sensorValue < 855)) {
            caracter = '4';
        } else if ((sensorValue > 832) && (sensorValue < 842)) {
            caracter = '5';
        } else if ((sensorValue > 816) && (sensorValue < 825)) {
            caracter = '6';
        } else if ((sensorValue > 782) && (sensorValue < 790)) {
            caracter = '7';
        } else if ((sensorValue > 770) && (sensorValue < 780)) {
            caracter = '8';
        } else if ((sensorValue > 755) && (sensorValue < 768)) {
            caracter = '9';
        } else if ((sensorValue > 728) && (sensorValue < 738)) {
            caracter = '*';
        } else if ((sensorValue > 718) && (sensorValue < 725)) {
            caracter = '0';
        } else if ((sensorValue > 700) && (sensorValue < 715)) {
            caracter = '#';
        }
    }
    return caracter;
}
/* //// Read Value functions */

/* Low level functions */

float readAdc(int channel) {
  byte adcPrimaryConfig = 0b00000001;     // only contains the start bit
  byte adcSecondaryConfig;
  if (channel == 0) {
      adcSecondaryConfig = 0b10100000;
  } else {
      adcSecondaryConfig = 0b11100000;
  }
  noInterrupts(); // disable interupts to prepare to send address data to the ADC.  
  digitalWrite(adcChipSelectPin,LOW); // take the Chip Select pin low to select the ADC.
  SPI.transfer(adcPrimaryConfig); //  send in the primary configuration address byte to the ADC.  
  byte adcPrimaryByte = SPI.transfer(adcSecondaryConfig); // read the primary byte, also sending in the secondary address byte.  
  byte adcSecondaryByte = SPI.transfer(0x00); // read the secondary byte, also sending 0 as this doesn't matter. 
  digitalWrite(adcChipSelectPin,HIGH); // take the Chip Select pin high to de-select the ADC.
  interrupts(); // Enable interupts.
  byte adcPrimaryByteMask = 0b00001111;      // b00001111 isolates the 4 LSB for the value returned. 
  adcPrimaryByte &= adcPrimaryByteMask; // Limits the value of the primary byte to the 4 LSB:
  int digitalValue = (adcPrimaryByte << 8) | adcSecondaryByte; // Shifts the 4 LSB of the primary byte to become the 4 MSB of the 12 bit digital value, this is then ORed to the secondary byte value that holds the 8 LSB of the digital value.
  float value = ((float)digitalValue * 4.096) / 4095.000; // The digital value is converted to an analogue voltage using a VREF of 2.048V.

  /* Serial.print("Channel: "); Serial.print(channel); */
  /* Serial.print(" Raw value: "); Serial.print(digitalValue); */
  /* Serial.print(" calculat value: "); Serial.println(value, 4); */

  return value; // Returns the value from the function
}

void setDac(int value)
{
    /*
       Sets default DAC registers B00110000, 1st bit "write to register",
       2nd Bit bypasses input Buffer, 3rd bit sets output gain to 1x, 4th
       bit controls active low shutdown. LSB are insignifigant here.
    */
    byte dacRegister = 0b00110000;
    // Isolates the last 8 bits of the 12 bit value, B0000000011111111.
    int dacSecondaryByteMask = 0b0000000011111111;
    /*
       Value is a maximum 12 Bit value, it is shifted to the right by 8
       bytes to get the first 4 MSB out of the value for entry into th
       Primary Byte, then ORed with the dacRegister  
    */
    byte dacPrimaryByte = (value >> 8) | dacRegister;
    // compares the 12 bit value to isolate the 8 LSB and reduce it to a single byte. 
    byte dacSecondaryByte = value & dacSecondaryByteMask;

    noInterrupts(); // disable interupts to prepare to send data to the DAC
    digitalWrite(dacChipSelectPin,LOW); // take the Chip Select pin low to select the DAC:
    SPI.transfer(dacPrimaryByte); //  send in the Primary Byte:
    SPI.transfer(dacSecondaryByte);// send in the Secondary Byte
    digitalWrite(dacChipSelectPin,HIGH);// take the Chip Select pin high to de-select the DAC:
    interrupts(); // Enable interupts
}
/* //// Low level functions */

void setup()
{
    pinMode(dacChipSelectPin, OUTPUT);
    digitalWrite(dacChipSelectPin, HIGH);
    pinMode(adcChipSelectPin, OUTPUT);
    digitalWrite(adcChipSelectPin, HIGH);
    Serial.begin(9600);
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV16);
    setDac(0);

    lcd.begin(16,2);
    lcd.backlight();
    lcd.clear();
    delay(500);
    printWelcome();
    delay(500);
    drawMenu(menu);
}

void loop()
{
    if (load) {
        currentMillis = millis();
        if ((currentMillis - previousMillis) > interval) {
            previousMillis = currentMillis;
            drawOnLoad();
            if (menu == '2') {
                updatePower();
            }
        }
    }
    keyPressed = readKeypad();
    if (keyPressed == '#') {
        if (load) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Stop load first !");
            draw = true;
            
        } else {
            setValueMenu(menu);
        }
    }
    if (keyPressed == '*') {
        load = !load;
        if (load) {
            /* Start ! */
            if (menu == '1') {
                startCC();
            }
            if (menu == '2') {
                /* startCP(); */
            }
        } else {
            /* Stop ! */
            setDac(0);
            draw = true;
            drawMenu(menu);
        }
    }
    if (isDigit(keyPressed)) {
        menu = keyPressed;
        draw = true;
        drawMenu(menu);
    }
    draw = false;
}
