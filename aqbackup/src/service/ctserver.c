/***************************************************************************
 $RCSfile: ctserver.c,v $
 -------------------
 cvs         : $Id: ctserver.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <chameleon/chameleon.h>
#include "ctserver.h"
#include "ctservice.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <chameleon/debug.h>


ERRORCODE CTServer_FreeMsgLayerUserData(IPCMESSAGELAYER *ml) {
  CTService_PeerData_free(IPCMessageLayer_GetUserData(ml));
  return 0;
}



CTSERVERDATA* CTServer_new(){
  CTSERVERDATA *sd;

  sd=(CTSERVERDATA*)malloc(sizeof(CTSERVERDATA));
  assert(sd);
  memset(sd,0,sizeof(CTSERVERDATA));
  return sd;
}


void CTServer_free(CTSERVERDATA *sd){
  if (sd) {
    if (sd->service)
      IPCServiceLayer_free(sd->service);
    free(sd->address);
    free(sd);
  }
}


ERRORCODE CTServer_Init(CTSERVERDATA *sd,
			CONFIGGROUP *root) {
  ERRORCODE err;
  IPCTRANSPORTLAYERTABLE *tl;
  IPCMESSAGELAYER *ml;
  const char *addr;
  int port;
  const char *tp;
  SOCKETPTR sk;
  const char *fms;

  assert(sd);
  assert(root);

  /* get config variables */
  addr=Config_GetValue(root,
		       "address",
		       "127.0.0.1",
		       0);
  port=Config_GetIntValue(root,
			  "port",
			  CTSERVICE_DEFAULT_PORT,
			  0);
  fms=Config_GetValue(root,
		      "access",
		      CTSERVICE_DEFAULT_ACCESS,
		      0);
  tp=Config_GetValue(root,
		     "type",
		     "net",
		     0);
  if (strcasecmp(tp,"local")==0) {
#ifdef OS_WIN32
    DBG_ERROR("Local type not supported on this plattform");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_CONFIG);
#endif
    sd->serverPipe=addr;
    sd->port=-1;
    /* remove possibly existing socket file */
    if (remove(addr)==0) {
      DBG_WARN("Socket file \"%s\" existed, removed it",
	       addr);
    }
    tl=IPC_TransportLayerUnix_new();
  }
  else if (strcasecmp(tp,"net")==0) {
    tl=IPC_TransportLayerTCP_new();
    sd->port=port;
  }
  else {
    DBG_ERROR("Unknown type \"%s\"",tp);
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_CONFIG);
  }

  /* set address */
  sd->address=strdup(addr);
  err=IPC_TransportLayer_SetAddress(tl, addr);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    tl->free(tl);
    return err;
  }
  /* set port */
  IPC_TransportLayer_SetPort(tl, port);

  /* set socket options */
  sk=tl->getSocket(tl);
  if (sk) {
    err=Socket_SetReuseAddress(sk, 1);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      tl->free(tl);
      return err;
    }
  } /* if socket */

  /* make server listen */
  err=tl->listen(tl);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    tl->free(tl);
    return err;
  }

  /* ok, socket is listening, change mode if type is local */
#ifdef HAVE_CHMOD
  if (strcasecmp(tp,"local")==0) {
    int fmode;

    if (1!=sscanf(fms,"%o",&fmode)) {
      DBG_ERROR("Unknown file mode \"%s\"",fms);
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType(CTSERVICE_ERROR_TYPE),
		       CTSERVICE_ERROR_BAD_CONFIG);
    }
    if (chmod(addr,fmode)) {
      DBG_ERROR("Error on chmod(%s, %o): %s",
		addr,fmode,strerror(errno));
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType(CTSERVICE_ERROR_TYPE),
		       CTSERVICE_ERROR_SYSTEM_ERROR);
    }
  }
#endif

  /* create messagelayer */
  ml=IPCMessageLayer_new();
  /* install freeUserData callback. This one is called when the
   * messagelayer is freed */
  IPCMessageLayer_SetFreeUserDataCallback(ml, CTServer_FreeMsgLayerUserData);
  IPCMessageLayer_SetTransportLayer(ml, tl);
  IPCMessageLayer_SetStatus(ml, StateListening);
  IPCMessageLayer_SetMark(ml, sd->mark);
  /* create service layer */
  sd->service=IPCServiceLayer_new();

  /* add new message layer to service layer */
  IPCServiceLayer_AddMessageLayer(sd->service, ml);

  /* server and core are now up and running */
  return 0;
}


ERRORCODE CTServer_Fini(CTSERVERDATA *sd){
  DBG_ENTER;
  assert(sd);

  DBG_INFO("Shutting down service");
  IPCServiceLayer_ShutDown(sd->service);
  if (sd->serverPipe) {
    if (remove(sd->serverPipe)) {
      DBG_WARN("Could not remove socket file \"%s\" (%s)",
	       sd->serverPipe,
               strerror(errno));
    }
  }

  return 0;
}


ERRORCODE CTServer__HandleMessage(CTSERVERDATA *sd,
				  IPCMESSAGELAYER *ml,
				  IPCMESSAGE *msg) {
  CTSERVICEDATA *svd;
  ERRORCODE err;
  char *keydata;
  int keylen;
  unsigned char *cryptedkey;
  int cryptedkeylen;
  CRYP_RSAKEY *clientKey;
  int msgversion;
  int msgCode;

  DBG_ENTER;
  assert(sd);
  assert(ml);
  assert(msg);

  /* check for userdata in message layer. If there is none, then this
   * message layer belongs to a new client. In that case we have to
   * register a new client and store it's data within the message layer. */
  svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
  if (svd==0) {
    /* this is a new client */
    IPCTRANSPORTLAYERTABLE *tlt;
    char addrbuffer[256];

    /* set mark, since only a server can accept new clients */
    IPCMessageLayer_SetMark(ml, sd->mark);
    tlt=IPCMessageLayer_GetTransportLayer(ml);
    assert(tlt);
    assert(tlt->getPeerAddress);
    assert(tlt->getPeerPort);
    err=tlt->getPeerAddress(tlt, addrbuffer, sizeof(addrbuffer));
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      return err;
    }
    if (strlen(addrbuffer)<1) {
      DBG_NOTICE("New client, no address");
    }
    else {
      DBG_NOTICE("New client: %s:%d",
		 addrbuffer,
		 tlt->getPeerPort(tlt));
    }

    /* create new data for this client */
    svd=CTService_PeerData_new();
    assert(svd);
    IPCMessageLayer_SetUserData(ml, svd);
    IPCMessageLayer_SetFreeUserDataCallback(ml,CTServer_FreeMsgLayerUserData);
    /* assign an id in case the next higher level does not */
    IPCMessageLayer_SetId(ml, ++(sd->nextClientId));
    /* let the next higher level handle this new client */
    if (sd->clientUp) {
      err=sd->clientUp(sd,ml);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_LEAVE;
	return err;
      }
    }
    else {
      DBG_ERROR("Callback for new client not set");
      DBG_LEAVE;
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType(CTSERVICE_ERROR_TYPE),
		       CTSERVICE_ERROR_NO_CLIENT);
    }

  }

  /* get message code */
  err=IPCMessage_FirstIntParameter(msg, &msgCode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_NOTICE("Disconnecting client.");
    IPCMessageLayer_ShutDown(ml);
    DBG_LEAVE;
    return err;
  }

  /* get message version */
  err=IPCMessage_NextIntParameter(msg, &msgversion);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_NOTICE("Disconnecting client.");
    IPCMessageLayer_ShutDown(ml);
    DBG_LEAVE;
    return err;
  }
  /* now we surely have service data */

  /*_________________________________________________________________________
   *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
   *               Handling of a closed channel
   *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
   */
  if (svd->channelState==ChannelClosed) {
    /* channel is not open, so the only message allowed is
     * CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS */
    IPCMESSAGE *keymsg;
    char *keyd;
    int keydsize;
    IPCMESSAGE *replymsg;

    /* check for key exchange message */
    if (msgCode==CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS) {
      /*_____________________________________________________________________
       *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
       *               Handle a request for a secure channel
       *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
       */
      if ((msgversion&0xff00)!=
	  (CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS_VERSION&0xff00)) {
	DBG_ERROR("Key exchange: Bad message version, disconnecting");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return 0;
      }
      /* ok, message is a request for a key exchange. So the message should
       * contain the peers key as the next argument. */
      err=IPCMessage_NextParameter(msg, &keyd, &keydsize);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }

      /* if new message version then read the client id */
      if ((msgversion>0x0100)) {
	char *clientid;
	int clientidsize;

	err=IPCMessage_NextParameter(msg, &clientid, &clientidsize);
	if (!Error_IsOk(err)) {
	  DBG_ERROR_ERR(err);
	  DBG_NOTICE("Disconnecting client.");
	  IPCMessageLayer_ShutDown(ml);
	  DBG_LEAVE;
	  return err;
	}
	if (clientidsize) {
	  if (clientid[clientidsize-1]!=0) {
	    DBG_WARN("Bad client id (trailing zero missing!)")
	  }
	  else {
	    DBG_NOTICE("New client is \"%s\"",
		       clientid);
	  }
	}
	else {
	  DBG_WARN("Uups, no client id ?");
	}
      } /* if new message version */

      keymsg=IPCMessage_new();
      IPCMessage_UseBuffer(keymsg, keyd, keydsize);
      clientKey=Cryp_RsaKey_new();
      DBG_INFO("Reading remote key");
      err=Cryp_RsaKey_FromMessage(clientKey,
				  keymsg);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(keymsg);
	Cryp_RsaKey_free(clientKey);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      IPCMessage_free(keymsg);

      /* create the session key */
      DBG_INFO("Creating session key");
      svd->sessionKey=Cryp_BlowfishKey_new();
      err=Cryp_BlowfishKey_GenerateKey(svd->sessionKey);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	Cryp_RsaKey_free(clientKey);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* get session key data */
      err=Cryp_BlowfishKey_GetKey(svd->sessionKey, &keydata, &keylen);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	Cryp_RsaKey_free(clientKey);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* encrypt session key */
      DBG_INFO("Encrypting session key");
      err=Cryp_Encrypt((CRYP_RSAKEY*)clientKey,
		       CryptAlgoRSA,
		       keydata,
		       keylen,
		       &cryptedkey,
		       &cryptedkeylen);
      Cryp_RsaKey_free(clientKey);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	free(cryptedkey);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }

      /* finally create message */
      DBG_INFO("Creating session key message");
      replymsg=IPCMessage_new();
      IPCMessage_SetBuffer(replymsg,0, IPCMESSAGE_MAXMSGSIZE);
      /* msg code */
      err=IPCMessage_AddIntParameter(replymsg,
				     CTSERVICE_MSGCODE_RP_EXCHANGE_KEYS);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	free(cryptedkey);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* msg version */
      err=IPCMessage_AddIntParameter(replymsg,
				     CTSERVICE_MSGCODE_RP_EXCHANGE_KEYS_VERSION);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	free(cryptedkey);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* add session key */
      err=IPCMessage_AddParameter(replymsg, cryptedkey, cryptedkeylen);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      free(cryptedkey);

      err=IPCMessage_BuildMessage(replymsg);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }

      /* send reply */
      DBG_DEBUG("Sending message");
      err=IPCServiceLayer_SendMessage(sd->service, ml, replymsg);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* channel is open now */
      svd->channelState=ChannelOpen;
      IPCMessageLayer_UseEncryption(ml,1);
      DBG_NOTICE("Channel is open (encrypted)");
    }

    else if (msgCode==CTSERVICE_MSGCODE_RQ_OPEN) {
      /*_____________________________________________________________________
       *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
       * Handle a request for an  insecure channel (only unix domain sockets)
       *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
       */
      char *clientid;
      int clientidsize;

      if ((msgversion&0xff00)!=
	  (CTSERVICE_MSGCODE_RQ_OPEN_VERSION&0xff00)) {
	DBG_ERROR("Opening: Bad message version, disconnecting");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return 0;
      }

      /* this is a request for an unencrypted channel,
       * check whether this is allowed (for now only unix domain sockets
       * may be used for plain text channels) */
      if (IPCMessageLayer_GetType(ml)!=TransportLayerTypeUnix) {
	DBG_ERROR("Opening: Encryption needed on this transport layer, "
		  "disconnecting");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return 0;
      }

      /* read and log client id */
      err=IPCMessage_NextParameter(msg, &clientid, &clientidsize);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      if (clientidsize) {
	if (clientid[clientidsize-1]!=0) {
	  DBG_WARN("Bad client id (trailing zero missing!)")
	}
	else {
	  DBG_NOTICE("New client is \"%s\" (no encryption)",
		     clientid);
	}
      }
      else {
	DBG_WARN("Uups, no client id ?");
      }

      /* create reply message */
      replymsg=IPCMessage_new();
      IPCMessage_SetBuffer(replymsg,0, IPCMESSAGE_MAXMSGSIZE);
      /* msg code */
      err=IPCMessage_AddIntParameter(replymsg,
				     CTSERVICE_MSGCODE_RP_OPEN);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* msg version */
      err=IPCMessage_AddIntParameter(replymsg,
				     CTSERVICE_MSGCODE_RP_OPEN_VERSION);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* finalize message */
      err=IPCMessage_BuildMessage(replymsg);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* send reply */
      DBG_DEBUG("Sending message");
      err=IPCServiceLayer_SendMessage(sd->service, ml, replymsg);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	IPCMessage_free(replymsg);
	DBG_NOTICE("Disconnecting client.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return err;
      }
      /* channel is open now */
      svd->channelState=ChannelOpen;
      IPCMessageLayer_UseEncryption(ml,0);
      DBG_NOTICE("Channel is open (no encryption)");
    }
    else {
      DBG_ERROR("Channel not open, unawaited message, shutting down client");
      IPCMessageLayer_ShutDown(ml);
      DBG_LEAVE;
      return 0;
    }
  }
  /*_________________________________________________________________________
   *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
   *               Handling an already open channel
   *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
   */
  else if (svd->channelState==ChannelOpen) {
    /* channel is open, so check whether the message is encrypted */
    IPCMESSAGE *decryptedMsg;

    if (IPCMessageLayer_UsesEncryption(ml)){
      /*_____________________________________________________________________
       *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
       *                  Handle an encrypted request
       *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
       */
      DBG_DEBUG("Decrypting message");
      decryptedMsg=CTService_DecryptMessage(IPCMessageLayer_GetUserData(ml),
					    msg);
      if (decryptedMsg==0) {
	DBG_ERROR("Could not decrypt message, disconnecting.");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return 0;
      }
      /* Really handle message */
      DBG_DEBUG("Handling message");
      if (sd->handleRequest)
	err=sd->handleRequest(sd,
			      ml,
			      decryptedMsg);
      else {
	DBG_NOTICE("No request handler, request ignored.");
	err=0;
      }
      IPCMessage_free(decryptedMsg);
      if (!Error_IsOk(err)) {
	DBG_DEBUG_ERR(err);
	DBG_LEAVE;
	return err;
      }
    }
    else {
      /*_____________________________________________________________________
       *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
       *   Handle an unencrypted request (only for unix domain sockets)
       *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
       */
      /* this is an unencrypted message, check whether this is allowed */
      if (IPCMessageLayer_GetType(ml)!=TransportLayerTypeUnix) {
	DBG_ERROR("Encryption needed on this transport layer, "
		  "disconnecting");
	IPCMessageLayer_ShutDown(ml);
	DBG_LEAVE;
	return 0;
      }
      /* Really handle message */
      DBG_DEBUG("Handling message");
      if (sd->handleRequest)
	err=sd->handleRequest(sd,
			      ml,
			      msg);
      else {
	DBG_NOTICE("No request handler, request ignored.");
	err=0;
      }
      if (!Error_IsOk(err)) {
	DBG_DEBUG_ERR(err);
	DBG_LEAVE;
	return err;
      }
    }

  } /* if channel open */
  else {
    /*_____________________________________________________________________
     *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
     *               Handling of other channel states
     *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
     */
    DBG_ERROR("Bad channel state");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_CHANNEL_STATUS);
  }

  DBG_LEAVE;
  return 0;
}


ERRORCODE CTServer_Work(CTSERVERDATA *sd,
			int timeout,
			int maxmsg){
  ERRORCODE err1, err2;
  IPCMESSAGELAYER *ml;
  IPCMESSAGE *msg;
  int msgcount;

  DBG_ENTER;
  assert(sd);
  assert(sd->service);

  DBG_DEBUG("Checking connections");
  err1=IPCServiceLayer_Work(sd->service, timeout);
  if (!Error_IsOk(err1)) {
    DBG_DEBUG_ERR(err1);
  }
  DBG_DEBUG("Removing disconnected clients");
  CTServer_RemoveDisconnected(sd);

  msgcount=0;
  DBG_DEBUG("Handling some messages");
  while(msgcount<maxmsg) {
    err2=IPCServiceLayer_NextMessage(sd->service, &ml, &msg, sd->mark);
    if (!Error_IsOk(err2))
      break;
    err2=CTServer__HandleMessage(sd, ml, msg);
    if (!Error_IsOk(err2)) {
      DBG_DEBUG_ERR(err2);
    }
    else {
      DBG_INFO("Message handled.");
    }
    IPCMessage_free(msg);
    msgcount++;
  } /* while */

  DBG_LEAVE;
  return err1;
}


ERRORCODE CTServer_SendResponse(CTSERVERDATA *sd,
				IPCMESSAGELAYER *ml,
				IPCMESSAGE *msg) {
  CTSERVICEDATA *svd;
  IPCMESSAGE *encryptedMsg;
  ERRORCODE err;

  svd=(CTSERVICEDATA*)IPCMessageLayer_GetUserData(ml);
  if (svd->channelState!=ChannelOpen) {
    DBG_ERROR("Channel not open");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_CHANNEL_STATUS);
  }

  /* check whether there is a physical connection */
  if (IPCMessageLayer_GetStatus(ml)==StateDisconnected) {
    DBG_ERROR("Not connected");
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_CHANNEL_STATUS);
  }

  if (IPCMessageLayer_UsesEncryption(ml)) {
    /* is connected and open, so encrypt the message and send it */
    encryptedMsg=CTService_EncryptMessage(svd,msg);
    if (!encryptedMsg) {
      DBG_ERROR("Could not encrypt message, will ignore it");
    }
    else {
      err=IPCServiceLayer_SendMessage(sd->service,ml,encryptedMsg);
      if (!Error_IsOk(err)) {
	DBG_ERROR_ERR(err);
	DBG_LEAVE;
	IPCMessage_free(encryptedMsg);
	return err;
      }
    }
    /* encrypted version has been enqueued, so we don't need the original
     * anymore */
    IPCMessage_free(msg);
  }
  else {
    /* no encryption used, so simply send the response */
    err=IPCServiceLayer_SendMessage(sd->service,ml,msg);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      DBG_LEAVE;
      return err;
    }
  }
  return 0;
}


void CTServer_RemoveDisconnected(CTSERVERDATA *sd){
  IPCMESSAGELAYER *curr, *prev, *next;
  int id;

  DBG_ENTER;
  assert(sd);
  assert(sd->service);
  curr=sd->service->messageLayers;
  prev=0;
  while(curr) {
    next=curr->next;
    if (IPCMessageLayer_GetStatus(curr)==StateDisconnected &&
	IPCMessageLayer_IsPersistent(curr)==0) {
      DBG_INFO("Removing a client");

      /* unlink */
      if (prev)
	prev->next=curr->next;
      else
	sd->service->messageLayers=curr->next;

      id=IPCMessageLayer_GetId(curr);
      if (id!=-1) {
	/* unregister client associated with this message layer */
	if (sd->clientDown) {
	  ERRORCODE lerr;

	  DBG_INFO("Will unregister client");
	  lerr=sd->clientDown(sd, curr);
	  if (!Error_IsOk(lerr)) {
	    DBG_ERROR_ERR(lerr);
	  }
	}
	else {
	  DBG_WARN("No call back handler for clientDown");
	}
	IPCMessageLayer_SetId(curr, -1);
	DBG_NOTICE("Unregistered client %d", id);
      }

      /* free */
      IPCMessageLayer_free(curr);

    } /* if disconnected */
    prev=curr;
    curr=next;
  } /* while */
  DBG_LEAVE;
}


void CTServer_SetRequestHandler(CTSERVERDATA *sd,
				CTSERVER_HANDLEREQUESTPTR p){
  assert(sd);
  sd->handleRequest=p;
}


void CTServer_SetClientUpHandler(CTSERVERDATA *sd,
				 CTSERVER_CLIENTUPPTR p){
  assert(sd);
  sd->clientUp=p;
}


void CTServer_SetClientDownHandler(CTSERVERDATA *sd,
				   CTSERVER_CLIENTDOWNPTR p){
  assert(sd);
  sd->clientDown=p;
}


void CTServer_SetPrivateData(CTSERVERDATA *sd,
			     void *p){
  assert(sd);
  sd->privateData=p;
}


void *CTServer_GetPrivateData(CTSERVERDATA *sd){
  assert(sd);
  return sd->privateData;
}


ERRORCODE CTServer_SendErrorMessage(CTSERVERDATA *sd,
				    IPCMESSAGELAYER *ml,
				    IPCMESSAGE *req,
				    ERRORCODE errcode) {
  ERRORCODE err;
  IPCMESSAGE *msg;
  int newerrcode;
  char errorbuffer[256];
  int requestid;
  int i;

  DBG_ENTER;
  assert(sd);

  /* get request id */
  err=IPCMessage_FirstIntParameter(req, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if (i>=0x10000) {
    /* open message, so no request id */
    requestid=0;
  }
  else {
    /* requestid */
    err=IPCMessage_IntParameter(req, 2, &requestid);
    if (!Error_IsOk(err)) {
      DBG_ERROR_ERR(err);
      DBG_LEAVE;
      return err;
    }
  }

  /* create response */
  msg=CTService_Message_Create(CTSERVICE_MSGCODE_RP_ERROR,
			       CTSERVICE_MSGCODE_RP_ERROR_VERSION,
			       ++(sd->nextMessageId),
			       requestid,
			       300);
  if (!msg) {
    DBG_ERROR("Could not create message");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_MESSAGE);
  }

  /* create new error code */
  if (Error_GetType(errcode)==Error_FindType(CTSERVICE_ERROR_TYPE))
    newerrcode=-Error_GetCode(errcode);
  else
    newerrcode=-CTSERVICE_ERROR_REMOTE;

  /* add error id */
  err=IPCMessage_AddIntParameter(msg, newerrcode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return err;
  }

  /* add error message */
  if (Error_ToString(errcode,
		     errorbuffer,
		     sizeof(errorbuffer))==0)
    strcpy(errorbuffer,"Unspecified error");
  err=IPCMessage_AddParameter(msg, errorbuffer, strlen(errorbuffer)+1);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return err;
  }

  /* finalize message */
  err=IPCMessage_BuildMessage(msg);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return err;
  }

  /* enqueue the message for sending */
  err=CTServer_SendResponse(sd,ml,msg);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return err;
  }

  DBG_INFO("Message enqueued for sending");
  DBG_LEAVE;
  return 0;
}



