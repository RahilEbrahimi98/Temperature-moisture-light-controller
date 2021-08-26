#include <avr/io.h>
#include <avr/interrupt.h>
#include <lcd.h>
#include <keypad.h>
#include <avr/eeprom.h>
#include <shtxx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Variables for password part
#define PASSWORD_LENGTH 4
char password[PASSWORD_LENGTH + 1];
char user_password[PASSWORD_LENGTH + 1];
unsigned char pressed_key_integer;
char pressed_key_ascii[8];

// VAriable for T/C
long int overflow_count = 0;

// VAriable for SPI
char ignore;

// Variables for ADC
float temperature;
float humidity;
float ldrValue;

float read_adc();

int main() {
    // LCD initialization
    DDRD = 0xFF;
    LCD_Init();

    // Keypad initialization
    keypad_init();
    
    // Set and check password
    LCD_String("Set Password:");
    LCD_Command(0xC0); // Go to 2nd line
    /* Set password (COMMENT AFTER FIRST RUN TO SEE HOW PASSWORD WILL BE SAVED IN EEPROM) */
    for(int i = 0; i < PASSWORD_LENGTH; i++) {
        do {
            pressed_key_integer = key_scan();
        } while (pressed_key_integer == 255);
        // sprintf(pressed_key_ascii, "%d", pressed_key_integer);
        itoa(pressed_key_integer, pressed_key_ascii, 10);
        LCD_String(pressed_key_ascii);
        password[i] = pressed_key_integer;
        eeprom_write_byte((uint8_t *) i, (uint8_t) pressed_key_integer);
    }
    /* Read password form eeprom (COMMENT OUT AFTER PASSWORD SET TO READ PASSWORD FROM EEPROM) */
    // for (int i = 0; i < 4; i++) {
    //     uint8_t value = eeprom_read_byte((const uint8_t*) i);
    //     password[i] = value;
    //     itoa(value, pressed_key_ascii, 10);
    //     LCD_String(pressed_key_ascii);
    // }
    // _delay_ms(300);
    while (1) {
        LCD_Clear();
        LCD_String("Enter Password:");
        LCD_Command(0xC0); // Go to 2nd line
        for(int i = 0; i < PASSWORD_LENGTH; i++) {
            do {
                pressed_key_integer = key_scan();
            } while (pressed_key_integer == 255);
            // sprintf(pressed_key_ascii, "%d", pressed_key_integer);
            itoa(pressed_key_integer, pressed_key_ascii, 10);
            LCD_String(pressed_key_ascii);
            user_password[i] = pressed_key_integer;
        }
        LCD_Clear();

        if(strcmp(password, user_password) == 0) {
            LCD_String("System Started");
            _delay_ms(300);
            LCD_Clear();
            break;
        }
        else {}
    }

    // LDR initialization
    ADMUX  |= (1 << REFS0); // ADC Voltage Reference: AVCC, cap. on AREF
    ADMUX  |= (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0); // Select ADC2 Single ended as analog input
    ADCSRA |= (1 << ADEN); // ADC Enable
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // ADC Prescalar: 64

    // SHT initialization
    DDRA = (0<<DDA1) | (1<<DDA0); // Data is input and Clk is output
    PORTA = (1<<PORTA1) | (0<<PORTA0); // Pull up Data

    // SPI initialization
    DDRB = (1<<DDB7) | (0<<DDB6) | (1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2);
    PORTB = (1<<PORTB4) | (1<<PORTB3) | (1<<PORTB2);
    SPCR |= (1<<SPE); // SPI Enable
    SPCR |= (0<<CPOL); // SPI Clock Polarity: Low
    SPCR |= (0<<CPHA); // SPI Clock Phase: Leading edge sample / Trailing Edge setup
    SPCR |= (1<<SPR1) | (1<<SPR0); // SPI Clock Rate: f/128 = 62.5 KHz
    SPSR |= (0<<SPI2X);
    SPCR |= (0<<DORD); // SPI Data Order: MSB First
    SPCR |= (1<<MSTR); // SPI Type: Master

    // T/C initialization
    TCCR0 = (0<<CS02) | (1<<CS01) | (0<<CS00); // Prescaler = 8 for t/c 0
    TCNT0 = 0; // Count from 0 in t/c 0
    TIMSK |= (1 << TOIE0); // Enable timer over flow interrupt for t/c 0

    sei(); // Enable global interrupt
    while(1) {
        
        // Read necessary data
        temperature = read_sensor(1); // Read temperature
        humidity = read_sensor(0); // Read humidity
        ldrValue = read_adc(); // Read light intensity
        ldrValue = (ldrValue*5)/1023; // Real voltage value
        // ldrValue = (500*(5-ldrValue))/ldrValue; // Voltage to resistance

        // (T(t/c) = prescalar / f(micro) = 8 / 8MHz = 1microSecond)
        // t/c 0 overflow happens each 2^8 * 1microSecond = 256 microSecond
        // after 2 seconds t/c overflows 2 second / 256 microsecond = 7812.5 times (overflow_count = 7812)
        // overflow_count * 256 + TCNT0 = 7812 * 256 + TCNT0 = 2000000 So TCNT0 = 128 = .5 * 256
        // so when overflow_count = 7812 & TCNT0 = 128 the condition of if will be satisfied
        if(overflow_count * 256 + TCNT0 >= 1999999) { // 2,000,000 microSecond = 2 second
            overflow_count = 0;
            TCNT0 = 0;

            // Send temperature for slave 1
            PORTB &= ~(1<<PORTB2); // Select Slave #1
            SPDR = temperature;
            while(((SPSR >> SPIF) & 1) == 0);
            ignore = SPDR;
            PORTB |= (1<<PORTB2); // Deselect Slave #1

            // Send humidity for slave 2
            PORTB &= ~(1<<PORTB3); // Select Slave #2
            SPDR = humidity;
            while(((SPSR >> SPIF) & 1) == 0);
            ignore = SPDR;
            PORTB |= (1<<PORTB3); // Deselect Slave #2



            // Send ldrValue for slave 3
            PORTB &= ~(1<<PORTB4); // Select Slave #3
            if(ldrValue < 0.01) {
                SPDR = 0;
            }
            else if(ldrValue > 0.01 && ldrValue <= 1) {
                SPDR = 1;
            }
            else if(ldrValue > 1 && ldrValue <= 2 ) {
                SPDR = 2;
            }
            else {
                SPDR = 3;
            }
            while(((SPSR >> SPIF) & 1) == 0);
            ignore = SPDR;
            PORTB |= (1<<PORTB4); // Deselect Slave #3
        }
    }
}

// Function to read from ADC
float read_adc(){
    _delay_us(10); // Delay needed for the stabilization of the ADC input voltage
    ADCSRA |= (1 << ADSC); // Start the AD conversion
    while ((ADCSRA & (1 << ADIF)) == 0); // Wait for the AD conversion to complete
    ADCSRA |= (1 << ADIF);
    return ADCW;
}

ISR(TIMER0_OVF_vect) {
    overflow_count = overflow_count + 1;
}