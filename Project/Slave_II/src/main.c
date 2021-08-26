#include <avr/io.h>
#include <lcd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char humidity[8];

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
        // sprintf(humidity,"T=%d", SPDR);
        itoa(SPDR, humidity, 10); // We need ascii code to show them on LCD
        
        if(SPDR < 50){
            if(SPDR > 30){
                LCD_Char(':');
                LCD_Char(')');
            }
            else{
                LCD_Char(':');
                LCD_Char('(');

            }
        }
        else{
            LCD_Char(':');
            LCD_Char('(');
        }
        LCD_Command(0xC0);
        for(int i = 0; i < strlen(humidity); i++) {
            LCD_Char(humidity[i]);
        }
        _delay_ms(1000); // Some wait too see data, before LCD_Clear()
    }
}