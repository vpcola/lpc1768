#include "mbed.h"
#include "rtos.h"
#include "EthernetInterface.h"
#include "Arial12x12.h"
#include "Arial24x23.h"
#include "SPI_TFT_ILI9341.h"
#include "Shell.h"
#include "HTU21D.h"
//#include "USBHostMSD.h"

Serial pc(p28, p27); // (USBTX, USBRX);
DigitalOut myled(LED1);
EthernetInterface eth;

Mutex i2cMutex;
I2C i2c(p9 , p10);
HTU21D htu21d(p9,p10,i2cMutex);

#define IO_EXT_ADDR (0x21 << 1)

LocalFileSystem lcl("local"); // mosi, miso, sck, cs 
SPI_TFT_ILI9341 TFT(p11,p12,p13,p15, p16, p17 ); // mosi, miso, sck, cs, reset, dc
DigitalOut lcdOn(p14);

#define SHELL_STACK_SIZ 1024
// Pre-allocate the shell's stack (on global mem)
unsigned char shellStack[SHELL_STACK_SIZ];
Shell shell(&pc);

static uint32_t get_mem()
{
   // In order to get free mem within RTOS
   // we need to get the main thread's stack pointer
   // and subtract it with the top of the heap
   // ------+-------------------+   Last Address of RAM (INITIAL_SP)
   //       | Scheduler Stack   |
   //       +-------------------+
   //       | Main Thread Stack |
   //       |         |         |
   //       |         v         |
   //       +-------------------+ <- bottom_of_stack/__get_MSP()
   // RAM   |                   | 
   //       |  Available RAM    |  
   //       |                   |  
   //       +-------------------+ <- top_of_heap
   //       |         ^         |
   //       |         |         |
   //       |       Heap        |
   //       +-------------------+ <- __end__ / HEAP_START (linker defined var)
   //       | ZI                |
   //       +-------------------+
   //       | ZI: Shell Stack   |
   //       +-------------------+
   //       | ZI: Idle Stack    |
   //       +-------------------+
   //       | ZI: Timer Stack   |
   //       +-------------------+
   //       | RW                |  
   // ------+===================+  First Address of RAM
   //       |                   |
   // Flash |                   |
   //

   uint32_t bottom_of_stack = __get_MSP();
   char     * top_of_heap =  (char *) malloc(sizeof(char));
   uint32_t diff = bottom_of_stack - (uint32_t) top_of_heap;

   free((void *) top_of_heap);
   
   return diff;    
}

// Local Commands
/**
 *  \brief Gets the amount of free memory
 *  \param none
 *  \return none
 **/
static void cmd_mem(Stream * chp, int argc, char * argv[])
{
   chp->printf("Available Memory : %d bytes\r\n",
        get_mem());
}

/**
 *  \brief List Directories and files 
 *  \param none
 *  \return int
 **/
static void cmd_ls(Stream * chp, int argc, char * argv[])
{
   DIR * dp;
   struct dirent * dirp;
   char dirroot[256];
   
   if (argc >= 1)
       sprintf(dirroot, "/local/%s", argv[0]);
   else
       sprintf(dirroot, "/local");
   
   chp->printf("Listing directory [%s]\r\n", dirroot);
   
   dp = opendir(dirroot);           
   while((dirp = readdir(dp)) != NULL)
   {
       chp->printf("\t%s\r\n", dirp->d_name);
   }
   closedir(dp);
}

static void cmd_load(Stream * chp, int argc, char * argv[])
{
   char filename[256];
   
   if (argc != 1)
   {
       chp->printf("load <bitmapfile>\r\n");
       return;
   }
   
   sprintf(filename, "/sd/%s", argv[0]);
       // Load a bitmap startup file
   int err = TFT.BMP_16(0,0, filename);
   if (err != 1) TFT.printf(" - Err: %d", err); 
}

/**
 *  \brief Gets sensor data on HTU21D
 *  \param none
 *  \return int
 **/
static void cmd_sensor(Stream * chp, int argc, char * argv[])
{
    chp->printf("Temperature : %d Â°C\r\n", htu21d.sample_ctemp());
    chp->printf("Humitdity : %d%%\r\n", htu21d.sample_humid());
}

/**
 * \brief Initialize LCD
 * \param none
 * \return void
 **/
void init_LCD()
{
    pc.printf("Initializing LCD Screen ...\r\n");

    // Turn on the LCD
    lcdOn = 1;

    TFT.claim(stdout);  
    TFT.set_orientation(2);
    TFT.background(Black);    // set background to black
    TFT.foreground(White);    // set chars to white
    TFT.cls();                // clear the screen


    TFT.set_font((unsigned char*) Arial12x12);
    TFT.locate(0,0);

    printf("Hello World of EMBED!\n");

}

void led1_thread(void const *args)
{
    char data[2];
    int count = 0;

    // Initialize the io extension on i2c bus
    data[0] = 0x00;
    data[1] = 0x00;
    i2cMutex.lock();
    i2c.write(IO_EXT_ADDR, (const char *) data, 2);
    i2cMutex.unlock();

    while (true) {
        myled = !myled;
        data[0] = 0x14;
        data[1] = count;
        i2cMutex.lock();
        i2c.write(IO_EXT_ADDR, (const char *) data, 2);
        i2cMutex.unlock();
        count++;
        Thread::wait(1000);
    }
}

int main() {
    Thread * thread;


    pc.baud(115200);

    pc.printf("\r\nStarting Mbed ...\r\n");
    //Initialize the LCD
    pc.printf("Initializing LCD ...\r\n");
    init_LCD();
    printf("Initializing USB Mass Storage ...\r\n");

    
    printf("Inititalizing ethernet ....\r\n");
    eth.init(); // Use DHCP
    eth.connect();
    printf("IP Address is %s\n", eth.getIPAddress());
    
    // After initializing the ethernet interface
    // run it in its own thread
    printf("Starting blinker thread ...\r\n");
    thread = new Thread(led1_thread);

    // Start the shell
    printf("Starting debug shell ...\r\n");
    shell.addCommand("ls", cmd_ls);
    shell.addCommand("load", cmd_load);
    shell.addCommand("mem", cmd_mem);
    shell.addCommand("sensor", cmd_sensor);
    shell.start(osPriorityNormal, SHELL_STACK_SIZ, shellStack);
    printf("Shell now running!\r\n");
    printf("Available Memory : %d\r\n", get_mem());
        
    // Do something logical here
    // other than looping
    while(1) {
        printf("Temperature : %d °C\r\n", htu21d.sample_ctemp());
        printf("Humitdity : %d%%\r\n", htu21d.sample_humid());
        wait(10);
    }
    
    thread->terminate();
    delete thread;
}
