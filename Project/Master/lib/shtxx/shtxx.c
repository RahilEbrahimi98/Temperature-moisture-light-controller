#include <avr/io.h>
#include <util/delay.h>

#define SHT_DDR DDRA
#define SHT_PIN PINA
#define SHT_PORT PORTA
#define SHT_SCK_PIN_NO 0
#define SHT_DATA_PIN_NO 1
#define MEASURE_TEMP 0x03 
#define MEASURE_HUMI 0x05 
#define RESET        0x1e 

void sht_start(void) {
    SHT_DDR |= (1<<SHT_DATA_PIN_NO); // DATA is output

    SHT_PORT |= (1<<SHT_DATA_PIN_NO);
    SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
    SHT_PORT |= (1<<SHT_SCK_PIN_NO);
    SHT_PORT &= ~(1<<SHT_DATA_PIN_NO);
    SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
    SHT_PORT |= (1<<SHT_SCK_PIN_NO);
    SHT_PORT |= (1<<SHT_DATA_PIN_NO);
    SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
}
//##########################################################################

char sht_write(unsigned char Byte) { 
    unsigned char i, error = 0;

    SHT_DDR |= (1<<SHT_DATA_PIN_NO); // Data is an output 
    _delay_us(5);
    for(i = 0x80; i > 0; i /= 2) {
        SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
        if(i & Byte) {
            PORTA |= (1<<1);
            SHT_PORT |= (1<<SHT_DATA_PIN_NO);
        }
        else {
            SHT_PORT &= ~(1<<SHT_DATA_PIN_NO);
        } 
        SHT_PORT |= (1<<SHT_SCK_PIN_NO);
    } 
    SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
    SHT_DDR &= ~(1<<SHT_DATA_PIN_NO); // DATA is input
    SHT_PORT |= (1<<SHT_SCK_PIN_NO);
    error = (SHT_PIN >> SHT_DATA_PIN_NO) & 1;
    SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);

    return(error);
} 
//###################################################
unsigned char sht_read(unsigned char ack) {
    unsigned char i, val = 0;

    SHT_DDR &= ~(1<<SHT_DATA_PIN_NO); // DATA is INPUT

    for(i = 0x80; i > 0; i /= 2) {
        SHT_PORT |= (1<<SHT_SCK_PIN_NO);
        if(((SHT_PIN >> SHT_DATA_PIN_NO) & 1) == 1) {
            val = val | i;
        }
        SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
    } 
    SHT_DDR |= (1<<SHT_DATA_PIN_NO); // DATA is output
    if(ack == 0) {
        SHT_PORT |= (1<<SHT_DATA_PIN_NO);
    }
    else {
        SHT_PORT &= ~(1<<SHT_DATA_PIN_NO);
    }
    SHT_PORT |= (1<<SHT_SCK_PIN_NO);
    SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);

    return(val);
}
//########################################################
void connection_reset(void) {
    unsigned char i;
    SHT_DDR |= (1<<SHT_DATA_PIN_NO);
    SHT_PORT |= (1<<SHT_DATA_PIN_NO);
    for (i=0;i<9;i++) {
        SHT_PORT |= (1<<SHT_SCK_PIN_NO);
        _delay_us(2);
        SHT_PORT &= ~(1<<SHT_SCK_PIN_NO);
        _delay_us(2);
    }
    SHT_PORT |= (1<<SHT_DATA_PIN_NO);
    sht_start();
    _delay_ms(100);
}
//####################################################
void sht_reset() {
    sht_start();
    sht_write(RESET);

    _delay_ms(100);
}
//#####################################################
// Read the sensor value. Reg is register to read from
unsigned int ReadSensor(int Reg) {
    unsigned char msb, lsb, crc;

    sht_start();
    sht_write(Reg);

    while(((SHT_PIN >> SHT_DATA_PIN_NO) & 1) == 1);

    msb = sht_read(1);
    lsb = sht_read(1);
    crc = sht_read(0);

    return(((unsigned short) msb << 8) | (unsigned short) lsb); 
}
//######################################################
float read_sensor(char humidity0temperture1) {
    long int income,temp;
    float out,out0,t;
    switch(humidity0temperture1) {
        case 0:
            income = ReadSensor(MEASURE_HUMI);
            out0=(-2.0468+(0.0367*income)+(-1.5955E-6*(income*income))); 
            temp=income;
            _delay_ms(500);
            ReadSensor(MEASURE_TEMP);
            t = -40.1 + 0.01*income;
            out=(t-25)*(0.01+0.00008*temp)+out0;
            break;
        case 1:
            income = ReadSensor(MEASURE_TEMP);
            out = -40.1 + 0.01*income;
            break; 
    }
        return(out);
}