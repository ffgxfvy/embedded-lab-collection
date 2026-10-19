#include "stdio.h"
#include "init.h"
#include "timer.h"
#include "led.h"
#include "key.h" 
#include "seg.h"
#include "ds1302.h"
#include "ds18b20.h"
#include "relay.h"

//轮询变量定义区
unsigned char cnt_led,cnt_key,cnt_seg,cnt_rtc;
unsigned int cnt_t;

//函数声明区
void task_proc();
void led_proc();
void key_proc();
void seg_proc();
void rtc_proc();
void Delay750ms(void);
void mode_con_tran();

//led专用变量
static unsigned char ucled=0x00;

//按键专用变量
unsigned char key_val=0,key_down=0,key_old=0,key_up=0;

//数码管专用变量
unsigned char seg_str[16];
unsigned char seg_buf[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char pos;

//ds1302专用变量
unsigned char ucrtc[3]={23,59,55};

//ds18b20专用变量
unsigned int t_val;

//用户变量
unsigned char mode=0;
unsigned char t_set=23;
bit time_flag;
bit led_flag;
unsigned int cnt_led_flag=0;
bit relay_flag;
unsigned char cnt_led3;
bit led3_flag=0;
unsigned char mode_con=0;
	

int main()
{
	
	system_init();
	
	Timer1_Init();
	
	rtc_write(ucrtc);
	
	//温度初始化延时
	t_read();
	Delay750ms();
	
	while(1)
	{
		
		task_proc();
	
	
	
	}
}	


//中断轮询函数
void tim_isr() interrupt 3{
	
	if(cnt_led<30) cnt_led++;
	if(cnt_key<20) cnt_key++;
	if(cnt_seg<100) cnt_seg++;
	if(cnt_rtc<90) cnt_rtc++;
	if(cnt_t<800) cnt_t++;
	
	//L1指令灯5s延时
	if(cnt_led_flag<5000) cnt_led_flag++;
	if(cnt_led_flag==5000){
		
		led_flag=0;
		cnt_led_flag=0;
	
	}
	
	//L3闪烁
	if(relay_flag==1){
		
	   if(cnt_led3<100) cnt_led3++;
	   if(cnt_led3>=100){
			 		
		   ucled^=0x04;
		   //if(led3_flag==0) ucled|=0x04;//亮
		   //else ucled&=0xfb;//灭
		   //led3_flag^=1;//反转实现一亮一灭
		   cnt_led3-=100;
			 
	   } 
   }
	else if(relay_flag==0){
		
		ucled&=~0x04;
		
	}


	//数码管刷新
	seg_disp(seg_buf,pos);
	if(++pos==8) pos=0;
	

}

void task_proc(){
	
	if(cnt_led>=30){
		
		led_proc();
		mode_con_tran();
		cnt_led-=30;
	
	}
	
	if(cnt_key>=20){
		
		key_proc();
		cnt_key-=20;
	
	}
	
	if(cnt_seg>=100){
		
		seg_proc();
		cnt_seg-=100;
	
	}
	
	if(cnt_rtc>=90){
		
		rtc_proc();
		cnt_rtc-=90;
	
	}
	
	if(cnt_t>=800){
	
		t_val=t_read();
		cnt_t-=800;
	
	}


}	

void led_proc()
{
	EA=0;
	
				  //功能：整点时led1亮5s然后灭
	        if(ucrtc[1]==0&&ucrtc[2]==0){
								
		        led_flag=1;//在中断函数中开启5000ms的计时
		        ucled|=0x01;

		      }
	        //延时5s后关闭led1
	        if(led_flag==0){
		
		        ucled&=0xfe;

		      }
	
	led_read(ucled);
	
  EA=1;

}	

void key_proc(){
	
	key_val=key_read();
	key_down=key_val&(key_val^key_old);
	key_up=~key_val&(key_val^key_old);
	key_old=key_val;
	
	switch(key_down){
		
		case 12:mode++;if(mode==3) mode=0;break;
		
		case 13:
			mode_con++;
		  if(mode_con==2) mode_con=0;
			break;
		
    case 16:
			if(mode==2){
				t_set++;
			  if(t_set==100) t_set=99;
			}
      break;

    case 17:
			if(mode==2){
				t_set--;
				if(t_set==9) t_set=10;
      }
      else if(mode==1) time_flag=1;break;
		
	}
	
	switch(key_up){
		
		case 17:if(mode==1) time_flag=0;
	
	}
}

void seg_proc(){
	
	//温度0；时间1；参数2
	switch(mode){
		
		case 0:
			//sprintf(seg_str,"u1   23.5");
		  sprintf(seg_str,"u1   %02u.%u",(unsigned int)t_val/100,(unsigned int)(t_val/10%10));
		  break;
		case 1:
			if(time_flag==0) sprintf(seg_str,"u2 %02u-%02u",(unsigned int)ucrtc[0],(unsigned int)ucrtc[1]);
		  if(time_flag==1) sprintf(seg_str,"u2 %02u-%02u",(unsigned int)ucrtc[1],(unsigned int)ucrtc[2]);
		  break;
		case 2:
			sprintf(seg_str,"u3    %02u",(unsigned int)t_set);
		  break;
		
	}
	
	//if(mode==0) sprintf(seg_str,"u1   23.5");
	//else if(mode==1&time_flag==0) sprintf(seg_str,"u2 23-25");
	//else if(mode==1&time_flag==1) sprintf(seg_str,"u2 53-20");
	//else if(mode==2) sprintf(seg_str,"u3    %02u",(unsigned int)t);
	
	seg_tran(seg_str,seg_buf);

}

void rtc_proc(){
	
	EA=0;
	rtc_read(ucrtc);
	EA=1;

}

void Delay750ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 35;
	j = 51;
	k = 182;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void mode_con_tran(){
	
    if(mode_con==0){
				
    //温度控制模式：功能1：led2指示；功能2：温度大于设定值打开继电器；功能3：继电器打开时led3闪烁
				
				  //功能1：led2指示
		      ucled|=0x02;

				  //功能2：温度大于设定值打开继电器
		      if(t_val/100<t_set){//温度小于设定值
						relay(0);//断开继电器
			      relay_flag=0;
		      }
					else if((t_val/100==t_set)&&(t_val%100-t_val%10==0)){//温度等于设定值
						
						relay(0);//断开继电器
			      relay_flag=0;
					
					}
		      else{//温度不小于设定值
			      relay(1);//打开继电器
			      relay_flag=1;
	        }
					
			}
			
			if(mode_con==1){
				
			//时间控制模式：功能：整点时继电器闭合5s然后断开
				
					//关闭温度控制模式指示灯
				  ucled&=0xfd;
				  
				  //功能：整点时继电器闭合5s然后断开
				  //整点时继电器闭合
	        if(ucrtc[1]==0&ucrtc[2]==0){
						
			 		  relay(1);
		        relay_flag=1;
						
		      }
	        //延时5s后关闭继电器
	        if(led_flag==0){
		
		   		  relay(0);
		        relay_flag=0;
						
		      }

	    }
}

