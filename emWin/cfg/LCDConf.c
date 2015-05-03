/****************************************************************************
 * @file     LCDConf.c
 * @brief    Display controller configuration
 * @version  1.0
 * @date     09. May. 2012
 *
 * @note
 * Copyright (C) 2012 NXP Semiconductors(NXP), All rights reserved.
 */

#include "GUI.h"
#include "LCD_X_SPI.h"

#ifndef _WINDOWS
#include "GUIDRV_FlexColor.h"
#endif

/*********************************************************************
*
*       Layer configuration
*
**********************************************************************
*/
//
// Color depth
//
#define LCD_BITSPERPIXEL 16 /* Currently the values 16 and 18 are supported */
//
// Physical display size
//
#define XSIZE_PHYS 320
#define YSIZE_PHYS 240

//
// Color conversion
//
#define COLOR_CONVERSION GUICC_565

//
// Display driver
//
#define DISPLAY_DRIVER GUIDRV_FLEXCOLOR

//
// Orientation
//
//#define DISPLAY_ORIENTATION (0)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_Y)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_MIRROR_Y)
#define DISPLAY_ORIENTATION (GUI_SWAP_XY)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_SWAP_XY)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_Y | GUI_SWAP_XY)
//#define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_MIRROR_Y | GUI_SWAP_XY)

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif
#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif
#ifndef   DISPLAY_ORIENTATION
  #define DISPLAY_ORIENTATION 0
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

#define wr_reg(reg, data) LCD_X_SPI_Write00(reg); LCD_X_SPI_Write01(data);

/*********************************************************************
*
*       _InitController
*
* Purpose:
*   Initializes the display controller
*/
static void _InitController(void) {
#ifndef WIN32

  GUI_X_Delay(10);
  LCD_X_SPI_Init();
  GUI_X_Delay(10);

  /* Start Initial Sequence ------------------------------------------------*/
  wr_reg(0x01, 0x0100);               /* Set SS bit                         */
  wr_reg(0x02, 0x0700);               /* Set 1 line inversion               */
  wr_reg(0x04, 0x0000);               /* Resize register                    */
  wr_reg(0x08, 0x0207);               /* 2 lines front, 7 back porch        */
  wr_reg(0x09, 0x0000);               /* Set non-disp area refresh cyc ISC  */
  wr_reg(0x0A, 0x0000);               /* FMARK function                     */
  wr_reg(0x0C, 0x0000);               /* RGB interface setting              */
  wr_reg(0x0D, 0x0000);               /* Frame marker Position              */
  wr_reg(0x0F, 0x0000);               /* RGB interface polarity             */

  /* Power On sequence -----------------------------------------------------*/
  wr_reg(0x10, 0x0000);               /* Reset Power Control 1              */
  wr_reg(0x11, 0x0000);               /* Reset Power Control 2              */
  wr_reg(0x12, 0x0000);               /* Reset Power Control 3              */
  wr_reg(0x13, 0x0000);               /* Reset Power Control 4              */
  GUI_X_Delay(200);                   /* Discharge cap power voltage (200ms)*/
  wr_reg(0x10, 0x12B0);               /* SAP, BT[3:0], AP, DSTB, SLP, STB   */
  wr_reg(0x11, 0x0007);               /* DC1[2:0], DC0[2:0], VC[2:0]        */
  GUI_X_Delay(50);                    /* Delay 50 ms                        */
  wr_reg(0x12, 0x01BD);               /* VREG1OUT voltage                   */
  GUI_X_Delay(50);                    /* Delay 50 ms                        */
  wr_reg(0x13, 0x1400);               /* VDV[4:0] for VCOM amplitude        */
  wr_reg(0x29, 0x000E);               /* VCM[4:0] for VCOMH                 */
  GUI_X_Delay(50);                    /* Delay 50 ms                        */
  wr_reg(0x20, 0x0000);               /* GRAM horizontal Address            */
  wr_reg(0x21, 0x0000);               /* GRAM Vertical Address              */
  /* Adjust the Gamma Curve ------------------------------------------------*/
  wr_reg(0x30, 0x0B0D);
  wr_reg(0x31, 0x1923);
  wr_reg(0x32, 0x1C26);
  wr_reg(0x33, 0x261C);
  wr_reg(0x34, 0x2419);
  wr_reg(0x35, 0x0D0B);
  wr_reg(0x36, 0x1006);
  wr_reg(0x37, 0x0610);
  wr_reg(0x38, 0x0706);
  wr_reg(0x39, 0x0304);
  wr_reg(0x3A, 0x0E05);
  wr_reg(0x3B, 0x0E01);
  wr_reg(0x3C, 0x010E);
  wr_reg(0x3D, 0x050E);
  wr_reg(0x3E, 0x0403);
  wr_reg(0x3F, 0x0607);
  /* Set GRAM area ---------------------------------------------------------*/
  wr_reg(0x50, 0x0000);               /* Horizontal GRAM Start Address      */
  wr_reg(0x51, (XSIZE_PHYS-1));           /* Horizontal GRAM End   Address      */
  wr_reg(0x52, 0x0000);               /* Vertical   GRAM Start Address      */
  wr_reg(0x53, (YSIZE_PHYS-1));            /* Vertical   GRAM End   Address      */

  /* Set Gate Scan Line ----------------------------------------------------*/
  wr_reg(0x60, 0x2700);
  wr_reg(0x61, 0x0001);               /* NDL,VLE, REV                       */
  wr_reg(0x6A, 0x0000);               /* Set scrolling line                 */

  /* Partial Display Control -----------------------------------------------*/
  wr_reg(0x80, 0x0000);
  wr_reg(0x81, 0x0000);
  wr_reg(0x82, 0x0000);
  wr_reg(0x83, 0x0000);
  wr_reg(0x84, 0x0000);
  wr_reg(0x85, 0x0000);

  /* Panel Control ---------------------------------------------------------*/
  wr_reg(0x90, 0x0010);
  wr_reg(0x92, 0x0000);
  wr_reg(0x93, 0x0003);
  wr_reg(0x95, 0x0110);
  wr_reg(0x97, 0x0000);
  wr_reg(0x98, 0x0000);
  /* Set GRAM write direction
     I/D=11 (Horizontal : increment, Vertical : increment)                  */
  /* AM=1   (address is updated in vertical writing direction)              */
  wr_reg(0x03, 0x1038);
  wr_reg(0x07, 0x0137);               /* 262K color and display ON          */
#endif
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/
void LCD_X_Config(void) {
  GUI_DEVICE * pDevice;
  GUI_PORT_API PortAPI;
  CONFIG_FLEXCOLOR Config = {0};

  //
  // Set display driver and color conversion for 1st layer
  //
  pDevice = GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);

  //
  // Display driver configuration, required for Lin-driver
  //
  if (DISPLAY_ORIENTATION & GUI_SWAP_XY) {
    LCD_SetSizeEx (0, YSIZE_PHYS,   XSIZE_PHYS);
    LCD_SetVSizeEx(0, VYSIZE_PHYS,  VXSIZE_PHYS);
  } else {
    LCD_SetSizeEx (0, XSIZE_PHYS,   YSIZE_PHYS);
    LCD_SetVSizeEx(0, VXSIZE_PHYS,  VYSIZE_PHYS);
  }

  //
  // Function pointers for 8 bit interface
  //
  PortAPI.pfWrite16_A0  = LCD_X_SPI_Write00;
  PortAPI.pfWrite16_A1  = LCD_X_SPI_Write01;
  PortAPI.pfWriteM16_A1 = LCD_X_SPI_WriteM01;
  PortAPI.pfReadM16_A1  = LCD_X_SPI_ReadM01;

  GUIDRV_FlexColor_SetFunc(pDevice, &PortAPI, GUIDRV_FLEXCOLOR_F66709, GUIDRV_FLEXCOLOR_M16C0B16);

  //
  // Orientation
  //
  Config.Orientation = DISPLAY_ORIENTATION;
  //Config.NumDummyReads = 2;		/* 5 dummy bytes are required when reading GRAM by SPI. 1 byte is read in LCD_X_SPI_WriteM01, so 4 bytes are left */
  Config.NumDummyReads = 1; // For ILI9341 4 bytes are required when reading GRAM by SPI.
  GUIDRV_FlexColor_Config(pDevice, &Config);
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;

  GUI_USE_PARA(LayerIndex);
  GUI_USE_PARA(pData);
  switch (Cmd) {
  //
  // Required
  //
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    _InitController();
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/
