#include "mbed.h"
#include "cmsis_os.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
// cpp headers
#include <string>
#include <cctype>



#include "Shell.h"


static char *_strtok(char *str, const char *delim, char **saveptr) 
{
  char *token;
  if (str)
    *saveptr = str;
  token = *saveptr;

  if (!token)
    return NULL;

  token += strspn(token, delim);
  *saveptr = strpbrk(token, delim);
  if (*saveptr)
    *(*saveptr)++ = '\0';

  return *token ? token : NULL;
}


Shell::Shell(Stream * channel)
    :_chp(channel)
{
    _commands.clear();
}

void Shell::addCommand(std::string name, shellcmd_t func)
{
    _commands.insert(std::pair<std::string, shellcmd_t>(name, func));
}

void Shell::start(osPriority priority,
                int stackSize,
                unsigned char *stack_pointer)
{
    _thread = new Thread(Shell::threadHelper, this, priority, stackSize, stack_pointer);
}

void Shell::threadHelper(const void * arg)
{
    Shell * pinstance = static_cast<Shell *>(const_cast<void *>(arg));

    pinstance->shellMain();
}

void Shell::shellUsage(const char *p) 
{
     _chp->printf("Usage: %s\r\n", p);
}

void Shell::listCommands()
{
    std::map<std::string, shellcmd_t>::iterator it;
    for (it = _commands.begin(); it != _commands.end(); it++)
        _chp->printf("%s ", (*it).first.c_str());
}

bool Shell::cmdExec(char * name, int argc, char *argv[])
{
    std::map<std::string, shellcmd_t>::iterator it;
    it = _commands.find(std::string(name));
    if (it != _commands.end())
    {
        it->second(_chp, argc, argv);
        return true;
    }

    return false;
}


void Shell::shellMain() 
{
  int n;
  char *lp, *cmd, *tokp;

  _chp->printf("\r\nEmbed/RX Shell\r\n");
  while (true) {
    _chp->printf(">> ");
    if (shellGetLine(line, sizeof(line))) {
      _chp->printf("\r\nlogout");
      break;
    }
    // Get the command
    lp = _strtok(line, " \t", &tokp);
    cmd = lp;

    // Get the arguments
    n = 0;
    while ((lp = _strtok(NULL, " \t", &tokp)) != NULL) {
      if (n >= SHELL_MAX_ARGUMENTS) {
        _chp->printf("too many arguments\r\n");
        cmd = NULL;
        break;
      }
      args[n++] = lp;
    }
    args[n] = NULL;

    // Determine the command
    if (cmd != NULL) 
    {
      if (strcasecmp(cmd, "exit") == 0)     // If "exit"
      {
        if (n > 0) {
          shellUsage("exit");
          continue;
        }
                // Break here breaks the outer loop
                // hence, we exit the shell.
        break;
      }
      else if (strcasecmp(cmd, "help") == 0) // If "help"
      {
        if (n > 0) {
          shellUsage("help");
          continue;
        }
        _chp->printf("Commands: help exit ");
        listCommands();
        _chp->printf("\r\n");
      }
      else if (!cmdExec(cmd, n, args))       // Finally call exec on the command
      {
        // If the command is unknown
        _chp->printf("%s", cmd);
        _chp->printf(" ?\r\n");
      }
    } // cmd != NULL
  }
}

bool Shell::shellGetLine(char *line, unsigned size) 
{
  char *p = line;

  while (true) {
    char c;

    if ((c = _chp->getc()) == 0)
      return true;
    if (c == 4) {
      _chp->printf("^D");
      return true;
    }
    if (c == 8) {
      if (p != line) {
        _chp->putc(c);
        _chp->putc(0x20);
        _chp->putc(c);
        p--;
      }
      continue;
    }
    if (c == '\r') {
      _chp->printf("\r\n");
      *p = 0;
      return false;
    }
    if (c < 0x20)
      continue;
    if (p < line + size - 1) {
      _chp->putc(c);
      *p++ = (char)c;
    }
  }
}
