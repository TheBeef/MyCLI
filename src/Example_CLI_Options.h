/*******************************************************************************
 * FILENAME: CLI_Options.h
 * 
 * PROJECT:
 *    MyCLI
 *
 * FILE DESCRIPTION:
 *    This file has CLI options in it.  The CLI includes this file from the
 *    library.
 *
 *    This is an example you need to copy into your compile search path
 *    and rename to "CLI_Options.h"
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
 *    Paul Hutchinson (27 Mar 2022)
 *       Created
 *
 *******************************************************************************/
#ifndef __CLI_OPTIONS_H_
#define __CLI_OPTIONS_H_

/***  HEADER FILES TO INCLUDE          ***/

/***  DEFINES                          ***/
#define CLI_MAX_PROMPTS                 1               // The number of command prompt we can have
#define CLI_MAX_ARGS                    10              // The max number of argv's pointers (allocated on the stack)
#define CLI_MICRO_BUFFSIZE              40              // MICRO BUILD ONLY.  The number of bytes in the line editing buffer for the micro version
#define STRNCMP                         strncmp
#define STRLEN                          strlen

/* If this is defined then the command help system (CLI_CmdHelpStart()) is
   converted to blanks in the preprocessor.  This removes all the help
   strings from your code without you needing to change your code. */
//#define CLI_REMOVE_CMDHELP


/***  MACROS                           ***/

/***  TYPE DEFINITIONS                 ***/

/***  CLASS DEFINITIONS                ***/

/***  GLOBAL VARIABLE DEFINITIONS      ***/

/***  EXTERNAL FUNCTION PROTOTYPES     ***/

#endif
