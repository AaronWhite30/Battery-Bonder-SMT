#include <avr/io.h>
#include <stdio.h>

#define F_CPU 16000000UL

#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD.h"

#define powerGate PIND1
#define powerSwitch PIND3

#define encoderChannel1 PIND5
#define encoderChannel2 PIND6
#define pulseSwitch PIND2
#define pulseOut PINB1

int clockCycleMaxPosition = 20;
int clockCycleMinPosition = 1;
int clockCyclePosition = 1;
int pulseRequest = 0;

static const int clockCycleTicks[20] =
{80, 158, 236, 314, 392, 470, 548, 626, 704, 782, 860, 938, 1016, 1094, 1172, 1250, 1328, 1406, 1484, 1562};

static const char clockCycleMillisecondsDisplay[20][10] =
{"   5ms", "  10ms", "  15ms", "  20ms", "  25ms", "  30ms", "  35ms", "  40ms", "  45ms", "  50ms", "  55ms",
"  60ms", "  65ms", "  70ms", "  75ms", "  80ms", "  85ms", "  90ms", "  95ms", " 100ms"};

void setup(){

	DDRD |= (1<<powerGate);
	PORTD |= (1<<powerGate);
	
	lcd_init();
	lcd_clear_screen();
	
	while(~PIND & (1<<powerSwitch)){}
		
	DDRD &= ~(1<<powerSwitch);		
	DDRD &= ~(1<<pulseSwitch);
	DDRD &= ~(1<<encoderChannel1);
	DDRD &= ~(1<<encoderChannel2);
	
	DDRB |= (1<<pulseOut);
	
	EICRA |= (1<<ISC01);
	EIMSK |= (1<<INT0);
}

void set_pulse_width(int cycles){
	OCR1A = 0xffff-(cycles-1);
}

void pulse_setup(int cycles){
	
	TCCR1B = 0; //Timer/Counter control register. Set to 0 inactivating timer
	TCNT1 = 0x0000; //Timer/Counter. Set to 0, start count from the bottom
	
	//OCR1A is the MATCH register
	ICR1 = 0; //ICR1 is the TOP register
	
	set_pulse_width(cycles);

	//COM1A0 COM1A1 OC1A Set on Match, clear on BOTTOM.
	TCCR1A = (1<<COM1A0) | (1<<COM1A1) | (1<<WGM11);
	
	//WGM11 WGM12 WGM13 ICR1 = TOP, mode 14 fast pwm
	//CS10 CS12 1024 prescaler
	TCCR1B = (1<<WGM12) | (1<<WGM13) | (1<<CS10) | (1<<CS12); //Set TCCR1B to begin counting
}

ISR(INT0_vect){
	if(1<<pulseSwitch){
		pulseRequest = 1;
	}
}

void pulse_set_and_fire(int cycles) {
	
	int pulseWidth = 0xffff-(cycles-1);
	OCR1A = pulseWidth;
	
	_delay_us(65);
	
	TCNT1 = pulseWidth-1;
}

void pulse_send(int cycles){
	pulse_set_and_fire(cycles);
}

void splash_screen(){
	
	lcd_clear_screen();
	
	lcd_set_cursor_1stLine();
	_delay_ms(3);
	lcd_display(" Battery");
	_delay_ms(3);
	lcd_set_cursor_2ndLine();
	_delay_ms(3);
	lcd_display(" Bonder");
	
	_delay_ms(2500);
	
	lcd_clear_screen();
}

void updateMainDisplay(){
	
	lcd_clear_screen();
	_delay_ms(3);
	
	lcd_set_cursor_1stLine();
	_delay_ms(3);
	
	lcd_display(clockCycleMillisecondsDisplay[clockCyclePosition-1]);
	_delay_ms(3);
	
}

void updateEepromClockCyclePosition(){
	
	while(!eeprom_is_ready()){}
	eeprom_update_word((uint16_t*)11, clockCyclePosition);
}

void loadEepromClockCyclePosition(){
	
	while(!eeprom_is_ready()){}
	clockCyclePosition = eeprom_read_word((uint16_t*)11);
	
	if((clockCyclePosition > clockCycleMaxPosition) | (clockCyclePosition < clockCycleMinPosition)){
		clockCyclePosition = clockCycleMinPosition;
		while(!eeprom_is_ready()){}
		updateEepromClockCyclePosition();
	}
}

void pollPowerSwitch(){
	
	if(~PIND & (1<<powerSwitch)){
		PORTD &= ~(1<<powerGate);
	}
}

int main(void) {
	
	cli();
	
	setup();
	splash_screen();
	loadEepromClockCyclePosition();
	pulse_setup(clockCycleTicks[clockCyclePosition-1]);
	updateMainDisplay();
	
	sei();
	
	while (1) {
		
		if (~PIND & (1<<encoderChannel1)) {
			
				//Wait for pulse to stop then test the other channel
				while(~PIND & (1<<encoderChannel1)){}
				if (~PIND & (1<<encoderChannel2)) {
				
					//Left turn
					clockCyclePosition--;
					if(clockCyclePosition < clockCycleMinPosition){
						clockCyclePosition = clockCycleMinPosition;
					}

					updateEepromClockCyclePosition();
					updateMainDisplay();
				
				}
			
		}else if (~PIND & (1<<encoderChannel2)) {
			
				//Wait for pulse to stop then test the other channel
				while(~PIND & (1<<encoderChannel2)){}
				if (~PIND & (1<<encoderChannel1)) {
				
					//Right Turn
					clockCyclePosition++;
					if(clockCyclePosition > clockCycleMaxPosition){
						clockCyclePosition = clockCycleMaxPosition;
					}
				
					updateEepromClockCyclePosition();
					updateMainDisplay();
				}
			
		}else if(pulseRequest){
			
			pulseRequest = 0;
			pulse_send(clockCycleTicks[clockCyclePosition-1]);
		}
		
		pollPowerSwitch();
		
	}
	return(0);
}

