/****************************************************************************
 * @file     LCD_X_SPI.h
 * @brief    FlexColor SPI driver
 * @version  1.0
 * @date     09. May. 2012
 *
 * @note
 * Copyright (C) 2012 NXP Semiconductors(NXP), All rights reserved.
 */

#ifndef LCD_X_SPI_H_
#define LCD_X_SPI_H_

#define ILI9341_SPI_MOSI p11
#define ILI9341_SPI_MISO p12
#define ILI9341_SPI_SCK  p13
#define ILI9341_LCD_ON	 p14
#define ILI9341_LCD_CS	 p15
#define ILI9341_LCD_RST  p16
#define ILI9341_LCD_DC	 p17

void LCD_X_SPI_Init (void);							/* Initializes the LCD */
void LCD_X_SPI_Write00 (U16 c);						/* Write single command */
void LCD_X_SPI_Write01 (U16 c);						/* Write single data word */
void LCD_X_SPI_WriteM01(U16 * pData, int NumWords);	/* Write multiple data words */
void LCD_X_SPI_ReadM01 (U16 * pData, int NumWords);	/* Read multiple data words */

#endif /* LCD_X_SPI_H_ */
