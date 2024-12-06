#include <Arduino.h>
#include <Wire.h>

#define CLOCK 6  // D-
#define DATA 7   // D+

bool hadPressed = 0;
unsigned char KEY = 0;
volatile bool extendedKey = false;

const char keymap[] = {
    0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   '`',
    0,   0,   0,   0,    0,   0,   'q', '1',  0,   0,   0,   'z', 's', 'a', 'w',
    '2', 0,   0,   'c',  'x', 'd', 'e', '4',  '3', 0,   0,   ' ', 'v', 'f', 't',
    'r', '5', 0,   0,    'n', 'b', 'h', 'g',  'y', '6', 0,   0,   0,   'm', 'j',
    'u', '7', '8', 0,    0,   ',', 'k', 'i',  'o', '0', '9', 0,   0,   '.', '/',
    'l', ';', 'p', '-',  0,   0,   0,   '\'', 0,   '[', '=', 0,   0,   0,   0,
    13,  ']', 0,   '\\', 0,   0,   0,   0,    0,   0,   0,   0,   8,   0,   0,
    '1', 0,   '4', '7',  0,   0,   0,   '0',  '.', '2', '5', '6', '8', 0,   0,
    0,   '+', '3', '-',  '*', '9', 0,   0,    0,   0,   27,  0};

void requestEvent() {
    if (hadPressed == 1) {
        hadPressed = 0;
        Wire.write(KEY);
        return;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(CLOCK,
            INPUT_PULLUP);  // For most keyboards the builtin pullups are
                            // sufficient, so the 10k pullups can be omitted
    pinMode(DATA, INPUT_PULLUP);
    pinMode(13, OUTPUT);
    bitSet(PCICR, PCIE2);   // Enable pin change interrupts on pin D0-D7
    bitSet(PCMSK2, CLOCK);  // Pin change interrupt on Clock pin
    Wire.begin(0x5f);
    Wire.onRequest(requestEvent);
}

uint8_t lastscan;

ISR(PCINT2_vect) {
    uint16_t scanval = 0;
    for (int i = 0; i < 11; i++) {
        while (digitalRead(CLOCK));
        scanval |= digitalRead(DATA) << i;
        while (!digitalRead(CLOCK));
    }
    scanval >>= 1;
    scanval &= 0xFF;

    if (scanval == 0xE0) {  // Check if the key is extended (2 bytes)
        extendedKey = true;
        return;
    }

    if (scanval == 0xF0) {
        lastscan = 0xF0;
        return;
    }
    if (lastscan != 0xF0) {
        if (extendedKey) {
            switch (scanval) {
                case 0x75:
                    Serial.println("Up");
                    KEY = 181;
                    hadPressed = 1;
                    break;
                case 0x72:
                    Serial.println("Down");
                    KEY = 182;
                    hadPressed = 1;
                    break;
                case 0x6B:
                    Serial.println("Left");
                    KEY = 180;
                    hadPressed = 1;
                    break;
                case 0x74:
                    Serial.println("Right");
                    KEY = 183;
                    hadPressed = 1;
                    break;
                default:
                    Serial.println("Unknown extended key");
                    break;
            }
            extendedKey = false;
        } else {
            Serial.println(keymap[scanval]);
            KEY = keymap[scanval];
            hadPressed = 1;
        }
    }
    lastscan = scanval;
    bitSet(PCIFR, PCIF2);
}

void loop() {}