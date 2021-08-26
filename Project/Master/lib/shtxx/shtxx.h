#include "shtxx.c"

void sht_start(void);
char sht_write(unsigned char Byte);
unsigned char sht_read(unsigned char ack);
void connection_reset(void);
void sht_reset();
unsigned int ReadSensor(int Reg);
float read_sensor(char humidity0temperture1);