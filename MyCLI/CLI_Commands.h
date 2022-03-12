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
 *    Copyright 2012 6464033 Canada Inc.
 *
 *    This software is the property of 6464033 Canada Inc. and may not be
 *    reused in any manner except under express written permission of
 *    6464033 Canada Inc.
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
