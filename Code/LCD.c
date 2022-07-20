#include <avr/io.h>
#include <util/delay.h>

#define lcd_bus PORTC
#define lcd_bus_direction DDRC

#define lcdRegisterSelect PC5
#define lcdEnable PC4
#define lcdData1 PC3
#define lcdData2 PC2
#define lcdData3 PC1
#define lcdData4 PC0

void lcd_cmd_write(char a){
	if(a & 0x80) lcd_bus|=(1<<lcdData4); else lcd_bus&= ~(1<<lcdData4);
	if(a & 0x40) lcd_bus|=(1<<lcdData3); else lcd_bus&= ~(1<<lcdData3);
	if(a & 0x20) lcd_bus|=(1<<lcdData2); else lcd_bus&= ~(1<<lcdData2);
	if(a & 0x10) lcd_bus|=(1<<lcdData1); else lcd_bus&= ~(1<<lcdData1);
	
	lcd_bus &=~(1<<lcdRegisterSelect);
	lcd_bus |=(1<<lcdEnable);
	
	_delay_ms(3);
	
	lcd_bus &=~(1<<lcdEnable);
	
	_delay_ms(3);
	
	if(a & 0x08) lcd_bus|=(1<<lcdData4); else lcd_bus&= ~(1<<lcdData4);
	if(a & 0x04) lcd_bus|=(1<<lcdData3); else lcd_bus&= ~(1<<lcdData3);
	if(a & 0x02) lcd_bus|=(1<<lcdData2); else lcd_bus&= ~(1<<lcdData2);
	if(a & 0x01) lcd_bus|=(1<<lcdData1); else lcd_bus&= ~(1<<lcdData1);
	
	lcd_bus &=~(1<<lcdRegisterSelect);
	lcd_bus |=(1<<lcdEnable);
	
	_delay_ms(3);
	
	lcd_bus &=~(1<<lcdEnable);
	
	_delay_ms(3);
}

void lcd_init(){
	lcd_bus_direction |= ((1<<lcdRegisterSelect)|(1<<lcdEnable)|(1<<lcdData4)|(1<<lcdData3)|(1<<lcdData2)|(1<<lcdData1));
	
	_delay_ms(3);
	
	lcd_cmd_write(0x01);
	lcd_cmd_write(0x02);
	lcd_cmd_write(0x28);
	lcd_cmd_write(0x06);
	lcd_cmd_write(0x0c);

}

void lcd_data_write(char a){
	
	if(a & 0x80) lcd_bus|=(1<<lcdData4); else lcd_bus&= ~(1<<lcdData4);
	if(a & 0x40) lcd_bus|=(1<<lcdData3); else lcd_bus&= ~(1<<lcdData3);
	if(a & 0x20) lcd_bus|=(1<<lcdData2); else lcd_bus&= ~(1<<lcdData2);
	if(a & 0x10) lcd_bus|=(1<<lcdData1); else lcd_bus&= ~(1<<lcdData1);
	
	lcd_bus |=(1<<lcdRegisterSelect)|(1<<lcdEnable);
	
	_delay_ms(3);
	
	lcd_bus &=~(1<<lcdEnable);
	
	_delay_ms(3);
	
	if(a & 0x08) lcd_bus|=(1<<lcdData4); else lcd_bus&= ~(1<<lcdData4);
	if(a & 0x04) lcd_bus|=(1<<lcdData3); else lcd_bus&= ~(1<<lcdData3);
	if(a & 0x02) lcd_bus|=(1<<lcdData2); else lcd_bus&= ~(1<<lcdData2);
	if(a & 0x01) lcd_bus|=(1<<lcdData1); else lcd_bus&= ~(1<<lcdData1);
	
	lcd_bus |=(1<<lcdRegisterSelect)|(1<<lcdEnable);
	
	_delay_ms(3);
	
	lcd_bus &=~(1<<lcdEnable);
	
	_delay_ms(3);
	
}

void lcd_display(const char *p){
	while(*p!='\0')
	{
		lcd_data_write(*p);
		p++; _delay_ms(3);
	}
}

void lcd_clear_screen(){
	lcd_cmd_write(0x01);
	_delay_ms(3);
}

void lcd_set_cursor_1stLine(){
	lcd_cmd_write(0x80);
	_delay_ms(3);
}

void lcd_set_cursor_2ndLine(){
	lcd_cmd_write(0xC0);
	_delay_ms(3);
}

void lcd_shift_cursor_right(int shiftsRight){
	int i=0;
	for(i=0;i<shiftsRight;i++){
		lcd_cmd_write(0x14);
	}
	_delay_ms(3);
}