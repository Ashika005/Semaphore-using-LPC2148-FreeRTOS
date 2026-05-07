																		   #include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

int a[5]={8,3,5,6,7};
int index=0;

void lcd1(void *parm);
void lcd2(void *parm);
void lcd3(void *parm);
void display(const char *);
void trans(char );

SemaphoreHandle_t m,m2;

int main()
{
	PINSEL0=1|1<<2;
	U0LCR=0x83;
	U0DLL=98;
	U0DLM=0;
// 	U0FDR=1<<4;
	U0LCR=0x03;
	PINSEL1=0;
	IO0DIR=0;
	IO1DIR=~0;
	m=xSemaphoreCreateCounting(5,5);
	if(m==NULL)
		display("Failed To Create Counting Semaphore\r\n");
	else
		display("Successfully Created Counting Semaphore\r\n");
	
	m2=xSemaphoreCreateMutex();
	if(m2==NULL)
		display("Failed To Create Mutex\r\n");
	else
		display("Successfully Created Mutex\r\n");
	
	xTaskCreate(lcd1,"Welcome",90,NULL,0,NULL);
	xTaskCreate(lcd2,"hello",90,NULL,0,NULL);
	xTaskCreate(lcd3,"3rd",90,NULL,0,NULL);
	vTaskStartScheduler();
	while(1);
}
void trans(char a)
{
while((U0LSR&(1<<5))==0);
U0THR=a;
}
void display(const char *a)
{
	while(*a)
	{
	trans(*a++);
	}
}
void lcd1(void *parm)
{
	char b;
	while(1)
	{
		if((IO0PIN&(1<<12))==(1<<12))
		{	
			b=uxSemaphoreGetCount(m);
			if(xSemaphoreTake(m,1000)==1)
			{			
				if(xSemaphoreTake(m2,3000)==1)
				{
	
					display("\r\nArray Value is ");
					trans(a[index++]+48);
					if(index==5) 
						index=0;
					display("\rIn Take");
					display("\rBefore Taken - ");
					trans(b+48);
					b=uxSemaphoreGetCount(m);
					display("\rAfter Taken - ");
					trans(b+48);
				    display("\rAfter Taken - ");
					xSemaphoreGive(m2);
				}
			while((IO0PIN&(1<<12))==(1<<12));
		
			}
		}
	}
}
void lcd3(void *parm)
{
	char b;
	while(1)
	{
		if((IO0PIN&(1<<13))==(1<<13))
		{
			b=uxSemaphoreGetCount(m);
			if(xSemaphoreTake(m,1000)==1)
			{			
				if(xSemaphoreTake(m2,3000)==1)
				{
	
					display("\r\nArray Value is ");
					trans(a[index++]+48);
					if(index==5) 
						index=0;
					display("\rIn Take");
					display("\rBefore Taken - ");
					trans(b+48);
					b=uxSemaphoreGetCount(m);
					display("\rAfter Taken - ");
					trans(b+48);
					xSemaphoreGive(m2);
				}
			}
		while((IO0PIN&(1<<13))==(1<<13));
		
	}
	}
}
void lcd2(void *parm)
{
	char b;
	while(1)
	{
		if((IO0PIN&(1<<14))==(1<<14))
		{
			if(xSemaphoreTake(m2,3000)==1)
			{
				b=uxSemaphoreGetCount(m);
				display("\rIn give");
				display("\rBefore Given - ");
				trans(b+48);
				xSemaphoreGive(m);
				b=uxSemaphoreGetCount(m);
				display("\rAfter Given - ");
				trans(b+48);
				while((IO0PIN&(1<<14))==(1<<14));
				xSemaphoreGive(m2);
			}
		}
		
	}
}
