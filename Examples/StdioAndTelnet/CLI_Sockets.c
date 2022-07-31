/*******************************************************************************
 * FILENAME: CLI_Sockets.c
 *
 * PROJECT:
 *    MyCLI
 *
 * FILE DESCRIPTION:
 *    This file has the IO for the CLI in it.
 *
 * COPYRIGHT:
 *    Copyright 2022 Paul Hutchinson
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
 *    Paul Hutchinson (30 Jul 2022)
 *
 ******************************************************************************/

/*** HEADER FILES TO INCLUDE  ***/
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>

#include "main.h"
#include "CLI_Sockets.h"
#include "stdbool.h"

/*** DEFINES                  ***/
//#define TELNET_PORT                             23
#define TELNET_PORT                             2323

/*** MACROS                   ***/

/*** TYPE DEFINITIONS         ***/

/*** FUNCTION PROTOTYPES      ***/

/*** VARIABLE DEFINITIONS     ***/
int m_TelnetListenSocket=-1;
int m_TelnetConnectionSocket=-1;

/*******************************************************************************
 * NAME:
 *    InitTelnetSocket
 *
 * SYNOPSIS:
 *    bool InitTelnetSocket(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function allocates the telnet listening socket and reads it for
 *    new connections.
 *
 * RETURNS:
 *    true -- Telnet socket was allocated and is ready to go
 *    false -- There was an error.
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
bool InitTelnetSocket(void)
{
    struct sockaddr_in servaddr;
    int SetOptions;
    int opt;

    // socket create and verification
    m_TelnetListenSocket=socket(AF_INET,SOCK_STREAM,0);
    if(m_TelnetListenSocket<0)
        return false;

    SetOptions=SO_REUSEADDR|SO_REUSEPORT;

    opt=1;
    if(setsockopt(m_TelnetListenSocket,SOL_SOCKET,SetOptions,&opt,sizeof(opt)))
    {
        close(m_TelnetListenSocket);
        m_TelnetListenSocket=-1;
        return false;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(TELNET_PORT);

    // Binding newly created socket to given IP and verification
    if((bind(m_TelnetListenSocket,(struct sockaddr *)&servaddr,
            sizeof(servaddr)))!=0)
    {
        close(m_TelnetListenSocket);
        m_TelnetListenSocket=-1;
        return false;
    }

    // Now server is ready to listen and verification
    if((listen(m_TelnetListenSocket,1))!=0)
    {
        close(m_TelnetListenSocket);
        m_TelnetListenSocket=-1;
        return false;
    }

    return true;
}

/*******************************************************************************
 * NAME:
 *    ShutdownTelnetSocket
 *
 * SYNOPSIS:
 *    void ShutdownTelnetSocket(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function closes the telnet socket and connections.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void ShutdownTelnetSocket(void)
{
    close(m_TelnetListenSocket);
    if(m_TelnetConnectionSocket>=0)
        close(m_TelnetConnectionSocket);
}

/*******************************************************************************
 * NAME:
 *    TelnetSocketTick
 *
 * SYNOPSIS:
 *    void TelnetSocketTick(void);
 *
 * PARAMETERS:
 *    NONE
 *
 * FUNCTION:
 *    This function is the tick function for the telnet sockets.  It handles
 *    new connections coming in.  It needs to be called regularly.
 *
 * RETURNS:
 *    NONE
 *
 * SEE ALSO:
 *    
 ******************************************************************************/
void TelnetSocketTick(void)
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    socklen_t addrlen;
    struct sockaddr_in NewConnectionAddr;

    if(m_TelnetListenSocket<0)
        return;

    /* Only accept new connections if we don't already have one */
    if(m_TelnetConnectionSocket<0)
    {
        /* Waiting for a connection */
        FD_ZERO(&rfds);
        FD_SET(m_TelnetListenSocket,&rfds);

        /* Return right away */
        tv.tv_sec=0;
        tv.tv_usec=0;

        retval=select(m_TelnetListenSocket+1,&rfds,NULL,NULL,&tv);
        if(retval==0)
        {
            /* Timeout */
        }
        else if(retval==-1)
        {
            /* Error */
        }
        else
        {
            /* New connection available */
            addrlen = sizeof(NewConnectionAddr);
            m_TelnetConnectionSocket=accept(m_TelnetListenSocket,
                    (struct sockaddr *)&NewConnectionAddr,&addrlen);

            Switch2Telnet();

            CLI_SendTelnetInitConnectionMsg(g_Prompt);

            /* Send out a banner */
            write(m_TelnetConnectionSocket,"Welcome to MyCLI\r\n",18);
            CLI_DrawPrompt(g_Prompt);
        }
    }
}

bool TelnetSocket_IsCharAvailable(void)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;

    if(m_TelnetConnectionSocket<0)
        return false;

    /* Using telnet */
    FD_ZERO(&fds);
    FD_SET(m_TelnetConnectionSocket,&fds);

    return select(m_TelnetConnectionSocket+1,&fds,NULL,NULL,&tv);
}

char TelnetSocket_GetChar(void)
{
    int bytes;
    unsigned char c;

    if(m_TelnetConnectionSocket<0)
        return 0;

    /* Telnet */
    bytes=read(m_TelnetConnectionSocket,&c,1);
    if(bytes<0)
    {
        c=0;
    }
    else if(bytes==0)
    {
        /* 0=connection closed (because we where already told there was
           data) */
        close(m_TelnetConnectionSocket);
        m_TelnetConnectionSocket=-1;

        c=0;
        Switch2Stdio();
    }
    return c;
}

void TelnetSocket_PutChar(char c)
{
    if(m_TelnetConnectionSocket<0)
        return;

    if(c=='\n')
        write(m_TelnetConnectionSocket,"\r",1);
    write(m_TelnetConnectionSocket,&c,1);
}

void TelnetSocket_CloseConnection(void)
{
    if(m_TelnetConnectionSocket<0)
        return;

    Switch2Stdio();

    close(m_TelnetConnectionSocket);
    m_TelnetConnectionSocket=-1;
}
