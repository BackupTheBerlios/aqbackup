/***************************************************************************
 $RCSfile: inetaddr.h,v $
                             -------------------
    cvs         : $Id: inetaddr.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
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
 * @file chameleon/inetaddr.h
 * @short This file contains the internet address handling module
 */

#ifndef MOD_INETADDR_H
#define MOD_INETADDR_H


#include "chameleon/error.h"
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
 * @defgroup inetaddr Internet Address Module
 * @short This module handles internet addresses
 *
 * This module allows using of internet IP addresses. It is also capable of
 * resolving addresses and hostnames.
 * @author Martin Preuss<martin@libchipcard.de>
 */
//@{

#define INETADDR_ERROR_MEMORY_FULL          1
#define INETADDR_ERROR_BAD_ADDRESS          2
#define INETADDR_ERROR_BUFFER_OVERFLOW      3
#define INETADDR_ERROR_HOST_NOT_FOUND       4
#define INETADDR_ERROR_NO_ADDRESS           5
#define INETADDR_ERROR_NO_RECOVERY          6
#define INETADDR_ERROR_TRY_AGAIN            7
#define INETADDR_ERROR_UNKNOWN_DNS_ERROR    8
#define INETADDR_ERROR_BAD_ADDRESS_FAMILY   9
#define INETADDR_ERROR_UNSUPPORTED          10

/**
 * Address family (in most cases this is AddressFamilyIP)
 */
typedef enum {
  AddressFamilyIP=0,
  AddressFamilyUnix
} AddressFamily;


/**
 * @internal
 */
CHIPCARD_API struct INETADDRESSSTRUCT {
  AddressFamily af;
  int size;
  struct sockaddr *address;
};

/**
 * You shoukd treat this type as opaque. Its members are not part of the API,
 * i.e. they are subject to changes without notice !
 */
CHIPCARD_API typedef struct INETADDRESSSTRUCT INETADDRESS;

/**
 * Just a pointer to an INETADDRESS for conveniance.
 */
CHIPCARD_API typedef INETADDRESS *INETADDRESSPTR;


/**
 * @name Initializing
 *
 * These functions are converned with initialisation issues.
 */
//@{

/**
 * Initialize this module.
 */
CHIPCARD_API ERRORCODE InetAddr_ModuleInit();

/**
 * De-Initialize this module.
 */
CHIPCARD_API ERRORCODE InetAddr_ModuleFini();
//@}


/**
 * @name Construction and destruction
 *
 * These functions allocate and free administrative data about IP addresses.
 */
//@{

CHIPCARD_API INETADDRESS *InetAddr_new(AddressFamily af);
CHIPCARD_API void InetAddr_free(INETADDRESS *ia);


/**
 * Deinitializes an internet address structure. This frees all ressources
 * allocated by a previous call to @ref InetAddr_Create.
 * @param ia pointer to a INETADDRESS variable
 */
CHIPCARD_API ERRORCODE InetAddr_Destroy(INETADDRESSPTR ia);

//@}


/**
 * @name Get and set address
 *
 * These functions allow getting and setting of IP addresses either by
 * hostname or host address.
 */
//@{

/**
 * Sets the IP address.
 * @return error code
 * @param ia INETADDRESS to manipulate
 * @param addr IP address in 3-dot-notation ("1.2.3.4")
 */
CHIPCARD_API ERRORCODE InetAddr_SetAddress(INETADDRESSPTR ia, const char *addr);

/**
 * Sets the IP name and resolves its address.
 * @return error code
 * @param ia INETADDRESS to manipulate
 * @param name hostname whose address is to be resolved in 3-dot-notation
 */
CHIPCARD_API ERRORCODE InetAddr_SetName(INETADDRESSPTR ia, const char *name);

/**
 * Gets the IP address stored in the INETADDRESS.
 * @return error code
 * @param ia INETADDRESS to use
 * @param buffer pointer to a buffer to receive the address
 * @param bsize size of the buffer in bytes
 */
CHIPCARD_API ERRORCODE InetAddr_GetAddress(const INETADDRESS *ia, char *buffer, int bsize);

/**
 * Gets the host name stored in the INETADDRESS. If there is none, then the
 * IP address stored in the INETADDRESS will be used to resolve the hostname.
 * @return error code
 * @param ia INETADDRESS to use
 * @param buffer pointer to a buffer to receive the name
 * @param bsize size of the buffer in bytes
 */
CHIPCARD_API ERRORCODE InetAddr_GetName(const INETADDRESS *ia, char *buffer, int bsize);
//@}

/**
 * @name Get and set port
 *
 * These functions allow getting and setting of the port.
 */
//@{

/**
 * Return the port stored in the INETADDRESS
 * @param ia INETADDRESS to use
 */
CHIPCARD_API int InetAddr_GetPort(const INETADDRESS *ia);

/**
 * Set the port in the given INETADDRESS.
 * @return error code
 * @param ia INETADDRESS to manipulate
 * @param port port to set (0-65535)
 */
CHIPCARD_API ERRORCODE InetAddr_SetPort(INETADDRESSPTR ia, int port);
//@}

#ifdef __cplusplus
}
#endif

//@} defgroup


#endif // MOD_INETADDR_H




