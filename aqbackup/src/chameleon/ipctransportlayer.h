/***************************************************************************
 $RCSfile: ipctransportlayer.h,v $
                             -------------------
    cvs         : $Id: ipctransportlayer.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
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

#ifndef IPCTRANSPORTLAYER_H
#define IPCTRANSPORTLAYER_H "$Id: ipctransportlayer.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $"


#include <chameleon/error.h>
#include <chameleon/inetaddr.h>
#include <chameleon/inetsocket.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
  TransportLayerTypeTCP=0,
  TransportLayerTypeUnix
} TransportLayerType;


CHIPCARD_API struct IPCTRANSPORTLAYERTABLESTRUCT;
CHIPCARD_API typedef struct IPCTRANSPORTLAYERTABLESTRUCT IPCTRANSPORTLAYERTABLE;


CHIPCARD_API typedef ERRORCODE (*IPCTRANS_STARTCONNECTPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_FINISHCONNECTPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_LISTENPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_ACCEPTPTR)(IPCTRANSPORTLAYERTABLE *tl,
						     struct IPCTRANSPORTLAYERTABLESTRUCT **t);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_DISCONNECTPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef struct SOCKETSTRUCT* (*IPCTRANS_GETSOCKETPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_READPTR)(IPCTRANSPORTLAYERTABLE *tl,
						   char *buffer,
						   int *bsize);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_WRITEPTR)(IPCTRANSPORTLAYERTABLE *tl,
						    const char *buffer,
						    int *bsize);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_CANREADPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_CANWRITEPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef ERRORCODE (*IPCTRANS_GETADDRPTR)(IPCTRANSPORTLAYERTABLE *tl,
					 char *buffer,
					 int bsize);
CHIPCARD_API typedef int (*IPCTRANS_GETPORTPTR)(IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API typedef void (*IPCTRANS_FREEPTR)(IPCTRANSPORTLAYERTABLE *tl);


CHIPCARD_API struct IPCTRANSPORTLAYERTABLESTRUCT {
  IPCTRANS_STARTCONNECTPTR startConnect;
  IPCTRANS_FINISHCONNECTPTR finishConnect;
  IPCTRANS_LISTENPTR listen;
  IPCTRANS_ACCEPTPTR accept;
  IPCTRANS_DISCONNECTPTR disconnect;
  IPCTRANS_READPTR read;
  IPCTRANS_WRITEPTR write;
  IPCTRANS_CANREADPTR canRead;
  IPCTRANS_CANWRITEPTR canWrite;
  IPCTRANS_GETSOCKETPTR getSocket;
  IPCTRANS_GETADDRPTR getPeerAddress;
  IPCTRANS_GETPORTPTR getPeerPort;
  IPCTRANS_FREEPTR free;
  char address[128];
  int port;
  void *privateData;
  TransportLayerType type;
};




CHIPCARD_API IPCTRANSPORTLAYERTABLE *IPC_TransportLayerTCP_new();

CHIPCARD_API IPCTRANSPORTLAYERTABLE *IPC_TransportLayerUnix_new();

ERRORCODE IPC_TransportLayer_GetAddress(IPCTRANSPORTLAYERTABLE *tl,
					char *buffer,
					int bsize);
ERRORCODE IPC_TransportLayer_SetAddress(IPCTRANSPORTLAYERTABLE *tl,
					const char *buffer);
int IPC_TransportLayer_GetPort(IPCTRANSPORTLAYERTABLE *tl);
void IPC_TransportLayer_SetPort(IPCTRANSPORTLAYERTABLE *tl, int port);
TransportLayerType IPC_TransportLayer_GetType(IPCTRANSPORTLAYERTABLE *tl);

#ifdef __cplusplus
}
#endif


#endif /* IPCTRANSPORTLAYER_H */

