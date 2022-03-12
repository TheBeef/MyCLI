/*******************************************************************************
 * FILENAME: CLI_Commands.c
 *
 * PROJECT:
 *    CLI
 *
 * FILE DESCRIPTION:
 *    This file has the command handler in it.
 *
 * COPYRIGHT:
 *    Copyright 2012 Paul Hutchinson
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
 *    Paul Hutchinson (18 Jun 2012)
 *
 ******************************************************************************/

/*** HEADER FILES TO INCLUDE  ***/
#include "CLI_Commands.h"
#include "Cli.h"
#include "CLI_HAL.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*** DEFINES                  ***/

/*** MACROS                   ***/

/*** TYPE DEFINITIONS         ***/

/*** VARIABLE DEFINITIONS     ***/
const char *m_CLIPrompt;            // The command prompt string

/*** FUNCTION PROTOTYPES      ***/
static void CLIPrintStr(const char *str);
static void CLI_RunCMD(char *Line,const struct CLICommand *Cmd);

/*******************************************************************************
 * NAME:
 *    CLI_Init
 *
 * SYNOPSIS:
 *    void CLI_Init(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function init's the command area.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_Init(void)
{
    m_CLIPrompt=">";
}

/*******************************************************************************
 * NAME:
 *    CLI_SetPromptStr
 *
 * SYNOPSIS:
 *    void CLI_SetPromptStr(const char *Prompt);
 *
 * PARAMETERS:
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
void CLI_SetPromptStr(const char *Prompt)
{
    m_CLIPrompt=Prompt;
}

/*******************************************************************************
 * NAME:
 *    CLI_PollCmdPrompt
 *
 * SYNOPSIS:
 *    void CLI_PollCmdPrompt(void);
 *
 * PARAMETERS:
 *    NONE
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
void CLI_PollCmdPrompt(void)
{
    char *Line; // The line we got from the input
    int cmd;    // The command index we are looking at
    int len;    // The len of the current command we are looking at

    Line=CLI_GetLine();
    if(Line!=NULL)
    {
        /* We got a line, scan the commands */
        for(cmd=0;cmd<g_CLICmdsCount;cmd++)
        {
            len=strlen(g_CLICmds[cmd].Cmd);
            if(strncmp(Line,g_CLICmds[cmd].Cmd,len)==0 &&
                    (Line[len]==0 || Line[len]==' '))
            {
                /* Found a command, run it */
                CLI_RunCMD(Line,&g_CLICmds[cmd]);
                break;
            }
        }
        if(cmd==g_CLICmdsCount && *Line!=0)
            CLIPrintStr("Command not found.\r\n\r\n");

        CLI_DrawPrompt();

        /* We are done with the buffer, reset for the next input */
        CLI_ResetInputBuffer();
    }
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
        HAL_CLI_PutChar(*Str);
        Str++;
    }
}

/*******************************************************************************
 * NAME:
 *    CLI_RunCMD
 *
 * SYNOPSIS:
 *    static void CLI_RunCMD(char *Line,const struct CLICommand *Cmd)
 *
 * PARAMETERS:
 *    Line [I] -- The line that starting this
 *    Cmd [I] -- The command to run.
 *
 * FUNCTION:
 *    This function builds the args for the command and then run's it.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
static void CLI_RunCMD(char *Line,const struct CLICommand *Cmd)
{
    unsigned int r;         // Temp var (for loops)
    unsigned int len;       // The len of the input line
    uint16_t Argc;          // The number of args
    char *Argv[MAX_ARGS];   // The argv's we are sending

    len=strlen(Line);

    /* Split up the command line */
    Argc=0;
    Argv[Argc++]=Line; // The name of the command
    for(r=0;r<len;r++)
    {
        if(Line[r]==' ')
        {
            Line[r]=0;
            Argv[Argc++]=&Line[r+1];
            if(Argc>MAX_ARGS)
            {
                CLIPrintStr("CLI_RunCMD:MAX_ARGS to small.\r\n");
                return;
            }
        }
    }

    if(Cmd->Exec!=NULL)
        Cmd->Exec(Argc,(const char **)Argv);
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
    unsigned int MaxWidth;  // The max width of the cmd strings
    unsigned int len;       // The length of the current string
    unsigned int cmd;       // The command we are on

    MaxWidth=8;
    for(cmd=0;cmd<g_CLICmdsCount;cmd++)
    {
        len=strlen(g_CLICmds[cmd].Cmd);
        if(len>MaxWidth)
            MaxWidth=len;
    }

    MaxWidth++; // Add a space between the command names and the help

    for(cmd=0;cmd<g_CLICmdsCount;cmd++)
    {
        CLIPrintStr(g_CLICmds[cmd].Cmd);

        len=strlen(g_CLICmds[cmd].Cmd);
        for(;len<MaxWidth;len++)
            HAL_CLI_PutChar(' ');

        CLIPrintStr(g_CLICmds[cmd].Help);
        CLIPrintStr("\r\n");
    }
}

/*******************************************************************************
 * NAME:
 *    CLI_DrawPrompt
 *
 * SYNOPSIS:
 *    void CLI_DrawPrompt(void);
 *
 * PARAMETERS:
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
void CLI_DrawPrompt(void)
{
    CLIPrintStr(m_CLIPrompt);
}
