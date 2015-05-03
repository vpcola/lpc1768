/* mbed library for 240*320 pixel display TFT based on ILI9341 LCD Controller
 * Copyright (c) 2013, 2014 Peter Drescher - DC2PD
 * Special version for STM Nucleo -L152
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

 // 24.06.14   initial version
 // 25.06.14   add Nucleo F103RB 

// only include this file if target is L152 or F103RB : 
#if defined TARGET_NUCLEO_L152RE || defined TARGET_NUCLEO_F103RB

#include "SPI_TFT_ILI9341.h"
#include "mbed.h"

#if defined TARGET_NUCLEO_L152RE
#include "stm32l1xx_dma.h"
#define use_ram
#endif

#if defined TARGET_NUCLEO_F103RB
#include "stm32f10x_dma.h"
#endif

#define BPP         16                  // Bits per pixel

//extern Serial pc;
//extern DigitalOut xx;     // debug !!

DMA_InitTypeDef DMA_InitStructure; 


SPI_TFT_ILI9341::SPI_TFT_ILI9341(PinName mosi, PinName miso, PinName sclk, PinName cs, PinName reset, PinName dc, const char *name)
    : GraphicsDisplay(name), SPI(mosi,miso,sclk,NC), _cs(cs), _reset(reset), _dc(dc)
{

    format(8,3);                 // 8 bit spi mode 3
    frequency(10000000);         // 10 Mhz SPI clock : result 2 / 4 = 8
    orientation = 0;
    char_x = 0;
    if(_spi.spi == SPI_1){       // test which SPI is in use
         spi_num = 1;
    }
    if(_spi.spi == SPI_2){
         spi_num = 2;
    }
    #ifdef SPI_3                // there is no SPI 3 on all devices
    if(_spi.spi == SPI_3){
         spi_num = 3;
    }
    #endif
    tft_reset();
}

// we define a fast write to the SPI port
// we use the bit banding address to get the flag without masking

#define bit_SPI1_txe  *((volatile unsigned int *)0x42260104)
#define SPI1_DR       *((volatile unsigned int *)0x4001300C)
#define bit_SPI2_txe  *((volatile unsigned int *)0x42070104)
#define SPI2_DR       *((volatile unsigned int *)0x4000380C)
#define bit_SPI3_txe  *((volatile unsigned int *)0x42078104)
#define SPI3_DR       *((volatile unsigned int *)0x40003C0C)

void SPI_TFT_ILI9341::f_write(int data){

switch(spi_num){  // used SPI port
case (1):
    while(bit_SPI1_txe == 0);  // wait for SPI1->SR TXE flag
    SPI1_DR = data;
    break;

case (2):
    while( bit_SPI2_txe  == 0);  // wait for SPI2->SR TXE flag
    SPI2_DR = data;
    break;

case (3):
    while( bit_SPI3_txe == 0);  // wait for SPI3->SR TXE flag
    SPI3_DR = data;
    break;

    }
}

// wait for SPI not busy
// we have to wait for the last bit to switch the cs off
// we use the bit banding address to get the flag without masking

#define bit_SPI1_bsy  *((volatile unsigned int *)0x4226011C)
#define bit_SPI2_bsy  *((volatile unsigned int *)0x4207011C)
#define bit_SPI3_bsy  *((volatile unsigned int *)0x4207811C)

void inline SPI_TFT_ILI9341::spi_bsy(void){    
switch(spi_num){        // decide which SPI is to use
case (1):
    while(bit_SPI1_bsy == 1);   // SPI1->SR bit 7
    break;

case (2):
    while(bit_SPI2_bsy == 1);   // SPI2->SR bit 7
    break;

case (3):
    while(bit_SPI3_bsy == 1);   // SPI2->SR bit 7
    break;
    }
}


// switch fast between 8 and 16 bit mode
#define bit_SPI1_dff  *((volatile unsigned int *)0x4226002C)
#define bit_SPI2_dff  *((volatile unsigned int *)0x4207002C)
#define bit_SPI3_dff  *((volatile unsigned int *)0x4207802C)
void SPI_TFT_ILI9341::spi_16(bool s){
switch(spi_num){        // decide which SPI is to use
case(1):
    if(s) bit_SPI1_dff = 1; // switch to 16 bit Mode
    else  bit_SPI1_dff = 0; // switch to 8  bit Mode
    break;

case(2):
    if(s) bit_SPI2_dff = 1; // switch to 16 bit Mode
    else  bit_SPI2_dff = 0; // switch to 8  bit Mode
    break;

case(3):
    if(s) bit_SPI3_dff = 1; // switch to 16 bit Mode
    else  bit_SPI3_dff = 0; // switch to 8  bit Mode
    break;
    }
}


int SPI_TFT_ILI9341::width()
{
    if (orientation == 0 || orientation == 2) return 240;
    else return 320;
}


int SPI_TFT_ILI9341::height()
{
    if (orientation == 0 || orientation == 2) return 320;
    else return 240;
}


void SPI_TFT_ILI9341::set_orientation(unsigned int o)
{
    orientation = o;
    wr_cmd(0x36);                     // MEMORY_ACCESS_CONTROL
    switch (orientation) {
        case 0:
            f_write(0x48);
            break;
        case 1:
            f_write(0x28);
            break;
        case 2:
            f_write(0x88);
            break;
        case 3:
            f_write(0xE8);
            break;
    }
    spi_bsy();    // wait for end of transfer
    _cs = 1;
    WindowMax();
}


// write command to tft register
// use fast command
void SPI_TFT_ILI9341::wr_cmd(unsigned char cmd)
{
    _dc = 0;
    _cs = 0;
    f_write(cmd);
    spi_bsy();
    _dc = 1;
}

void SPI_TFT_ILI9341::wr_dat(unsigned char dat)
{
   f_write(dat);
   spi_bsy();  // wait for SPI send
}

// the ILI9341 can read
char SPI_TFT_ILI9341::rd_byte(unsigned char cmd)
{
    // has to change !!
    return(0);
}

// read 32 bit
int SPI_TFT_ILI9341::rd_32(unsigned char cmd)
{
    // has to change !!!
    return(0);
}

int  SPI_TFT_ILI9341::Read_ID(void){
    int r;
    r = rd_byte(0x0A);
    r = rd_byte(0x0A);
    r = rd_byte(0x0A);
    r = rd_byte(0x0A);
    return(r);
}


// Init code based on MI0283QT datasheet
// this code is called only at start
// no need to be optimized

void SPI_TFT_ILI9341::tft_reset()
{
    _cs = 1;                           // cs high
    _dc = 1;                           // dc high
    _reset = 0;                        // display reset

    wait_us(50);
    _reset = 1;                       // end hardware reset
    wait_ms(5);

    wr_cmd(0x01);                     // SW reset
    wait_ms(5);
    wr_cmd(0x28);                     // display off

    /* Start Initial Sequence ----------------------------------------------------*/
    wr_cmd(0xCF);
    f_write(0x00);
    f_write(0x83);
    f_write(0x30);
    spi_bsy();
     _cs = 1;

     wr_cmd(0xED);
     f_write(0x64);
     f_write(0x03);
     f_write(0x12);
     f_write(0x81);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xE8);
     f_write(0x85);
     f_write(0x01);
     f_write(0x79);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xCB);
     f_write(0x39);
     f_write(0x2C);
     f_write(0x00);
     f_write(0x34);
     f_write(0x02);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xF7);
     f_write(0x20);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xEA);
     f_write(0x00);
     f_write(0x00);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xC0);                     // POWER_CONTROL_1
     f_write(0x26);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xC1);                     // POWER_CONTROL_2
     f_write(0x11);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xC5);                     // VCOM_CONTROL_1
     f_write(0x35);
     f_write(0x3E);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xC7);                     // VCOM_CONTROL_2
     f_write(0xBE);
     spi_bsy();
     _cs = 1;

     wr_cmd(0x36);                     // MEMORY_ACCESS_CONTROL
     f_write(0x48);
     spi_bsy();
     _cs = 1;

     wr_cmd(0x3A);                     // COLMOD_PIXEL_FORMAT_SET
     f_write(0x55);                 // 16 bit pixel
     spi_bsy();
     _cs = 1;

     wr_cmd(0xB1);                     // Frame Rate
     f_write(0x00);
     f_write(0x1B);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xF2);                     // Gamma Function Disable
     f_write(0x08);
     spi_bsy();
     _cs = 1;

     wr_cmd(0x26);
     f_write(0x01);                 // gamma set for curve 01/2/04/08
     spi_bsy();
     _cs = 1;

     wr_cmd(0xE0);                     // positive gamma correction
     f_write(0x1F);
     f_write(0x1A);
     f_write(0x18);
     f_write(0x0A);
     f_write(0x0F);
     f_write(0x06);
     f_write(0x45);
     f_write(0x87);
     f_write(0x32);
     f_write(0x0A);
     f_write(0x07);
     f_write(0x02);
     f_write(0x07);
     f_write(0x05);
     f_write(0x00);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xE1);                     // negativ gamma correction
     f_write(0x00);
     f_write(0x25);
     f_write(0x27);
     f_write(0x05);
     f_write(0x10);
     f_write(0x09);
     f_write(0x3A);
     f_write(0x78);
     f_write(0x4D);
     f_write(0x05);
     f_write(0x18);
     f_write(0x0D);
     f_write(0x38);
     f_write(0x3A);
     f_write(0x1F);
     spi_bsy();
     _cs = 1;

     WindowMax ();

     //wr_cmd(0x34);                     // tearing effect off
     //_cs = 1;

     //wr_cmd(0x35);                     // tearing effect on
     //_cs = 1;

     wr_cmd(0xB7);                       // entry mode
     f_write(0x07);
     spi_bsy();
     _cs = 1;

     wr_cmd(0xB6);                       // display function control
     f_write(0x0A);
     f_write(0x82);
     f_write(0x27);
     f_write(0x00);
     spi_bsy();
     _cs = 1;

     wr_cmd(0x11);                     // sleep out
     spi_bsy();
     _cs = 1;

     wait_ms(100);

     wr_cmd(0x29);                     // display on
     spi_bsy();
     _cs = 1;

     wait_ms(100);

    //    Configure the DMA controller init-structure
    DMA_StructInit(&DMA_InitStructure);
    switch(spi_num){        // decide which SPI is to use
    case (1):
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);    // SPI1 and SPI2 are using DMA 1
        DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t) &(SPI1->DR);
        break;
    case (2):
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);    // SPI1 and SPI2 are using DMA 1
        DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t) &(SPI2->DR);
        break;
    case (3):
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);    // SPI3 is using DMA 2
        DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t) &(SPI3->DR);
        break;
    }
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_BufferSize  = 0;
    DMA_InitStructure.DMA_PeripheralInc  = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize  = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority  = DMA_Priority_High;
 }


// speed optimized
// write direct to SPI1 register !
void SPI_TFT_ILI9341::pixel(int x, int y, int color)
{
    wr_cmd(0x2A);
    spi_16(1);         // switch to 8 bit Mode
    f_write(x);
    spi_bsy();
    _cs = 1;

    spi_16(0);        // switch to 8 bit Mode
    wr_cmd(0x2B);
    spi_16(1);
    f_write(y);
    spi_bsy();
    _cs = 1;
    spi_16(0);

    wr_cmd(0x2C); // send pixel
    spi_16(1);
    f_write(color);
    spi_bsy();
    _cs = 1;
    spi_16(0);
}

// optimized
// write direct to SPI1 register !
void SPI_TFT_ILI9341::window (unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    wr_cmd(0x2A);
    spi_16(1);
    f_write(x);
    f_write(x+w-1);
    spi_bsy();
    _cs = 1;
    spi_16(0);

    wr_cmd(0x2B);
    spi_16(1);
    f_write(y) ;
    f_write(y+h-1);
    spi_bsy();
    _cs = 1;
    spi_16(0);
}


void SPI_TFT_ILI9341::WindowMax (void)
{
    window (0, 0, width(),  height());
}

// optimized
// use DMA to transfer pixel data to the screen
void SPI_TFT_ILI9341::cls (void)
{
    // we can use the fillrect function 
   fillrect(0,0,width()-1,height()-1,_background);
}    

void SPI_TFT_ILI9341::circle(int x0, int y0, int r, int color)
{

    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        pixel(x0-x, y0+y,color);
        pixel(x0+x, y0+y,color);
        pixel(x0+x, y0-y,color);
        pixel(x0-x, y0-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

void SPI_TFT_ILI9341::fillcircle(int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        vline(x0-x, y0-y, y0+y, color);
        vline(x0+x, y0-y, y0+y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}


// optimized for speed
void SPI_TFT_ILI9341::hline(int x0, int x1, int y, int color)
{
    int w,j;
    w = x1 - x0 + 1;
    window(x0,y,w,1);
    _dc = 0;
    _cs = 0;
    f_write(0x2C); // send pixel
    spi_bsy();
    _dc = 1;
    spi_16(1);

    for (j=0; j<w; j++) {
        f_write(color);
    }
    spi_bsy();
    spi_16(0);
    _cs = 1;
    WindowMax();
    return;
}

// optimized for speed
void SPI_TFT_ILI9341::vline(int x, int y0, int y1, int color)
{
    int h,y;
    h = y1 - y0 + 1;
    window(x,y0,1,h);
    _dc = 0;
    _cs = 0;
    f_write(0x2C); // send pixel
    spi_bsy();
    _dc = 1;
    spi_16(1);
                // switch to 16 bit Mode 3
    for (y=0; y<h; y++) {
        f_write(color);
    }
    spi_bsy();
    spi_16(0);
    _cs = 1;
    WindowMax();
    return;
}


void SPI_TFT_ILI9341::line(int x0, int y0, int x1, int y1, int color)
{
    //WindowMax();
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    if (dx == 0) {        /* vertical line */
        if (y1 > y0) vline(x0,y0,y1,color);
        else vline(x0,y1,y0,color);
        return;
    }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    if (dy == 0) {        /* horizontal line */
        if (x1 > x0) hline(x0,x1,y0,color);
        else  hline(x1,x0,y0,color);
        return;
    }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            pixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        pixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            pixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        pixel(x0, y0, color);
    }
    return;
}


void SPI_TFT_ILI9341::rect(int x0, int y0, int x1, int y1, int color)
{

    if (x1 > x0) hline(x0,x1,y0,color);
    else  hline(x1,x0,y0,color);

    if (y1 > y0) vline(x0,y0,y1,color);
    else vline(x0,y1,y0,color);

    if (x1 > x0) hline(x0,x1,y1,color);
    else  hline(x1,x0,y1,color);

    if (y1 > y0) vline(x1,y0,y1,color);
    else vline(x1,y1,y0,color);

    return;
}



// optimized for speed
// use DMA
void SPI_TFT_ILI9341::fillrect(int x0, int y0, int x1, int y1, int color)
{

    int h = y1 - y0 + 1;
    int w = x1 - x0 + 1;
    int pixel = h * w;
    unsigned int dma_transfer;
    window(x0,y0,w,h);

    wr_cmd(0x2C);  // send pixel
    spi_16(1);
    DMA_InitStructure.DMA_MemoryBaseAddr  = (uint32_t) &color;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MemoryInc_Disable;

    switch(spi_num){        // decide which SPI is to use
    case (1):
        DMA_Init(DMA1_Channel3, &DMA_InitStructure);  // init the DMA
        do{
            if(pixel < 0x10000) {
                dma_transfer = pixel;
                pixel = 0;
            }
            else {
                dma_transfer = 0xffff;
                pixel = pixel - 0xffff;
            }
            DMA_SetCurrDataCounter(DMA1_Channel3, dma_transfer);
            SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx,ENABLE);
            DMA_Cmd(DMA1_Channel3, ENABLE);
            while(DMA_GetCurrDataCounter(DMA1_Channel3) != 0);     // wait for end of transfer
            DMA_Cmd(DMA1_Channel3, DISABLE);
        }while(pixel > 0);
        break;

    case (2):
        DMA_Init(DMA1_Channel5, &DMA_InitStructure);  // init the DMA
        do{
            if(pixel < 0x10000) {
                dma_transfer = pixel;
                pixel = 0;
            }
            else {
                dma_transfer = 0xffff;
                pixel = pixel - 0xffff;
            }
            DMA_SetCurrDataCounter(DMA1_Channel5, dma_transfer);
            SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx,ENABLE);
            DMA_Cmd(DMA1_Channel5, ENABLE);
            while(DMA_GetCurrDataCounter(DMA1_Channel5) != 0);     // wait for end of transfer
            DMA_Cmd(DMA1_Channel5, DISABLE);
        }while(pixel > 0);
        break;

    case (3):
        DMA_Init(DMA2_Channel2, &DMA_InitStructure);  // init the DMA
        do{
            if(pixel < 0x10000) {
                dma_transfer = pixel;
                pixel = 0;
            }
            else {
                dma_transfer = 0xffff;
                pixel = pixel - 0xffff;
            }
            DMA_SetCurrDataCounter(DMA2_Channel2, dma_transfer);
            SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx,ENABLE);
            DMA_Cmd(DMA2_Channel2, ENABLE);
            while(DMA_GetCurrDataCounter(DMA2_Channel2) != 0);     // wait for end of transfer
            DMA_Cmd(DMA2_Channel2, DISABLE);
        }while(pixel > 0);
        break;
    }
    spi_bsy();
    spi_16(0);
    _cs = 1;
    WindowMax();
    return;
}

void SPI_TFT_ILI9341::locate(int x, int y)
{
    char_x = x;
    char_y = y;
}

int SPI_TFT_ILI9341::columns()
{
    return width() / font[1];
}


int SPI_TFT_ILI9341::rows()
{
    return height() / font[2];
}


int SPI_TFT_ILI9341::_putc(int value)
{
    if (value == '\n') {    // new line
        char_x = 0;
        char_y = char_y + font[2];
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    } else {
        character(char_x, char_y, value);
    }
    return value;
}


// speed optimized
// will use dma
void SPI_TFT_ILI9341::character(int x, int y, int c)
{
    unsigned int hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;
    #ifdef use_ram
    unsigned int pixel;
    unsigned int p;
    unsigned int dma_count,dma_off;
    uint16_t *buffer;
    #endif

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line

    if (char_x + hor > width()) {
        char_x = 0;
        char_y = char_y + vert;
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    }
    window(char_x, char_y,hor,vert);           // setup char box
    wr_cmd(0x2C);
    spi_16(1);                                 // switch to 16 bit Mode
    #ifdef use_ram
    pixel = hor * vert;                        // calculate buffer size
    buffer = (uint16_t *) malloc (2*pixel);    // we need a buffer for the font
    if(buffer != NULL) {                       // there is memory space -> use dma
        zeichen = &font[((c -32) * offset) + 4]; // start of char bitmap
        w = zeichen[0];                          // width of actual char
        p = 0;
        // construct the font into the buffer
        for (j=0; j<vert; j++) {            //  vert line
            for (i=0; i<hor; i++) {         //  horz line
                z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
                b = 1 << (j & 0x07);
                if (( z & b ) == 0x00) {
                    buffer[p] = _background;
                } else {
                    buffer[p] = _foreground;
                }
                p++;
            }
        }
        // copy the buffer with DMA SPI to display
        dma_off = 0;  // offset for DMA transfer
        DMA_InitStructure.DMA_MemoryBaseAddr  = (uint32_t)  (buffer + dma_off);
        DMA_InitStructure.DMA_MemoryInc  = DMA_MemoryInc_Enable;

        switch(spi_num){        // decide which SPI is to use
        case (1):
        DMA_Init(DMA1_Channel3, &DMA_InitStructure);  // init the DMA
        // start DMA
         do {
            if (pixel > 0X10000) {         // this is a giant font !
                dma_count = 0Xffff;
                pixel = pixel - 0Xffff;
            } else {
                dma_count = pixel;
                pixel = 0;
            }
            DMA_SetCurrDataCounter(DMA1_Channel3, dma_count);
            SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx,ENABLE);
            DMA_Cmd(DMA1_Channel3, ENABLE);
            while(DMA_GetCurrDataCounter(DMA1_Channel3) != 0);     // wait for end of transfer
            DMA_Cmd(DMA1_Channel3, DISABLE);
        }while(pixel > 0);
        break;

        case (2):
        DMA_Init(DMA1_Channel5, &DMA_InitStructure);  // init the DMA
        // start DMA
         do {
            if (pixel > 0X10000) {         // this is a giant font !
                dma_count = 0Xffff;
                pixel = pixel - 0Xffff;
            } else {
                dma_count = pixel;
                pixel = 0;
            }
            DMA_SetCurrDataCounter(DMA1_Channel5, dma_count);
            SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx,ENABLE);
            DMA_Cmd(DMA1_Channel5, ENABLE);
            while(DMA_GetCurrDataCounter(DMA1_Channel5) != 0);     // wait for end of transfer
            DMA_Cmd(DMA1_Channel5, DISABLE);
        }while(pixel > 0);
        break;

        case (3):
        DMA_Init(DMA2_Channel2, &DMA_InitStructure);  // init the DMA
        // start DMA
         do {
            if (pixel > 0X10000) {         // this is a giant font !
                dma_count = 0Xffff;
                pixel = pixel - 0Xffff;
            } else {
                dma_count = pixel;
                pixel = 0;
            }
            DMA_SetCurrDataCounter(DMA2_Channel2, dma_count);
            SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx,ENABLE);
            DMA_Cmd(DMA2_Channel2, ENABLE);
            while(DMA_GetCurrDataCounter(DMA2_Channel2) != 0);     // wait for end of transfer
            DMA_Cmd(DMA2_Channel2, DISABLE);
        }while(pixel > 0);
        break;

        }
        spi_bsy();
        free ((uint16_t *) buffer);
        spi_16(0);
    }
    
    else{
        #endif
        zeichen = &font[((c -32) * offset) + 4]; // start of char bitmap
        w = zeichen[0];                          // width of actual char
        for (j=0; j<vert; j++) {  //  vert line
            for (i=0; i<hor; i++) {   //  horz line
                z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
                b = 1 << (j & 0x07);
                if (( z & b ) == 0x00) {
                  f_write(_background);
                } else {
                  f_write(_foreground);
                }
            }
        }
        spi_bsy();
        _cs = 1;
        spi_16(0);
    #ifdef use_ram   
    }
    #endif    
    _cs = 1;
    WindowMax();
    if ((w + 2) < hor) {                   // x offset to next char
        char_x += w + 2;
    } else char_x += hor;
}


void SPI_TFT_ILI9341::set_font(unsigned char* f)
{
    font = f;
}


void SPI_TFT_ILI9341::Bitmap(unsigned int x, unsigned int y, unsigned int w, unsigned int h,unsigned char *bitmap)
{
    unsigned int  j;
    int padd;
    unsigned short *bitmap_ptr = (unsigned short *)bitmap;

    unsigned int i;

    // the lines are padded to multiple of 4 bytes in a bitmap
    padd = -1;
    do {
        padd ++;
    } while (2*(w + padd)%4 != 0);
    window(x, y, w, h);
    bitmap_ptr += ((h - 1)* (w + padd));
    wr_cmd(0x2C);  // send pixel
    spi_16(1);
    for (j = 0; j < h; j++) {         //Lines
        for (i = 0; i < w; i++) {     // one line
                f_write(*bitmap_ptr);    // one line
                bitmap_ptr++;
        }
        bitmap_ptr -= 2*w;
        bitmap_ptr -= padd;
    }
    spi_bsy();
    _cs = 1;
    spi_16(0);
    WindowMax();
}


// local filesystem is not implemented but you can add a SD card to a different SPI

int SPI_TFT_ILI9341::BMP_16(unsigned int x, unsigned int y, const char *Name_BMP)
{

#define OffsetPixelWidth    18
#define OffsetPixelHeigh    22
#define OffsetFileSize      34
#define OffsetPixData       10
#define OffsetBPP           28

    char filename[50];
    unsigned char BMP_Header[54];
    unsigned short BPP_t;
    unsigned int PixelWidth,PixelHeigh,start_data;
    unsigned int    i,off;
    int padd,j;
    unsigned short *line;

    // get the filename
    i=0;
    while (*Name_BMP!='\0') {
        filename[i++]=*Name_BMP++;
    }
    filename[i] = 0;

    FILE *Image = fopen((const char *)&filename[0], "rb");  // open the bmp file
    if (!Image) {
        return(0);      // error file not found !
    }

    fread(&BMP_Header[0],1,54,Image);      // get the BMP Header

    if (BMP_Header[0] != 0x42 || BMP_Header[1] != 0x4D) {  // check magic byte
        fclose(Image);
        return(-1);     // error no BMP file
    }

    BPP_t = BMP_Header[OffsetBPP] + (BMP_Header[OffsetBPP + 1] << 8);
    if (BPP_t != 0x0010) {
        fclose(Image);
        return(-2);     // error no 16 bit BMP
    }

    PixelHeigh = BMP_Header[OffsetPixelHeigh] + (BMP_Header[OffsetPixelHeigh + 1] << 8) + (BMP_Header[OffsetPixelHeigh + 2] << 16) + (BMP_Header[OffsetPixelHeigh + 3] << 24);
    PixelWidth = BMP_Header[OffsetPixelWidth] + (BMP_Header[OffsetPixelWidth + 1] << 8) + (BMP_Header[OffsetPixelWidth + 2] << 16) + (BMP_Header[OffsetPixelWidth + 3] << 24);
    if (PixelHeigh > height() + y || PixelWidth > width() + x) {
        fclose(Image);
        return(-3);      // to big
    }

    start_data = BMP_Header[OffsetPixData] + (BMP_Header[OffsetPixData + 1] << 8) + (BMP_Header[OffsetPixData + 2] << 16) + (BMP_Header[OffsetPixData + 3] << 24);

    line = (unsigned short *) malloc (2 * PixelWidth); // we need a buffer for a line
    if (line == NULL) {
        return(-4);         // error no memory
    }

    // the bmp lines are padded to multiple of 4 bytes
    padd = -1;
    do {
        padd ++;
    } while ((PixelWidth * 2 + padd)%4 != 0);

    window(x, y,PixelWidth ,PixelHeigh);
    wr_cmd(0x2C);  // send pixel
    spi_16(1);
    for (j = PixelHeigh - 1; j >= 0; j--) {               //Lines bottom up
        off = j * (PixelWidth  * 2 + padd) + start_data;   // start of line
        fseek(Image, off ,SEEK_SET);
        fread(line,1,PixelWidth * 2,Image);       // read a line - slow
        for (i = 0; i < PixelWidth; i++) {        // copy pixel data to TFT
            f_write(line[i]);                  // one 16 bit pixel
        }
     }
    spi_bsy();
    _cs = 1;
    spi_16(0);
    free (line);
    fclose(Image);
    WindowMax();
    return(1);
}

#endif

