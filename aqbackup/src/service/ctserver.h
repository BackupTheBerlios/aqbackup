/***************************************************************************
 $RCSfile: ctserver.h,v $
 -------------------
 cvs         : $Id: ctserver.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
 begin       : Thu Nov 28 2002
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


#ifndef CTSERVER_H
#define CTSERVER_H "$Id: ctserver.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $"

#ifdef __cplusplus
extern "C" {
#endif


#include <chameleon/error.h>
#include <chameleon/ipcservicelayer.h>
#include <chameleon/cryp.h>
#include <chameleon/conf.h>
#include <ctservice.h>

typedef struct CTSERVERDATASTRUCT CTSERVERDATA;

/**
 * This is a pointer to a callback function which really handles a
 * request. The function pointed to may call @ref CTServer_SendResponse to
 * send a response.
 */
typedef ERRORCODE (*CTSERVER_HANDLEREQUESTPTR)(CTSERVERDATA *sd,
					       IPCMESSAGELAYER *ml,
					       IPCMESSAGE *msg);

typedef ERRORCODE (*CTSERVER_CLIENTUPPTR)(CTSERVERDATA *sd,
					  IPCMESSAGELAYER *ml);
typedef ERRORCODE (*CTSERVER_CLIENTDOWNPTR)(CTSERVERDATA *sd,
					    IPCMESSAGELAYER *ml);



struct CTSERVERDATASTRUCT {
  IPCSERVICELAYER *service;
  CTSERVER_CLIENTUPPTR clientUp;
  CTSERVER_CLIENTDOWNPTR clientDown;
  CTSERVER_HANDLEREQUESTPTR handleRequest;
  const char *serverPipe;
  int nextMessageId;
  int nextClientId;
  void *privateData;
  int mark;
  char *address;
  int port;
};



CTSERVERDATA* CTServer_new();
void CTServer_free(CTSERVERDATA *sd);


ERRORCODE CTServer_Init(CTSERVERDATA *sd,
			CONFIGGROUP *root);
ERRORCODE CTServer_Fini(CTSERVERDATA *sd);

ERRORCODE CTServer_Work(CTSERVERDATA *sd,
			int timeout,
			int maxmsg);

ERRORCODE CTServer_SendResponse(CTSERVERDATA *sd,
				IPCMESSAGELAYER *ml,
				IPCMESSAGE *msg);

ERRORCODE CTServer_SendErrorMessage(CTSERVERDATA *sd,
				    IPCMESSAGELAYER *ml,
				    IPCMESSAGE *req,
				    ERRORCODE errcode);


void CTServer_RemoveDisconnected(CTSERVERDATA *sd);

void CTServer_SetRequestHandler(CTSERVERDATA *sd,
				CTSERVER_HANDLEREQUESTPTR p);

void CTServer_SetClientUpHandler(CTSERVERDATA *sd,
				 CTSERVER_CLIENTUPPTR p);
void CTServer_SetClientDownHandler(CTSERVERDATA *sd,
				   CTSERVER_CLIENTDOWNPTR p);

void CTServer_SetPrivateData(CTSERVERDATA *sd,
			     void *p);
void *CTServer_GetPrivateData(CTSERVERDATA *sd);


#ifdef __cplusplus
}
#endif


#endif /* CTSERVER_H */


