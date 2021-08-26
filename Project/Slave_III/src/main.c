#include <avr/io.h>
#include <lcd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char light_voltage[8];

void setPWM(int duty){
    OCR0 = duty;
    _delay_ms(8);
}

int main() {
    // SPI initialization
    SPCR |= (1<<SPE); // SPI Enable
    SPCR |= (0<<CPOL); // SPI Clock Polarity: Low
    SPCR |= (0<<CPHA); // SPI Clock Phase: Leading edge sample / Trailing Edge setup
    SPCR |= (1<<SPR1) | (1<<SPR0); // SPI Clock Rate: f/128 = 62.5 KHz
    SPSR |= (0<<SPI2X);
    SPCR |= (0<<DORD); // SPI Data Order: MSB First
    SPCR |= (0<<MSTR); // SPI Type: Slave

    // LCD initialization
    DDRC = 0XFF;
    LCD_Init();

    while(1) {
        LCD_Clear();
        SPDR = '0'; // Slave data doesn't matter for master
        while (((SPSR >> SPIF) & 1) == 0); // Wait till get data from master


        TCCR0 = (1 << WGM00) | (1 << WGM01) | (0 << CS01) | (1 << CS00) | (1 << COM01) | (0 << COM00);
	    DDRB|=(1<<PB3);  /*set OC0 pin as output*/

        setPWM(SPDR * 84);
        // sprintf(light_voltage,"T=%d", SPDR);
        itoa(SPDR, light_voltage, 10); // We need ascii code to show them on LCD
        
        for(int i = 0; i < strlen(light_voltage); i++) {
            LCD_Char(light_voltage[i]);
        }
        _delay_ms(1000); // Some wait too see data, before LCD_Clear()
    }

    
}