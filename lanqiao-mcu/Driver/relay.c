#include "relay.h"

unsigned char temp=0x00;
unsigned char temp_old=0xff;

void relay(bit enable){
	
	if(enable) temp|=0x10;//开启继电器
	else temp&=~0x10;//关闭继电器
	
	if(temp!=temp_old){
		
		P0=temp;
		P2=P2&0x1f|0xa0;
		P2&=0x1f;
		
		temp_old=temp;
	
	}

}