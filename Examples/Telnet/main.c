#include "CLI.h"
#include "CLI_Options.h"
#include "CLI_Sockets.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

char g_LineBuff[100];
char g_HistoryBuff[1000];

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
struct CLIHandle *g_Prompt;

int main(void)
{
    g_Prompt=CLI_GetHandle();
    if(g_Prompt==NULL)
    {
        printf("Failed to get prompt handle\n");
        return 1;
    }

    CLI_InitPrompt(g_Prompt);
    CLI_SetLineBuffer(g_Prompt,g_LineBuff,100);
    CLI_SetHistoryBuffer(g_Prompt,g_HistoryBuff,100);

    if(!InitTelnetSocket())
    {
        printf("Failed in to init telnet socket\n");
        return 1;
    }

    while(!quit)
    {
        CLI_RunCmdPrompt(g_Prompt);
        TelnetSocketTick();
    }

    ShutdownTelnetSocket();

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
