/**
 * @file            bus_spi.c
 * @brief           SPI总线管理
 * @author          test
 * @date            2019年03月26日 星期四
 * @version         初稿
 * @par             
 * @par History:
 * 1.日    期:      
 *   作    者:      test
 *   修改内容:      创建文件
		版权说明：
		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且保留版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
#include "stm32f10x.h"
#include "bus_vspi.h"


#define BUS_VSPI_DEBUG

#ifdef BUS_VSPI_DEBUG
#define VSPI_DEBUG	printf 
#else
#define VSPI_DEBUG(a, ...)
#endif

#define VSPI_MOSI_PORT 	GPIOC
#define VSPI_MOSI_PIN 	GPIO_Pin_5

#define VSPI_MISO_PORT 	GPIOC
#define VSPI_MISO_PIN 	GPIO_Pin_4

#define VSPI_CLK_PORT 	GPIOB
#define VSPI_CLK_PIN 	GPIO_Pin_0

#define VSPI_CS_PORT 	GPIOB
#define VSPI_CS_PIN 	GPIO_Pin_1

/**
 *@brief:      mcu_vspi_init
 *@details:    初始化虚拟SPI
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_vspi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	printf( "vspi init\r\n");

	GPIO_SetBits(VSPI_MOSI_PORT, VSPI_MOSI_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_MOSI_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(VSPI_MOSI_PORT, &GPIO_InitStructure);

	GPIO_SetBits(VSPI_MISO_PORT, VSPI_MISO_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_MISO_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   	GPIO_Init(VSPI_MISO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(VSPI_CLK_PORT, VSPI_CLK_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_CLK_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(VSPI_CLK_PORT, &GPIO_InitStructure);

	GPIO_SetBits(VSPI_CS_PORT, VSPI_CS_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = VSPI_CS_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(VSPI_CS_PORT, &GPIO_InitStructure);
	
	return 0;
}


/**
 *@brief:      vspi_delay
 *@details:    虚拟SPI时钟延时
 *@param[in]   u32 delay  
 *@param[out]  无
 *@retval:     
 */
void vspi_delay(u32 delay)
{
	volatile u32 i=delay;

	while(i>0)
	{
		i--;	
	}

}

u32 VspiDelay = 0;


/**
 *@brief:      mcu_vspi_open
 *@details:    打开虚拟SPI
 *@param[in]   SPI_DEV dev    
               SPI_MODE mode  
               u16 pre        
 *@param[out]  无
 *@retval:     
 */
s32 mcu_vspi_open(SPI_MODE mode, u16 khz)
{
	VspiDelay = khz;
	GPIO_ResetBits(VSPI_CS_PORT, VSPI_CS_PIN);
    return 0;
}
/**
 *@brief:      mcu_vspi_close
 *@details:    关闭虚拟SPI
 *@param[in]   SPI_DEV dev  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_vspi_close(void)
{
	GPIO_SetBits(VSPI_CS_PORT, VSPI_CS_PIN);
    return 0;
}
/**
 *@brief:      mcu_vspi_transfer
 *@details:       虚拟SPI通信
 *@param[in]   SPI_DEV dev  
               u8 *snd      
               u8 *rsv      
               s32 len      
 *@param[out]  无
 *@retval:     

 		node->clk = 0, CLK时钟1.5M 2018.06.02
 */
s32 mcu_vspi_transfer(u8 *snd, u8 *rsv, s32 len)
{
	u8 i;
	u8 data;
	s32 slen;
	u8 misosta;

	volatile u16 delay;
	
    if( ((snd == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return -1;
    }

	slen = 0;

	while(1)
	{
		if(slen >= len)
			break;

		if(snd == NULL)
			data = 0xff;
		else
			data = *(snd+slen);
		
		for(i=0; i<8; i++)
		{
			GPIO_ResetBits(VSPI_CLK_PORT, VSPI_CLK_PIN);
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
			if(data&0x80)
				GPIO_SetBits(VSPI_MOSI_PORT, VSPI_MOSI_PIN);
			else
				GPIO_ResetBits(VSPI_MOSI_PORT, VSPI_MOSI_PIN);
			
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
			data<<=1;
			GPIO_SetBits(VSPI_CLK_PORT, VSPI_CLK_PIN);
			
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
			misosta = GPIO_ReadInputDataBit(VSPI_MISO_PORT, VSPI_MISO_PIN);
			if(misosta == Bit_SET)
			{
				data |=0x01;
			}
			else
			{
				data &=0xfe;
			}
			
			delay = VspiDelay;
			while(delay>0)
			{
				delay--;	
			}
			
		}

		if(rsv != NULL)
			*(rsv+slen) = data;
		
		slen++;
	}

	return slen;
}


