/**
 * @file            dev_ILI9341.c
 * @brief           TFT LCD 驱动芯片ILI6341驱动程序
 * @author          wujique
 * @date            2017年11月8日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月8日 星期三
 *   作    者:         wujique
 *   修改内容:   创建文件
       	1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
#include "stm32f10x.h"
#include "dev_ILI9341.h"

#define DEV_ILI9341_DEBUG

#ifdef DEV_ILI9341_DEBUG
#define ILI9341_DEBUG	printf 
#else
#define ILI9341_DEBUG(a, ...)
#endif

//扫描方向定义
//BIT 0 标识LR，1 R-L，0 L-R
//BIT 1 标识UD，1 D-U，0 U-D
//BIT 2 标识LR/UD，1 DU-LR，0 LR-DU
#define LR_BIT_MASK 0X01
#define UD_BIT_MASK 0X02
#define LRUD_BIT_MASK 0X04

#define L2R_U2D  (0) //从左到右,从上到下
#define L2R_D2U  (0 + UD_BIT_MASK)//从左到右,从下到上
#define R2L_U2D  (0 + LR_BIT_MASK) //从右到左,从上到下
#define R2L_D2U  (0 + UD_BIT_MASK + LR_BIT_MASK) //从右到左,从下到上

#define U2D_L2R  (LRUD_BIT_MASK)//从上到下,从左到右
#define U2D_R2L  (LRUD_BIT_MASK + LR_BIT_MASK) //从上到下,从右到左
#define D2U_L2R  (LRUD_BIT_MASK + UD_BIT_MASK) //从下到上,从左到右
#define D2U_R2L  (LRUD_BIT_MASK + UD_BIT_MASK+ LR_BIT_MASK) //从下到上,从右到左	 
/*
	这两个定义，
	定义的是LCD内部显存的page和colum方向的像素
	不是用户角度的XY坐标宽度
*/
#define TFTLCD_FRAME_W 320
#define TFTLCD_FRAME_H 240

struct _strDevLcd
{
	/*驱动需要的变量*/
	u8  dir;	//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u8  scandir;//扫描方向
	u16 width;	//LCD 宽度 
	u16 height;	//LCD 高度
};
/*
	定义一个LCD，宽和高对应的是XY坐标。
*/
struct _strDevLcd DevLcd={
			#if 0
			.dir = W_LCD,
			.width = TFTLCD_FRAME_W,
			.height = TFTLCD_FRAME_H,
			#else
			.dir = H_LCD,
			.width = TFTLCD_FRAME_H,
			.height = TFTLCD_FRAME_W,
			#endif
	};
/* 做一下转换，将结构体实例转换为指针 */			
struct _strDevLcd *lcd = &DevLcd;

/*---------------------------------------------------------------------------*/
extern volatile u16 *LcdReg;
extern volatile u16 *LcdData;

#define COG_RST_PIN                    GPIO_Pin_15                  
#define COG_RST_GPIO_PORT              GPIOA 

#define COG_BL_PIN                    GPIO_Pin_13                  
#define COG_BL_GPIO_PORT              GPIOC 


void drv_tftlcd_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	GPIO_SetBits(COG_RST_GPIO_PORT, COG_RST_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = COG_RST_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(COG_RST_GPIO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(COG_BL_GPIO_PORT, COG_BL_PIN);
	/* Configure PD0 and PD2 in output pushpull mode */
   	GPIO_InitStructure.GPIO_Pin = COG_BL_PIN;
   	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   	GPIO_Init(COG_BL_GPIO_PORT, &GPIO_InitStructure);
}


s32 bus_lcd_rst(u8 sta)
{
	switch(sta)
	{
		case 1:
			GPIO_SetBits(COG_RST_GPIO_PORT, COG_RST_PIN);
			break;
			
		case 0:
			GPIO_ResetBits(COG_RST_GPIO_PORT, COG_RST_PIN);
			break;
			
		default:
			return -1;

	}

	return 0;
}

/*
	bus_lcd_write_data
	bus_lcd_write_cmd
	这两个函数，通过I2C驱动将数据或命令发送到LCD。
	OLED屏幕只有I2C和电源地，没有其他控制信号。
	
*/
s32 bus_lcd_write_data(u8 *data, u32 len)
{
	u32 i;
	u16 *p;
	p = (u16 *)data;
	for(i=0; i<len; i++)
	{
		*LcdData = *(p+i);	
	}
	return 0;
}
s32 bus_lcd_w_data(u16 color, u32 len)
{
	u32 i;

	for(i=len; i>0; i--)
	{
		*LcdData = color;	
	}

	return 0;
}
s32 bus_lcd_read_data(u8 *data, u32 len)
{
	u32 i;
	
	u16 *p;
	p = (u16 *)data;
	
	for(i=0; i<len; i++)
	{
		*(p+i) = *LcdData;	
	}

	return 0;	
}

s32 bus_lcd_write_cmd(u8 cmd)
{
	*LcdReg = cmd;
	return 0;
}

s32 bus_lcd_bl(u8 sta)
{
	if(sta ==1)
	{
		GPIO_SetBits(COG_BL_GPIO_PORT, COG_BL_PIN);
	}
	else
	{
		GPIO_ResetBits(COG_BL_GPIO_PORT, COG_BL_PIN);	
	}
	return 0;
}

/*

	9341驱动

*/

/*9341命令定义*/
#define ILI9341_CMD_WRAM 0x2c
#define ILI9341_CMD_SETX 0x2a
#define ILI9341_CMD_SETY 0x2b


void drv_ILI9341_lcd_bl(u8 sta)
{
	bus_lcd_bl(sta);
}
	
/**
 *@brief:      drv_ILI9341_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ILI9341_scan_dir(u8 dir)
{
	u16 regval=0;

	/*设置从左边到右边还是右边到左边*/
	switch(dir)
	{
		case R2L_U2D:
		case R2L_D2U:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<6); 
			break;	 
	}

	/*设置从上到下还是从下到上*/
	switch(dir)
	{
		case L2R_D2U:
		case R2L_D2U:
		case D2U_L2R:
		case D2U_R2L:
			regval|=(1<<7); 
			break;	 
	}

	/*
		设置先左右还是先上下 Reverse Mode
		如果设置为1，LCD控制器已经将行跟列对调了，
		因此需要在显示中进行调整
	*/
	switch(dir)
	{
		case U2D_L2R:
		case D2U_L2R:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<5);
			break;	 
	}
	/*
		还可以设置RGB还是GBR
		还可以设置调转上下
	*/	
	regval|=(1<<3);//0:GBR,1:RGB  跟R61408相反

	bus_lcd_write_cmd(0x36);
	u16 tmp[2];
	tmp[0] = regval;
	bus_lcd_write_data((u8*)tmp, 1);


}

/**
 *@brief:      drv_ILI9341_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_set_cp_addr(u16 sc, u16 ec, u16 sp, u16 ep)
{
	u16 tmp[4];

	bus_lcd_write_cmd(ILI9341_CMD_SETX);
	tmp[0] = (sc>>8);
	tmp[1] = (sc&0XFF);
	tmp[2] = (ec>>8);
	tmp[3] = (ec&0XFF);
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((ILI9341_CMD_SETY));
	tmp[0] = (sp>>8);
	tmp[1] = (sp&0XFF);
	tmp[2] = (ep>>8);
	tmp[3] = (ep&0XFF);
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((ILI9341_CMD_WRAM));
	
	return 0;
}

/**
 *@brief:      drv_ILI9341_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9341_display_onoff(u8 sta)
{
	if(sta == 1)
		bus_lcd_write_cmd((0x29));
	else
		bus_lcd_write_cmd((0x28));

	return 0;
}

/**
 *@brief:      drv_ILI9341_init
 *@details:    初始化FSMC，并且读取ILI9341的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_init(void)
{
	u16 data;

	u16 tmp[16];

	drv_tftlcd_io_init();
	
	bus_lcd_rst(1);
	delay(50000);
	bus_lcd_rst(0);
	delay(50000);
	bus_lcd_rst(1);
	delay(50000);

	bus_lcd_write_cmd((0x00d3));
	/*读4个字节，第一个字节是dummy read， 第二字节是0x00， 第三字节是93，第四字节是41*/
	bus_lcd_read_data((u8*)tmp, 4);
	
	data = tmp[2]; 
	data<<=8;
	data |= tmp[3];

	ILI9341_DEBUG("read reg:%04x\r\n", data);

	if(data != 0x9341)
	{
		ILI9341_DEBUG("lcd drive no 9341\r\n");	

		return -1;
	}

	bus_lcd_write_cmd((0xCF));//Power control B
	tmp[0] = 0x00;
	tmp[1] = 0xC1;
	tmp[2] = 0x30;
	bus_lcd_write_data((u8*)tmp, 3);
	

	bus_lcd_write_cmd((0xED));//Power on sequence control 
	tmp[0] = 0x64;
	tmp[1] = 0x03;
	tmp[2] = 0x12;
	tmp[3] = 0x81;
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((0xE8));//Driver timing control A
	tmp[0] = 0x85;
	//tmp[1] = 0x01;
	tmp[1] = 0x10;
	tmp[2] = 0x7A;
	bus_lcd_write_data((u8*)tmp, 3);

	bus_lcd_write_cmd((0xCB));//Power control 
	tmp[0] = 0x39;
	tmp[1] = 0x2C;
	tmp[2] = 0x00;
	tmp[3] = 0x34;
	tmp[4] = 0x02;
	bus_lcd_write_data((u8*)tmp, 5);

	bus_lcd_write_cmd((0xF7));//Pump ratio control
	tmp[0] = 0x20;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xEA));//Driver timing control
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xC0));
	tmp[0] = 0x1B;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xC1));
	tmp[0] = 0x01;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xC5));
	tmp[0] = 0x30;
	tmp[1] = 0x30;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xC7));
	tmp[0] = 0xB7;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0x36));
	tmp[0] = 0x48;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0x3A));
	tmp[0] = 0x55;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xB1));
	tmp[0] = 0x00;
	tmp[1] = 0x1A;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xB6));
	tmp[0] = 0x0A;
	tmp[1] = 0xA2;
	bus_lcd_write_data((u8*)tmp, 2);

	bus_lcd_write_cmd((0xF2));
	tmp[0] = 0x00;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0x26));
	tmp[0] = 0x01;
	bus_lcd_write_data((u8*)tmp, 1);

	bus_lcd_write_cmd((0xE0));
	tmp[0] = 0x0F; tmp[1] = 0x2A; tmp[2] = 0x28; tmp[3] = 0x08;
	tmp[4] = 0x0E; tmp[5] = 0x08; tmp[6] = 0x54; tmp[7] = 0xa9;
	tmp[8] = 0x43; tmp[9] = 0x0a; tmp[10] = 0x0F; tmp[11] = 0x00;
	tmp[12] = 0x00; tmp[13] = 0x00; tmp[14] = 0x00;
	bus_lcd_write_data((u8*)tmp, 15);

	bus_lcd_write_cmd((0XE1));
	tmp[0] = 0x00; tmp[1] = 0x15; tmp[2] = 0x17; tmp[3] = 0x07;
	tmp[4] = 0x11; tmp[5] = 0x06; tmp[6] = 0x2B; tmp[7] = 0x56;
	tmp[8] = 0x3C; tmp[9] = 0x05; tmp[10] = 0x10; tmp[11] = 0x0F;
	tmp[12] = 0x3F; tmp[13] = 0x3F; tmp[14] = 0x0F;
	bus_lcd_write_data((u8*)tmp, 15);	

	bus_lcd_write_cmd((0x2B));
	tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x01; tmp[3] = 0x3f;
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((0x2A));
	tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x00; tmp[3] = 0xef;
	bus_lcd_write_data((u8*)tmp, 4);

	bus_lcd_write_cmd((0x11));
	delay(12000);
	bus_lcd_write_cmd((0x29));

	delay(5000);
	
	return 0;
}
/**
 *@brief:      drv_ILI9341_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_xy2cp(u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{
	/*
		显示XY轴范围
	*/
	if(sx >= lcd->width)
		sx = lcd->width-1;
	
	if(ex >= lcd->width)
		ex = lcd->width-1;
	
	if(sy >= lcd->height)
		sy = lcd->height-1;
	
	if(ey >= lcd->height)
		ey = lcd->height-1;
	/*
		XY轴，实物角度来看，方向取决于横屏还是竖屏
		CP轴，是控制器显存角度，
		XY轴的映射关系取决于扫描方向
	*/
	if(
		(((lcd->scandir&LRUD_BIT_MASK) == LRUD_BIT_MASK)
		&&(lcd->dir == H_LCD))
		||
		(((lcd->scandir&LRUD_BIT_MASK) == 0)
		&&(lcd->dir == W_LCD))
		)
		{
			*sc = sy;
			*ec = ey;
			*sp = sx;
			*ep = ex;
		}
	else
	{
		*sc = sx;
		*ec = ex;
		*sp = sy;
		*ep = ey;
	}
	
	return 0;
}
/**
 *@brief:      drv_ILI9341_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
s32 drv_ILI9341_drawpoint( u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);

	u16 tmp[2];
	tmp[0] = color;
	bus_lcd_write_data((u8*)tmp, 1);

	return 0;
}
/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_color_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9341_set_cp_addr(hsa, hea, vsa, vea);

	width = hea - hsa + 1;//得到填充的宽度
	height = vea - vsa + 1;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	
	u32 cnt;
	
	cnt = height*width;
	
	bus_lcd_w_data(color, cnt);

	return 0;

}

/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);

	width=(ec+1)-sc;
	height=(ep+1)-sp;

	printf("fill width:%d, height:%d\r\n", width, height);
	
	bus_lcd_write_data((u8 *)color, height*width);	
 
	return 0;

} 

s32 drv_ILI9341_prepare_display(u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;
	
	printf("XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);
	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	printf("cp:-%d-%d-%d-%d-\r\n", sc, ec, sp, ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);	
	return 0;
}

s32 drv_ILI9341_flush(u16 *color, u32 len)
{
	bus_lcd_write_data((u8 *)color,  len);	

	return 0;
} 
s32 drv_ILI9341_update(void)
{
	return 0;	
}

/*---------------------------------------------------------------------------*/
#include "font.h"
/*

	显示接口

*/
/**
 *@brief:	   line
 *@details:    画一条线
 *@param[in]   int x1			
			   int y1			
			   int x2			
			   int y2			
			   unsigned colidx	
 *@param[out]  无
 *@retval:	   
 */
void line (int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs (dx) < abs (dy)) 
	{
		if (y1 > y2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2)
		{
			drv_ILI9341_drawpoint( x1 >> 16, y1, colidx);
			x1 += dx;
			y1++;
		}
	} 
	else 
	{
		if (x1 > x2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) 
		{
			drv_ILI9341_drawpoint( x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

/**
 *@brief:	  put_cross
 *@details:   画十字
 *@param[in]  int x 		   
			  int y 		   
			  unsigned colidx  
 *@param[out]  无
 *@retval:	   
 */
void put_cross(int x, int y, unsigned colidx)
{
	
	line ( x - 10, y, x - 2, y, colidx);
	line ( x + 2, y, x + 10, y, colidx);
	line ( x, y - 10, x, y - 2, colidx);
	line ( x, y + 2, x, y + 10, colidx);

	line ( x - 6, y - 9, x - 9, y - 9, colidx + 1);
	line ( x - 9, y - 8, x - 9, y - 6, colidx + 1);
	line ( x - 9, y + 6, x - 9, y + 9, colidx + 1);
	line ( x - 8, y + 9, x - 6, y + 9, colidx + 1);
	line ( x + 6, y + 9, x + 9, y + 9, colidx + 1);
	line ( x + 9, y + 8, x + 9, y + 6, colidx + 1);
	line ( x + 9, y - 6, x + 9, y - 9, colidx + 1);
	line ( x + 8, y - 9, x + 6, y - 9, colidx + 1);

}

/**
 *@brief:	   put_char
 *@details:    显示一个英文
 *@param[in]   int x	   
			   int y	   
			   int c	   
			   int colidx  
 *@param[out]  无
 *@retval:	   
 */
void put_char(int x, int y, int c, int colidx)
{
	int i,j,bits;
	u8* p;
	

	p = (u8*)font_vga_8x8.path;//need fix
	for (i = 0; i < font_vga_8x8.height; i++) 
	{
		bits =	p[font_vga_8x8.height * c + i];
		for (j = 0; j < font_vga_8x8.width; j++, bits <<= 1)
		{
			if (bits & 0x80)
			{
				drv_ILI9341_drawpoint(x + j, y + i, colidx);
			}
		}
	}
}
/**
 *@brief:	   put_string
 *@details:    显示一个字符串
 *@param[in]   int x			
			   int y			
			   char *s			
			   unsigned colidx	
 *@param[out]  无
 *@retval:	   
 */
void put_string(int x, int y, char *s, unsigned colidx)
{
	int i;
	
	for (i = 0; *s; i++, x += font_vga_8x8.width, s++)
		put_char(x, y, *s, colidx);
}


void dev_lcd_init(void)
{
	drv_ILI9341_scan_dir(L2R_U2D);
	drv_ILI9341_display_onoff(1);
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, WHITE);
	drv_ILI9341_update();
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, BLUE);
	drv_ILI9341_update();	
}

/*
	测试程序
*/
void dev_tftlcd_test(void)
{
	/* 需要根据需求调整扫描方向 */
	drv_ILI9341_scan_dir(L2R_U2D);
	drv_ILI9341_display_onoff(1);
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, RED);
	drv_ILI9341_update();
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, GREEN);
	drv_ILI9341_update();
	drv_ILI9341_color_fill(0, lcd->width, 0, lcd->height, BLUE);
	drv_ILI9341_update();
	put_string(0,0, "Hello word!", BLACK);
}



