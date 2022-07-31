/*******************************************************************************
 * FILENAME: CLI.c
 *
 * PROJECT:
 *    MyCLI
 *
 * FILE DESCRIPTION:
 *    This file has the smallest version of the CLI and prompt in it.  It
 *    is very basic.
 *
 *    The micro version removes support for:
 *      - Telnet
 *      - Auto complete
 *      - All but basic line editing
 *      - History
 *      - Help system (see CLI_REMOVE_CMDHELP to remove help from your code too)
 *      - Custom prompts
 *      - Password mode
 *      - Multiple prompts
 *
 *    You should also look at CLI_REMOVE_CMDHELP to remove the help strings.
 *
 *    This also uses 'CLI_MICRO_BUFFSIZE' from options for the line edit buffer.
 *
 * COPYRIGHT:
 *    Copyright 2010 Paul Hutchinson
 *
 *    Permission is hereby granted, free of charge, to any person obtaining
 *    a copy of this software and associated documentation files
 *    (the "Software"), to deal in the Software without restriction,
 *    including without limitation the rights to use, copy, modify,
 *    merge, publish, distribute, sublicense, and/or sell copies of the
 *    Software, and to permit persons to whom the Software is furnished
 *    to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included
 *    in all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * CREATED BY:
 *    Paul Hutchinson (27 Mar 2022)
 *
 ******************************************************************************/

/*** HEADER FILES TO INCLUDE  ***/
#include "../CLI.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>

/*** DEFINES                  ***/
#define HELP_INDENT                                     4

/*** MACROS                   ***/

/*** TYPE DEFINITIONS         ***/

/*** FUNCTION PROTOTYPES      ***/
static void CLIPrintStr(const char *Str);

/*** VARIABLE DEFINITIONS     ***/
static char m_CLI_LineBuff[CLI_MICRO_BUFFSIZE];

/*******************************************************************************
 * NAME:
 *    CLI_GetHandle
 *
 * SYNOPSIS:
 *    struct CLIHandle *CLI_GetHandle(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function does nothing but return success.
 *
 * RETURNS:
 *    A fake handle.
 *
 * LIMITATIONS:
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
struct CLIHandle *CLI_GetHandle(void)
{
    return (struct CLIHandle *)-1;
}

/*******************************************************************************
 * NAME:
 *    CLI_InitPrompt
 *
 * SYNOPSIS:
 *    void CLI_InitPrompt(struct CLIHandle *Handle);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the command prompt.
 *
 * FUNCTION:
 *    This function init's command prompt.  This needs to be called before
 *    you can use the prompt.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_InitPrompt(struct CLIHandle *Handle)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_GetLine
 *
 * SYNOPSIS:
 *    char *CLI_GetLine(struct CLIHandle *Handle);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the command prompt.
 *
 * FUNCTION:
 *    This function gets an input line from the user.  It is non-blocking.
 *
 * RETURNS:
 *    A pointer to the line buffer or NULL if the line is not ready yet.
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
char *CLI_GetLine(struct CLIHandle *Handle)
{
    unsigned char c;

    if(!CLI_IS_CHAR_AVAILABLE())
        return NULL;

    c=CLI_GETCHAR();
    switch(c)
    {
        case 0:     // We ignore 0's
        break;
        case 9:     /* Tab */
        break;
        case 10:    // No new lines please
        break;
        case 27:    /* ANSI codes */
        break;
        case '\r':
            /* We are done */
            CLI_PUTCHAR('\n');
            CLI_PUTCHAR('\r');
            return m_CLI_LineBuff;
        break;
        case '\b':
        case 127:
            if(m_CLI_LineBuff[0]!=0)
            {
                m_CLI_LineBuff[STRLEN(m_CLI_LineBuff)-1]=0;
                CLI_PUTCHAR('\b');
                CLI_PUTCHAR(' ');
                CLI_PUTCHAR('\b');
            }
        break;
        default:
            if(STRLEN(m_CLI_LineBuff)<sizeof(m_CLI_LineBuff)-1)
            {
                m_CLI_LineBuff[STRLEN(m_CLI_LineBuff)+1]=0;
                m_CLI_LineBuff[STRLEN(m_CLI_LineBuff)]=c;
                CLI_PUTCHAR(c);
            }
        break;
        case 255:   // Telnet command
        break;
    }
    return NULL;
}

/*******************************************************************************
 * NAME:
 *    CLI_SetLineBuffer
 *
 * SYNOPSIS:
 *    void CLI_SetLineBuffer(struct CLIHandle *Handle,char *LineBuff,
 *              int MaxSize);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the command prompt.
 *    LineBuff [I] -- A static buffer that the cmd prompt will use to store
 *                    the line as the user types it.
 *    MaxSize [I] -- The max size of 'LineBuff'.
 *
 * FUNCTION:
 *    This function sets the line buffer to use for user input.  This is
 *    done this way so the main app can control how much memory is used for
 *    the command prompt.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    CLI_Init()
 ******************************************************************************/
void CLI_SetLineBuffer(struct CLIHandle *Handle,char *LineBuff,int MaxSize)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_SetHistoryBuffer
 *
 * SYNOPSIS:
 *    void CLI_SetHistoryBuffer(struct CLIHandle *Handle,char *HistoryBuff,
 *              int MaxSize);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the command prompt.
 *    HistoryBuff [I] -- The buffer to store the history.  NULL for none.
 *    MaxSize [I] -- The number of bytes in the buffer.
 *
 * FUNCTION:
 *    This function sets the buffer that will be used for the history buffer.
 *    The history buffer is used with the up/down arrows to bring back
 *    previously input lines.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_SetHistoryBuffer(struct CLIHandle *Handle,char *HistoryBuff,
        int MaxSize)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_SetPasswordMode
 *
 * SYNOPSIS:
 *    void CLI_SetPasswordMode(struct CLIHandle *Handle,bool OnOff);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the command prompt.
 *    OnOff [I] -- true = no echoing of password chars, false = normal mode
 *
 * FUNCTION:
 *    This function turns on/off password mode
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_SetPasswordMode(struct CLIHandle *Handle,bool OnOff)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_SetPromptStr
 *
 * SYNOPSIS:
 *    void CLI_SetPromptStr(struct CLIHandle *Handle,const char *Prompt);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the command prompt.
 *    Prompt [I] -- The new prompt to use
 *
 * FUNCTION:
 *    This function changes the command prompt string.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_SetPromptStr(struct CLIHandle *Handle,const char *Prompt)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_RunCmdPrompt
 *
 * SYNOPSIS:
 *    void CLI_RunCmdPrompt(struct CLIHandle *Handle);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the prompt to work on
 *
 * FUNCTION:
 *    This function process keys from the user and figures out the command to
 *    run.  You must call this regularly for it to work.
 *
 *    This is non-blocking.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_RunCmdPrompt(struct CLIHandle *Handle)
{
    char *Line; // The line we got from the input

    Line=CLI_GetLine(NULL);
    if(Line!=NULL)
    {
        if(!CLI_RunLine(Handle,Line))
            CLIPrintStr("Bad CMD\r\n\r\n");

        /* We are done with the buffer, reset for the next input */
        *m_CLI_LineBuff=0;

        CLI_DrawPrompt(NULL);
    }
}

/*******************************************************************************
 * NAME:
 *    CLI_RunLine
 *
 * SYNOPSIS:
 *    bool CLI_RunLine(struct CLIHandle *Handle,char *Line);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the prompt to work on
 *    Line [I] -- The raw line to process.  This will be overwritten.
 *
 * FUNCTION:
 *    This function takes a raw line and runs it.
 *
 * RETURNS:
 *    true -- Command was found.
 *    false -- The command is unknown.
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
bool CLI_RunLine(struct CLIHandle *Handle,char *Line)
{
    unsigned int cmd;    // The command index we are looking at
    int len;    // The len of the current command we are looking at
    uint16_t Argc;          // The number of args
    char *Argv[CLI_MAX_ARGS];   // The argv's we are sending
    char *Pos;  // The current pos in the line we are processing
    char *StartOfLastArg;   // The pos of the last arg

    /* Empty lines do not make errors */
    if(*Line==0)
        return true;

    /* We got a line, scan the commands */
    for(cmd=0;cmd<g_CLICmdsCount;cmd++)
    {
        len=STRLEN(g_CLICmds[cmd].Cmd);
        if(STRNCMP(Line,g_CLICmds[cmd].Cmd,len)==0 &&
                (Line[len]==0 || Line[len]==' '))
        {
            /* Found a command, run it */
            /* Split up the command line */
            Argc=0;
            Pos=Line;
            StartOfLastArg=Line;
            while(*Pos!=0)
            {
                if(*Pos==' ')
                {
                    *Pos=0;
                    Argv[Argc++]=StartOfLastArg;
                    if(Argc>CLI_MAX_ARGS)
                        return false;

                    Pos++;
                    while(*Pos==' ')
                        Pos++;
                    StartOfLastArg=Pos;
                }
                else
                {
                    Pos++;
                }
            }

            /* Add in the last arg */
            Argv[Argc++]=StartOfLastArg;

            g_CLICmds[cmd].Exec(Argc,(const char **)Argv);

            break;
        }
    }

    if(cmd==g_CLICmdsCount)
        return false;
    return true;
}

/*******************************************************************************
 * NAME:
 *    CLIPrintStr
 *
 * SYNOPSIS:
 *    static void CLIPrintStr(const char *Str);
 *
 * PARAMETERS:
 *    Str [I] -- The string to print
 *
 * FUNCTION:
 *    This function is a helper function that prints a string using the HAL.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
static void CLIPrintStr(const char *Str)
{
    while(*Str!=0)
    {
        CLI_PUTCHAR(*Str);
        Str++;
    }
}

/*******************************************************************************
 * NAME:
 *    CLI_DisplayHelp
 *
 * SYNOPSIS:
 *    void CLI_DisplayHelp(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function outputs the help for the commands that are in 'g_CLICmds'.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_DisplayHelp(void)
{
    unsigned int cmd;       // The command we are on

    for(cmd=0;cmd<g_CLICmdsCount;cmd++)
    {
        CLIPrintStr(g_CLICmds[cmd].Cmd);
        CLIPrintStr(" -- ");
        CLIPrintStr(g_CLICmds[cmd].Help);
        CLIPrintStr("\r\n");
    }
}

/*******************************************************************************
 * NAME:
 *    CLI_DrawPrompt
 *
 * SYNOPSIS:
 *    void CLI_DrawPrompt(struct CLIHandle *Handle);
 *
 * PARAMETERS:
 *    Handle [I] -- The handle to the prompt to work on
 *    NONE
 *
 * FUNCTION:
 *    This function prints the prompt.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_DrawPrompt(struct CLIHandle *Handle)
{
    CLI_PUTCHAR('>');
}

/*******************************************************************************
 * NAME:
 *    CLI_SendTelnetInitConnectionMsg
 *
 * SYNOPSIS:
 *    void CLI_SendTelnetInitConnectionMsg(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function prints a init seq of Telnet commands.  This is not
 *    supported in this version of the prompt but is included so you can
 *    just link in.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_SendTelnetInitConnectionMsg(struct CLIHandle *Handle)
{
}

/* Don't really like #ifdef's but for it to work with the CLI_Options.h
   we need to remove them */
#ifndef CLI_REMOVE_CMDHELP
 #error Micro Prompt not supported with help enabled (set CLI_REMOVE_CMDHELP in Options)
#else
void CLI_ShowCmdHelp(void)
{
    CLIPrintStr("No help\r\n");
}
#endif
