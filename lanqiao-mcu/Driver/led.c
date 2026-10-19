#include "led.h"

void led_read(unsigned char ucled){
	
	P0=~ucled;
	P2=P2&0x1f|0x80;
	P2&=0x1f;

}