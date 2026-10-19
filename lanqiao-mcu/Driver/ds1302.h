#include "STC15F2K60S2.H"
#include "intrins.h"

sbit SDA=P2^3;
sbit SCK=P1^7;
sbit RST=P1^3;

void rtc_write(unsigned char *ucrtc);
void rtc_read(unsigned char *ucrtc);