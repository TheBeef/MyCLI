/*******************************************************************************
 * FILENAME: CLI.h
 * 
 * PROJECT:
 *    MyCLI
 *
 * FILE DESCRIPTION:
 *    This file has the API for the MyCLI library.
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
 * HISTORY:
 *    Paul Hutchinson (12 Mar 2022)
 *       Created
 *
 *******************************************************************************/
#ifndef __CLI_H_
#define __CLI_H_

/***  HEADER FILES TO INCLUDE          ***/
#include "CLI_Options.h"    // User provided, has the options for the CLI lib

#include <stdbool.h>

/***  DEFINES                          ***/

/***  MACROS                           ***/

/***  TYPE DEFINITIONS                 ***/
struct CLIHandle;   // Private struct

struct CLICommand
{
    const char *Cmd;
    const char *Help;
    void (*Exec)(int argc,const char **argv);
};

/***  CLASS DEFINITIONS                ***/

/***  GLOBAL VARIABLE DEFINITIONS      ***/
extern unsigned int g_CLICmdsCount;         // The number of commands
extern const struct CLICommand g_CLICmds[]; // The commands we are using

/***  EXTERNAL FUNCTION PROTOTYPES     ***/
struct CLIHandle *CLI_GetHandle(void);
void CLI_InitPrompt(struct CLIHandle *Handle);
char *CLI_GetLine(struct CLIHandle *Handle);
void CLI_SetLineBuffer(struct CLIHandle *Handle,char *LineBuff,int MaxSize);
void CLI_SetHistoryBuffer(struct CLIHandle *Handle,char *HistoryBuff,
        int MaxSize);
void CLI_SetPasswordMode(struct CLIHandle *Handle,bool OnOff);
void CLI_DrawPrompt(struct CLIHandle *Handle);
void CLI_RunCmdPrompt(struct CLIHandle *Handle);
void CLI_DisplayHelp(void);
bool CLI_RunLine(struct CLIHandle *Handle,char *Line);

void CLI_CmdHelp_Start(void);
void CLI_CmdHelp_Arg(const char *Label,const char *Desc);
void CLI_CmdHelp_SubArg(const char *Label,const char *Desc);
void CLI_CmdHelp_OptionString(int Level,const char *Option,const char *Desc);
void CLI_CmdHelp_DotDotDot(void);
void CLI_CmdHelp_End(void);
void CLI_ShowCmdHelp(void);

#endif
