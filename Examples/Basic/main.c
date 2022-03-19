#include "CLI.h"
#include "CLI_HAL.h"
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

struct termios orig_termios;

char g_LineBuff[100];
char g_HistoryBuff[1000];

void SetupIO(void);
void ShutDownIO(void);

int quit=false;

void quitfn(int argc,const char **argv);
void helpfn(int argc,const char **argv);

const struct CLICommand g_CLICmds[]=
{
    {"Quit","Quit the program",quitfn},
    {"Help","Get help",helpfn}
};

unsigned int g_CLICmdsCount=sizeof(g_CLICmds)/sizeof(struct CLICommand);

int main(void)
{
    struct CLIHandle *Prompt;

    SetupIO();

    Prompt=CLI_GetHandle();
    if(Prompt==NULL)
        printf("Failed to get prompt handle\n");

    CLI_InitPrompt(Prompt);
    CLI_SetLineBuffer(Prompt,g_LineBuff,100);
    CLI_SetHistoryBuffer(Prompt,g_HistoryBuff,100);

    CLI_DrawPrompt(Prompt);
    while(!quit)
    {
        CLI_RunCmdPrompt(Prompt);
    }

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
