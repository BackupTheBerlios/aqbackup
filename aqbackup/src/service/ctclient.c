/***************************************************************************
 $RCSfile: ctclient.c,v $
 -------------------
 cvs         : $Id: ctclient.c,v 1.1 2003/06/07 21:07:51 aquamaniac Exp $
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <chameleon/chameleon.h>
#include "ctclient.h"
#include "ctversion.h"
#include <chameleon/debug.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>


ERRORCODE CTClient_FreeMsgLayerUserData(IPCMESSAGELAYER *ml) {
  DBG_ENTER;
  DBG_INFO("Freeing Peerdata.");
  CTService_PeerData_free(IPCMessageLayer_GetUserData(ml));
  DBG_LEAVE;
  return 0;
}


CTCLIENTDATA *CTClient_new(){
  CTCLIENTDATA *cd;

  DBG_ENTER;
  cd=(CTCLIENTDATA*)malloc(sizeof(CTCLIENTDATA));
  assert(cd);
  memset(cd,0,sizeof(CTCLIENTDATA));
  DBG_LEAVE;
  return cd;
}


void CTClient_free(CTCLIENTDATA *cd){
  DBG_ENTER;
  if (cd) {
    if (cd->service) {
      /* we can free the IPCServiceLayer here even if we are in shared
       * mode, because the called function will only really free that object
       * if its reference counter reaches zero.*/
      IPCServiceLayer_free(cd->service);
    }
    free(cd->clientIdString);
    free(cd);
  }
  DBG_LEAVE;
}


void CTClient__AddDismissed(CTCLIENTDATA *cd, int requestId){
  int p;

  assert(cd);
  p=((cd->nextDismissedRequest)++)%CTCLIENT_MAX_DISMISSED_REQUESTS;
  cd->nextDismissedRequest=cd->nextDismissedRequest%CTCLIENT_MAX_DISMISSED_REQUESTS;
  cd->dismissedRequests[p]=requestId;
}


int CTClient__CheckDismissed(CTCLIENTDATA *cd, int requestId){
  int i;

  for (i=0; i<CTCLIENT_MAX_DISMISSED_REQUESTS; i++)
    if (cd->dismissedRequests[i]==requestId)
      return 1;
  return 0;
}




ERRORCODE CTClient_Init(CTCLIENTDATA *cd){
  DBG_ENTER;
  assert(cd);
  /* create service layer */
  cd->service=IPCServiceLayer_new();
  cd->shared=0;

  DBG_LEAVE;
  return 0;
}


ERRORCODE CTClient_InitShared(CTCLIENTDATA *cd,
			      IPCSERVICELAYER *service){
  DBG_ENTER;
  assert(cd);
  /* share service layer */
  IPCServiceLayer_share(service);
  cd->service=service;
  cd->shared=1;

  DBG_LEAVE;
  return 0;
}


ERRORCODE CTClient_Fini(CTCLIENTDATA *cd){
  DBG_ENTER;

  if (cd)
    IPCServiceLayer_ShutDown(cd->service);
  else {
    DBG_WARN("Your program called CTClient_Fini with a NULL pointer,"
	     " it does no harm, but you should eventually fix it ;-)");
  }
  DBG_LEAVE;
  return 0;
}


ERRORCODE CTClient_AddServer(CTCLIENTDATA *cd,
			     const char *addr,
			     int port,
			     int *id) {
  ERRORCODE err;
  IPCTRANSPORTLAYERTABLE *tl;
  IPCMESSAGELAYER *ml;
  CTSERVICEDATA *svd;

  DBG_ENTER;
  /* create and setup transportlayer */
  if (port==-1) {
    /* unix domain sockets are to be used */
#ifdef OS_WIN32
    DBG_ERROR("Unix domain sockets not supported on this plattform");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_INVALID);
#endif
    tl=IPC_TransportLayerUnix_new();

    /* set address */
    err=IPC_TransportLayer_SetAddress(tl, addr);
    if (!Error_IsOk(err)) {
      DBG_NOTICE_ERR(err);
      tl->free(tl);
      DBG_LEAVE;
      return err;
    }
  }
  else {
    tl=IPC_TransportLayerTCP_new();
    /* set address */
    err=IPC_TransportLayer_SetAddress(tl, addr);
    if (!Error_IsOk(err)) {
      DBG_NOTICE_ERR(err);
      tl->free(tl);
      DBG_LEAVE;
      return err;
    }
    /* set port */
    IPC_TransportLayer_SetPort(tl, port);
  }

  /* create message layer */
  ml=IPCMessageLayer_new();
  *id=++(cd->nextPeerId);
  IPCMessageLayer_SetId(ml, *id);
  IPCMessageLayer_SetMark(ml, cd->mark);
  /* this is to make sure this clients doesn't get removed when CTClient
   * and CTServer share one IPCServiceLayer, and the CTService tries to
   * remove all disconnected message layers */
  IPCMessageLayer_SetPersistence(ml, 1);
  svd=CTService_PeerData_new();

  IPCMessageLayer_SetUserData(ml, svd);
  /* install freeUserData callback. This one is called when the
   * messagelayer is freed */
  IPCMessageLayer_SetFreeUserDataCallback(ml, CTClient_FreeMsgLayerUserData);
  IPCMessageLayer_SetTransportLayer(ml, tl);
  IPCMessageLayer_SetStatus(ml, StateDisconnected);

  /* message layer complete, add it */
  IPCServiceLayer_AddMessageLayer(cd->service, ml);

  /* done */
  DBG_LEAVE;
  return 0;
}


void CTClient_SetClientIdString(CTCLIENTDATA *cd,
				const char *cid) {
  assert(cd);
  assert(cid);
  cd->clientIdString=strdup(cid);
}


/* takes over msg */
void CTClient__HandleResponse(CTCLIENTDATA *cd,
			      IPCMESSAGELAYER *ml,
			      IPCMESSAGE *msg) {
  CTSERVICEDATA *svd;
  CTSERVICEREQUEST *req;
  int requestid;
  ERRORCODE err;

  DBG_ENTER;
  assert(cd);
  assert(ml);
  assert(msg);

  svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
  assert(svd);

  /* get msg reply code */
  err=IPCMessage_IntParameter(msg, 3, &requestid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return;
  }

  /* search for request with the given id */
  req=CTService_Request_FindRequest(requestid,&(svd->requests));
  if (req) {
    /* request found, is it abandoned ? */
    if (req->abandoned) {
      /* yes, so remove it, discard message received */
      IPCMessage_free(msg);
      CTService_Request_RemoveRequest(req, &(svd->requests));
      CTService_Request_free(req);
    }
    else {
      /* request found, enqueue response there */
      req->responseCount++;
      IPCMessage_AddMessage(msg, &(req->responses));
    }
  } /* while */
  else {
    /* request not found */
    if (CTClient__CheckDismissed(cd, requestid)==0){
      DBG_WARN("Got an unrequested message, dismissing (requestid=%d)",
	       requestid);
    }
    else {
      DBG_NOTICE("Request withdrawn/abandoned, dismissing response");
    }
    IPCMessage_free(msg);
  }

  DBG_LEAVE;
}


ERRORCODE CTClient_CheckErrorMessage(CTCLIENTDATA *cd,
				     IPCMESSAGE *msg) {
  ERRORCODE err;
  int msgCode;
  int msgVersion;

  DBG_ENTER;

  /* get message code */
  err=IPCMessage_FirstIntParameter(msg, &msgCode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }

  /* get message version */
  err=IPCMessage_NextIntParameter(msg, &msgVersion);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if (msgCode==CTSERVICE_MSGCODE_RP_ERROR) {
    int localerrcode;
    char *errmsg;
    int errmsgsize;

    /* got an error message, interprete it */
    if ((msgVersion&0xff00)!=
	(CTSERVICE_MSGCODE_RP_ERROR_VERSION&0xff00)) {
	DBG_ERROR("Error message: Bad message version.");
	DBG_LEAVE;
	err=Error_New(0,
		      ERROR_SEVERITY_ERR,
		      Error_FindType(CTSERVICE_ERROR_TYPE),
		      CTSERVICE_ERROR_BAD_MESSAGE_VERSION);
	return err;
      }
      /* get error code */
      err=IPCMessage_IntParameter(msg, 4, &localerrcode);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_LEAVE;
	return err;
      }
      /* get error message */
      err=IPCMessage_NextParameter(msg, &errmsg, &errmsgsize);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_LEAVE;
	return err;
      }
      /* print error message */
      if (errmsgsize) {
	errmsg[errmsgsize-1]=0;
	DBG_ERROR("Remote error: %s",errmsg);
      }
      /* return error code */
      err=Error_New(0,
		    ERROR_SEVERITY_ERR,
		    Error_FindType(CTSERVICE_ERROR_TYPE),
		    localerrcode);
      DBG_NOTICE_ERR(err);
      DBG_LEAVE;
      return err;
    }

  DBG_LEAVE;
  return 0;
}


/* takes over ownership for msg */
ERRORCODE CTClient__HandleMessage(CTCLIENTDATA *cd,
				  IPCMESSAGELAYER *ml,
				  IPCMESSAGE *msg) {
  CTSERVICEDATA *svd;
  ERRORCODE err;
  int msgCode;
  int msgVersion;

  DBG_ENTER;
  svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
  assert(svd);

  /* get message code */
  err=IPCMessage_FirstIntParameter(msg, &msgCode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_NOTICE("Disconnecting client.");
    IPCMessageLayer_ShutDown(ml);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return err;
  }
  DBG_INFO("Got a message (code 0x%08x)",msgCode);

  /* get message version */
  err=IPCMessage_NextIntParameter(msg, &msgVersion);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_NOTICE("Disconnecting client.");
    IPCMessageLayer_ShutDown(ml);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return err;
  }

  if (svd->channelState==ChannelOpening) {
    CTSERVICEREQUEST *req;
    unsigned char *keyd;
    int keydsize;
    char *cryptedkey;
    int cryptedkeylen;

    if (msgCode==CTSERVICE_MSGCODE_RP_EXCHANGE_KEYS) {
      if (IPCMessageLayer_GetType(ml)==TransportLayerTypeUnix) {
	DBG_ERROR("No secure channel requested, disconnecting.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return 0;
      }
      if ((msgVersion&0xff00)!=
	  (CTSERVICE_MSGCODE_RP_EXCHANGE_KEYS_VERSION&0xff00)) {
	DBG_ERROR("Bad message version.");
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return 0;
      }

      /* read session key */
      DBG_INFO("Reading session key");
      err=IPCMessage_NextParameter(msg, &cryptedkey, &cryptedkeylen);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return err;
      }
      /* decrypt session key */
      DBG_INFO("Decrypting session key");
      err=Cryp_Decrypt(svd->tempKey,
		       CryptAlgoRSA,
		       cryptedkey,
		       cryptedkeylen,
		       &keyd,
		       &keydsize);
      Cryp_RsaKey_free(svd->tempKey);
      svd->tempKey=0;
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return err;
      }

      svd->sessionKey=Cryp_BlowfishKey_new();
      IPCMessageLayer_UseEncryption(ml,1);
      err=Cryp_BlowfishKey_SetKey(svd->sessionKey,keyd, keydsize);
      free(keyd);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return err;
      }
    }
    else if (msgCode==CTSERVICE_MSGCODE_RP_OPEN) {
      if (IPCMessageLayer_GetType(ml)!=TransportLayerTypeUnix) {
	DBG_ERROR("No insecure channel requested, disconnecting.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return 0;
      }
      if ((msgVersion&0xff00)!=
	  (CTSERVICE_MSGCODE_RP_OPEN_VERSION&0xff00)) {
	DBG_ERROR("Bad message version, disconnecting.");
	IPCMessageLayer_ShutDown(ml);
	IPCMessage_free(msg);
	DBG_LEAVE;
	return 0;
      }
      IPCMessageLayer_UseEncryption(ml,0);
    }
    else {
      DBG_ERROR("Channel not open, unawaited message.");
      IPCMessage_free(msg);
      DBG_LEAVE;
      return 0;
    }

    /* channel is open now */
    svd->channelState=ChannelOpen;
    DBG_NOTICE("Channel is open to %04x (%s encrypted )",
	       IPCMessageLayer_GetId(ml),
	       IPCMessageLayer_UsesEncryption(ml)?"":"not");

    /* ok, now encrypt and add all enqueued messages */
    DBG_DEBUG("Moving enqueued messages");
    req=svd->requests;
    while (req) {
      IPCMESSAGE *encryptedMsg;
      CTSERVICEREQUEST *next;

      if (req->message) {
	if (IPCMessageLayer_UsesEncryption(ml)) {
	  /* encrypt the message and enqueue it */
	  encryptedMsg=CTService_EncryptMessage(svd,req->message);
	  if (encryptedMsg) {
	    err=IPCServiceLayer_SendMessage(cd->service,ml,encryptedMsg);
	    if (!Error_IsOk(err)) {
	      DBG_NOTICE_ERR(err);
	      IPCMessage_free(encryptedMsg);
	    }
	    else {
	      /* encrypted version of the message successfully enqueued,
	       * so free it */
	      IPCMessage_free(req->message);
	      req->message=0;
	    }
	  } /* if encryptedMsg */
	  else {
	    DBG_ERROR("Could not encrypt message, will ignore it");
	  }
	}
	else {
	  /* no encryption needed, send message directly */
	  err=IPCServiceLayer_SendMessage(cd->service,ml,req->message);
	  if (!Error_IsOk(err)) {
	    DBG_NOTICE_ERR(err);
	  }
	  else {
	    /* encrypted version of the message successfully enqueued,
	     * so unlink it. Don't free it, it now belongs to the
	     * message layer ! */
	    req->message=0;
	  }
	}
      } /* if there is a message inside the request */
      next=req->next;
      req=next;
    } /* while */
    DBG_DEBUG("Enqueued messages moved");
    IPCMessage_free(msg);
  } /* if channelOpening */

  else if (svd->channelState==ChannelOpen) {
    /* channel is open */
    IPCMESSAGE *decryptedMsg;

    err=CTClient_CheckErrorMessage(cd,
				   msg);
    if (!Error_IsOk(err)) {
      DBG_NOTICE_ERR(err);
      IPCMessage_free(msg);
      DBG_LEAVE;
      return err;
    }

    if (IPCMessageLayer_UsesEncryption(ml)) {
      decryptedMsg=CTService_DecryptMessage(svd,
					    msg);
      if (!decryptedMsg) {
	DBG_ERROR("Could not decrypt message.");
	IPCMessage_free(msg);
	DBG_LEAVE;
	return 0;
      }
      /* find request for which this message is a response and enqueue the
       * decrypted message there */
      CTClient__HandleResponse(cd, ml, decryptedMsg);
      IPCMessage_free(msg);
    }
    else {
      /* message not encrypted, so handle it directly
       * do not free the msg, since it will taken over by the handler */
      CTClient__HandleResponse(cd, ml, msg);
    }
  }
  else {
    DBG_ERROR("Whaaat ? Channel not open, but we got a message ?!");
  }

  DBG_LEAVE;
  return 0;
}


ERRORCODE CTClient_Work(CTCLIENTDATA *cd,
			int timeout,
			int maxmsg){
  ERRORCODE err1, err2;
  int msgcount;
  IPCMESSAGELAYER *ml;
  IPCMESSAGE *msg;

  DBG_ENTER;

  /* check for IPC messages if not in shared mode
   * in shared mode the CTServer will do this
   */
  if (cd->shared==0) {
    err1=IPCServiceLayer_Work(cd->service, timeout);
    if (!Error_IsOk(err1)) {
      if (Error_GetType(err1)==Error_FindType(IPCMESSAGE_ERROR_TYPE) &&
	  Error_GetCode(err1)==IPCMESSAGE_ERROR_NO_TRANSPORTLAYER)
	err1=Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType(CTSERVICE_ERROR_TYPE),
		       CTSERVICE_ERROR_NO_TRANSPORT_LAYER);
      DBG_DEBUG_ERR(err1);
    }
  }
  else
    err1=0;

  msgcount=0;
  while(msgcount<maxmsg) {
    err2=IPCServiceLayer_NextMessage(cd->service, &ml, &msg,
				     cd->mark);
    if (!Error_IsOk(err2)) {
      DBG_DEBUG("No next message");
      break;
    }
    err2=CTClient__HandleMessage(cd, ml, msg);
    if (!Error_IsOk(err2)) {
      DBG_DEBUG_ERR(err2);
    }
    else {
      DBG_INFO("Message handled.");
    }
    msgcount++;
  } /* while */
  DBG_LEAVE;
  return err1;
}


ERRORCODE CTClient__Open(CTCLIENTDATA *cd, IPCMESSAGELAYER *ml) {
  IPCTRANSPORTLAYERTABLE *tl;
  ERRORCODE err;
  CTSERVICEDATA *svd;
  IPCMESSAGE *keymsg;
  IPCMESSAGE *requestmsg;
  char *idstring;

  DBG_ENTER;
  svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
  assert(svd);
  tl=IPCMessageLayer_GetTransportLayer(ml);
  if (!tl) {
    DBG_ERROR("No transport layer");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_TRANSPORTLAYER);
  }
  assert(tl->startConnect);
  DBG_INFO("Starting to connect");
  err=tl->startConnect(tl);
  DBG_INFO("Starting to connect done");
  if (!Error_IsOk(err)) {
    /* IPCMessageLayer_SetStatus(ml, StateUnreachable); */
    IPCMessageLayer_SetStatus(ml, StateDisconnected);
    DBG_NOTICE_ERR(err);
    DBG_LEAVE;
    /* change error to "Service unreachable" */
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_UNREACHABLE);
  }
  IPCMessageLayer_SetStatus(ml, StateConnecting);

  if (IPCMessageLayer_GetType(ml)==TransportLayerTypeUnix) {
    /* generate request for insecure channel */
    DBG_DEBUG("Creating request message");
    requestmsg=IPCMessage_new();
    IPCMessage_SetBuffer(requestmsg,0, IPCMESSAGE_MAXMSGSIZE);
    /* msg code */
    err=IPCMessage_AddIntParameter(requestmsg,
				   CTSERVICE_MSGCODE_RQ_OPEN);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }
    /* msg version */
    err=IPCMessage_AddIntParameter(requestmsg,
				   CTSERVICE_MSGCODE_RQ_OPEN_VERSION);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }

    if (cd->clientIdString)
      idstring=cd->clientIdString;
    else
      idstring="Unknown client";
    err=IPCMessage_AddParameter(requestmsg,
				idstring,
				strlen(idstring)+1);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }

    err=IPCMessage_BuildMessage(requestmsg);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }

    /* send request */
    DBG_DEBUG("Sending message");
    err=IPCServiceLayer_SendMessage(cd->service, ml, requestmsg);
    if (!Error_IsOk(err)) {
      DBG_NOTICE_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }
    /* channel is opening now */
    svd->channelState=ChannelOpening;
  }
  else {
    /* create key request */
    keymsg=IPCMessage_new();
    IPCMessage_SetBuffer(keymsg, 0,2048);
    svd->tempKey=Cryp_RsaKey_new();
    DBG_INFO("Creating RSA key");
    err=Cryp_RsaKey_Generate(svd->tempKey,0,0);
    DBG_INFO("Creating RSA key done");
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(keymsg);
      DBG_LEAVE;
      return err;
    }

    err=Cryp_RsaKey_ToMessage(svd->tempKey,
			      keymsg, 1);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(keymsg);
      DBG_LEAVE;
      return err;
    }

    DBG_DEBUG("Creating request message");
    requestmsg=IPCMessage_new();
    IPCMessage_SetBuffer(requestmsg,0, IPCMESSAGE_MAXMSGSIZE);
    /* msg code */
    err=IPCMessage_AddIntParameter(requestmsg,
				   CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(keymsg);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }
    /* msg version */
    err=IPCMessage_AddIntParameter(requestmsg,
				   CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS_VERSION);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(keymsg);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }
    /* our key */
    err=IPCMessage_AddParameter(requestmsg,
				IPCMessage_GetMessageBegin(keymsg),
				IPCMessage_GetMessageSize(keymsg));
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(keymsg);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }
    IPCMessage_free(keymsg);

    if (cd->clientIdString)
      idstring=cd->clientIdString;
    else
      idstring="Unknown client";
    err=IPCMessage_AddParameter(requestmsg,
				idstring,
				strlen(idstring)+1);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }

    err=IPCMessage_BuildMessage(requestmsg);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }

    /* send request */
    DBG_DEBUG("Sending message");
    err=IPCServiceLayer_SendMessage(cd->service, ml, requestmsg);
    if (!Error_IsOk(err)) {
      DBG_NOTICE_ERR(err);
      IPCMessage_free(requestmsg);
      DBG_LEAVE;
      return err;
    }
    /* channel is opening now */
    svd->channelState=ChannelOpening;
  }

  DBG_LEAVE;
  return 0;
}


ERRORCODE CTClient_SendRequest(CTCLIENTDATA *cd,
			       CTSERVICEREQUEST *req,
			       int id) {
  IPCMESSAGELAYER *ml;
  ERRORCODE err;
  CTSERVICEDATA *svd;

  DBG_ENTER;
  /* find matching message layer */
  ml=IPCServiceLayer_FindMessageLayer(cd->service,id);
  if (!ml) {
    DBG_ERROR("Message layer not found (%04x)",id);
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_MESSAGELAYER);
  }
  svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);

  /* check whether there is a physical connection */
  if (IPCMessageLayer_GetStatus(ml)==StateDisconnected) {
    /* no physical connection, try to establish one */
    svd->channelState=ChannelClosed;
    DBG_INFO("Not connected, starting to connect");
    err=CTClient__Open(cd, ml);
    if (!Error_IsOk(err)) {
      DBG_NOTICE_ERR(err);
      DBG_LEAVE;
      return err;
    }
  }

  /* check whether the channel is open */
  if (IPCMessageLayer_GetStatus(ml)==StateConnecting ||
      svd->channelState!=ChannelOpen) {
    /* still connecting or opening the channel, so do not send the request
     * at this time. At the end of this function the request will be
     * enqueued. As soon as the channel is open (i.e. the keys are exchanged)
     * all requests which still contain a message will be send. */
    DBG_INFO("Still connecting, encryption and sending postponed");
  }
  else if (svd->channelState==ChannelOpen) {
    /* is connected and open, so encrypt the message and send it */
    if (IPCMessageLayer_UsesEncryption(ml)) {
      IPCMESSAGE *encryptedMsg;

      encryptedMsg=CTService_EncryptMessage(svd,req->message);
      if (!encryptedMsg) {
	DBG_ERROR("Could not encrypt message, will ignore it");
      }
      else {
	err=IPCServiceLayer_SendMessage(cd->service,ml,encryptedMsg);
	if (!Error_IsOk(err)) {
	  DBG_NOTICE_ERR(err);
	  DBG_LEAVE;
	  IPCMessage_free(encryptedMsg);
	  return err;
	}
	/* to limit memory consumption we will free the message now. After
	 * the encrypted version of this message has been enqueued for
	 * sending we have no further need for this message anyway.
	 */
	IPCMessage_free(req->message);
	req->message=0;
      }
    } /* if encryption needed */
    else {
      /* no encryption needed, send the message directly */
      err=IPCServiceLayer_SendMessage(cd->service,ml,req->message);
      if (!Error_IsOk(err)) {
	DBG_NOTICE_ERR(err);
	DBG_LEAVE;
	return err;
      }
      /* unlink message from request, but do not free it ! Ownership
       * is taken over by messagelayer ! */
      req->message=0;
    }
  } /* if channel open */

  /* enqueue request.
   * If the message inside the request has been send this request will
   * no longer contain a message. If after the connection is established
   * all requests will be checked for a message included. If there is one it
   * will be enqueued for sending then.
   */
  CTService_Request_AddRequest(req, &(svd->requests));

  /* done */
  DBG_LEAVE;
  return 0;
}


CTSERVICEREQUEST *CTClient_FindRequest(CTCLIENTDATA *cd,
				       int requestid) {
  IPCMESSAGELAYER *ml;
  CTSERVICEREQUEST *req;

  ml=cd->service->messageLayers;
  while (ml) {
    CTSERVICEDATA *svd;
    if (cd->mark==0 || ml->mark==cd->mark) {
      svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
      assert(svd);
      /* look for request there */
      req=CTService_Request_FindRequest(requestid, &(svd->requests));
      if (req) {
	DBG_DEBUG("Request found");
	DBG_LEAVE;
	return req;
      }
    }
    ml=ml->next;
  } /* while */

  DBG_ERROR("Request not found");
  DBG_LEAVE;
  return 0;
}


void CTClient_DequeueRequest(CTCLIENTDATA *cd,
			     CTSERVICEREQUEST *req){
  IPCMESSAGELAYER *ml;

  DBG_ENTER;
  ml=IPCServiceLayer_FindMessageLayer(cd->service,
				      req->serviceId);
  if (ml) {
    CTSERVICEDATA *svd;
    svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
    assert(svd);
    /* dequeue request there */
    DBG_INFO("Removing request");
    CTService_Request_RemoveRequest(req, &(svd->requests));
  }
  else {
    DBG_ERROR("Message layer not found");
  }
  DBG_LEAVE;
}


ERRORCODE CTClient_CheckResponse(CTCLIENTDATA *cd,
				 int requestid){
  CTSERVICEREQUEST *req;
  IPCMESSAGE *msg;
  IPCMESSAGELAYER *ml;


  DBG_ENTER;
  /* search request */
  req=CTClient_FindRequest(cd, requestid);
  if (!req)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_NO_REQUEST);

  /* check is message layer is connectable */
  ml=IPCServiceLayer_FindMessageLayer(cd->service, req->serviceId);
  if (ml) {
    CTSERVICEDATA *svd;

    svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
    assert(svd);

    if (IPCMessageLayer_GetStatus(ml)==StateDisconnected) {
      DBG_NOTICE("Messagelayer disconnected, removing request");
      CTService_Request_RemoveRequest(req, &(svd->requests));
      CTService_Request_free(req);
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType(CTSERVICE_ERROR_TYPE),
		       CTSERVICE_ERROR_UNREACHABLE);
    }
    else if (IPCMessageLayer_GetStatus(ml)==StateUnreachable) {
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType(CTSERVICE_ERROR_TYPE),
		       CTSERVICE_ERROR_UNREACHABLE);
    }
  }
  else {
    DBG_ERROR("Message layer not found, we maybe lost the connection");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_UNREACHABLE);
  }

  /* check for response */
  msg=CTService_Request_PeekResponse(req);
  if (!msg) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_NO_MESSAGE);
  }
  DBG_LEAVE;
  return 0;
}





void CTClient_WithdrawRequest(CTCLIENTDATA *cd,
			      int requestid){
  CTSERVICEREQUEST *req;

  DBG_ENTER;
  /* search request */
  req=CTClient_FindRequest(cd, requestid);
  if (req) {
    CTClient__AddDismissed(cd, requestid);
    CTClient_DequeueRequest(cd, req);
    CTService_Request_free(req);
  }
  DBG_LEAVE;
}


void CTClient_AbandonRequest(CTCLIENTDATA *cd,
			     int requestid){
  CTSERVICEREQUEST *req;

  DBG_ENTER;
  /* search request */
  req=CTClient_FindRequest(cd, requestid);
  if (req) {
    CTClient__AddDismissed(cd, requestid);
    if (req->responseCount>0) {
      CTClient_DequeueRequest(cd, req);
      CTService_Request_free(req);
    }
    else
      req->abandoned=1;
  }
  DBG_LEAVE;
}



ERRORCODE CTClient_RemoveServer(CTCLIENTDATA *cd, int id) {
  IPCMESSAGELAYER *ml;

  DBG_ENTER;
  /* find matching message layer */
  ml=IPCServiceLayer_FindMessageLayer(cd->service,id);
  if (!ml) {
    DBG_ERROR("Message layer not found (%04x)",id);
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_MESSAGELAYER);
  }

  /* unlink the message layer */
  IPCServiceLayer_UnlinkMessageLayer(cd->service, ml);
  /* shutdown connection */
  IPCMessageLayer_ShutDown(ml);
  IPCMessageLayer_free(ml);

  /* done */
  DBG_LEAVE;
  return 0;
}



