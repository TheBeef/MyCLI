/*******************************************************************************
 * FILENAME: CLI_Commands.h
 * 
 * PROJECT:
 *    CLI
 *
 * FILE DESCRIPTION:
 *    This file is the .h file for the CLI_Commands.c file.  It does .h file
 *    stuff.
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
 * HISTORY:
 *    Paul Hutchinson (18 Jun 2012)
 *       Created
 *
 *******************************************************************************/
#ifndef __CLI_COMMANDS_H_
#define __CLI_COMMANDS_H_

/***  HEADER FILES TO INCLUDE          ***/

/***  DEFINES                          ***/
#define MAX_ARGS 10

/***  MACROS                           ***/

/***  TYPE DEFINITIONS                 ***/
struct CLICommand
{
    const char *Cmd;
    const char *Help;
    void (*Exec)(int argc,const char **argv);
};

/***  CLASS DEFINITIONS                ***/

/***  GLOBAL VARIABLE DEFINITIONS      ***/
extern int g_CLICmdsCount;                  // The number of commands
extern const struct CLICommand g_CLICmds[]; // The commands we are using

/***  EXTERNAL FUNCTION PROTOTYPES     ***/
void CLI_Init(void);
void CLI_SetPromptStr(const char *Prompt);
void CLI_PollCmdPrompt(void);
void CLI_DisplayHelp(void);
void CLI_DrawPrompt(void);

#endif   /* end of "#ifndef __CLI_COMMANDS_H_" */
