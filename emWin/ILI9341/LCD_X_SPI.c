/****************************************************************************
 * @file     LCD_X_SPI.c
 * @brief    FlexColor SPI driver
 * @version  1.0
 * @date     09. May. 2012
 *
 * @note
 * Copyright (C) 2012 NXP Semiconductors(NXP), All rights reserved.
 */

#include "GUI.h"
#include "mbed.h"
#include "rtos.h"
#include "LCD_X_SPI.h"

// Our global SPI instance for the display
static SPI ili9341(ILI9341_SPI_MOSI, ILI9341_SPI_MISO, ILI9341_SPI_SCK);
DigitalOut lcd_cs(ILI9341_LCD_CS);
DigitalOut lcd_on(ILI9341_LCD_ON);
DigitalOut lcd_rst(ILI9341_LCD_RST);
DigitalOut lcd_dc(ILI9341_LCD_DC);

/*********************** Hardware specific configuration **********************/

/* SPI Interface: SSP1

   PINS:
   - CS     = P0.6 (GPIO pin)
   - SCK    = P0.7 (SCK1)
   - SDO    = P0.8 (MISO1)
   - SDI    = P0.9 (MOSI1)                                                    */

#define PIN_CS      (1 << 6)

/*--------------- Graphic LCD interface hardware definitions -----------------*/

/* Pin CS setting to 0 or 1 */
#define LCD_CS(x)   ((x) ?  (lcd_cs = 1)   : (lcd_cs = 0))

#define SPI_START   (0x70)              /* Start byte for SPI transfer */
#define SPI_RD      (0x01)              /* WR bit 1 within start */
#define SPI_WR      (0x00)              /* WR bit 0 within start */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte */

/* local functions */
__inline void wr_cmd (unsigned char cmd);						/* Write command to LCD */
__inline void wr_dat (unsigned short dat);						/* Write data to LCD */
__inline unsigned char spi_tran (unsigned char byte);	/* Write and read a byte over SPI */
__inline void spi_tran_fifo (unsigned char byte);		/* Only write a byte over SPI (faster) */

/*******************************************************************************
* Initialize SPI (SSP) peripheral at 8 databit with a bitrate of 12.5Mbps      *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/
void LCD_X_SPI_Init(void)
{
  uint8_t Dummy;

  /* Enable clock for SSP1, clock = CCLK / 2 */
  LPC_SC->PCONP       |= 0x00000400;
  LPC_SC->PCLKSEL0    |= 0x00200000;	/* PCLK = CCLK / 2 = 50MHz */

  /* Configure the LCD Control pins */
  LPC_PINCON->PINSEL9 &= 0xF0FFFFFF;
  LPC_GPIO4->FIODIR   |= 0x30000000;
  LPC_GPIO4->FIOSET    = 0x20000000;

  /* SSEL1 is GPIO output set to high */
  LPC_GPIO0->FIODIR   |= 0x00000040;
  LPC_GPIO0->FIOSET    = 0x00000040;
  LPC_PINCON->PINSEL0 &= 0xFFF03FFF;
  LPC_PINCON->PINSEL0 |= 0x000A8000;

  /* Enable SPI in Master Mode, CPOL=1, CPHA=1 */
  /* 12.5 MBit used for Data Transfer @ 100MHz */
  LPC_SSP1->CR0        = 0x1C7;		/* SCR = 1 */
  LPC_SSP1->CPSR       = 0x02;		/* CPSDVSR  = 2. Bit frequency = PCLK / (CPSDVSR × [SCR+1]) = 50 / (2 × [1+1]) = 12.5Mbps */
  LPC_SSP1->CR1        = 0x02;

  while(LPC_SSP1->SR & (1<<2))
    Dummy = LPC_SSP1->DR;			/* Clear the Rx FIFO */

  LPC_GPIO4->FIOSET = 0x10000000;	/* Activate LCD backlight */
}

/*******************************************************************************
* Write command                                                                *
*   Parameter:    c: command to write                                          *
*   Return:                                                                    *
*******************************************************************************/
void LCD_X_SPI_Write00(U16 c)
{
  wr_cmd(c);
}

/*******************************************************************************
* Write data byte                                                              *
*   Parameter:    c: word to write                                             *
*   Return:                                                                    *
*******************************************************************************/
void LCD_X_SPI_Write01(U16 c)
{
  wr_dat(c);
}

/*******************************************************************************
* Write multiple data bytes                                                    *
*   Parameter:    pData:    pointer to words to write                          *
*                 NumWords: Number of words to write                           *
*   Return:                                                                    *
*******************************************************************************/
void LCD_X_SPI_WriteM01(U16 * pData, int NumWords)
{
  LCD_CS(0);
  spi_tran_fifo(SPI_START | SPI_WR | SPI_DATA);			/* Write : RS = 1, RW = 0 */

  while(NumWords--)
  {
	  spi_tran_fifo(((*pData) >>   8));					/* Write D8..D15 */
	  spi_tran_fifo(((*(pData++)) & 0xFF));				/* Write D0..D7 */
  }
  while(LPC_SSP1->SR & (1<<4));							/* wait until done */
  LCD_CS(1);
}

/*******************************************************************************
* Read multiple data bytes                                                     *
*   Parameter:    pData:    pointer to words to read                           *
*                 NumWords: Number of words to read                            *
*   Return:                                                                    *
*******************************************************************************/
void LCD_X_SPI_ReadM01(U16 * pData, int NumWords)
{
  LCD_CS(0);
  spi_tran_fifo(SPI_START | SPI_RD | SPI_DATA);			/* Read: RS = 1, RW = 1 */
  spi_tran_fifo(0);										/* Dummy byte 1 */
  while(NumWords--)
  {
	*pData = spi_tran(0) << 8;							/* Read D8..D15 */
	*(pData++) |= spi_tran(0);							/* Read D0..D7 */
  }
  while(LPC_SSP1->SR & (1<<4));							/* wait until done */
  LCD_CS(1);
}

/*******************************************************************************
* Write a command the LCD controller                                           *
*   Parameter:    cmd:    command to be written                                *
*   Return:                                                                    *
*******************************************************************************/
__inline void wr_cmd (unsigned char cmd)
{
  LCD_CS(0);
  spi_tran_fifo(SPI_START | SPI_WR | SPI_INDEX);		/* Write : RS = 0, RW = 0 */
  spi_tran_fifo(0);
  spi_tran_fifo(cmd);
  while(LPC_SSP1->SR & (1<<4));							/* wait until done */
  LCD_CS(1);
}

/*******************************************************************************
* Write data to the LCD controller                                             *
*   Parameter:    dat:    data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/
__inline void wr_dat (unsigned short dat)
{
  LCD_CS(0);
  spi_tran_fifo(SPI_START | SPI_WR | SPI_DATA);			/* Write : RS = 1, RW = 0 */
  spi_tran_fifo((dat >>   8));							/* Write D8..D15 */
  spi_tran_fifo((dat & 0xFF));							/* Write D0..D7 */
  while(LPC_SSP1->SR & (1<<4));							/* wait until done */
  LCD_CS(1);
}

/*******************************************************************************
* Transfer 1 byte over the serial communication, wait until done and return    *
* received byte                                                                *
*   Parameter:    byte:   byte to be sent                                      *
*   Return:               byte read while sending                              *
*******************************************************************************/
__inline unsigned char spi_tran (unsigned char byte)
{
  uint8_t Dummy;

  while(LPC_SSP1->SR & (1<<4) || LPC_SSP1->SR & (1<<2))	/* while SSP1 busy or Rx FIFO not empty ... */
	  Dummy = LPC_SSP1->DR;								/* ... read Rx FIFO */
  LPC_SSP1->DR = byte;									/* Transmit byte */
  while (!(LPC_SSP1->SR & (1<<2)));						/* Wait until RNE set */
  return (LPC_SSP1->DR);
}

/*******************************************************************************
* Put byte in SSP1 Tx FIFO. Used for faster SPI writing                        *
*   Parameter:    byte:   byte to be sent                                      *
*   Return:                                                                    *
*******************************************************************************/
__inline void spi_tran_fifo (unsigned char byte)
{
  while (!(LPC_SSP1->SR & (1<<1)));						/* wait until TNF set */
  LPC_SSP1->DR = byte;
}

/*************************** End of file ****************************/
