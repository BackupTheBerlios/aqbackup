/***************************************************************************
 $RCSfile: ipctransportlayer.c,v $
                             -------------------
    cvs         : $Id: ipctransportlayer.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Wed Nov 06 2002
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


#include <chameleon/ipctransportlayer.h>
#include <chameleon/ipcmessage.h>
#include <chameleon/debug.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>


/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                   General Transport layer functions
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


ERRORCODE IPC_TransportLayer_GetAddress(IPCTRANSPORTLAYERTABLE *tl,
					char *buffer,
					int bsize){
  int s;

  assert(tl);
  assert(buffer);
  s=strlen(tl->address)+1;
  if (bsize<s)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType("IPC"),
		     IPCMESSAGE_ERROR_BUFFER_TOO_SMALL);
  memmove(buffer, tl->address, s);
  return 0;
}


ERRORCODE IPC_TransportLayer_SetAddress(IPCTRANSPORTLAYERTABLE *tl,
					const char *buffer) {
  int s;

  assert(tl);
  assert(buffer);
  s=strlen(buffer)+1;
  if (s>sizeof(tl->address))
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType("IPC"),
		     IPCMESSAGE_ERROR_BUFFER_TOO_SMALL);
  memmove(tl->address, buffer, s);
  return 0;
}


int IPC_TransportLayer_GetPort(IPCTRANSPORTLAYERTABLE *tl){
  assert(tl);
  return tl->port;
}


void IPC_TransportLayer_SetPort(IPCTRANSPORTLAYERTABLE *tl, int port){
  assert(tl);
  tl->port=port;
}


TransportLayerType IPC_TransportLayer_GetType(IPCTRANSPORTLAYERTABLE *tl){
  assert(tl);
  return tl->type;
}





/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *               Transport layer for Unix Domain Sockets
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



ERRORCODE IPC_TransportLayerTCP_StartConnect(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;
  INETADDRESS *addr;

  assert(tl);

  if (tl->privateData==0) {
    tl->privateData=Socket_new();
    err=Socket_Open((struct SOCKETSTRUCT*)(tl->privateData),SocketTypeTCP);
    if (!Error_IsOk(err))
      return err;
  }

  err=Socket_SetBlocking((SOCKETPTR)(tl->privateData),0);
  if (!Error_IsOk(err))
    return err;

  addr=InetAddr_new(AddressFamilyIP);
  err=InetAddr_SetAddress(addr, tl->address);
  if (!Error_IsOk(err))
    err=InetAddr_SetName(addr,tl->address);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    return err;
  }
  err=InetAddr_SetPort(addr, tl->port);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    return err;
  }

  err=Socket_Connect((SOCKETPTR)(tl->privateData),addr);
  InetAddr_free(addr);
  // not yet finished or real error ?
  if (Error_GetType(err)!=Error_FindType("Socket") ||
      Error_GetCode(err)!=SOCKET_ERROR_IN_PROGRESS) {
    // real error, so return that error
    return err;
  }

  return 0;
}


ERRORCODE IPC_TransportLayerTCP_FinishConnect(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;

  assert(tl);

  // get socket error
  err=Socket_GetSocketError((SOCKETPTR)(tl->privateData));
  if (!Error_IsOk(err))
    return err;

  // make socket blocking again
  err=Socket_SetBlocking((SOCKETPTR)(tl->privateData),1);
  if (!Error_IsOk(err))
    return err;
  return 0;
}


ERRORCODE IPC_TransportLayerTCP_Listen(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;
  INETADDRESS *addr;

  assert(tl);

  if (tl->privateData==0) {
    tl->privateData=Socket_new();
    err=Socket_Open((struct SOCKETSTRUCT*)(tl->privateData),SocketTypeTCP);
    if (!Error_IsOk(err))
      return err;
  }

  /* create inet address */
  addr=InetAddr_new(AddressFamilyIP);
  err=InetAddr_SetAddress(addr, tl->address);
  if (!Error_IsOk(err))
    err=InetAddr_SetName(addr,tl->address);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    return err;
  }
  err=InetAddr_SetPort(addr, tl->port);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    return err;
  }

  err=Socket_SetReuseAddress((struct SOCKETSTRUCT*)(tl->privateData), 1);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    return err;
  }

  err=Socket_Bind((SOCKETPTR)(tl->privateData), addr);
  InetAddr_free(addr);
  if (!Error_IsOk(err))
    return err;

  err=Socket_Listen((SOCKETPTR)(tl->privateData), 10);
  if (!Error_IsOk(err))
    return err;
  return 0;
}


ERRORCODE IPC_TransportLayerTCP_Accept(IPCTRANSPORTLAYERTABLE *tl,
				       struct IPCTRANSPORTLAYERTABLESTRUCT **t){
  SOCKETPTR s;
  ERRORCODE err;
  INETADDRESS *iaddr;

  assert(tl);

  s=Socket_new();
  iaddr=InetAddr_new(AddressFamilyIP);

  err=Socket_Accept((SOCKETPTR)(tl->privateData), iaddr,s);
  InetAddr_free(iaddr);
  if (!Error_IsOk(err)) {
    Socket_free(s);
    return err;
  }
  *t=IPC_TransportLayerTCP_new();
  (*t)->privateData=s;
  return 0;
}


ERRORCODE IPC_TransportLayerTCP_Disconnect(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;

  assert(tl);

  if (tl->privateData) {
    err=Socket_Close((SOCKETPTR)(tl->privateData));
    Socket_free((SOCKETPTR)(tl->privateData));
    tl->privateData=0;
    if (!Error_IsOk(err))
      return err;
  }
  return 0;
}


struct SOCKETSTRUCT*
IPC_TransportLayerTCP_GetSocket(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;

  assert(tl);

  if (tl->privateData==0) {
    tl->privateData=Socket_new();
    err=Socket_Open((struct SOCKETSTRUCT*)(tl->privateData),SocketTypeTCP);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      return 0;
    }
  }
  return (struct SOCKETSTRUCT*)(tl->privateData);
}


ERRORCODE IPC_TransportLayerTCP_Read(IPCTRANSPORTLAYERTABLE *tl,
				  char *buffer,
				     int *bsize){
  assert(tl);
  return Socket_Read((SOCKETPTR)(tl->privateData),buffer,bsize);
}


ERRORCODE IPC_TransportLayerTCP_Write(IPCTRANSPORTLAYERTABLE *tl,
				      const char *buffer,
				      int *bsize){
  assert(tl);
  return Socket_Write((SOCKETPTR)(tl->privateData),buffer,bsize);
}


ERRORCODE IPC_TransportLayerTCP_CanRead(IPCTRANSPORTLAYERTABLE *tl){
  return Socket_WaitForRead((SOCKETPTR)(tl->privateData),0);
}


ERRORCODE IPC_TransportLayerTCP_CanWrite(IPCTRANSPORTLAYERTABLE *tl){
  assert(tl);

  return Socket_WaitForWrite((SOCKETPTR)(tl->privateData),0);
}


ERRORCODE IPC_TransportLayerTCP_GetPeerAddress(IPCTRANSPORTLAYERTABLE *tl,
					       char *buffer,
					       int bsize){
  ERRORCODE err;
  INETADDRESS *addr;

  assert(tl);

  addr=InetAddr_new(AddressFamilyIP);
  /* get peer address */
  err=Socket_GetPeerAddr((SOCKETPTR)(tl->privateData), addr);
  if (!Error_IsOk(err)) {
    InetAddr_free(addr);
    return err;
  }

  /* copy to buffer while converting to char */
  err=InetAddr_GetAddress(addr,buffer,bsize);
  InetAddr_free(addr);
  if (!Error_IsOk(err))
    return err;

  /* done */
  return 0;
}


int IPC_TransportLayerTCP_GetPeerPort(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;
  INETADDRESS *addr;
  int i;

  assert(tl);

  /* get peer address */
  addr=InetAddr_new(AddressFamilyIP);
  err=Socket_GetPeerAddr((SOCKETPTR)(tl->privateData), addr);
  if (!Error_IsOk(err)) {
    InetAddr_free(addr);
    return -1;
  }

  /* return port */
  i=InetAddr_GetPort(addr);
  InetAddr_free(addr);
  return i;
}


void IPC_TransportLayerTCP_free(IPCTRANSPORTLAYERTABLE *p){
  assert(p);
  if (p->privateData)
    Socket_free((SOCKETPTR)(p->privateData));
  free(p);
}


IPCTRANSPORTLAYERTABLE *IPC_TransportLayerTCP_new(){
  IPCTRANSPORTLAYERTABLE *t;

  t=(IPCTRANSPORTLAYERTABLE*)malloc(sizeof(IPCTRANSPORTLAYERTABLE));
  assert(t);
  memset(t, 0, sizeof(IPCTRANSPORTLAYERTABLE));
  t->startConnect=IPC_TransportLayerTCP_StartConnect;
  t->finishConnect=IPC_TransportLayerTCP_FinishConnect;
  t->listen=IPC_TransportLayerTCP_Listen;
  t->accept=IPC_TransportLayerTCP_Accept;
  t->disconnect=IPC_TransportLayerTCP_Disconnect;
  t->read=IPC_TransportLayerTCP_Read;
  t->write=IPC_TransportLayerTCP_Write;
  t->canRead=IPC_TransportLayerTCP_CanRead;
  t->canWrite=IPC_TransportLayerTCP_CanWrite;
  t->getSocket=IPC_TransportLayerTCP_GetSocket;
  t->getPeerAddress=IPC_TransportLayerTCP_GetPeerAddress;
  t->getPeerPort=IPC_TransportLayerTCP_GetPeerPort;
  t->free=IPC_TransportLayerTCP_free;
  t->type=TransportLayerTypeTCP;
  return t;
}




/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *               Transport layer for Unix Domain Sockets
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



ERRORCODE IPC_TransportLayerUnix_StartConnect(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;
  INETADDRESS *addr;

  assert(tl);

  if (tl->privateData==0) {
    tl->privateData=Socket_new();
    err=Socket_Open((struct SOCKETSTRUCT*)(tl->privateData),SocketTypeUnix);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      return err;
    }
  }

  err=Socket_SetBlocking((SOCKETPTR)(tl->privateData),0);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return err;
  }

  addr=InetAddr_new(AddressFamilyUnix);
  err=InetAddr_SetAddress(addr, tl->address);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    DBG_ERROR_ERR(err);
    return err;
  }

  err=Socket_Connect((SOCKETPTR)(tl->privateData),addr);
  InetAddr_free(addr);
  // not yet finished or real error ?
  if (!Error_IsOk(err)) {
    if (Error_GetType(err)!=Error_FindType("Socket") ||
	Error_GetCode(err)!=SOCKET_ERROR_IN_PROGRESS) {
      // real error, so return that error
      DBG_ERROR_ERR(err);
      return err;
    }
  }

  return 0;
}


ERRORCODE IPC_TransportLayerUnix_FinishConnect(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;

  assert(tl);

  // get socket error
  err=Socket_GetSocketError((SOCKETPTR)(tl->privateData));
  if (!Error_IsOk(err))
    return err;

  // make socket blocking again
  err=Socket_SetBlocking((SOCKETPTR)(tl->privateData),1);
  if (!Error_IsOk(err))
    return err;
  return 0;
}


ERRORCODE IPC_TransportLayerUnix_Listen(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;
  INETADDRESS *addr;

  assert(tl);

  if (tl->privateData==0) {
    tl->privateData=Socket_new();
    err=Socket_Open((struct SOCKETSTRUCT*)(tl->privateData),SocketTypeUnix);
    if (!Error_IsOk(err))
      return err;
  }

  /* create inet address */
  addr=InetAddr_new(AddressFamilyUnix);
  err=InetAddr_SetAddress(addr, tl->address);
  if (!Error_IsOk(err))
    err=InetAddr_SetName(addr,tl->address);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    InetAddr_free(addr);
    return err;
  }

  err=Socket_Bind((SOCKETPTR)(tl->privateData), addr);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  InetAddr_free(addr);
  if (!Error_IsOk(err))
    return err;

  err=Socket_Listen((SOCKETPTR)(tl->privateData), 10);
  if (!Error_IsOk(err))
    return err;
  return 0;
}


ERRORCODE IPC_TransportLayerUnix_Accept(IPCTRANSPORTLAYERTABLE *tl,
				       struct IPCTRANSPORTLAYERTABLESTRUCT **t){
  SOCKETPTR s;
  ERRORCODE err;
  INETADDRESS *iaddr;

  assert(tl);

  s=Socket_new();
  iaddr=InetAddr_new(AddressFamilyUnix);

  err=Socket_Accept((SOCKETPTR)(tl->privateData), iaddr,s);
  InetAddr_free(iaddr);
  if (!Error_IsOk(err)) {
    Socket_free(s);
    return err;
  }
  *t=IPC_TransportLayerUnix_new();
  (*t)->privateData=s;
  return 0;
}


ERRORCODE IPC_TransportLayerUnix_Disconnect(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;

  assert(tl);

  if (tl->privateData) {
    err=Socket_Close((SOCKETPTR)(tl->privateData));
    Socket_free((SOCKETPTR)(tl->privateData));
    tl->privateData=0;
    if (!Error_IsOk(err))
      return err;
  } /* if privateData */
  return 0;
}


struct SOCKETSTRUCT*
IPC_TransportLayerUnix_GetSocket(IPCTRANSPORTLAYERTABLE *tl){
  ERRORCODE err;

  assert(tl);

  if (tl->privateData==0) {
    tl->privateData=Socket_new();
    err=Socket_Open((struct SOCKETSTRUCT*)(tl->privateData),SocketTypeUnix);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      return 0;
    }
  }
  return (struct SOCKETSTRUCT*)(tl->privateData);
}


ERRORCODE IPC_TransportLayerUnix_Read(IPCTRANSPORTLAYERTABLE *tl,
				  char *buffer,
				     int *bsize){
  assert(tl);
  return Socket_Read((SOCKETPTR)(tl->privateData),buffer,bsize);
}


ERRORCODE IPC_TransportLayerUnix_Write(IPCTRANSPORTLAYERTABLE *tl,
				      const char *buffer,
				      int *bsize){
  assert(tl);
  return Socket_Write((SOCKETPTR)(tl->privateData),buffer,bsize);
}


ERRORCODE IPC_TransportLayerUnix_CanRead(IPCTRANSPORTLAYERTABLE *tl){
  return Socket_WaitForRead((SOCKETPTR)(tl->privateData),0);
}


ERRORCODE IPC_TransportLayerUnix_CanWrite(IPCTRANSPORTLAYERTABLE *tl){
  assert(tl);

  return Socket_WaitForWrite((SOCKETPTR)(tl->privateData),0);
}


ERRORCODE IPC_TransportLayerUnix_GetPeerAddress(IPCTRANSPORTLAYERTABLE *tl,
					       char *buffer,
					       int bsize){
  ERRORCODE err;
  INETADDRESS *addr;

  assert(tl);

  addr=InetAddr_new(AddressFamilyUnix);
  /* get peer address */
  err=Socket_GetPeerAddr((SOCKETPTR)(tl->privateData), addr);
  if (!Error_IsOk(err)) {
    InetAddr_free(addr);
    return err;
  }

  /* copy to buffer while converting to char */
  err=InetAddr_GetAddress(addr,buffer,bsize);
  InetAddr_free(addr);
  if (!Error_IsOk(err))
    return err;

  /* done */
  return 0;
}


int IPC_TransportLayerUnix_GetPeerPort(IPCTRANSPORTLAYERTABLE *tl){
  assert(tl);
  /* no port for unix domain sockets */
  return 0;
}


void IPC_TransportLayerUnix_free(IPCTRANSPORTLAYERTABLE *p){
  assert(p);
    if (p->privateData)
      Socket_free((SOCKETPTR)(p->privateData));
    free(p);
}


IPCTRANSPORTLAYERTABLE *IPC_TransportLayerUnix_new(){
  IPCTRANSPORTLAYERTABLE *t;

  t=(IPCTRANSPORTLAYERTABLE*)malloc(sizeof(IPCTRANSPORTLAYERTABLE));
  assert(t);
  memset(t, 0, sizeof(IPCTRANSPORTLAYERTABLE));
  t->startConnect=IPC_TransportLayerUnix_StartConnect;
  t->finishConnect=IPC_TransportLayerUnix_FinishConnect;
  t->listen=IPC_TransportLayerUnix_Listen;
  t->accept=IPC_TransportLayerUnix_Accept;
  t->disconnect=IPC_TransportLayerUnix_Disconnect;
  t->read=IPC_TransportLayerUnix_Read;
  t->write=IPC_TransportLayerUnix_Write;
  t->canRead=IPC_TransportLayerUnix_CanRead;
  t->canWrite=IPC_TransportLayerUnix_CanWrite;
  t->getSocket=IPC_TransportLayerUnix_GetSocket;
  t->getPeerAddress=IPC_TransportLayerUnix_GetPeerAddress;
  t->getPeerPort=IPC_TransportLayerUnix_GetPeerPort;
  t->free=IPC_TransportLayerUnix_free;
  t->type=TransportLayerTypeUnix;
  return t;
}







