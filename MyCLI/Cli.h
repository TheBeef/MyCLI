/*******************************************************************************
 * FILENAME: Cli.h
 * 
 * PROJECT:
 *    CLI
 *
 * FILE DESCRIPTION:
 *    This is the .h file for the Cli.c file.  It does .h file stuff.
 *
 * COPYRIGHT:
 *    Copyright 2010 Paul Hutchinson
 *
 * HISTORY:
 *    phutchinson (14 Oct 2010)
 *       Created
 *
 *******************************************************************************/
#ifndef __CLI_H_
#define __CLI_H_

/***  HEADER FILES TO INCLUDE          ***/
#include <stdint.h>
#include <stdbool.h>

/***  DEFINES                          ***/

/***  MACROS                           ***/

/***  TYPE DEFINITIONS                 ***/

/***  CLASS DEFINITIONS                ***/

/***  GLOBAL VARIABLE DEFINITIONS      ***/

/***  EXTERNAL FUNCTION PROTOTYPES     ***/
void CLI_InitPrompt(void);
char *CLI_GetLine(void);
void CLI_ResetInputBuffer(void);
void CLI_SetLineBuffer(char *LineBuff,int MaxSize);
void CLI_SetHistoryBuffer(char *HistoryBuff,int MaxSize);
void CLI_SetPasswordMode(bool OnOff);
void CLI_ResetHistory(void);

#endif   /* end of "#ifndef __CLI_H_" */
