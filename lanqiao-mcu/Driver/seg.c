#include "seg.h"

void seg_tran(unsigned char *str,unsigned char *buf){

	unsigned char i,j;
	for(i=0,j=0;i<8;i++,j++){
		
		switch(str[j]){
			
			case '0':buf[i]=0xc0;break;
			case '1':buf[i]=0xf9;break;
			case '2':buf[i]=0xa4;break;
			case '3':buf[i]=0xb0;break;
			case '4':buf[i]=0x99;break;
			case '5':buf[i]=0x92;break;
			case '6':buf[i]=0x82;break;
			case '7':buf[i]=0xf8;break;
			case '8':buf[i]=0x80;break;
			case '9':buf[i]=0x90;break;
			case '-':buf[i]=0xbf;break;
			case 'u':buf[i]=0xc1;break;
			default:buf[i]=0xff;break;
		
		}
		if(str[j+1]=='.'){
			
			buf[i]&=0x7f;
			j++;
		
		}
	
	}

}

void seg_disp(unsigned char *buf,unsigned char pos){
	
	//消影
	P0=0xff;
	P2=P2&0x1f|0xe0;
	P2&=0x1f;
	
	//位选
	P0=1<<pos;
	P2=P2&0x1f|0xc0;
	P2&=0x1f;
	
	//段选
	P0=buf[pos];
	P2=P2&0x1f|0xe0;
	P2&=0x1f;
	
}