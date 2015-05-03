#ifndef _SERIAL_SHELL_H_
#define _SERIAL_SHELL_H_

#include "mbed.h"
#include "rtos.h"

#include <string>
#include <map>

#define SHELL_MAX_LINE_LENGTH       64
#define SHELL_MAX_ARGUMENTS         4

typedef void (*shellcmd_t) (Stream *, int , char **);

class Shell {
    public:
        Shell(Stream * channel);
        virtual ~Shell() {}

        void addCommand(std::string name, shellcmd_t func);
        void start(osPriority priority = osPriorityNormal,
                int stackSize = 1024, 
                unsigned char *stack_pointer=NULL);

    private:
        static void threadHelper(const void * arg);

        void shellMain();
        void shellUsage(const char *p); 
        bool shellGetLine(char *line, unsigned size);
        void listCommands();
        bool cmdExec(char * name, int argc, char *argv[]);

        Stream * _chp;
        Thread * _thread;

        // UART/Serial buffers for
        // parsing command line
        char line[SHELL_MAX_LINE_LENGTH];
        char *args[SHELL_MAX_ARGUMENTS + 1];

        // commands
        std::map<std::string, shellcmd_t> _commands; 
};

#endif
