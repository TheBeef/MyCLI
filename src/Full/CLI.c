// Maybe:AEPrompt??

/*******************************************************************************
 * FILENAME: CLI.c
 *
 * PROJECT:
 *    MyCLI
 *
 * FILE DESCRIPTION:
 *    This file has the full feature version of the CLI and prompt in it.
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
 *    Paul Hutchinson (12 Mar 2022)
 *
 ******************************************************************************/

/*** HEADER FILES TO INCLUDE  ***/
#include "../CLI.h"
#include "CLI_HAL.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>

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

typedef enum
{
    e_CLITelnetOpt_None,
    e_CLITelnetOpt_Cmd,
    e_CLITelnetOpt_SE,
    e_CLITelnetOpt_NOP,
    e_CLITelnetOpt_DM,
    e_CLITelnetOpt_BRK,
    e_CLITelnetOpt_IP,
    e_CLITelnetOpt_AO,
    e_CLITelnetOpt_AYT,
    e_CLITelnetOpt_EC,
    e_CLITelnetOpt_EL,
    e_CLITelnetOpt_GA,
    e_CLITelnetOpt_SB,
    e_CLITelnetOpt_WILL,
    e_CLITelnetOpt_WONT,
    e_CLITelnetOpt_DO,
    e_CLITelnetOpt_DONT,
    e_CLITelnetOpt_IAC,
    e_CLITelnetMAX
} e_CLITelnetType;

struct CLIHandlePrv
{
    char *LineBuff;                 // The line editing buffer
    unsigned int MaxLineSize;       // The size of 'LineBuff'
    unsigned int LineBuffInsertPos; // Where are with in the 'LineBuff'
    e_CLILastKeyType LastKeyType;   // Was the last key pressed an array (up/down) key
    char *HistoryBuff;              // The buffer we are using for the history
    uint16_t HistoryBuffSize;       // the size of 'm_CLIHistoryBuff'
    char *HistoryPos;               // Where are we in the history buff
    bool PasswordMode;              // Are we asking for a password
    e_CLITelnetType TelnetOpt;      // What operation are we going to do on the next byte
    uint8_t ESCPos;                 // The pos in the ANSI escape seq we have gotten
    uint32_t ESCStart;              // The time that the ESC key was pressed
    const char *Prompt;             // The command prompt string
};

/*** FUNCTION PROTOTYPES      ***/
static void CLI_EchoEndOfPromptLine(struct CLIHandlePrv *CLI,unsigned int len);
static void CLI_EraseCurrentLine(struct CLIHandlePrv *CLI);
static void CLI_ResetHistory(struct CLIHandlePrv *CLI);
static void CLI_ResetInputBuffer(struct CLIHandlePrv *CLI);
static void CLIPrintStr(const char *Str);
static void CLI_RunCMD(char *Line,const struct CLICommand *Cmd);

/*** VARIABLE DEFINITIONS     ***/
static uint8_t m_CLI_AllocatedPrompts;
static struct CLIHandlePrv m_CLI_Prompts[CLI_MAX_PROMPTS];

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
 *    This function gets the next available CLI handle.  This is like an
 *    allocate function except it doesn't allocate memory and just takes
 *    from a pool.  When the pool is empty it return NULL.
 *
 *    There is no way to free a prompt.
 *
 * RETURNS:
 *    A handle to the prompt or NULL if there are no more available
 *
 * NOTES:
 *    This is done this way so that you can change prompts that use
 *    different amounts of memory (struct CLIHandlePrv is different) without
 *    you having to change any of your code (just change the CLI version
 *    you link in).
 *
 * LIMITATIONS:
 *    You must make a CLI_Options.h in your include path and set
 *    'CLI_MAX_PROMPTS' in it.
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
struct CLIHandle *CLI_GetHandle(void)
{
    if(m_CLI_AllocatedPrompts>=CLI_MAX_PROMPTS)
        return NULL;

    return (struct CLIHandle *)&m_CLI_Prompts[m_CLI_AllocatedPrompts++];
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;

    CLI->LineBuff=NULL;
    CLI->MaxLineSize=0;
    CLI->LineBuffInsertPos=0;
    CLI->LastKeyType=e_CLILastKey_Other;
    CLI->HistoryBuff=NULL;
    CLI->HistoryBuffSize=0;
    CLI->HistoryPos=NULL;
    CLI->PasswordMode=false;
    CLI->TelnetOpt=e_CLITelnetOpt_None;
    CLI->ESCPos=0;
    CLI->ESCStart=0;
    CLI->Prompt=">";
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;
    unsigned char c;
    unsigned int p;
    unsigned int l;
    unsigned int bytes;
    char *h;    // History buffer pointer

    c=0;

    if(CLI->TelnetOpt!=e_CLITelnetOpt_None && HAL_CLI_IsCharAvailable())
    {
        c=HAL_CLI_GetChar();

        switch(CLI->TelnetOpt)
        {
            case e_CLITelnetOpt_Cmd:
                /* https://users.cs.cf.ac.uk/Dave.Marshall/Internet/node141.html */
                switch(c)
                {
                    case 240: // SE:End of subnegotiation parameters.
                        CLI->TelnetOpt=e_CLITelnetOpt_SE;
                    break;
                    case 241: // NOP:No operation
                        CLI->TelnetOpt=e_CLITelnetOpt_NOP;
                    break;
                    case 242: // DM:Data mark. Indicates the position of a Synch event within the data stream. This should always be accompanied by a TCP urgent notification.
                        CLI->TelnetOpt=e_CLITelnetOpt_DM;
                    break;
                    case 243: // BRK:Break. Indicates that the "break" or "attention" key was hit.
                        CLI->TelnetOpt=e_CLITelnetOpt_BRK;
                    break;
                    case 244: // IP:Suspend, interrupt or abort the process to which the NVT is connected.
                        CLI->TelnetOpt=e_CLITelnetOpt_IP;
                    break;
                    case 245: // AO:Abort output. Allows the current process to run to completion but do not send its output to the user.
                        CLI->TelnetOpt=e_CLITelnetOpt_AO;
                    break;
                    case 246: // AYT:Are you there. Send back to the NVT some visible evidence that the AYT was received.
                        CLI->TelnetOpt=e_CLITelnetOpt_AYT;
                    break;
                    case 247: // EC:Erase character. The receiver should delete the last preceding undeleted character from the data stream.
                        CLI->TelnetOpt=e_CLITelnetOpt_EC;
                    break;
                    case 248: // EL:Erase line. Delete characters from the data stream back to but not including the previous CRLF.
                        CLI->TelnetOpt=e_CLITelnetOpt_EL;
                    break;
                    case 249: // GA:Go ahead. Used, under certain circumstances, to tell the other end that it can transmit.
                        CLI->TelnetOpt=e_CLITelnetOpt_GA;
                    break;
                    case 250: // SB:Subnegotiation of the indicated option follows.
                        CLI->TelnetOpt=e_CLITelnetOpt_SB;
                    break;
                    case 251: // WILL:Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
                        CLI->TelnetOpt=e_CLITelnetOpt_WILL;
                    break;
                    case 252: // WONT:Indicates the refusal to perform, or continue performing, the indicated option.
                        CLI->TelnetOpt=e_CLITelnetOpt_WONT;
                    break;
                    case 253: // DO:Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option.
                        CLI->TelnetOpt=e_CLITelnetOpt_DO;
                    break;
                    case 254: // DONT:Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.
                        CLI->TelnetOpt=e_CLITelnetOpt_DONT;
                    break;
                    case 255: // IAC:Interpret as command
                        CLI->TelnetOpt=e_CLITelnetOpt_IAC;
                    break;
                    default:
                        CLI->TelnetOpt=e_CLITelnetOpt_None;
                }
            break;
            case e_CLITelnetOpt_SE:
            case e_CLITelnetOpt_NOP:
            case e_CLITelnetOpt_DM:
            case e_CLITelnetOpt_BRK:
            case e_CLITelnetOpt_IP:
            case e_CLITelnetOpt_AO:
            case e_CLITelnetOpt_AYT:
            case e_CLITelnetOpt_EC:
            case e_CLITelnetOpt_EL:
            case e_CLITelnetOpt_GA:
                CLI->TelnetOpt=e_CLITelnetOpt_None;
            break;
            case e_CLITelnetOpt_SB:
                if(c==255)
                    CLI->TelnetOpt=e_CLITelnetOpt_Cmd;
            break;
            case e_CLITelnetOpt_WILL:
            case e_CLITelnetOpt_WONT:
                switch(c)
                {
                    case 1: // Echo
                        if(CLI->TelnetOpt==e_CLITelnetOpt_WILL)
                        {
                            HAL_CLI_PutChar(255);   // IAC
                            HAL_CLI_PutChar(253);   // DO (we will do this)
                            HAL_CLI_PutChar(1);     // Echo
                        }
                        else
                        {
                            HAL_CLI_PutChar(255);   // IAC
                            HAL_CLI_PutChar(252);   // WONT (we don't support this)
                            HAL_CLI_PutChar(1);     // Echo
                        }
                    break;
                    case 3: // Suppress go ahead
                        HAL_CLI_PutChar(255);       // IAC
                        HAL_CLI_PutChar(253);       // DO
                        HAL_CLI_PutChar(3);         // Suppress go ahead
                    break;
                    default:
                        /* We don't support this */
                        HAL_CLI_PutChar(255);       // IAC
                        HAL_CLI_PutChar(252);       // WONT
                        HAL_CLI_PutChar(c);         // What ever they asked for
                    break;
                }
                CLI->TelnetOpt=e_CLITelnetOpt_None;
            break;
            case e_CLITelnetOpt_DO:
            case e_CLITelnetOpt_DONT:
                switch(c)
                {
                    case 1: // Echo
                    break;
                    case 3: // Suppress go ahead
                    break;
                    default:
                        /* We don't support this */
                        HAL_CLI_PutChar(255);       // IAC
                        HAL_CLI_PutChar(252);       // WONT
                        HAL_CLI_PutChar(c);         // What ever they asked for
                    break;
                }
                CLI->TelnetOpt=e_CLITelnetOpt_None;
            break;
            case e_CLITelnetOpt_IAC:
            default:
                CLI->TelnetOpt=e_CLITelnetOpt_None;
            break;
        }
        return NULL;
    }

    /* If we got a ESC we wait 1/4 second for a second char and if we don't
       get one assume this was really an ESC (and not an escape seq) */
    if(CLI->ESCPos>0)
    {
        if(HAL_CLI_GetMilliSecCounter()-CLI->ESCStart>250)
            c=27;
    }

    if(HAL_CLI_IsCharAvailable() || c!=0)
    {
        if(c==0)
            c=HAL_CLI_GetChar();

        if(CLI->ESCPos>0)
        {
            CLI->ESCPos++;
            switch(CLI->ESCPos)
            {
                case 1: // ESC Char
                break;
                case 2: // [
                    if(c!='[')
                    {
                        /* Not ANSI */
                        CLI->ESCPos=0;
                    }
                    if(c==27)
                    {
                        /* ESC again */
                        CLI_EraseCurrentLine(CLI);
                        CLI_ResetInputBuffer(CLI);
                    }
                break;
                case 3:
                    switch(c)
                    {
                        case 'C':   // Right
                            if(CLI->LineBuffInsertPos<strlen(CLI->LineBuff))
                            {
                                if(CLI->PasswordMode)
                                {
                                    HAL_CLI_PutChar('*');
                                }
                                else
                                {
                                    HAL_CLI_PutChar(CLI->
                                            LineBuff[CLI->LineBuffInsertPos]);
                                }
                                CLI->LineBuffInsertPos++;
                            }
                            CLI->ESCPos=0;
                        break;
                        case 'D':   // Left
                            if(CLI->LineBuffInsertPos>0)
                                CLI->LineBuffInsertPos--;
                            HAL_CLI_PutChar('\b');
                            CLI->ESCPos=0;
                        break;
                        case '4':   // End
                            for(;CLI->LineBuff[CLI->LineBuffInsertPos]!=0;
                                    CLI->LineBuffInsertPos++)
                            {
                                if(CLI->PasswordMode)
                                {
                                    HAL_CLI_PutChar('*');
                                }
                                else
                                {
                                    HAL_CLI_PutChar(CLI->
                                            LineBuff[CLI->LineBuffInsertPos]);
                                }
                            }
                        break;
                        case '3':   // Del
                            l=strlen(CLI->LineBuff);
                            if(CLI->LineBuffInsertPos<l)
                            {
                                /* Copy the char above the current pos */
                                for(p=CLI->LineBuffInsertPos;p<l;p++)
                                    CLI->LineBuff[p]=CLI->LineBuff[p+1];

                                CLI_EchoEndOfPromptLine(CLI,l-1);
                            }
                        break;
                        case '1':   // Home
                            for(;CLI->LineBuffInsertPos>0;
                                    CLI->LineBuffInsertPos--)
                            {
                                HAL_CLI_PutChar('\b');
                            }
                        break;
                        case 'A':   // Up
                            if(CLI->HistoryBuff!=NULL)
                            {
                                CLI_EraseCurrentLine(CLI);

                                p=1;
                                /* If the last key was down and we wheren't
                                   at the end of the history then we need to
                                   skip one */
                                if(CLI->LastKeyType==e_CLILastKey_Down)
                                {
                                    if(CLI->HistoryPos<
                                        CLI->HistoryBuff+CLI->HistoryBuffSize-1)
                                    {
                                        if(*(CLI->HistoryPos+1)!=0)
                                        {
                                            /* We need to skip an entry */
                                            p=2;
                                        }
                                    }
                                }

                                /* Find the prev entry in the history */
                                while(CLI->HistoryPos>CLI->HistoryBuff)
                                {
                                    if(*(CLI->HistoryPos-1)==0)
                                    {
                                        if(--p==0)
                                            break;
                                    }
                                    CLI->HistoryPos--;
                                }
                                strcpy(CLI->LineBuff,CLI->HistoryPos);

                                /* Move to the end of the prev entry if we
                                   can */
                                if(CLI->HistoryPos>CLI->HistoryBuff)
                                    CLI->HistoryPos--;

                                l=strlen(CLI->LineBuff);
                                for(CLI->LineBuffInsertPos=0;
                                        CLI->LineBuffInsertPos<l;
                                        CLI->LineBuffInsertPos++)
                                {
                                    if(CLI->PasswordMode)
                                    {
                                        HAL_CLI_PutChar('*');
                                    }
                                    else
                                    {
                                        HAL_CLI_PutChar(CLI->LineBuff[
                                                CLI->LineBuffInsertPos]);
                                    }
                                }
                            }
                            CLI->LastKeyType=e_CLILastKey_Up;
                            CLI->ESCPos=0;
                        break;
                        case 'B':   // Down
                            if(CLI->HistoryBuff!=NULL)
                            {
                                CLI_EraseCurrentLine(CLI);

                                /* Find the next entry in the history */
                                if(*(CLI->HistoryPos+1)!=0)
                                {
                                    if(CLI->HistoryPos!=CLI->HistoryBuff)
                                        CLI->HistoryPos++;

                                    if(CLI->LastKeyType==e_CLILastKey_Up)
                                    {
                                        /* Last key was up, we need to skip
                                           an entry */
                                        CLI->HistoryPos=
                                                strchr(CLI->HistoryPos+1,0);
                                        if(*(CLI->HistoryPos+1)!=0)
                                            CLI->HistoryPos++;
                                    }
                                }

                                strcpy(CLI->LineBuff,CLI->HistoryPos);

                                l=strlen(CLI->LineBuff);
                                for(CLI->LineBuffInsertPos=0;
                                        CLI->LineBuffInsertPos<l;
                                        CLI->LineBuffInsertPos++)
                                {
                                    if(CLI->PasswordMode)
                                    {
                                        HAL_CLI_PutChar('*');
                                    }
                                    else
                                    {
                                        HAL_CLI_PutChar(CLI->LineBuff[
                                                CLI->LineBuffInsertPos]);
                                    }

                                    /* Drag 'CLI->CLIHistoryPos' with us */
                                    CLI->HistoryPos++;
                                }
                            }
                            CLI->LastKeyType=e_CLILastKey_Down;
                            CLI->ESCPos=0;
                        break;
                        default:
//                            printf("GOT:%c (%d)\n",c,c);
                            CLI->ESCPos=0;
                        break;
                    }
                    c=0;
                break;
                case 4:
                    if(c=='~')
                        c=0;
                    CLI->ESCPos=0;
                break;
                default:
                    CLI->ESCPos=0;
                break;
            }
        }

        if(CLI->ESCPos==0)
        {
            switch(c)
            {
                case 0:     // We ignore 0's
                break;
                case 9:     /* Tab */
                    CLI->LastKeyType=e_CLILastKey_Other;
                break;
                case 10:    // No new lines please
                break;
                case 27:    /* ANSI codes */
                    CLI->ESCPos=1;
                    CLI->ESCStart=HAL_CLI_GetMilliSecCounter();
                break;
                case '\r':
                    /* We are done */
                    HAL_CLI_PutChar('\n');
                    HAL_CLI_PutChar('\r');

                    /* Add to the end of the history buffer (if we have one,
                       it's not a blank line, and the last key was not an
                       arrow) */
                    if(CLI->HistoryBuff!=NULL && *CLI->LineBuff!=0)
                    {
                        if(CLI->LastKeyType==e_CLILastKey_Other)
                        {
                            l=strlen(CLI->LineBuff)+1;  // We need space for the \0

                            h=NULL;
                            while(l<=CLI->HistoryBuffSize)
                            {
                                /* Search from the end of the buffer to the last
                                   entry */
                                for(h=CLI->HistoryBuff+CLI->HistoryBuffSize-1;
                                        h>CLI->HistoryBuff;h--)
                                {
                                    if(*h!=0)
                                    {
                                        /* Move past the 0 */
                                        h+=2;
                                        break;
                                    }
                                }

                                if(h>CLI->HistoryBuff+CLI->HistoryBuffSize-1)
                                {
                                    /* Error we are past the end of the buffer.
                                       Likely means the buffer is overflowing */
                                    h=CLI->HistoryBuff+CLI->HistoryBuffSize-1;
                                }

                                /* Ok, 'h' is at the insert point.  Make sure
                                   there's space */
                                p=CLI->HistoryBuff+CLI->HistoryBuffSize-h;
                                if(l>p)
                                {
                                    /* No space we need to drop the oldest entry */
                                    h=strchr(CLI->HistoryBuff,0)+1; // +1 for the \0

                                    /* Hu? We didn't find a \0? */
                                    if(h==NULL)
                                        break;

                                    /* Figure out how many bytes to copy (the
                                       size-the strlen of the first entry) */
                                    bytes=CLI->HistoryBuffSize-(h-CLI->HistoryBuff);
                                    memcpy(CLI->HistoryBuff,h,bytes);

                                    /* The number of bytes to clear at the end
                                       of the buffer (strlen of first entry +
                                       old padding space) */
                                    bytes=h-CLI->HistoryBuff+p;
                                    memset(CLI->HistoryBuff+
                                            CLI->HistoryBuffSize-bytes,0,bytes);

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
                                    l<=CLI->HistoryBuff+CLI->HistoryBuffSize-h)
                            {
                                strcpy(h,CLI->LineBuff);
                                CLI->HistoryPos=h+l-1;
                            }
                        }
                        else
                        {
                            /* Move to the end of the current history buffer */
                            if(*(CLI->HistoryPos+1)!=0)
                                CLI->HistoryPos=strchr(CLI->HistoryPos+1,0);
                        }
                    }
                    CLI->LastKeyType=e_CLILastKey_Other;
                    return CLI->LineBuff;
                break;
                case '\b':
                case 127:
                    if(CLI->LineBuffInsertPos>0)
                    {
                        CLI->LineBuffInsertPos--;

                        /* Copy the char above the current pos */
                        l=strlen(CLI->LineBuff);
                        for(p=CLI->LineBuffInsertPos;p<l;p++)
                            CLI->LineBuff[p]=CLI->LineBuff[p+1];

                        /* Move over the char we just deleted, then redraw
                           the end of the line */
                        HAL_CLI_PutChar('\b');
                        CLI_EchoEndOfPromptLine(CLI,l-1);
                    }
                    CLI->LastKeyType=e_CLILastKey_Other;
                break;
                default:
                    l=strlen(CLI->LineBuff);
                    if(l<CLI->MaxLineSize-1)
                    {
                        /* Make room */
                        for(p=l;p>CLI->LineBuffInsertPos;p--)
                            CLI->LineBuff[p+1]=CLI->LineBuff[p];
                        CLI->LineBuff[CLI->LineBuffInsertPos+1]=
                                CLI->LineBuff[CLI->LineBuffInsertPos];
                        CLI->LineBuff[CLI->LineBuffInsertPos]=c;

                        CLI_EchoEndOfPromptLine(CLI,l+1);

                        CLI->LineBuffInsertPos++;

                        /* Move one char over */
                        if(CLI->PasswordMode)
                            HAL_CLI_PutChar('*');
                        else
                            HAL_CLI_PutChar(c);
                    }
                    CLI->LastKeyType=e_CLILastKey_Other;
                break;
                case 255:   // Telnet command
                    CLI->TelnetOpt=e_CLITelnetOpt_Cmd;
                break;
            }
        }
    }
    return NULL;
}

/*******************************************************************************
 * NAME:
 *    CLI_EchoEndOfPromptLine
 *
 * SYNOPSIS:
 *    static void CLI_EchoEndOfPromptLine(struct CLIHandlePrv *CLI,
 *              unsigned int len);
 *
 * PARAMETERS:
 *    CLI [I] -- The private data from the command prompt.
 *    len [I] -- The number of chars in the input line buffer.
 *
 * FUNCTION:
 *    This is helper function for the CLI_GetLine() function.  It outputs
 *    the line from the current position and then back spaces to the caret
 *    position.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    CL_GetLine()
 ******************************************************************************/
static void CLI_EchoEndOfPromptLine(struct CLIHandlePrv *CLI,unsigned int len)
{
    unsigned int p;

    /* Echo the rest of the line */
    for(p=CLI->LineBuffInsertPos;p<len;p++)
    {
        if(CLI->PasswordMode)
            HAL_CLI_PutChar('*');
        else
            HAL_CLI_PutChar(CLI->LineBuff[p]);
    }

    /* Add a space on the end to kill off any deleted chars */
    HAL_CLI_PutChar(' ');

    /* Now back up to where we should be */
    for(p=CLI->LineBuffInsertPos;p<len+1;p++)
        HAL_CLI_PutChar('\b');
}

/*******************************************************************************
 * NAME:
 *    CLI_ResetInputBuffer
 *
 * SYNOPSIS:
 *    static void CLI_ResetInputBuffer(struct CLIHandlePrv *CLI);
 *
 * PARAMETERS:
 *    CLI [I] -- The private data from the command prompt.
 *
 * FUNCTION:
 *    This function reset the input buffer to blank.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    CLI_GetLine(), CLI_EraseCurrentLine()
 ******************************************************************************/
static void CLI_ResetInputBuffer(struct CLIHandlePrv *CLI)
{
    CLI->LineBuffInsertPos=0;
    *CLI->LineBuff=0;
    CLI->ESCPos=0;
}

/*******************************************************************************
 * NAME:
 *    CLI_EraseCurrentLine
 *
 * SYNOPSIS:
 *    static void CLI_EraseCurrentLine(struct CLIHandlePrv *CLI);
 *
 * PARAMETERS:
 *    CLI [I] -- The private data from the command prompt.
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
 ******************************************************************************/
static void CLI_EraseCurrentLine(struct CLIHandlePrv *CLI)
{
    unsigned int p;
    unsigned int len;

    len=strlen(CLI->LineBuff);

    /* Goto the start of the line */
    for(;CLI->LineBuffInsertPos>0;CLI->LineBuffInsertPos--)
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;

    CLI->LineBuff=LineBuff;
    CLI->MaxLineSize=MaxSize;

    CLI_ResetInputBuffer(CLI);
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;

    CLI->HistoryBuff=HistoryBuff;
    CLI->HistoryPos=HistoryBuff;
    CLI->HistoryBuffSize=MaxSize;
    if(HistoryBuff==NULL)
        CLI->HistoryBuffSize=0;

    CLI_ResetHistory(CLI);
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;

    CLI->PasswordMode=OnOff;
}

/*******************************************************************************
 * NAME:
 *    CLI_ResetHistory
 *
 * SYNOPSIS:
 *    static void CLI_ResetHistory(struct CLIHandlePrv *CLI);
 *
 * PARAMETERS:
 *    CLI [I] -- The private data from the command prompt.
 *
 * FUNCTION:
 *    This function resets the history (if there is one)
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
static void CLI_ResetHistory(struct CLIHandlePrv *CLI)
{
    if(CLI->HistoryBuff!=NULL)
    {
        memset(CLI->HistoryBuff,0,CLI->HistoryBuffSize);
        CLI->HistoryPos=CLI->HistoryBuff;
    }
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;

    CLI->Prompt=Prompt;
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;
    char *Line; // The line we got from the input

    Line=CLI_GetLine(Handle);
    if(Line!=NULL)
    {
        if(!CLI_RunLine(Handle,Line))
            CLIPrintStr("Command not found.\r\n\r\n");

        /* We are done with the buffer, reset for the next input */
        CLI_ResetInputBuffer(CLI);

        CLI_DrawPrompt(Handle);
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
//    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;
    unsigned int cmd;    // The command index we are looking at
    int len;    // The len of the current command we are looking at

    /* Empty lines do not make errors */
    if(*Line==0)
        return true;

    /* We got a line, scan the commands */
    for(cmd=0;cmd<g_CLICmdsCount;cmd++)
    {
        len=strlen(g_CLICmds[cmd].Cmd);
        if(STRNCMP(Line,g_CLICmds[cmd].Cmd,len)==0 &&
                (Line[len]==0 || Line[len]==' '))
        {
            /* Found a command, run it */
            CLI_RunCMD(Line,&g_CLICmds[cmd]);
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
 *    Line [I] -- The line that starting this.  This will be overwritten.
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
    char *Argv[CLI_MAX_ARGS];   // The argv's we are sending

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
            if(Argc>CLI_MAX_ARGS)
            {
                CLIPrintStr("CLI_RunCMD:CLI_MAX_ARGS to small.\r\n");
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
    struct CLIHandlePrv *CLI=(struct CLIHandlePrv *)Handle;

    CLIPrintStr(CLI->Prompt);
}

/*******************************************************************************
 * NAME:
 *    CLI_CmdHelpStart
 *
 * SYNOPSIS:
 *    void CLI_CmdHelpStart(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function starts a help block.  The help block is used for auto
 *    complete and displaying help.  This must be called before you call
 *    any other help functions (except CLI_ShowCmdHelp())
 *
 * RETURNS:
 *    NONE
 *
 * EXAMPLE:
 *    The ALL CAPS is the what is effected by this function.
 *
 *    if(argc==0)
 *    {
 *      CLI_CmdHelp_Start();
 *
 *      CLI_CmdHelp_Arg("Disk","What disk to work on");
 *      CLI_CmdHelp_OptionString(0,"df0","Floppy disk 1");
 *      CLI_CmdHelp_OptionString(0,"df1","Floppy disk 2");
 *      CLI_CmdHelp_OptionString(0,"df0","Hard drive 1");
 *
 *      CLI_CmdHelp_Arg("Function","Preform disk functions");
 *      // Read fn
 *      CLI_CmdHelp_OptionString(1,"Read","Read from the disk");
 *      CLI_CmdHelp_SubArg("Offset","The offset into the disk to read");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to read");
 *      // Write fn
 *      CLI_CmdHelp_OptionString(1,"Write","Write to the disk");
 *      CLI_CmdHelp_SubArg("Offset","The number of bytes to write");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to write");
 *      // Format fn
 *      CLI_CmdHelp_OptionString(1,"Format","Format the disk");
 *
 *      CLI_HelpEnd(false);
 *      return;
 *    }
 *
 *    Outputs:
 *      USAGE:
 *          Cmd [Disk] [Function]
 *
 *      WHERE:
 *          Disk -- What disk to work on
 *              df0 -- Floppy disk 1
 *              df1 -- Floppy disk 2
 *              df0 -- Hard drive 1
 *          Function -- Preform disk functions
 *                  Read -- Read from the disk
 *                          Offset -- The offset into the disk to read
 *                          Bytes -- The number of bytes to read
 *                  Write -- Write to the disk
 *                          Offset -- The number of bytes to write
 *                          Bytes -- The number of bytes to write
 *                  Format -- Format the disk
 *
 * SEE ALSO:
 *    CLI_CmdHelp_Arg(), CLI_CmdHelp_SubArg(), CLI_CmdHelp_OptionString()
 *    CLI_CmdHelp_DotDotDot(), CLI_CmdHelp_End(), CLI_ShowCmdHelp()
 ******************************************************************************/
void CLI_CmdHelp_Start(void)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_CmdHelp_Arg
 *
 * SYNOPSIS:
 *    void CLI_CmdHelp_Arg(const char *Label,const char *Desc);
 *
 * PARAMETERS:
 *    Label [I] -- What is the name of this arg
 *    Desc [I] -- The description of this arg
 *
 * FUNCTION:
 *    This function describes an argument for the current command.
 *
 * RETURNS:
 *    NONE
 *
 * EXAMPLE:
 *    The ALL CAPS is the what is effected by this function.
 *
 *    if(argc==0)
 *    {
 *      CLI_CmdHelp_Start();
 *
 *      CLI_CmdHelp_Arg("Disk","What disk to work on");
 *      CLI_CmdHelp_OptionString(0,"df0","Floppy disk 1");
 *      CLI_CmdHelp_OptionString(0,"df1","Floppy disk 2");
 *      CLI_CmdHelp_OptionString(0,"df0","Hard drive 1");
 *
 *      CLI_CmdHelp_Arg("Function","Preform disk functions");
 *      // Read fn
 *      CLI_CmdHelp_OptionString(1,"Read","Read from the disk");
 *      CLI_CmdHelp_SubArg("Offset","The offset into the disk to read");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to read");
 *      // Write fn
 *      CLI_CmdHelp_OptionString(1,"Write","Write to the disk");
 *      CLI_CmdHelp_SubArg("Offset","The number of bytes to write");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to write");
 *      // Format fn
 *      CLI_CmdHelp_OptionString(1,"Format","Format the disk");
 *
 *      CLI_HelpEnd(false);
 *      return;
 *    }
 *
 *    Outputs:
 *      USAGE:
 *          Cmd [Disk] [Function]
 *
 *      WHERE:
 *          DISK -- WHAT DISK TO WORK ON
 *              df0 -- Floppy disk 1
 *              df1 -- Floppy disk 2
 *              df0 -- Hard drive 1
 *          FUNCTION -- PREFORM DISK FUNCTIONS
 *                  Read -- Read from the disk
 *                          Offset -- The offset into the disk to read
 *                          Bytes -- The number of bytes to read
 *                  Write -- Write to the disk
 *                          Offset -- The number of bytes to write
 *                          Bytes -- The number of bytes to write
 *                  Format -- Format the disk
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_CmdHelp_Arg(const char *Label,const char *Desc)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_CmdHelp_SubArg
 *
 * SYNOPSIS:
 *    void CLI_CmdHelp_SubArg(const char *Label,const char *Desc);
 *
 * PARAMETERS:
 *    Label [I] -- What is the name of this arg
 *    Desc [I] -- The description of this arg
 *
 * FUNCTION:
 *    This function describes a sub argument to the last arg that was output.
 *    
 *    Sub Arguments are where the after the command takes the previous argument
 *    the meaning of following arguments change (arguments have different
 *    meaning depending on the arguments you send it).
 *
 *    For example the test command can take 2 different arg 1's:
 *          test Repeat 'A' 10
 *      OR
 *          test Print Version
 *
 *    In this case 'A' and 10 are SubArg's to "Repeat" and "Version" is a
 *    sub argument to "Print"
 *
 * RETURNS:
 *    NONE
 *
 * EXAMPLE:
 *    The ALL CAPS is the what is effected by this function.
 *
 *    if(argc==0)
 *    {
 *      CLI_CmdHelp_Start();
 *
 *      CLI_CmdHelp_Arg("Disk","What disk to work on");
 *      CLI_CmdHelp_OptionString(0,"df0","Floppy disk 1");
 *      CLI_CmdHelp_OptionString(0,"df1","Floppy disk 2");
 *      CLI_CmdHelp_OptionString(0,"df0","Hard drive 1");
 *
 *      CLI_CmdHelp_Arg("Function","Preform disk functions");
 *      // Read fn
 *      CLI_CmdHelp_OptionString(1,"Read","Read from the disk");
 *      CLI_CmdHelp_SubArg("Offset","The offset into the disk to read");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to read");
 *      // Write fn
 *      CLI_CmdHelp_OptionString(1,"Write","Write to the disk");
 *      CLI_CmdHelp_SubArg("Offset","The number of bytes to write");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to write");
 *      // Format fn
 *      CLI_CmdHelp_OptionString(1,"Format","Format the disk");
 *
 *      CLI_HelpEnd(false);
 *      return;
 *    }
 *
 *    Outputs:
 *      USAGE:
 *          Cmd [Disk] [Function]
 *
 *      WHERE:
 *          Disk -- What disk to work on
 *              df0 -- Floppy disk 1
 *              df1 -- Floppy disk 2
 *              df0 -- Hard drive 1
 *          Function -- Preform disk functions
 *                  Read -- Read from the disk
 *                          OFFSET -- THE OFFSET INTO THE DISK TO READ
 *                          BYTES -- THE NUMBER OF BYTES TO READ
 *                  Write -- Write to the disk
 *                          OFFSET -- THE NUMBER OF BYTES TO WRITE
 *                          BYTES -- THE NUMBER OF BYTES TO WRITE
 *                  Format -- Format the disk
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_CmdHelp_SubArg(const char *Label,const char *Desc)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_CmdHelp_OptionString
 *
 * SYNOPSIS:
 *    void CLI_CmdHelp_OptionString(int Level,const char *Option,
 *              const char *Desc);
 *
 * PARAMETERS:
 *    Level [I] -- What level does this option applies to. (This starts at
 *                 0 for the first arg (argv[1]) and so forth.
 *    Option [I] -- The option the user can input.  This is normally a string
 *                  the user can type out.
 *    Desc [I] -- The description for what this option does.
 *
 * FUNCTION:
 *    This function adds a value the user can input for an arguemnt.
 *
 *    For example if you had a "Print" argument that output different things
 *    depending on the argument after it you can use this to list the values
 *    the user can input.
 *          test Print Version
 *          test Print Status
 *          test Print NetworkStatus
 *      You use this function to add Version, Status, and NetworkStatus after
 *      the "Print" arg.
 *
 * RETURNS:
 *    NONE
 *
 * EXAMPLE:
 *    The ALL CAPS is the what is effected by this function.
 *
 *    if(argc==0)
 *    {
 *      CLI_CmdHelp_Start();
 *
 *      CLI_CmdHelp_Arg("Disk","What disk to work on");
 *      CLI_CmdHelp_OptionString(0,"df0","Floppy disk 1");
 *      CLI_CmdHelp_OptionString(0,"df1","Floppy disk 2");
 *      CLI_CmdHelp_OptionString(0,"df0","Hard drive 1");
 *
 *      CLI_CmdHelp_Arg("Function","Preform disk functions");
 *      // Read fn
 *      CLI_CmdHelp_OptionString(1,"Read","Read from the disk");
 *      CLI_CmdHelp_SubArg("Offset","The offset into the disk to read");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to read");
 *      // Write fn
 *      CLI_CmdHelp_OptionString(1,"Write","Write to the disk");
 *      CLI_CmdHelp_SubArg("Offset","The number of bytes to write");
 *      CLI_CmdHelp_SubArg("Bytes","The number of bytes to write");
 *      // Format fn
 *      CLI_CmdHelp_OptionString(1,"Format","Format the disk");
 *
 *      CLI_HelpEnd(false);
 *      return;
 *    }
 *
 *    Outputs:
 *      USAGE:
 *          Cmd [Disk] [Function]
 *
 *      WHERE:
 *          Disk -- What disk to work on
 *              DF0 -- FLOPPY DISK 1
 *              DF1 -- FLOPPY DISK 2
 *              DF0 -- HARD DRIVE 1
 *          Function -- Preform disk functions
 *                  READ -- READ FROM THE DISK
 *                          Offset -- The offset into the disk to read
 *                          Bytes -- The number of bytes to read
 *                  WRITE -- WRITE TO THE DISK
 *                          Offset -- The number of bytes to write
 *                          Bytes -- The number of bytes to write
 *                  FORMAT -- FORMAT THE DISK
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_CmdHelp_OptionString(int Level,const char *Option,const char *Desc)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_CmdHelp_DotDotDot
 *
 * SYNOPSIS:
 *    void CLI_CmdHelp_DotDotDot(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function outputs a ... at the end of the command.  With some commands
 *    you may want to let the user input an unlimited list of args this
 *    lets the user know they can put as many extra args as they want.
 *
 *    For example "echo" takes a number of args.
 *
 * RETURNS:
 *    NONE
 *
 * EXAMPLE:
 *    The ALL CAPS is the what is effected by this function.
 *
 *    CLI_CmdHelp_Start();
 *
 *    CLI_CmdHelp_Arg("Bytes","Bytes to write to device");
 *    CLI_CmdHelp_OptionString(0,"write","Write bytes");
 *    CLI_CmdHelp_DotDotDot();
 *
 *    CLI_HelpEnd();
 *
 *    Outputs:
 *      USAGE:
 *          Write2Dev [Bytes] ...
 *
 *      WHERE:
 *          Bytes -- Bytes to write to device
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_CmdHelp_DotDotDot(void)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_CmdHelp_End
 *
 * SYNOPSIS:
 *    void CLI_CmdHelp_End(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function ends the help.  After this you should just return from
 *    the command function.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_CmdHelp_End(void)
{
}

/*******************************************************************************
 * NAME:
 *    CLI_ShowCmdHelp
 *
 * SYNOPSIS:
 *    void CLI_ShowCmdHelp(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function displays the help for the current command being run.
 *
 *    The help system will call the command again with a argc==0 may times
 *    to output different parts of the help.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void CLI_ShowCmdHelp(void)
{
}

