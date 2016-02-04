#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
int interval = 1000;
unsigned long k_currentMillis = 0;
unsigned long k_previousMillis = 0;
int k_interval = 250;
int dacChipSelectPin = 9;
int current = 0;
int currentUpLimit = 4095;
long power = 0;
int sensorPin = A0;
char menu = '1';
bool draw = true;
bool load = false;
char keyPressed = 'z';
int ampsIn = 0;
double voltsIn = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
    pinMode(dacChipSelectPin, OUTPUT);
    digitalWrite(dacChipSelectPin, HIGH);
    Serial.begin(9600);
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    lcd.begin(16,2);
    lcd.backlight();
    lcd.clear();
    lcd.cursor();
    lcd.blink();
    delay(500);
    printWelcome();
    delay(500);
    drawMenu(menu);
}

void loop() {
    if (load) {
        currentMillis = millis();
        if ((currentMillis - previousMillis) > interval) {
            previousMillis = currentMillis;
            drawOnLoad();
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
            if (menu == '1') {
                startCC();
            }
            if (menu == '2') {
                /* startCP(); */
            }
        } else {
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

int readAmps()
{
    return 1000;
}

float readVolts()
{
    return 5.2;
}


void startCC()
{
    if (current > currentUpLimit) {
        current = currentUpLimit;
    }
    if (current > 0) {
        setDac(current);
    }

}

void drawOnLoad()
{

    char line1[16];
    char line2[16];
    float c_power;
    float say_power;

    voltsIn = readVolts();
    ampsIn = readAmps();

    lcd.clear();
    lcd.setCursor(0,0);

    switch (menu) {
        case '1':
            sprintf(line1, "CC %4dmA %4dmA", current, ampsIn);
            lcd.print(line1);
            lcd.setCursor(0,1);
            lcd.print(voltsIn, 3);
            lcd.print("V ");
            c_power = voltsIn * ((float)ampsIn / 1000);
            lcd.print(c_power, 3);
            lcd.print("W");
            break;
        case '2':
            c_power = voltsIn * ((float)ampsIn / 1000);
            say_power = power / 100;
            lcd.print("CP ");
            lcd.print(say_power, 2);
            lcd.print("W ");
            lcd.print(c_power, 3);
            lcd.print("W");
            lcd.setCursor(0,1);
            lcd.print(voltsIn, 3);
            lcd.print("V");
            sprintf(line2, " %4dmA", ampsIn);
            lcd.print(line2);
            break;
        default:
            Serial.println("foo");
    }
    
}

void printWelcome()
{
    lcd.setCursor(0,0);
    lcd.print("CC Dummy Load");
    lcd.setCursor(0,1);
    lcd.print(".....");
}

void drawMenu(int menu_section)
{
    if (draw) {
        lcd.clear();
        char line2[16];
        switch (menu_section) {
            case '1':
                lcd.setCursor(0,0);
                lcd.print("Constant Current");
                lcd.setCursor(5,1);
                sprintf(line2, "%4d mA ", current);
                lcd.print(line2);
                break;
            case '2':
                lcd.setCursor(0,0);
                lcd.print("Constant Power");
                lcd.setCursor(5,1);
                sprintf(line2, "%4d mW ", power);
                lcd.print(line2);
                break;
            default:
                Serial.print("do nothing");
        }
    }
}

void setValueMenu(char menu_section)
{
        lcd.clear();
        char line2[16];

        lcd.setCursor(0,0);

        switch (menu_section) {
            case '1':
                lcd.print("Set Current");
                lcd.setCursor(10,1);
                lcd.print("mA");
                lcd.setCursor(5,1);
                current = setValue(5);
                break;
            case '2':
                lcd.print("Set Power");
                lcd.setCursor(11,1);
                lcd.print(" mW");
                lcd.setCursor(4,1);
                power = setValue(4);
                break;
            default:
                lcd.setCursor(0,0);
                lcd.print("No mode !");
                sprintf(line2, "buuuu ...");
                menu = 1;
        }
        draw = true;
        drawMenu(menu);
}


int setValue(int pos) {
    /* int digits = 0; */
    /* String inString; */
    /* while (digits < 4) { */
    /*     char newchar = readKeypad(); */
    /*     if (isDigit(newchar)) { */
    /*         lcd.setCursor(digits+5,1); */
    /*         lcd.print(newchar); */

    /*         inString += (char)newchar; */
    /*         digits++; */
    /*     } */
    /* } */
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

char readKeypad() {
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

/*
   Function to set the DAC, Accepts the Value to be sent and the cannel
   of the DAC to be used.
*/
void setDac(int value) {
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
