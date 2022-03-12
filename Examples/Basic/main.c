#include "Cli.h"
#include "CLI_HAL.h"
#include "CLI_Commands.h"
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define USECMDS 1

struct termios orig_termios;

char g_LineBuff[100];
char g_HistoryBuff[1000];

void SetupIO(void);
void ShutDownIO(void);

int quit=false;

void quitfn(int argc,const char **argv);
void helpfn(int argc,const char **argv);

#ifdef USECMDS
const struct CLICommand g_CLICmds[]=
{
    {"quit","Quit the program",quitfn},
    {"help","Get help",helpfn}
};

int g_CLICmdsCount=sizeof(g_CLICmds)/sizeof(struct CLICommand);

#endif

int main(void)
{
    char *Line;

    SetupIO();

    CLI_SetLineBuffer(g_LineBuff,100);
    CLI_SetHistoryBuffer(g_HistoryBuff,100);
    CLI_InitPrompt();

#ifdef USECMDS
    CLI_Init();

    CLI_DrawPrompt();
    while(!quit)
    {
        CLI_PollCmdPrompt();
    }

#else
    for(;;)
    {
        Line=CLI_GetLine();
        if(Line!=NULL)
        {
            printf("GOT:%s\r\n",Line);

            /* We got a line, see if it's a command or login/password */
            if(strcmp(Line,"quit")==0)
                break;

            /* We are done with the buffer, reset for the next input */
            CLI_ResetInputBuffer();
        }
    }
#endif

    ShutDownIO();

    return 0;
}

void SetupIO(void)
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(ShutDownIO);

    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

void ShutDownIO(void)
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

bool HAL_CLI_IsCharAvailable(void)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

char HAL_CLI_GetChar(void)
{
    int r;
    unsigned char c;

    if ((r = read(0, &c, sizeof(c))) < 0)
    {
        return 0;
    }
    else
    {
        return c;
    }
}

void HAL_CLI_PutChar(char c)
{
    putchar(c);
    fflush(stdout);
}

uint32_t HAL_CLI_GetMilliSecCounter(void)
{
    struct timeval start;

    gettimeofday(&start, NULL);

    return ((start.tv_sec) * 1000 + start.tv_usec/1000.0) + 0.5;
}

void quitfn(int argc,const char **argv)
{
    quit=true;
}

void helpfn(int argc,const char **argv)
{
    CLI_DisplayHelp();
}