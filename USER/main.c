#include "led.h"
#include "key.h"
#include "beep.h"
#include "delay.h"
#include "sys.h"
#include "timer.h"
#include "usart.h"
#include "dht11.h"
#include "bh1750.h"
#include "oled.h"
#include "exti.h"
#include "stdio.h"
#include "esp8266.h"
#include "onenet.h"

u8 alarmFlag = 0;//�Ƿ񱨾��ı�־
u8 alarm_is_free = 10;//�������Ƿ��ֶ�������������ֶ���������Ϊ0


u8 humidityH;
u8 humidityL;
u8 temperatureH;
u8 temperatureL;
extern char oledBuf[20];
float Light = 0;

char PUB_BUF[256];//�ϴ����ݵ�buf
const char *devSubTopic[] = {"/mysmarthome/sub"};
const char devPubTopic[] = "/mysmarthome/pub";
/************************************************
 ALIENTEK��ӢSTM32������ʵ��1
 �����ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
 int main(void)
 {	
	unsigned short timeCount = 0;	//���ͼ������
	unsigned char *dataPtr = NULL;
	delay_init();	    //��ʱ������ʼ��	 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�	 
	LED_Init();		  	//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();         	//��ʼ���밴�����ӵ�Ӳ���ӿ�
	EXTIX_Init();         	//��ʼ���ⲿ�ж����� 
	BEEP_Init(); 
	DHT11_Init();
	BH1750_Init();
	
	 
	OLED_Init();  
	OLED_ColorTurn(0);//0������ʾ��1 ��ɫ��ʾ
  OLED_DisplayTurn(0);//0������ʾ 1 ��Ļ��ת��ʾ
	OLED_Clear();
	TIM3_Int_Init(4999,7199);
	TIM2_Int_Init(2499,7199);
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	Usart1_Init(115200);//debug����
	Usart2_Init(115200);//stm32-8266ͨѶ���� 
	 
	ESP8266_Init();					//��ʼ��ESP8266
	while(OneNet_DevLink())			//����OneNET
		delay_ms(500);
	
	BEEP = 1;				//������ʾ����ɹ�
	delay_ms(250);
	BEEP = 0;
	
	OneNet_Subscribe(devSubTopic, 1);
	
	while(1)
	{
		
		if(timeCount % 40 == 0)//1000ms / 25 = 40 Լһ��ִ��һ��
		{
			//��ʪ�ȴ�������ȡ����
			DHT11_Read_Data(&humidityH,&humidityL,&temperatureH,&temperatureL);
			UsartPrintf(USART_DEBUG,"ʪ�ȣ�%d.%d�¶ȣ�%d.%d" ,humidityH,humidityL,temperatureH,temperatureL);
			//���նȴ�������ȡ����
			if(!i2c_CheckDevice(BH1750_Addr))
			{
				Light = LIght_Intensity();
				UsartPrintf(USART_DEBUG,"��ǰ����ǿ��Ϊ��%.1f lx\r\n", Light); 
			}
			if(alarm_is_free == 10)//����������Ȩ�Ƿ���� alarm_is_free > 0 ��ʼֵΪ10
			{
				if((humidityH < 80)&&(temperatureH < 30)&&(Light < 10000))alarmFlag = 0;
				else alarmFlag = 1;
			} 
			if(alarm_is_free < 10)alarm_is_free++;
			//UsartPrintf(USART_DEBUG,"alarm_is_free = %d\r\n",alarm_is_free);
			//UsartPrintf(USART_DEBUG,"alarmFlag = %d\r\n",alarmFlag);
		}	
		if(++timeCount >= 200)	// 	5000ms /25 = 200ms	//���ͼ��5s
	 {
		 UsartPrintf(USART_DEBUG, "OneNet_Publish\r\n");
		 sprintf(PUB_BUF,"{\"Hum\":%d.%d,\"Temp\":%d.%d,\"Light\":%.1f}",
		  humidityH,humidityL,temperatureH,temperatureL,Light);
		 OneNet_Publish(devPubTopic, PUB_BUF);
			timeCount = 0;
			ESP8266_Clear();
		}
		
			dataPtr = ESP8266_GetIPD(3);
			if(dataPtr != NULL)
				OneNet_RevPro(dataPtr);
			delay_ms(10);
			
	}
}


 /**
 *****************����ע�ӵĴ�����ͨ�����ÿ⺯����ʵ��IO���Ƶķ���*****************************************
int main(void)
{ 
 
	delay_init();		  //��ʼ����ʱ����
	LED_Init();		        //��ʼ��LED�˿�
	while(1)
	{
			GPIO_ResetBits(GPIOB,GPIO_Pin_5);  //LED0��Ӧ����GPIOB.5���ͣ���  ��ͬLED0=0;
			GPIO_SetBits(GPIOE,GPIO_Pin_5);   //LED1��Ӧ����GPIOE.5���ߣ��� ��ͬLED1=1;
			delay_ms(300);  		   //��ʱ300ms
			GPIO_SetBits(GPIOB,GPIO_Pin_5);	   //LED0��Ӧ����GPIOB.5���ߣ���  ��ͬLED0=1;
			GPIO_ResetBits(GPIOE,GPIO_Pin_5); //LED1��Ӧ����GPIOE.5���ͣ��� ��ͬLED1=0;
			delay_ms(300);                     //��ʱ300ms
	}
} 
 
 ****************************************************************************************************
 ***/
 

	
/**
*******************����ע�͵��Ĵ�����ͨ�� ֱ�Ӳ����Ĵ��� ��ʽʵ��IO�ڿ���**************************************
int main(void)
{ 
 
	delay_init();		  //��ʼ����ʱ����
	LED_Init();		        //��ʼ��LED�˿�
	while(1)
	{
     GPIOB->BRR=GPIO_Pin_5;//LED0��
	   GPIOE->BSRR=GPIO_Pin_5;//LED1��
		 delay_ms(300);
     GPIOB->BSRR=GPIO_Pin_5;//LED0��
	   GPIOE->BRR=GPIO_Pin_5;//LED1��
		 delay_ms(300);

	 }
 }
**************************************************************************************************
**/

