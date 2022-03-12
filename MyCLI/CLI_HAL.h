/*******************************************************************************
 * FILENAME: CLI_HAL.h
 * 
 * PROJECT:
 *    CLI
 *
 * FILE DESCRIPTION:
 *    This file has the HAL function you need to write to make the CLI work.
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
#ifndef __CLI_HAL_H_
#define __CLI_HAL_H_

/***  HEADER FILES TO INCLUDE          ***/

/***  DEFINES                          ***/

/***  MACROS                           ***/

/***  TYPE DEFINITIONS                 ***/

/***  CLASS DEFINITIONS                ***/

/***  GLOBAL VARIABLE DEFINITIONS      ***/

/***  EXTERNAL FUNCTION PROTOTYPES     ***/

/*******************************************************************************
 * NAME:
 *    HAL_CLI_IsCharAvailable
 *
 * SYNOPSIS:
 *    bool HAL_CLI_IsCharAvailable(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function is called to see if there is at least one byte that can
 *    be read by HAL_CLI_GetChar().
 *
 * RETURNS:
 *    true -- There is a char available.
 *    false -- There are no bytes ready.
 *
 * SEE ALSO:
 *    HAL_CLI_GetChar()
 *******************************************************************************
 * REVISION HISTORY:
 *    Paul Hutchinson (18 Jun 2012)
 *       Created
 ******************************************************************************/
bool HAL_CLI_IsCharAvailable(void);
/*******************************************************************************
 * NAME:
 *    HAL_CLI_GetChar
 *
 * SYNOPSIS:
 *    char HAL_CLI_GetChar(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function is called to get a char from the input stream.  This
 *    function will only be called if HAL_CLI_IsCharAvailable() returns
 *    true, so it can block.
 *
 * RETURNS:
 *    The char that has been read.
 *
 * SEE ALSO:
 *    HAL_CLI_IsCharAvailable(), HAL_CLI_PutChar()
 *******************************************************************************
 * REVISION HISTORY:
 *    Paul Hutchinson (18 Jun 2012)
 *       Created
 ******************************************************************************/
char HAL_CLI_GetChar(void);
/*******************************************************************************
 * NAME:
 *    HAL_CLI_PutChar
 *
 * SYNOPSIS:
 *    void HAL_CLI_PutChar(char c);
 *
 * PARAMETERS:
 *    c [I] -- The char to output.
 *
 * FUNCTION:
 *    This function is called when the command line whats to output a char.
 *
 * RETURNS:
 *    NONE
 *
 * NOTES:
 *    It is expected that the output device will support \b \r and \n.
 *
 * SEE ALSO:
 *    HAL_CLI_GetChar()
 *******************************************************************************
 * REVISION HISTORY:
 *    Paul Hutchinson (18 Jun 2012)
 *       Created
 ******************************************************************************/
void HAL_CLI_PutChar(char c);
/*******************************************************************************
 * NAME:
 *    HAL_CLI_GetMilliSecCounter
 *
 * SYNOPSIS:
 *    uint32_t HAL_CLI_GetMilliSecCounter(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function is called to get a milli second counter.  This counter
 *    inc's every milli second (or there abouts) and does not wrap at 1 second.
 *    It is a continuous counter.
 *
 * RETURNS:
 *    The number of milli seconds that have gone by.
 *
 * SEE ALSO:
 *    
 *******************************************************************************
 * REVISION HISTORY:
 *    Paul Hutchinson (18 Jun 2012)
 *       Created
 ******************************************************************************/
uint32_t HAL_CLI_GetMilliSecCounter(void);

#endif   /* end of "#ifndef __CLI_HAL_H_" */
