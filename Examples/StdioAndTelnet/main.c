#include "CLI.h"
#include "CLI_Options.h"
#include "CLI_Sockets.h"
#include "CLI_Stdio.h"
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

char g_LineBuff[100];
char g_HistoryBuff[1000];
char m_PrintfBuff[1000];

int quit=false;

void quitfn(int argc,const char **argv);
void helpfn(int argc,const char **argv);
void exitfn(int argc,const char **argv);
void Drivefn(int argc,const char **argv);

const struct CLICommand g_CLICmds[]=
{
    {"Quit","Quit the program",quitfn},
    {"Help","Get help",helpfn},
    {"Exit","Exit the telnet session",exitfn},
    {"Drive","Example of the help / auto complete",Drivefn},
};

unsigned int g_CLICmdsCount=sizeof(g_CLICmds)/sizeof(struct CLICommand);
struct CLIHandle *g_Prompt;
bool m_UseTelnet;

int cprintf(const char *fmt,...);

int main(void)
{
    m_UseTelnet=false;

    g_Prompt=CLI_GetHandle();
    if(g_Prompt==NULL)
    {
        printf("Failed to get prompt handle\n");
        return 1;
    }

    CLI_InitPrompt(g_Prompt);
    CLI_SetLineBuffer(g_Prompt,g_LineBuff,100);
    CLI_SetHistoryBuffer(g_Prompt,g_HistoryBuff,100);

    SetupIO();
    if(!InitTelnetSocket())
    {
        printf("Failed in to init telnet socket\n");
        return 1;
    }

    CLI_DrawPrompt(g_Prompt);
    while(!quit)
    {
        CLI_RunCmdPrompt(g_Prompt);
        TelnetSocketTick();
    }

    ShutdownTelnetSocket();
    ShutDownIO();

    return 0;
}

void quitfn(int argc,const char **argv)
{
    quit=true;
}

void helpfn(int argc,const char **argv)
{
    CLI_DisplayHelp();
}

void exitfn(int argc,const char **argv)
{
    TelnetSocket_CloseConnection();
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
        cprintf("Floppy disk 1\r\n");
    }
    else if(strcasecmp(argv[1],"df1")==0)
    {
        cprintf("Floppy disk 1\r\n");
    }
    else if(strcasecmp(argv[1],"dh0")==0)
    {
        cprintf("Harddrive\n");
    }
    else
    {
        cprintf("unknown disk\r\n");
        return;
    }

    if(strcasecmp(argv[2],"Read")==0)
    {
        if(argc<5)
        {
            cprintf("Missing args\r\n");
            return;
        }
        bytes=atoi(argv[4]);
        offset=atoi(argv[3]);

        cprintf("Reading %d bytes at offset %d\r\n",bytes,offset);
    }
    else if(strcasecmp(argv[2],"Write")==0)
    {
        if(argc<5)
        {
            cprintf("Missing args\r\n");
            return;
        }
        bytes=atoi(argv[4]);
        offset=atoi(argv[3]);

        cprintf("Writing %d bytes at offset %d\r\n",bytes,offset);
    }
    else if(strcasecmp(argv[2],"Format")==0)
    {
        cprintf("Formating...\r\n");
    }
    else
    {
        cprintf("unknown function\r\n");
        return;
    }
}

uint32_t HAL_CLI_GetMilliSecCounter(void)
{
    struct timeval start;

    gettimeofday(&start, NULL);

    return ((start.tv_sec) * 1000 + start.tv_usec/1000.0) + 0.5;
}

int cprintf(const char *fmt,...)
{
    va_list args;
    int RetValue;
    int r;

    va_start(args,fmt);

    RetValue=vsnprintf(m_PrintfBuff,sizeof(m_PrintfBuff),fmt,args);

    va_end(args);

    for(r=0;r<RetValue;r++)
        HAL_CLI_PutChar(m_PrintfBuff[r]);

    return RetValue;
}

void Switch2Telnet(void)
{
    m_UseTelnet=true;
}

void Switch2Stdio(void)
{
    m_UseTelnet=false;
}

bool HAL_CLI_IsCharAvailable(void)
{
    if(m_UseTelnet)
        return TelnetSocket_IsCharAvailable();
    else
        return Stdio_IsCharAvailable();
}

char HAL_CLI_GetChar(void)
{
    if(m_UseTelnet)
        return TelnetSocket_GetChar();
    else
        return Stdio_GetChar();
}

void HAL_CLI_PutChar(char c)
{
    if(m_UseTelnet)
        TelnetSocket_PutChar(c);
    else
        Stdio_PutChar(c);
}

