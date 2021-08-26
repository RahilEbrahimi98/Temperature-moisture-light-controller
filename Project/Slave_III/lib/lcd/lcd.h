#include "LCD.c"

void LCD_Command(unsigned char cmnd);
void LCD_Char(unsigned char data);
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();