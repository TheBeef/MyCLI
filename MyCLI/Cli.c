/*******************************************************************************
 * FILENAME: CLIPrompt.c
 *
 * PROJECT:
 *    CLI
 *
 * FILE DESCRIPTION:
 *    This file has the input prompt in it.
 *
 * COPYRIGHT:
 *    Copyright 2010 Paul Hutchinson
 *
 * CREATED BY:
 *    phutchinson (14 Oct 2010)
 *
 ******************************************************************************/

/*** HEADER FILES TO INCLUDE  ***/
#include "Cli.h"
#include "CLI_HAL.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*** DEFINES                  ***/

/*** MACROS                   ***/

/*** TYPE DEFINITIONS         ***/
typedef enum
{
    e_CLILastKey_Up=0,
    e_CLILastKey_Down,
    e_CLILastKey_Other,
    e_CLILastKeyMAX
} e_CLILastKeyType;

/*** VARIABLE DEFINITIONS     ***/
static char *m_CLILineBuff;
static unsigned int m_CLIMaxLineSize;
static unsigned int m_LineBuffInsertPos;
static uint8_t m_ESCPos;
static char *m_CLIHistoryBuff;  // The buffer we are using for the history
static uint16_t m_CLIHistoryBuffSize;   // the size of 'm_CLIHistoryBuff'
static char *m_CLIHistoryPos;   // The point in the history we are at
static e_CLILastKeyType m_CLILastKeyType;  // Was the last key pressed an array (up/down) key
static uint32_t m_ESCStart; // The time that the ESC key was pressed
static bool m_PasswordMode; // Are we asking for a password

/*** FUNCTION PROTOTYPES      ***/
static void CL_EchoEndOfPromptLine(unsigned int len);
static void CL_EraseCurrentLine(void);

/*******************************************************************************
 * NAME:
 *    CLI_InitPrompt
 *
 * SYNOPSIS:
 *    void CLI_InitPrompt(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function init's command prompt.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (14 Oct 2010)
 *       Created
 ******************************************************************************/
void CLI_InitPrompt(void)
{
    m_CLILastKeyType=e_CLILastKey_Other;
    m_CLIHistoryBuff=NULL;
    m_PasswordMode=false;
    CLI_ResetInputBuffer();
}

/*******************************************************************************
 * NAME:
 *    CLI_GetLine
 *
 * SYNOPSIS:
 *    char *CLI_GetLine(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function gets an input line from the user.  It is non-blocking.
 *
 * RETURNS:
 *    A pointer to the line buffer or NULL if the line is not ready yet.
 *
 * SEE ALSO:
 *    
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (13 Oct 2010)
 *       Created
 ******************************************************************************/
char *CLI_GetLine(void)
{
    char c;
    unsigned int p;
    unsigned int l;
    unsigned int bytes;
    char *h;    // History buffer pointer

    /* If we got a ESC we wait 1/4 second for a second char and if we don't
       get one assume this was really an ESC (and not an escape seq) */
    c=0;
    if(m_ESCPos>0)
    {
        if(HAL_CLI_GetMilliSecCounter()-m_ESCStart>250)
            c=27;
    }

    if(HAL_CLI_IsCharAvailable() || c!=0)
    {
        if(c==0)
            c=HAL_CLI_GetChar();

        if(m_ESCPos>0)
        {
            m_ESCPos++;
            switch(m_ESCPos)
            {
                case 1: // ESC Char
                break;
                case 2: // [
                    if(c!='[')
                    {
                        /* Not ANSI */
                        m_ESCPos=0;
                    }
                    if(c==27)
                    {
                        /* ESC again */
                        CL_EraseCurrentLine();
                        CLI_ResetInputBuffer();
                    }
                break;
                case 3:
                    switch(c)
                    {
                        case 'C':   // Right
                            if(m_LineBuffInsertPos<strlen(m_CLILineBuff))
                            {
                                if(m_PasswordMode)
                                {
                                    HAL_CLI_PutChar('*');
                                }
                                else
                                {
                                    HAL_CLI_PutChar(
                                            m_CLILineBuff[m_LineBuffInsertPos]);
                                }
                                m_LineBuffInsertPos++;
                            }
                            m_ESCPos=0;
                        break;
                        case 'D':   // Left
                            if(m_LineBuffInsertPos>0)
                                m_LineBuffInsertPos--;
                            HAL_CLI_PutChar('\b');
                            m_ESCPos=0;
                        break;
                        case '4':   // End
                            for(;m_CLILineBuff[m_LineBuffInsertPos]!=0;m_LineBuffInsertPos++)
                            {
                                if(m_PasswordMode)
                                {
                                    HAL_CLI_PutChar('*');
                                }
                                else
                                {
                                    HAL_CLI_PutChar(
                                            m_CLILineBuff[m_LineBuffInsertPos]);
                                }
                            }
                        break;
                        case '3':   // Del
                            l=strlen(m_CLILineBuff);
                            if(m_LineBuffInsertPos<l)
                            {
                                /* Copy the char above the current pos */
                                for(p=m_LineBuffInsertPos;p<l;p++)
                                    m_CLILineBuff[p]=m_CLILineBuff[p+1];

                                CL_EchoEndOfPromptLine(l-1);
                            }
                        break;
                        case '1':   // Home
                            for(;m_LineBuffInsertPos>0;m_LineBuffInsertPos--)
                                HAL_CLI_PutChar('\b');
                        break;
                        case 'A':   // Up
                            if(m_CLIHistoryBuff!=NULL)
                            {
                                CL_EraseCurrentLine();

                                p=1;
                                /* If the last key was down and we wheren't
                                   at the end of the history then we need to
                                   skip one */
                                if(m_CLILastKeyType==e_CLILastKey_Down)
                                {
                                    if(m_CLIHistoryPos<
                                        m_CLIHistoryBuff+m_CLIHistoryBuffSize-1)
                                    {
                                        if(*(m_CLIHistoryPos+1)!=0)
                                        {
                                            /* We need to skip an entry */
                                            p=2;
                                        }
                                    }
                                }

                                /* Find the prev entry in the history */
                                while(m_CLIHistoryPos>m_CLIHistoryBuff)
                                {
                                    if(*(m_CLIHistoryPos-1)==0)
                                    {
                                        if(--p==0)
                                            break;
                                    }
                                    m_CLIHistoryPos--;
                                }
                                strcpy(m_CLILineBuff,m_CLIHistoryPos);

                                /* Move to the end of the prev entry if we
                                   can */
                                if(m_CLIHistoryPos>m_CLIHistoryBuff)
                                    m_CLIHistoryPos--;

                                l=strlen(m_CLILineBuff);
                                for(m_LineBuffInsertPos=0;m_LineBuffInsertPos<l;
                                        m_LineBuffInsertPos++)
                                {
                                    if(m_PasswordMode)
                                    {
                                        HAL_CLI_PutChar('*');
                                    }
                                    else
                                    {
                                        HAL_CLI_PutChar(m_CLILineBuff[
                                                m_LineBuffInsertPos]);
                                    }
                                }
                            }
                            m_CLILastKeyType=e_CLILastKey_Up;
                            m_ESCPos=0;
                        break;
                        case 'B':   // Down
                            if(m_CLIHistoryBuff!=NULL)
                            {
                                CL_EraseCurrentLine();

                                /* Find the next entry in the history */
                                if(*(m_CLIHistoryPos+1)!=0)
                                {
                                    if(m_CLIHistoryPos!=m_CLIHistoryBuff)
                                    m_CLIHistoryPos++;

                                    if(m_CLILastKeyType==e_CLILastKey_Up)
                                    {
                                        /* Last key was up, we need to skip
                                           an entry */
                                        m_CLIHistoryPos=
                                                strchr(m_CLIHistoryPos+1,0);
                                        if(*(m_CLIHistoryPos+1)!=0)
                                            m_CLIHistoryPos++;
                                    }
                                }

                                strcpy(m_CLILineBuff,m_CLIHistoryPos);

                                l=strlen(m_CLILineBuff);
                                for(m_LineBuffInsertPos=0;m_LineBuffInsertPos<l;
                                        m_LineBuffInsertPos++)
                                {
                                    if(m_PasswordMode)
                                    {
                                        HAL_CLI_PutChar('*');
                                    }
                                    else
                                    {
                                        HAL_CLI_PutChar(m_CLILineBuff[
                                                m_LineBuffInsertPos]);
                                    }

                                    /* Drag 'm_CLIHistoryPos' with us */
                                    m_CLIHistoryPos++;
                                }
                            }
                            m_CLILastKeyType=e_CLILastKey_Down;
                            m_ESCPos=0;
                        break;
                        default:
//                            printf("GOT:%c (%d)\n",c,c);
                            m_ESCPos=0;
                        break;
                    }
                    c=0;
                break;
                case 4:
                    if(c=='~')
                        c=0;
                    m_ESCPos=0;
                break;
                default:
                    m_ESCPos=0;
                break;
            }
        }

        if(m_ESCPos==0)
        {
            switch(c)
            {
                case 0: // We ignore 0's
                break;
                case '\r':
                    /* We are done */
                    HAL_CLI_PutChar('\n');
                    HAL_CLI_PutChar('\r');

                    /* Add to the end of the history buffer (if we have one,
                       it's not a blank line, and the last key was not an
                       arrow) */
                    if(m_CLIHistoryBuff!=NULL && *m_CLILineBuff!=0)
                    {
                        if(m_CLILastKeyType==e_CLILastKey_Other)
                        {
                            l=strlen(m_CLILineBuff)+1;  // We need space for the \0

                            h=NULL;
                            while(l<=m_CLIHistoryBuffSize)
                            {
                                /* Search from the end of the buffer to the last
                                   entry */
                                for(h=m_CLIHistoryBuff+m_CLIHistoryBuffSize-1;
                                        h>m_CLIHistoryBuff;h--)
                                {
                                    if(*h!=0)
                                    {
                                        /* Move past the 0 */
                                        h+=2;
                                        break;
                                    }
                                }

                                if(h>m_CLIHistoryBuff+m_CLIHistoryBuffSize-1)
                                {
                                    /* Error we are past the end of the buffer.
                                       Likely means the buffer is overflowing */
                                    h=m_CLIHistoryBuff+m_CLIHistoryBuffSize-1;
                                }

                                /* Ok, 'h' is at the insert point.  Make sure
                                   there's space */
                                p=m_CLIHistoryBuff+m_CLIHistoryBuffSize-h;
                                if(l>p)
                                {
                                    /* No space we need to drop the oldest entry */
                                    h=strchr(m_CLIHistoryBuff,0)+1; // +1 for the \0

                                    /* Hu? We didn't find a \0? */
                                    if(h==NULL)
                                        break;

                                    /* Figure out how many bytes to copy (the
                                       size-the strlen of the first entry) */
                                    bytes=m_CLIHistoryBuffSize-(h-m_CLIHistoryBuff);
                                    memcpy(m_CLIHistoryBuff,h,bytes);

                                    /* The number of bytes to clear at the end
                                       of the buffer (strlen of first entry +
                                       old padding space) */
                                    bytes=h-m_CLIHistoryBuff+p;
                                    memset(m_CLIHistoryBuff+
                                            m_CLIHistoryBuffSize-bytes,0,bytes);

                                    /* If we have no room, just abort the loop */
                                    if(bytes==0)
                                        break;
                                }
                                else
                                {
                                    break;
                                }
                            }

                            /* Copy in the new entry if we have space */
                            if(h!=NULL &&
                                    l<=m_CLIHistoryBuff+m_CLIHistoryBuffSize-h)
                            {
                                strcpy(h,m_CLILineBuff);
                                m_CLIHistoryPos=h+l-1;
                            }
                        }
                        else
                        {
                            /* Move to the end of the current history buffer */
                            if(*(m_CLIHistoryPos+1)!=0)
                                m_CLIHistoryPos=strchr(m_CLIHistoryPos+1,0);
                        }
                    }
                    m_CLILastKeyType=e_CLILastKey_Other;
                    return m_CLILineBuff;
                break;
                case '\b':
                case 127:
                    if(m_LineBuffInsertPos>0)
                    {
                        m_LineBuffInsertPos--;

                        /* Copy the char above the current pos */
                        l=strlen(m_CLILineBuff);
                        for(p=m_LineBuffInsertPos;p<l;p++)
                            m_CLILineBuff[p]=m_CLILineBuff[p+1];

                        /* Move over the char we just deleted, then redraw
                           the end of the line */
                        HAL_CLI_PutChar('\b');
                        CL_EchoEndOfPromptLine(l-1);
                    }
                    m_CLILastKeyType=e_CLILastKey_Other;
                break;
                case 27:    /* ANSI codes */
                    m_ESCPos=1;
                    m_ESCStart=HAL_CLI_GetMilliSecCounter();
                break;
                default:
                    l=strlen(m_CLILineBuff);
                    if(l<m_CLIMaxLineSize-1)
                    {
                        /* Make room */
                        for(p=l;p>m_LineBuffInsertPos;p--)
                            m_CLILineBuff[p+1]=m_CLILineBuff[p];
                        m_CLILineBuff[m_LineBuffInsertPos+1]=
                                m_CLILineBuff[m_LineBuffInsertPos];
                        m_CLILineBuff[m_LineBuffInsertPos]=c;

                        CL_EchoEndOfPromptLine(l+1);

                        m_LineBuffInsertPos++;

                        /* Move one char over */
                        if(m_PasswordMode)
                            HAL_CLI_PutChar('*');
                        else
                            HAL_CLI_PutChar(c);
                    }
                    m_CLILastKeyType=e_CLILastKey_Other;
                break;
            }
        }
    }
    return NULL;
}

/*******************************************************************************
 * NAME:
 *    CL_EchoEndOfPromptLine
 *
 * SYNOPSIS:
 *    static void CL_EchoEndOfPromptLine(unsigned int len);
 *
 * PARAMETERS:
 *    len [I] -- The number of chars in the input line buffer.
 *
 * FUNCTION:
 *    This is helper function for the CL_GetLine() function.  It outputs
 *    the line from the current position and then back spaces to the caret
 *    position.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    CL_GetLine()
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (13 Oct 2010)
 *       Created
 ******************************************************************************/
static void CL_EchoEndOfPromptLine(unsigned int len)
{
    unsigned int p;

    /* Echo the rest of the line */
    for(p=m_LineBuffInsertPos;p<len;p++)
    {
        if(m_PasswordMode)
            HAL_CLI_PutChar('*');
        else
            HAL_CLI_PutChar(m_CLILineBuff[p]);
    }

    /* Add a space on the end to kill off any deleted chars */
    HAL_CLI_PutChar(' ');

    /* Now back up to where we should be */
    for(p=m_LineBuffInsertPos;p<len+1;p++)
        HAL_CLI_PutChar('\b');
}

/*******************************************************************************
 * NAME:
 *    CLI_ResetInputBuffer
 *
 * SYNOPSIS:
 *    void CLI_ResetInputBuffer(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function reset the input buffer to blank.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    CL_GetLine(), CL_EraseCurrentLine()
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (13 Oct 2010)
 *       Created
 ******************************************************************************/
void CLI_ResetInputBuffer(void)
{
    m_LineBuffInsertPos=0;
    *m_CLILineBuff=0;
    m_ESCPos=0;
}

/*******************************************************************************
 * NAME:
 *    CL_EraseCurrentLine
 *
 * SYNOPSIS:
 *    static void CL_EraseCurrentLine(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function erases the current input line and puts the caret at the
 *    start of the line.
 *
 *    This does not delete the current line buffer.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    CLI_ResetInputBuffer()
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (14 Oct 2010)
 *       Created
 ******************************************************************************/
static void CL_EraseCurrentLine(void)
{
    unsigned int p;
    unsigned int len;

    len=strlen(m_CLILineBuff);

    /* Goto the start of the line */
    for(;m_LineBuffInsertPos>0;m_LineBuffInsertPos--)
        HAL_CLI_PutChar('\b');

    /* Space over the whole thing */
    for(p=0;p<len;p++)
        HAL_CLI_PutChar(' ');

    /* Now back up to the start */
    for(p=0;p<len;p++)
        HAL_CLI_PutChar('\b');
}

/*******************************************************************************
 * NAME:
 *    CLI_SetLineBuffer
 *
 * SYNOPSIS:
 *    void CLI_SetLineBuffer(char *LineBuff,int MaxSize);
 *
 * PARAMETERS:
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
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (13 Oct 2010)
 *       Created
 ******************************************************************************/
void CLI_SetLineBuffer(char *LineBuff,int MaxSize)
{
    m_CLILineBuff=LineBuff;
    m_CLIMaxLineSize=MaxSize;

    CLI_ResetInputBuffer();
}

/*******************************************************************************
 * NAME:
 *    CLI_SetHistoryBuffer
 *
 * SYNOPSIS:
 *    void CLI_SetHistoryBuffer(char *HistoryBuff,int MaxSize);
 *
 * PARAMETERS:
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
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (14 Oct 2010)
 *       Created
 ******************************************************************************/
void CLI_SetHistoryBuffer(char *HistoryBuff,int MaxSize)
{
    m_CLIHistoryBuff=HistoryBuff;
    m_CLIHistoryPos=HistoryBuff;
    m_CLIHistoryBuffSize=MaxSize;
    if(HistoryBuff==NULL)
        m_CLIHistoryBuffSize=0;

    CLI_ResetHistory();
}

/*******************************************************************************
 * NAME:
 *    CLI_SetPasswordMode
 *
 * SYNOPSIS:
 *    void CLI_SetPasswordMode(bool OnOff);
 *
 * PARAMETERS:
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
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (14 Oct 2010)
 *       Created
 ******************************************************************************/
void CLI_SetPasswordMode(bool OnOff)
{
    m_PasswordMode=OnOff;
}

/*******************************************************************************
 * NAME:
 *    CLI_ResetHistory
 *
 * SYNOPSIS:
 *    void CLI_ResetHistory(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function resets the history (if there is one)
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 *******************************************************************************
 * REVISION HISTORY:
 *    phutchinson (14 Oct 2010)
 *       Created
 ******************************************************************************/
void CLI_ResetHistory(void)
{
    if(m_CLIHistoryBuff!=NULL)
    {
        memset(m_CLIHistoryBuff,0,m_CLIHistoryBuffSize);
        m_CLIHistoryPos=m_CLIHistoryBuff;
    }
}
