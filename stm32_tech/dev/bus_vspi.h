#ifndef __BUS_VSPI_H_
#define __BUS_VSPI_H_

/*

SPIģʽ

*/
typedef enum{
	SPI_MODE_0 =0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3,
	SPI_MODE_MAX
}SPI_MODE;


extern s32 mcu_vspi_init(void);
extern s32 mcu_vspi_transfer( u8 *snd, u8 *rsv, s32 len);

#endif

