#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include "stm32f10x.h"

//时间结构体
typedef struct 
{
	vu8 hour;
	vu8 min;
	vu8 sec;			
	//公历日月年周
	vu16 w_year;
	vu8  w_month;
	vu8  w_date;
	vu8  week;		 
}_calendar_obj;					 
extern _calendar_obj calendar;	//日历结构体

extern uint8_t const mon_table[12];	//月份日期数据表
void Disp_Time(uint8_t x,uint8_t y,uint8_t size);//在制定位置开始显示时间
void Disp_Week(uint8_t x,uint8_t y,uint8_t size,uint8_t lang);//在指定位置显示星期

uint8_t bsp_InitRTC(void);
uint8_t Is_Leap_Year(u16 year);//平年,闰年判断
uint8_t RTC_Get(void);         //更新时间   
uint8_t RTC_Get_Week(u16 year,uint8_t month,uint8_t day);
uint8_t RTC_Set(u16 syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);//设置时间		

#endif
