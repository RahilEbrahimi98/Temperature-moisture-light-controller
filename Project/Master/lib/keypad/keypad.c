#include <avr/io.h>
#include <util/delay.h>

#define KEY_PORT    PORTC
#define KEY_DDR     DDRC
#define KEY_PIN     PINC
 
#define C1  4
#define C2  5
#define C3  6
#define C4  7

unsigned char table[16] = {
15, 0, 14, 13,
1,  2, 3,  12,
4,  5, 6,  11,
7,  8, 9,  10};
 
// When you want to read a PIN right after writing to PORT you should wait
const unsigned char Delay = 5;

void keypad_init(void){
    KEY_DDR = 0x0f;
    KEY_PORT = 0xf0;
}

unsigned char key_released(void) {  
    KEY_PORT = 0xf0;
    _delay_us(Delay);              
    if((KEY_PIN & 0xf0) == 0xf0)
        return 1;
    else
        return 0;
}

unsigned char key_pressed(void) {
    KEY_PORT = 0xf0;
    _delay_us(Delay);
    if( (KEY_PIN & 0xf0) != 0xf0 ) { // User presses some key
        return 1;
    }
    return 0;
}

unsigned char key_scan(void) {
 
    unsigned char i, key; 
    if(key_pressed()){
        for(i = 0; i < 4; i++){
            KEY_PORT = ~(1 << i); 
            _delay_us(Delay);
 
            if(((KEY_PIN >> C1) & 1) == 0)     key = table[i*4];
 
            if(((KEY_PIN >> C2) & 1) == 0)     key = table[i*4+1];
 
            if(((KEY_PIN >> C3) & 1) == 0)     key = table[i*4+2];
 
            if(((KEY_PIN >> C4) & 1) == 0)     key = table[i*4+3];            
        }              
        while(!key_released());     
        return key;                
    }
    
    else 
        return 255;
 
}