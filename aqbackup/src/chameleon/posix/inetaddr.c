/***************************************************************************
 $RCSfile: inetaddr.c,v $
                             -------------------
    cvs         : $Id: inetaddr.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __declspec
# if BUILDING_CHIPCARD_DLL
#  define CHIPCARD_API __declspec (dllexport)
# else /* Not BUILDING_CHIPCARD_DLL */
#  define CHIPCARD_API __declspec (dllimport)
# endif /* Not BUILDING_CHIPCARD_DLL */
#else
# define CHIPCARD_API
#endif


#include "chameleon/inetaddr.h"
#include <stdlib.h>
#include <assert.h>
#include <netdb.h>
#include <string.h>
#include <sys/un.h>

#include "chameleon/debug.h"

#define MEMTRACE

// forward declaration
const char *InetAddr_ErrorString(int c);

#ifdef MEMTRACE
static int INETADDR__Counter=0;
#endif


int inetaddr_is_initialized=0;
ERRORTYPEREGISTRATIONFORM inetaddr_error_descr= {
    InetAddr_ErrorString,
    0,
    "InetAddr"};



ERRORCODE InetAddr_ModuleInit(){
    if (!inetaddr_is_initialized) {
	if (!Error_RegisterType(&inetaddr_error_descr))
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     ERROR_TYPE_ERROR,
			     ERROR_COULD_NOT_REGISTER);
	inetaddr_is_initialized=1;
    }
    return 0;
}


ERRORCODE InetAddr_ModuleFini(){
    if (inetaddr_is_initialized) {
	inetaddr_is_initialized=0;
	if (!Error_UnregisterType(&inetaddr_error_descr))
	    return Error_New(0,
			     ERROR_SEVERITY_ERR,
			     ERROR_TYPE_ERROR,
			     ERROR_COULD_NOT_UNREGISTER);
    }
    return 0;
}


const char *InetAddr_ErrorString(int c) {
  const char *s;

  switch(c) {
  case INETADDR_ERROR_MEMORY_FULL:
    s="Memory full";
    break;
  case INETADDR_ERROR_BAD_ADDRESS:
    s="Bad address";
    break;
  case INETADDR_ERROR_BUFFER_OVERFLOW:
    s="Buffer overflow";
    break;
  case INETADDR_ERROR_HOST_NOT_FOUND:
    s="Host not found";
    break;
  case INETADDR_ERROR_NO_ADDRESS:
    s="No address";
    break;
  case INETADDR_ERROR_NO_RECOVERY:
    s="No recovery";
    break;
  case INETADDR_ERROR_TRY_AGAIN:
    s="Try again";
    break;
  case INETADDR_ERROR_UNKNOWN_DNS_ERROR:
    s="Unknown DNS error";
    break;
  case INETADDR_ERROR_BAD_ADDRESS_FAMILY:
    s="Unknown address family";
    break;
  default:
    s=(const char*)0;
  } // switch
  return s;
}


INETADDRESS *InetAddr_new(AddressFamily af){
  INETADDRESS *ia;

  ia=(INETADDRESS *)malloc(sizeof(INETADDRESS));
  assert(ia);
  memset(ia,0,sizeof(INETADDRESS));
  ia->af=af;
  switch(ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    ia->address=(struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    assert(ia->address);
    aptr=(struct sockaddr_in*)(ia->address);
    ia->size=sizeof(struct sockaddr_in);
    memset(ia->address,0,ia->size);
#ifdef PF_INET
    aptr->sin_family=PF_INET;
#else
    aptr->sin_family=AF_INET;
#endif
    break;
  }

  case AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    ia->address=(struct sockaddr*)malloc(sizeof(struct sockaddr_un));
    assert(ia->address);
    aptr=(struct sockaddr_un*)(ia->address);
#ifdef PF_UNIX
    aptr->sun_family=PF_UNIX;
#else
    aptr->sun_family=AF_UNIX;
#endif
    aptr->sun_path[0]=0;
    ia->size=sizeof(struct sockaddr_un);
    memset(ia->address,0,ia->size);
    break;
  }

  default:
    DBG_ERROR("Unknown address family (%d)",af);
    assert(0);
  } /* switch */
#ifdef MEMTRACE
  INETADDR__Counter++;
  DBG_INFO("InetAddr created, now %d",INETADDR__Counter);
#endif
  return ia;
}


void InetAddr_free(INETADDRESS *ia){
  if (ia) {
#ifdef MEMTRACE
    INETADDR__Counter--;
    DBG_INFO("Free InetAddr, makes %d",INETADDR__Counter);
#endif
    free(ia->address);
    free(ia);
  }
}


ERRORCODE InetAddr_SetAddress(INETADDRESSPTR ia, const char *addr){
  assert(ia);

  switch (ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    // reset
#ifdef PF_INET
    aptr->sin_family=PF_INET;
#else
    aptr->sin_family=AF_INET;
#endif
    aptr->sin_addr.s_addr=0;

    if (addr) {
      // ok, address to be set
      if (!inet_aton(addr,&aptr->sin_addr))
	// bad address, so maybe it rather is a name
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 inetaddr_error_descr.typ,
			 INETADDR_ERROR_BAD_ADDRESS);
    }
    break;
  }

  case AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    aptr=(struct sockaddr_un*)(ia->address);
#ifdef PF_UNIX
    aptr->sun_family=PF_UNIX;
#else
    aptr->sun_family=AF_UNIX;
#endif
    aptr->sun_path[0]=0;

    if (addr) {
      // ok, address to be set
      if ((strlen(addr)+1)>sizeof(aptr->sun_path)) {
	// bad address
	DBG_ERROR("Path too long (%d>%d)",
		  strlen(addr)+1,sizeof(aptr->sun_path));
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 inetaddr_error_descr.typ,
			 INETADDR_ERROR_BAD_ADDRESS);
      }
      strcpy(aptr->sun_path,addr);
      ia->size=SUN_LEN(aptr);
    }
    break;
  }
  default:
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     inetaddr_error_descr.typ,
		     INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */

  return 0;
}


// internal function
int InetAddr_TranslateHError(int herr) {
  int rv;

  switch(herr) {
  case HOST_NOT_FOUND:
    rv=INETADDR_ERROR_HOST_NOT_FOUND;
    break;
#ifdef NO_ADDRESS
  case NO_ADDRESS:
    rv=INETADDR_ERROR_NO_ADDRESS;
    break;
#endif
  case NO_RECOVERY:
	rv=INETADDR_ERROR_NO_RECOVERY;
	break;
    case TRY_AGAIN:
	rv=INETADDR_ERROR_TRY_AGAIN;
	break;
    default:
	rv=INETADDR_ERROR_UNKNOWN_DNS_ERROR;
	break;
    } // switch
    return rv;
}


ERRORCODE InetAddr_SetName(INETADDRESSPTR ia, const char *name){
  struct hostent *he;

  assert(ia);

  switch (ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    // try to resolve name
    he=gethostbyname(name);
    if (!he)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       inetaddr_error_descr.typ,
		       InetAddr_TranslateHError(h_errno));
    // name resolved, store address
    memcpy(&(aptr->sin_addr),
	   he->h_addr_list[0],
	   sizeof(struct in_addr));
    break;
  }

  case AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    aptr=(struct sockaddr_un*)(ia->address);
#ifdef PF_INET
    aptr->sun_family=PF_INET;
#else
    aptr->sun_family=AF_INET;
#endif
    aptr->sun_path[0]=0;

    if (name) {
      // ok, address to be set
      if ((strlen(name)+1)>sizeof(aptr->sun_path)) {
	// bad address
	DBG_ERROR("Path too long (%d>%d)",
		  strlen(name)+1,sizeof(aptr->sun_path));
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 inetaddr_error_descr.typ,
			 INETADDR_ERROR_BAD_ADDRESS);
      }
      strcpy(aptr->sun_path,name);
      ia->size=SUN_LEN(aptr);
    }
    break;
  }

  default:
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     inetaddr_error_descr.typ,
		     INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */
  return 0;
}


ERRORCODE InetAddr_GetAddress(const INETADDRESS *ia, char *buffer, int bsize){
  const char *s;

  assert(ia);
  assert(buffer);

  switch (ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    s=inet_ntoa(aptr->sin_addr);
    assert(s);
    if (strlen(s)+1>bsize)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       inetaddr_error_descr.typ,
		       INETADDR_ERROR_BUFFER_OVERFLOW);
    strcpy(buffer,s);
    break;
  }

  case AddressFamilyUnix: {
    struct sockaddr_un *aptr;
    int i;

    aptr=(struct sockaddr_un*)(ia->address);
    s=aptr->sun_path;
    i=ia->size;
    i-=sizeof(aptr->sun_family);
    if (i+1>bsize)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       inetaddr_error_descr.typ,
		       INETADDR_ERROR_BUFFER_OVERFLOW);
    memmove(buffer,s,i);
    buffer[i]=0;
    break;
  }

  default:
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     inetaddr_error_descr.typ,
		     INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */
  return 0;
}


ERRORCODE InetAddr_GetName(const INETADDRESS *ia, char *buffer, int bsize){
  struct hostent *he;
  struct in_addr lia;

  assert(ia);
  assert(buffer);

  switch (ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    // resolve name from address
    lia=aptr->sin_addr;
#ifdef PF_INET
    he=gethostbyaddr((char*)&lia,sizeof(lia),PF_INET);
#else
    he=gethostbyaddr((char*)&lia,sizeof(lia),AF_INET);
#endif
    if (!he)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       inetaddr_error_descr.typ,
		       InetAddr_TranslateHError(h_errno));

    // copy name into given buffer
    assert(he->h_name);
    if (strlen(he->h_name)+1>bsize)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       inetaddr_error_descr.typ,
		       INETADDR_ERROR_BUFFER_OVERFLOW);
    // copy the name into the buffer
    strcpy(buffer,he->h_name);
    break;
  }

  case AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    aptr=(struct sockaddr_un*)(ia->address);
    /* simply copy path */
    if (strlen(aptr->sun_path)+1>bsize)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       inetaddr_error_descr.typ,
		       INETADDR_ERROR_BUFFER_OVERFLOW);
    strcpy(buffer,aptr->sun_path);
    break;
  }

  default:
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     inetaddr_error_descr.typ,
		     INETADDR_ERROR_BAD_ADDRESS_FAMILY);

  } /* switch */

  // finished
  return 0;
}


int InetAddr_GetPort(const INETADDRESS *ia){
  int i;

  assert(ia);

  switch (ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    i=ntohs(aptr->sin_port);
    break;
  }

  default:
    DBG_WARN("not an IP address");
    i=0;
  } /* switch */

  return i;
}


ERRORCODE InetAddr_SetPort(INETADDRESSPTR ia, int port){
  assert(ia);

  switch (ia->af) {
  case AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    aptr->sin_port=htons(port);
    break;
  }

  default:
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     inetaddr_error_descr.typ,
		     INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */
  return 0;
}






