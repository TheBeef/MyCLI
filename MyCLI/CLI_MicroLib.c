/*******************************************************************************
 * FILENAME: CLI_MicroLib.c
 *
 * PROJECT:
 *    CLI
 *
 * FILE DESCRIPTION:
 *    This has the micro version of the command line and command processor.
 *
 * COPYRIGHT:
 *    Copyright 2012 6464033 Canada Inc.
 *
 *    This software is the property of 6464033 Canada Inc. and may not be
 *    reused in any manner except under express written permission of
 *    6464033 Canada Inc.
 *
 * CREATED BY:
 *    Paul Hutchinson (02 Jul 2012)
 *
 ******************************************************************************/

/*** HEADER FILES TO INCLUDE  ***/

/*** DEFINES                  ***/
#define MAX_ARGS 10 // The number of args that we can support
#define CLIMAXLINE 40
#define HAL_CLI_PutChar(x) putch(x)
#define HAL_CLI_GetChar() getch()
#define HAL_CLI_IsCharAvail() chavail()

#define CLI_CMD_COUNT (sizeof(m_CLICmds)/sizeof(struct CLICommand))

/*** MACROS                   ***/

/*** TYPE DEFINITIONS         ***/
struct CLICommand
{
    const char *Cmd;
    const char *Help;
    void (*Exec)(int argc,const char **argv);
};

/*** VARIABLE DEFINITIONS     ***/
static char m_CLILineBuff[CLIMAXLINE];
static char *m_LineBuffInsertPos;

void Test1(int argc,const char **argv);
const struct CLICommand m_CLICmds[]=
{
    {"t1","Testing",Test1},
};

/*** FUNCTION PROTOTYPES      ***/
static void CL_Help(void);
static void CL_PrintStr(const char *str);
static bool CLI_CmdPrompt(void);
static void CLI_InitCmdPrompt(void);

static void CLI_InitCmdPrompt(void)
{
    m_LineBuffInsertPos=m_CLILineBuff;
    CL_PrintStr(">");
}

static bool CLI_CmdPrompt(void)
{
    uint8_t c;  // The command we are looking at
    const struct CLICommand *cmdptr;
    int len;    // The length of the string we are fidding with
    char *Argv[MAX_ARGS];     // the argv array
    uint8_t count;     // The arg count

    if(HAL_CLI_IsCharAvail())
    {
        c=HAL_CLI_GetChar();

        switch(c)
        {
            case 0: // We ignore 0's
            break;
            case 10:    // Kill of new lines
            break;
            case '\r':
                /* We are done */
                CL_PrintStr("\r\n");

                if(m_LineBuffInsertPos!=m_CLILineBuff)
                {
                    *m_LineBuffInsertPos++=0;
                    /* We got a line, see if it's valid command (built in first) */
                    if(strncmp(m_CLILineBuff,"help",4)==0)
                    {
                        CL_Help();
                    }
                    else
                    {
                        for(c=0;c<CLI_CMD_COUNT;c++)
                        {
                            cmdptr=&m_CLICmds[c];
                            len=strlen(cmdptr->Cmd);

                            if(strncmp(m_CLILineBuff,cmdptr->Cmd,len)==0 &&
                                    (m_CLILineBuff[len]==0 ||
                                    m_CLILineBuff[len]==' '))
                            {
                                /* Found a command */
                                /* Split up the command line */
                                count=0;
                                Argv[count++]=m_CLILineBuff; // The name of the command

                                m_LineBuffInsertPos=m_CLILineBuff;
                                while(*m_LineBuffInsertPos!=0)
                                {
                                    if(*m_LineBuffInsertPos==' ')
                                    {
                                        *m_LineBuffInsertPos=0;
                                        Argv[count++]=m_LineBuffInsertPos+1;
                                        if(count>MAX_ARGS)
                                        {
                                            CL_PrintStr("!arg\r\n");
                                            break;
                                        }
                                    }
                                    m_LineBuffInsertPos++;
                                }

                                cmdptr->Exec(count,(const char **)Argv);
                                break;
                            }
                        }
                        if(c==CLI_CMD_COUNT)
                            CL_PrintStr("!Cmd\r\n\r\n");
                    }
                }

                CLI_InitCmdPrompt();
            break;
            case '\b':
            case 127:
                if(m_LineBuffInsertPos!=m_CLILineBuff)
                {
                    m_LineBuffInsertPos--;
                    *m_LineBuffInsertPos=0;

                    CL_PrintStr("\b \b");
                }
            break;
            case 27:    /* ANSI codes */
                while(m_LineBuffInsertPos-- > m_CLILineBuff)
                    CL_PrintStr("\b \b");
                m_LineBuffInsertPos=m_CLILineBuff;
                *m_CLILineBuff=0;
            break;
            default:
                if(m_LineBuffInsertPos<&m_CLILineBuff[CLIMAXLINE-1] &&
                        c>31 && c<127)
                {
                    *m_LineBuffInsertPos++=c;
                    HAL_CLI_PutChar(c);
                }
            break;
        }
    }
    return false;
}

static void CL_PrintStr(const char *str)
{
    while(*str!=0)
    {
        HAL_CLI_PutChar(*str);
        str++;
    }
}

static void CL_Help(void)
{
    unsigned int MaxWidth;  // The max width of the cmd strings
    unsigned int l; // The length of the current string
    unsigned int cmd;   // The command we are on
    unsigned int cmd;   // The command we are on
    unsigned int l; // The length of the current string
    const struct CLICommand *cmdptr;

    MaxWidth=9;
    for(cmd=0;cmd<CLI_CMD_COUNT;cmd++)
    {
        l=strlen(m_CLICmds[cmd].Cmd);
        if(l>MaxWidth)
            MaxWidth=l;
    }

    MaxWidth++; // At least 1 space between cmd's and help.

    /* Ok, now output the help */
    for(cmd=0;cmd<CLI_CMD_COUNT;cmd++)
    {
        cmdptr=&m_CLICmds[cmd];
        l=strlen(cmdptr->Cmd);
        CL_PrintStr(cmdptr->Cmd);
        for(;l<MaxWidth;l++)
            HAL_CLI_PutChar(' ');

        CL_PrintStr(cmdptr->Help);
        HAL_CLI_PutChar('\r');
        HAL_CLI_PutChar('\n');
    }
}

void Test1(int argc,const char **argv)
{
}
