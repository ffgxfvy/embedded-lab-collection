#include "ds1302.h"

void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

//
void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

//
unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}

//自己写的
void rtc_write(unsigned char *ucrtc){
	
	unsigned char temp;
	
	Write_Ds1302_Byte(0x8e,0x00);//关闭写保护
	Write_Ds1302_Byte(0x80,0x00);//关闭时钟振荡
	
	temp=(ucrtc[0]/10<<4)|(ucrtc[0]%10);
	Write_Ds1302_Byte(0x84,temp);
	
	temp=(ucrtc[1]/10<<4)|(ucrtc[1]%10);
	Write_Ds1302_Byte(0x82,temp); 
	
	temp=(ucrtc[2]/10<<4)|(ucrtc[2]%10);
	Write_Ds1302_Byte(0x80,temp); 
	
	Write_Ds1302_Byte(0x8e,0x80);//开启写保护
	
}

void rtc_read(unsigned char *ucrtc){
	
	unsigned char temp;
	
	temp=Read_Ds1302_Byte(0x85);
	ucrtc[0]=(temp>>4)*10+(temp&0x0f);
	
	temp=Read_Ds1302_Byte(0x83);
	ucrtc[1]=(temp>>4)*10+(temp&0x0f);
	
	temp=Read_Ds1302_Byte(0x81);
	ucrtc[2]=(temp>>4)*10+(temp&0x0f);

}

