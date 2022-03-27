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
void Drivefn(int argc,const char **argv);

const struct CLICommand g_CLICmds[]=
{
    {"Quit","Quit the program",quitfn},
    {"Help","Get help",helpfn},
    {"Drive","Example of the help / auto complete",Drivefn},
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

void Drivefn(int argc,const char **argv)
{
    int bytes;
    int offset;

    if(argc==0)
    {
        CLI_CmdHelp_Start();

        CLI_CmdHelp_Arg("Disk","What disk to work on");
        CLI_CmdHelp_OptionString(0,"df0","Floppy disk 1");
        CLI_CmdHelp_OptionString(0,"df1","Floppy disk 2");
        CLI_CmdHelp_OptionString(0,"dh0","Hard drive 1");

        CLI_CmdHelp_Arg("Function","Preform disk functions");
        // Read fn
        CLI_CmdHelp_OptionString(1,"Read","Read from the disk");
        CLI_CmdHelp_SubArg("Offset","The offset into the disk to read");
        CLI_CmdHelp_SubArg("Bytes","The number of bytes to read");
        // Write fn
        CLI_CmdHelp_OptionString(1,"Write","Write to the disk");
        CLI_CmdHelp_SubArg("Offset","The number of bytes to write");
        CLI_CmdHelp_SubArg("Bytes","The number of bytes to write");
        // Format fn
        CLI_CmdHelp_OptionString(1,"Format","Format the disk");

        CLI_CmdHelp_End();
        return;
    }

    if(argc<3)
    {
        CLI_ShowCmdHelp();
        return;
    }

    if(strcasecmp(argv[1],"df0")==0)
    {
        printf("Floppy disk 1\r\n");
    }
    else if(strcasecmp(argv[1],"df1")==0)
    {
        printf("Floppy disk 1\r\n");
    }
    else if(strcasecmp(argv[1],"dh0")==0)
    {
        printf("Harddrive\n");
    }
    else
    {
        printf("unknown disk\r\n");
        return;
    }

    if(strcasecmp(argv[2],"Read")==0)
    {
        if(argc<5)
        {
            printf("Missing args\r\n");
            return;
        }
        bytes=atoi(argv[4]);
        offset=atoi(argv[3]);

        printf("Reading %d bytes at offset %d\r\n",bytes,offset);
    }
    else if(strcasecmp(argv[2],"Write")==0)
    {
        if(argc<5)
        {
            printf("Missing args\r\n");
            return;
        }
        bytes=atoi(argv[4]);
        offset=atoi(argv[3]);

        printf("Writing %d bytes at offset %d\r\n",bytes,offset);
    }
    else if(strcasecmp(argv[2],"Format")==0)
    {
        printf("Formating...\r\n");
    }
    else
    {
        printf("unknown function\r\n");
        return;
    }
}
