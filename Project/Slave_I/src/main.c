#include <avr/io.h>
#include <lcd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Avr/interrupt.h>

char temperature[8];

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

    DDRD |= (1<<PD7);                              //set PD7 as PWM output
    TCCR2=(1<<WGM20)|(1<<WGM21)|(2<<COM20)|(2<<CS20); //timer2 for heater- fast pwm mode - prescale = 8 - clear oc on compare match
    //TIMSK|=(1<<OCIE2);
    OCR2 = 0;

    DDRB |= (1<<PB3);                              //set PB3 as PWM output
    TCCR0=(1<<WGM00)|(1<<WGM01)|(2<<COM00)|(2<<CS00); //timer0 for cooler - fast pwm mode - prescale = 8 - clear oc on compare match
    //TIMSK|=(1<<OCIE0);
    OCR0 = 0;
    sei();

    while(1) {
        LCD_Clear();
        SPDR = '0'; // Slave data doesn't matter for master
        while (((SPSR >> SPIF) & 1) == 0); // Wait till get data from master
        // sprintf(temperature,"T=%d", SPDR);
        itoa(SPDR, temperature, 10); // We need ascii code to show them on LCD
        

        for(int i = 0; i < strlen(temperature); i++) {
            LCD_Char(temperature[i]);
        }
        _delay_ms(500); // Some wait too see data, before LCD_Clear()

        if (SPDR < 20 )
        {
            OCR0 = 0;
            TIMSK&=(0<<OCIE0);
            TIMSK|=(1<<OCIE2); //heater
            
        }
        else if (SPDR > 25){
            OCR2 = 0;
            TIMSK&=(0<<OCIE2);
            TIMSK|=(1<<OCIE0); //cooler
            
        }
        else{
            TIMSK&=(0<<OCIE2);
            TIMSK&=(0<<OCIE0);
            OCR2 = 0;
            OCR0 = 0;
        }
    }
}

void heater(){
    if (SPDR > 15){ //speed = 1
        OCR2=128;
    }
    else if (SPDR > 10){ //speed = 2
        OCR2=192;
    }
    else if (SPDR >= 5){ //speed = 3
        OCR2=250;
    }
}

void cooler(){
    if (SPDR < 30){ //speed = 1
        OCR0=128;
    }
    else if (SPDR < 35){ //speed = 2
        OCR0=192;
    }
    else if (SPDR <= 40){ //speed = 3
        OCR0=250;
    }
}

ISR(TIMER2_COMP_vect) {
        //heater
        heater();
}

ISR(TIMER0_COMP_vect) {
     //cooler
    cooler();
    
}



    


