/*

*/
#include "stm32f10x.h"
#include "tslib.h"
#include "dev_touchscreen.h"
#include "dev_xpt2046.h"



#define DEV_TS_DEBUG

#ifdef DEV_TS_DEBUG
#define TS_DEBUG	printf 
#else
#define TS_DEBUG(a, ...)
#endif


/*  触摸屏接口：

	*/
s32 TpSgd = -2;
#define  DEV_TP_QUE_MAX (250)//队列长度, 不要改小
struct ts_sample DevTpSampleQue[DEV_TP_QUE_MAX];//扫描得到稳定的点的队列
volatile u16 TpQueWindex = 0;
volatile u16 TpQueRindex = 0;

/**
 *@brief:      dev_touchscreen_init
 *@details:    
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchscreen_init(void)
{
	TpSgd = -1;

	return dev_xpt2046_init();

}
/**
 *@brief:      dev_touchscreen_open
 *@details:    打开触摸屏，
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchscreen_open(void)
{
	/*启动转换*/

	TpSgd = 0;
	
	TpQueWindex = TpQueWindex;
	
	return dev_xpt2046_open();

}

s32 dev_touchscreen_close(void)
{
	TpSgd = -1;
	
	return dev_xpt2046_close();
}
/**
 *@brief:      dev_touchscreen_read
 *@details:    读触摸屏采样原始样点，应用层或者tslib调用
 *@param[in]   struct ts_sample *samp  
               int nr                  
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchscreen_read(struct ts_sample *samp, int nr)
{
	int i = 0;

	while(1)
	{
		if(i>=nr)
			break;

		if(TpQueWindex ==  TpQueRindex)
			break;

		samp->pressure = DevTpSampleQue[TpQueRindex].pressure;
		samp->x = DevTpSampleQue[TpQueRindex].x;
		samp->y = DevTpSampleQue[TpQueRindex].y;
		
		TpQueRindex++;
		if(TpQueRindex >= DEV_TP_QUE_MAX)
			TpQueRindex = 0;

		i++;
	}

	return i;
}
/**
 *@brief:      dev_touchscreen_write
 *@details:    将样点写入缓冲，底层调用
 *@param[in]   struct ts_sample *samp  
               int nr                  
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchscreen_write(struct ts_sample *samp, int nr)
{
	int index;
	struct ts_sample *p;
	
	index = 0;
	while(1)
	{
		if(index >= nr)
			break;	
		p = samp+index;
		
		DevTpSampleQue[TpQueWindex].pressure = p->pressure;//压力要搞清楚怎么计算
		DevTpSampleQue[TpQueWindex].x = p->x;
		DevTpSampleQue[TpQueWindex].y = p->y;
		TpQueWindex++;
		if(TpQueWindex >=  DEV_TP_QUE_MAX)
			TpQueWindex = 0;

		index++;
	}
		
	return index;
}

s32 dev_touchscreen_ioctrl(void)
{


	return 0;
}


