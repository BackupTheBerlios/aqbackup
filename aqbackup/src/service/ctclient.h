/***************************************************************************
 $RCSfile: ctclient.h,v $
 -------------------
 cvs         : $Id: ctclient.h,v 1.1 2003/06/07 21:07:51 aquamaniac Exp $
 begin       : Fri Nov 29 2002
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


#ifndef CTCLIENT_H
#define CTCLIENT_H


#ifdef __cplusplus
extern "C" {
#endif

#define CTCLIENT_MAX_DISMISSED_REQUESTS 64

#include <chameleon/chameleon.h>
#include <chameleon/ipcservicelayer.h>
#include <ctservice.h>


struct CTCLIENTDATASTRUCT {
  IPCSERVICELAYER *service;
  char *clientIdString;
  int nextPeerId;
  int nextMessageId;
  int shared;
  int mark;
  int dismissedRequests[CTCLIENT_MAX_DISMISSED_REQUESTS];
  int nextDismissedRequest;
};
typedef struct CTCLIENTDATASTRUCT CTCLIENTDATA;


CTCLIENTDATA *CTClient_new();
void CTClient_free(CTCLIENTDATA *cd);

ERRORCODE CTClient_Init(CTCLIENTDATA *cd);
ERRORCODE CTClient_InitShared(CTCLIENTDATA *cd,
			      IPCSERVICELAYER *service);
ERRORCODE CTClient_Fini(CTCLIENTDATA *cd);

/**
 * @param port port the server is listening on. If "-1" then unix domain
 * sockets are used, in this case the address should be the path to the
 * socket file.
 */
ERRORCODE CTClient_AddServer(CTCLIENTDATA *cd,
			     const char *addr,
			     int port,
			     int *id);

ERRORCODE CTClient_RemoveServer(CTCLIENTDATA *cd, int id);

void CTClient_SetClientIdString(CTCLIENTDATA *cd,
				const char *cid);

ERRORCODE CTClient_CheckErrorMessage(CTCLIENTDATA *cd,
				     IPCMESSAGE *msg);

ERRORCODE CTClient_Work(CTCLIENTDATA *cd,
			int timeout,
			int maxmsg);

ERRORCODE CTClient_SendRequest(CTCLIENTDATA *cd,
			       CTSERVICEREQUEST *req,
			       int id);

ERRORCODE CTClient_CheckResponse(CTCLIENTDATA *cd,
				 int requestid);

/**
 * This removes the request from the queue and deletes it
 */
void CTClient_WithdrawRequest(CTCLIENTDATA *cd,
			      int requestid);

/**
 * This only marks the request as abandoned. When the response for this
 * request arrives, it will be discarded and the request itself will
 * be deleted. However, if there already is a response to this request
 * this request will immediately be deleted.
 */
void CTClient_AbandonRequest(CTCLIENTDATA *cd,
			     int requestid);

CTSERVICEREQUEST *CTClient_FindRequest(CTCLIENTDATA *cd,
				       int requestid);
void CTClient_DequeueRequest(CTCLIENTDATA *cd,
			     CTSERVICEREQUEST *req);


#ifdef __cplusplus
}
#endif


#endif



