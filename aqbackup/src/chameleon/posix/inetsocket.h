/***************************************************************************
 $RCSfile: inetsocket.h,v $
                             -------------------
    cvs         : $Id: inetsocket.h,v 1.1 2003/06/07 21:07:51 aquamaniac Exp $
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file chameleon/socket.h
 * @short This file contains sockets and socket sets.
 */

#ifndef MOD_SOCKET_H
#define MOD_SOCKET_H

#include "chameleon/error.h"
#include "chameleon/inetaddr.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup socksandsets Sockets and Socket Sets
 *
 * This module handles sockets and socket sets.
 * @{
 */

#define SOCKET_ERROR_TYPE "Socket"
#define SOCKET_ERROR_BAD_SOCKETTYPE (-1)
#define SOCKET_ERROR_NOT_OPEN       (-2)
#define SOCKET_ERROR_TIMEOUT        (-3)
#define SOCKET_ERROR_IN_PROGRESS    (-4)
#define SOCKET_ERROR_STARTUP        (-5)
#define SOCKET_ERROR_INTERRUPTED    (-6)
#define SOCKET_ERROR_UNSUPPORTED    (-7)


/**
 *
 */
CHIPCARD_API typedef enum {
  SocketTypeTCP=1,
  SocketTypeUDP,
  SocketTypeRAW,
  SocketTypeUnix
} SOCKETTYPE;


CHIPCARD_API struct SOCKETSTRUCT {
  int socket;
  int type;
};

CHIPCARD_API typedef struct SOCKETSTRUCT* SOCKETPTR;

CHIPCARD_API struct SOCKETSETSTRUCT {
  fd_set set;
  int highest;
};

CHIPCARD_API typedef struct SOCKETSETSTRUCT SOCKETSET;
CHIPCARD_API typedef SOCKETSET* SOCKETSETPTR;


/**
 * Initializes this module.
 */
CHIPCARD_API ERRORCODE Socket_ModuleInit();

/**
 * Deinitializes this module.
 */
CHIPCARD_API ERRORCODE Socket_ModuleFini();

/**
 * @defgroup socketset Socket Set Functions
 *
 * These functions operate on socket sets. A socket set is used by the socket
 * function @ref Socket_Select() to check on which socket changes in state
 * occurred.
 * @{
 */

/**
 * @name Creation and destruction
 *
 * These functions initialize and de-initialize socket sets.
 * A socket set is a group of sockets. They are used for the function
 * @ref Socket_Select.
 */
//@{
CHIPCARD_API ERRORCODE SocketSet_Create(SOCKETSETPTR ssp);
CHIPCARD_API ERRORCODE SocketSet_Destroy(SOCKETSETPTR ssp);
CHIPCARD_API ERRORCODE SocketSet_Clear(SOCKETSETPTR ssp);
//@}

/**
 * @name Add, remove, check sockets
 *
 * These functions allow adding and removing sockets to/from a socket set
 * as well as checking whether a specific socket is part of a socket set.
 */
//@{
CHIPCARD_API ERRORCODE SocketSet_AddSocket(SOCKETSETPTR ssp,
					   const struct SOCKETSTRUCT *sp);
CHIPCARD_API ERRORCODE SocketSet_RemoveSocket(SOCKETSETPTR ssp,
					      const struct SOCKETSTRUCT *sp);
CHIPCARD_API int SocketSet_HasSocket(SOCKETSETPTR ssp,
				     const struct SOCKETSTRUCT *sp);
//@}
//end of group socketset
//@}


/**
 * @defgroup socket Socket Functions
 *
 * This group operates on IP sockets.
 * @{
 */

/**
 * @name Creation and Destruction
 */
//@{

/**
 * Constructor. You should always use this to create socket variables.
 */
CHIPCARD_API SOCKETPTR Socket_new();

/**
 * Destructor.
 */
CHIPCARD_API void Socket_free(SOCKETPTR sp);

/**
 * Arms the socket so that it can be used. This really creates a system
 * socket.
 */
CHIPCARD_API ERRORCODE Socket_Open(SOCKETPTR sp, SOCKETTYPE socketType);

/**
 * Unarms a socket thus closing any connection associated with this socket.
 */
CHIPCARD_API ERRORCODE Socket_Close(SOCKETPTR sp);
//@}

/**
 * @name Connecting and Disconnecting
 *
 * These functions allow active and passive connections to other hosts.
 */
//@{
CHIPCARD_API ERRORCODE Socket_Connect(SOCKETPTR sp, const INETADDRESS *addr);
CHIPCARD_API ERRORCODE Socket_Bind(SOCKETPTR sp, const INETADDRESS *addr);
CHIPCARD_API ERRORCODE Socket_Listen(SOCKETPTR sp, int backlog);
CHIPCARD_API ERRORCODE Socket_Accept(SOCKETPTR sp, INETADDRESSPTR addr, SOCKETPTR newsock);
//@}

/**
 * @name Informational Functions
 *
 * These functions return some usefull information about sockets or
 * connections.
 */
//@{

CHIPCARD_API SOCKETTYPE Socket_GetSocketType(SOCKETPTR sp);

CHIPCARD_API ERRORCODE Socket_GetPeerAddr(SOCKETPTR sp, INETADDRESSPTR addr);

/**
 * This function waits for a group of sockets to change their state.
 * @param rs socket set, wait for readability of those sockets
 * @param ws socket set, wait for writeability of those sockets
 * @param xs socket set, wait for "exceptional events" on those sockets
 * @param timeout time to wait in milliseconds. If <0 then this function
 * will wait forever, if ==0 then it won't wait at all.
 */
CHIPCARD_API ERRORCODE Socket_Select(SOCKETSETPTR rs,
				     SOCKETSETPTR ws,
				     SOCKETSETPTR xs,
				     int timeout);

/**
 * Wait until the given socket becomes readable or a timeout occurrs.
 * @param timout please see @ref Socket_Select for details
 */
CHIPCARD_API ERRORCODE Socket_WaitForRead(SOCKETPTR sp,int timeout);

/**
 * Wait until the given socket becomes writeable or a timeout occurrs.
 * @param timout please see @ref Socket_Select for details
 */
CHIPCARD_API ERRORCODE Socket_WaitForWrite(SOCKETPTR sp,int timeout);
//@}

/**
 * @name Data Exchange Functions
 *
 * These functions handle exchange of data with other hosts via the Internet
 * Protocol.
 */
//@{

/**
 * Read bytes from a socket.
 * @param buffer pointer to the buffer to receive the data
 * @param bsize pointer to an integer variable. Upon call this should hold
 * the number of bytes to read, upon return it will contain the number of
 * bytes actually read.
 */
CHIPCARD_API ERRORCODE Socket_Read(SOCKETPTR sp,
				   char *buffer,
				   int *bsize);

/**
 * Write bytes to an open socket.
 * @param buffer pointer to a buffer containing the bytes to be written
 * @param bsize pointer to an integer variable containing the number of bytes
 * to write. Upon return this variable holds the number of bytes actually
 * written. Please note that this function may write less bytes than expected!
 */
CHIPCARD_API ERRORCODE Socket_Write(SOCKETPTR sp,
				    const char *buffer,
				    int *bsize);

/**
 * Reads bytes from an UDP socket, which is connectionless.
 * @param addr pointer to an address struct to receive the address of the
 * peer we have received data from
 * @param buffer pointer to a buffer to store the received data in
 * @param bsize pointer to an integer variable. Upon call this should hold
 * the number of bytes to read, upon return it will contain the number of
 * bytes actually read.
 */
CHIPCARD_API ERRORCODE Socket_ReadFrom(SOCKETPTR sp,
				       INETADDRESSPTR addr,
				       char *buffer,
				       int *bsize);
/**
 * Writes data to an UDP socket, which is connectionless.
 * @param addr pointer to the address struct specifying the recipient
 * @param buffer pointer to a buffer containing the bytes to be written
 * @param bsize pointer to an integer variable containing the number of bytes
 * to write. Upon return this variable holds the number of bytes actually
 * written. Please note that this function may write less bytes than expected!
  */
CHIPCARD_API ERRORCODE Socket_WriteTo(SOCKETPTR sp,
				      const INETADDRESS *addr,
				      const char *buffer,
				      int *bsize);
//@}

/**
 * @name Socket Settings Functions
 *
 * These functions manipulate settings on a socket.
 */
//@{
CHIPCARD_API ERRORCODE Socket_SetBlocking(SOCKETPTR sp,
					  int fl);
CHIPCARD_API ERRORCODE Socket_SetBroadcast(SOCKETPTR sp,
					   int fl);
CHIPCARD_API ERRORCODE Socket_GetSocketError(SOCKETPTR sp);

CHIPCARD_API ERRORCODE Socket_SetReuseAddress(SOCKETPTR sp, int fl);
//@}

// end of group socket
//@}

// end of group socketsandsets
//@}

#ifdef __cplusplus
}
#endif

#endif // MOD_SOCKET_H



