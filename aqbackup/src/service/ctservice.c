/***************************************************************************
 $RCSfile: ctservice.c,v $
 -------------------
 cvs         : $Id: ctservice.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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
#include <ctservice.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <chameleon/debug.h>


#ifdef MEMORYTRACE
static int CTSERVICEREQUEST__Counter=0;
static int CTSERVICEDATA__Counter=0;
#endif


const char *CTService_ErrorString(int c);


int ctservice_is_initialized=0;
ERRORTYPEREGISTRATIONFORM ctservice_error_descr= {
  CTService_ErrorString,
  0,
  CTSERVICE_ERROR_TYPE};


ERRORCODE CTService_ModuleInit(){
  if (!ctservice_is_initialized) {
    if (!Error_RegisterType(&ctservice_error_descr))
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ERROR_TYPE_ERROR,
		       ERROR_COULD_NOT_REGISTER);
    ctservice_is_initialized=1;
  }
  return 0;
}


ERRORCODE CTService_ModuleFini(){
  if (ctservice_is_initialized) {
    ctservice_is_initialized=0;

    if (!Error_UnregisterType(&ctservice_error_descr)) {
      DBG_ERROR("Could not unregister");
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       0,
		       ERROR_COULD_NOT_UNREGISTER);
    }
  } /* if initialized */
  return 0;
}


CTSERVICEDATA *CTService_PeerData_new(){
  CTSERVICEDATA *pd;

  DBG_ENTER;
  pd=(CTSERVICEDATA *)malloc(sizeof(CTSERVICEDATA));
  assert(pd);
  memset(pd,0,sizeof(CTSERVICEDATA));
#ifdef MEMORYTRACE
  CTSERVICEDATA__Counter++;
  DBG_NOTICE("Created PeerData. now %d",CTSERVICEDATA__Counter);
#endif
  DBG_LEAVE;
  return pd;
}


void CTService_PeerData_free(CTSERVICEDATA *pd){
  CTSERVICEREQUEST *req;

  DBG_ENTER;
  if (pd) {
    /* free tempKey */
    if (pd->tempKey)
      Cryp_RsaKey_free(pd->tempKey);

    /* free sessionKey */
    if (pd->sessionKey)
      Cryp_BlowfishKey_free(pd->sessionKey);

    /* free requests */
    req=pd->requests;
    while(req) {
      CTSERVICEREQUEST *next;

      next=req->next;
      CTService_Request_free(req);
      req=next;
    } /* while */

    /* free the service data itself */
    free(pd);
#ifdef MEMORYTRACE
    CTSERVICEDATA__Counter--;
    DBG_NOTICE("Freed PeerData. now %d",CTSERVICEDATA__Counter);
#endif
  }
  DBG_LEAVE;
}


CTSERVICEREQUEST *CTService_Request_new(){
  CTSERVICEREQUEST *rq;

  DBG_ENTER;
  rq=(CTSERVICEREQUEST *)malloc(sizeof(CTSERVICEREQUEST));
  assert(rq);
  memset(rq,0,sizeof(CTSERVICEREQUEST));
#ifdef MEMORYTRACE
  CTSERVICEREQUEST__Counter++;
  DBG_NOTICE("Created request, now %d",CTSERVICEREQUEST__Counter);
#endif
  DBG_LEAVE;
  return rq;
}


void CTService_Request_free(CTSERVICEREQUEST *rq){
  DBG_ENTER;
  if (rq) {
    IPCMESSAGE *curr;
    IPCMESSAGE *next;

    DBG_INFO("Freeing request");

    /* remove message */
    if (rq->message)
      IPCMessage_free(rq->message);

    /* remove responses */
    curr=rq->responses;
    while(curr) {
      next=curr->next;
      IPCMessage_free(curr);
      curr=next;
    }

    /* free request itself */
    free(rq);
#ifdef MEMORYTRACE
    CTSERVICEREQUEST__Counter--;
    DBG_NOTICE("Freed request, now %d",CTSERVICEREQUEST__Counter);
#endif
  }
  DBG_LEAVE;
}


IPCMESSAGE *CTService_Request_NextResponse(CTSERVICEREQUEST *rq){
  IPCMESSAGE *msg;

  DBG_ENTER;
  assert(rq);
  msg=rq->responses;
  if (msg)
    rq->responses=msg->next;
  DBG_LEAVE;
  return msg;
}


IPCMESSAGE *CTService_Request_PeekResponse(CTSERVICEREQUEST *rq){
  IPCMESSAGE *msg;

  DBG_ENTER;
  assert(rq);
  msg=rq->responses;

  DBG_LEAVE;
  return msg;
}




IPCMESSAGE *CTService_EncryptMessage(CTSERVICEDATA *pd,
				     IPCMESSAGE *msg) {
  unsigned char *cryptbuffer;
  int cryptbuffersize;
  int endbuffersize;
  ERRORCODE err;
  IPCMESSAGE *tmpmsg;
  IPCMESSAGE *endmsg;

  DBG_ENTER;
  assert(pd);
  assert(msg);
  assert(pd->sessionKey);

  /* create intermediate message
   * This message contains:
   * - original message
   * - signature id (if sign!=0)
   * - signature    (if sign!=0)
   */
  tmpmsg=IPCMessage_new();
  IPCMessage_SetBuffer(tmpmsg,0,IPCMESSAGE_MAXMSGSIZE);

  /* add original message */
  DBG_VERBOUS("Adding original message");

  err=IPCMessage_AddParameter(tmpmsg,
			      IPCMessage_GetMessageBegin(msg),
			      IPCMessage_GetMessageSize(msg));
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(tmpmsg);
    DBG_LEAVE;
    return 0;
  }

  /* finalize intermediate message */
  DBG_VERBOUS("Finalizing message");
  err=IPCMessage_BuildMessage(tmpmsg);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(tmpmsg);
    DBG_LEAVE;
    return 0;
  }

  /* encrypt intermediate message */
  DBG_VERBOUS("Encrypting message");
  err=Cryp_Encrypt((CRYP_BFKEY*)pd->sessionKey,
		   CryptAlgoBlowfish,
		   IPCMessage_GetMessageBegin(tmpmsg),
		   IPCMessage_GetMessageSize(tmpmsg),
		   &cryptbuffer,
		   &cryptbuffersize);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(tmpmsg);
    DBG_LEAVE;
    return 0;
  }

  /* now the cryptbuffer contains the encrypted message  */

  /* free unneeded buffers */
  IPCMessage_free(tmpmsg);

  /* create final message */
  DBG_VERBOUS("Building final message");
  endbuffersize=cryptbuffersize+32;
  if (endbuffersize>IPCMESSAGE_MAXMSGSIZE)
    endbuffersize=IPCMESSAGE_MAXMSGSIZE;
  endmsg=IPCMessage_new();
  IPCMessage_SetBuffer(endmsg,0, endbuffersize);

  /* add message code */
  err=IPCMessage_AddIntParameter(endmsg,CTSERVICE_MSGCODE_BF);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(endmsg);
    free(cryptbuffer);
    DBG_LEAVE;
    return 0;
  }

  /* add message version */
  err=IPCMessage_AddIntParameter(endmsg,CTSERVICE_MSGCODE_BF_VERSION);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(endmsg);
    free(cryptbuffer);
    DBG_LEAVE;
    return 0;
  }

  /* add encrypted message */
  err=IPCMessage_AddParameter(endmsg,
			      cryptbuffer,
			      cryptbuffersize);
  free(cryptbuffer);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(endmsg);
    DBG_LEAVE;
    return 0;
  }

  /* finalize final message */
  err=IPCMessage_BuildMessage(endmsg);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(endmsg);
    DBG_LEAVE;
    return 0;
  }

  /* return encrypted message */
    DBG_LEAVE;
  return endmsg;
}


IPCMESSAGE *CTService_DecryptMessage(CTSERVICEDATA *pd,
				     IPCMESSAGE *msg) {
  ERRORCODE err;
  unsigned char *decryptbuffer;
  int decryptbuffersize;
  char *endbuffer;
  char *pencmsg;
  int encmsgsize;
  char *pomsg;
  int omsgsize;
  int i;
  IPCMESSAGE *tmpmsg;
  IPCMESSAGE *endmsg;

  assert(pd);
  assert(msg);
  assert(pd->sessionKey);

  /* get message code */
  err=IPCMessage_FirstIntParameter(msg,
				   &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR("No message code");
    DBG_ERROR_ERR(err);
    return 0;
  }
  if (i!=CTSERVICE_MSGCODE_BF) {
    DBG_ERROR("Bad message code");
    return 0;
  }

  /* get message version */
  err=IPCMessage_NextIntParameter(msg,
				  &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR("No message version");
    DBG_ERROR_ERR(err);
    return 0;
  }

  /* check message version (only major number) */
  if ((i&0xff00)!=(CTSERVICE_MSGCODE_BF_VERSION&0xff00)) {
    DBG_ERROR("Bad message version");
    return 0;
  }

  /* get encrypted message */
  err=IPCMessage_NextParameter(msg,
			       &pencmsg,
			       &encmsgsize);
  if (!Error_IsOk(err)) {
    DBG_ERROR("No encrypted message within");
    DBG_ERROR_ERR(err);
    return 0;
  }
  /* decrypt message */
  DBG_VERBOUS("Decrypting message");
  err=Cryp_Decrypt(pd->sessionKey,
		   CryptAlgoBlowfish,
		   pencmsg,
		   encmsgsize,
		   &decryptbuffer,
		   &decryptbuffersize);
  if (!Error_IsOk(err)) {
    DBG_ERROR("Error when decrypting");
    DBG_ERROR_ERR(err);
    return 0;
  }

  /* analyze message */
  DBG_VERBOUS("Analyzing Decrypted message");
  tmpmsg=IPCMessage_new();
  err=IPCMessage_SetBuffer(tmpmsg,
			   decryptbuffer,
			   decryptbuffersize);
  if (!Error_IsOk(err)) {
    DBG_ERROR("Could not set buffer");
    DBG_ERROR_ERR(err);
    IPCMessage_free(tmpmsg);
    return 0;
  }
  /* get original message */
  DBG_VERBOUS("Getting original message");
  err=IPCMessage_FirstParameter(tmpmsg,
				&pomsg,
				&omsgsize);
  if (!Error_IsOk(err)) {
    DBG_ERROR("No original message");
    DBG_ERROR_ERR(err);
    IPCMessage_free(tmpmsg);
    return 0;
  }

  /* copy original message into new buffer */
  endbuffer=(char*)malloc(omsgsize);
  assert(endbuffer);
  memmove(endbuffer, pomsg, omsgsize);
  IPCMessage_free(tmpmsg);

  /* create final message */
  endmsg=IPCMessage_new();
  err=IPCMessage_SetBuffer(endmsg,
			   endbuffer,
			   omsgsize);
  if (!Error_IsOk(err)) {
    DBG_ERROR("Could not set buffer in endmessage");
    DBG_ERROR_ERR(err);
    IPCMessage_free(endmsg);
    return 0;
  }

  /* return decoded message */
  DBG_DEBUG("Message decoded");
#if DEBUGMODE>5
  Chameleon_DumpString(IPCMessage_GetMessageBegin(endmsg),
		       IPCMessage_GetMessageSize(endmsg));
#endif
  return endmsg;
}


void CTService_Request_AddRequest(CTSERVICEREQUEST *req,
				  CTSERVICEREQUEST **head) {
  CTSERVICEREQUEST *curr;

  DBG_ENTER;
  assert(req);
  assert(head);

  curr=*head;
  if (!curr) {
    *head=req;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=req;
  }
  DBG_LEAVE;
}


void CTService_Request_RemoveRequest(CTSERVICEREQUEST *req,
				     CTSERVICEREQUEST **head) {
  CTSERVICEREQUEST *curr;

  DBG_ENTER;
  assert(req);
  assert(head);

  curr=*head;
  if (curr) {
    if (curr==req) {
      *head=curr->next;
    }
    else {
      /* find predecessor */
      while(curr->next!=req) {
	curr=curr->next;
      } /* while */
      if (curr)
	curr->next=req->next;
    }
  }
  DBG_LEAVE;
}


CTSERVICEREQUEST *CTService_Request_FindRequest(int id,
						CTSERVICEREQUEST **head) {
  CTSERVICEREQUEST *curr;

  DBG_ENTER;
  assert(head);

  curr=*head;

  while(curr) {
    if (curr->requestId==id) {
      DBG_VERBOUS("Found request %d", id);
      return curr;
    }
    curr=curr->next;
  } /* while */
  DBG_LEAVE;
  DBG_VERBOUS("Request %d not found", id);
  return 0;
}


IPCMESSAGE *CTService_Message_Create(int msgCode,
				     int msgVersion,
				     int msgId,
				     int msgReply,
				     int msgSize) {
  IPCMESSAGE *msg;
  ERRORCODE err;

  DBG_ENTER;
  DBG_VERBOUS("Creating message: %d, %d, %d, %d\n",
	      msgCode, msgVersion, msgId, msgReply);
  msg=IPCMessage_new();
  err=IPCMessage_SetBuffer(msg, 0, msgSize);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    return 0;
  }
  /* add msg code */
  err=IPCMessage_AddIntParameter(msg, msgCode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return 0;
  }
  /* add msg version */
  err=IPCMessage_AddIntParameter(msg,msgVersion);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return 0;
  }
  /* add message id */
  err=IPCMessage_AddIntParameter(msg, msgId);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return 0;
  }
  /* add msg reply code  */
  err=IPCMessage_AddIntParameter(msg, msgReply);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    IPCMessage_free(msg);
    DBG_LEAVE;
    return 0;
  }

  DBG_LEAVE;
  return msg;
}


CTSERVICEREQUEST *CTService_Request_Create(int serviceid,
					   int msgCode,
					   int msgVersion,
					   int msgId,
					   int msgReply,
					   int msgSize) {
  CTSERVICEREQUEST *req;

  DBG_ENTER;
  req=CTService_Request_new();
  req->requestId=msgId;
  req->serviceId=serviceid;
  req->message=CTService_Message_Create(msgCode,
					msgVersion,
					msgId,
					msgReply,
					msgSize);
  if (!req->message) {
    DBG_ERROR("Could not create message");
    CTService_Request_free(req);
    return 0;
  }

  DBG_LEAVE;
  return req;
}


ERRORCODE CTService_CheckMsgCodeAndVersion(IPCMESSAGE *msg,
					   int msgCode,
					   int msgVersion) {
  int i;
  ERRORCODE err;

  DBG_ENTER;
  assert(msg);

  /* check message code */
  err=IPCMessage_FirstIntParameter(msg, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if (i!=msgCode) {
    DBG_ERROR("Bad message code (%04x)",i);
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_MESSAGE_CODE);
  }

  /* check message version */
  err=IPCMessage_NextIntParameter(msg, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if ((i&0xff00)!=(msgVersion&0xff00)) {
    DBG_ERROR("Bad message version (%04x:%04x)",msgCode,i);
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_MESSAGE_VERSION);
  }
  DBG_LEAVE;
  return 0;
}




int Debug_CompareKeys(CRYP_RSAKEY *key1, CRYP_RSAKEY *key2) {
  IPCMESSAGE *kmsg1, *kmsg2;
  int ks1, ks2;
  ERRORCODE err;

  kmsg1=IPCMessage_new();
  IPCMessage_SetBuffer(kmsg1,0,4096);
  kmsg2=IPCMessage_new();
  IPCMessage_SetBuffer(kmsg2,0,4096);

  err=Cryp_RsaKey_ToMessage(key1, kmsg1, 1);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  err=Cryp_RsaKey_ToMessage(key2, kmsg2, 1);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }
  ks1=IPCMessage_GetMessageSize(kmsg1);
  ks2=IPCMessage_GetMessageSize(kmsg2);

  DBG_DEBUG("Sizes: Key1=%d, Key2=%d\n",ks1, ks2);
  if (ks1==ks2) {
    const char *ptr1, *ptr2;

    ptr1=IPCMessage_GetMessageBegin(kmsg1);
    ptr2=IPCMessage_GetMessageBegin(kmsg2);
    while(ks1) {
      if (*ptr1!=*ptr2) {
	DBG_ERROR("Keys differ !\n");
	break;
	ptr1++;
	ptr2++;
        ks1--;
      }
    } /* while */
  }
  IPCMessage_free(kmsg1);
  IPCMessage_free(kmsg2);
  if (!ks1)
    return 0;
  else
    return 1;
}


unsigned int Debug_CreateKeyFingerprint(CRYP_RSAKEY *key1) {
  IPCMESSAGE *kmsg1;
  int ks1;
  ERRORCODE err;
  const char *ptr1;
  unsigned int fp;

  kmsg1=IPCMessage_new();
  IPCMessage_SetBuffer(kmsg1,0,4096);

  err=Cryp_RsaKey_ToMessage(key1, kmsg1, 1);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
  }

  ks1=IPCMessage_GetMessageSize(kmsg1);

  ptr1=IPCMessage_GetMessageBegin(kmsg1);
  fp=0;
  while(ks1) {
    fp+=(unsigned char)(*ptr1);
    ptr1++;
    ks1--;
  } /* while */
  IPCMessage_free(kmsg1);

  DBG_DEBUG("Fingerprint is: %08x\n",fp);
  return fp;
}


unsigned int Debug_DataFingerPrint(const char *data, int size) {
  unsigned int code;

  assert(data);

  code=0;
  while(size) {
    code+=(unsigned char)(*data);
    data++;
    size--;
  }
  return code;
}


const char *CTService_ErrorString(int c){
  const char *s;
  int remote;

  remote=(c<0);
  if (remote)
    c=-c;

  switch(c) {
  case 0:
    s="Success";
    break;
  case CTSERVICE_ERROR_NO_REQUEST:
    s="No request";
    break;
  case CTSERVICE_ERROR_NO_MESSAGE:
    s="No message";
    break;
  case CTSERVICE_ERROR_BAD_MESSAGE_VERSION:
    if (!remote)
      s="Bad message version";
    else
      s="Bad message version [remote]";
    break;
  case CTSERVICE_ERROR_BAD_CHANNEL_STATUS:
    s="Bad channel status (in most cases not open)";
    break;
  case CTSERVICE_ERROR_BAD_MESSAGE_CODE:
    if (!remote)
      s="Bad message code";
    else
      s="Bad message code [remote]";
    break;
  case CTSERVICE_ERROR_BAD_BUFFERSIZE:
    if (!remote)
      s="Buffer too small or too big";
    else
      s="Buffer too small or too big [remote]";
    break;
  case CTSERVICE_ERROR_DRIVER:
    if (!remote)
      s="Driver error";
    else
      s="Driver error [remote]";
    break;
  case CTSERVICE_ERROR_INVALID:
    if (!remote)
      s="Invalid argument";
    else
      s="Invalid argument [remote]";
    break;
  case CTSERVICE_ERROR_BUFFER:
    if (!remote)
      s="Buffer error (most likely an internal error)";
    else
      s="Buffer error (most likely an internal error) [remote]";
    break;
  case CTSERVICE_ERROR_NO_COMMANDS:
    s="No commands loaded";
    break;
  case CTSERVICE_ERROR_NO_CONFIG:
    s="No/bad configuration file";
    break;
  case CTSERVICE_ERROR_UNREACHABLE:
    s="Service unreachable";
    break;
  case CTSERVICE_ERROR_NO_MESSAGELAYER:
    s="No message layer";
    break;
  case CTSERVICE_ERROR_NO_CLIENT:
    s="No client";
    break;
  case CTSERVICE_ERROR_REMOTE:
    s="Unspecified remote error";
    break;
  case CTSERVICE_ERROR_NO_TRANSPORT_LAYER:
    s="No open transport layer";
    break;
  case CTSERVICE_ERROR_BAD_CONFIG:
    s="Error in configuration file";
    break;
  case CTSERVICE_ERROR_SYSTEM_ERROR:
    s="System error";
    break;
  case CTSERVICE_ERROR_EXPIRED:
    s="Expired";
    break;
  case CTSERVICE_ERROR_INTERRUPTED:
    s="Systemcall interrupted.";
    break;
  case CTSERVICE_ERROR_INTERNAL:
    s="Internal error";
    break;
  case CTSERVICE_ERROR_NO_SERVICE:
    s="Service not available";
    break;
  case CTSERVICE_ERROR_CARD_REMOVED:
    s="Card removed";
    break;
  case  CTSERVICE_ERROR_CARD_LOCKED:
    s="Card locked";
    break;
  default:
    s=(const char*)0;
  } // switch
  return s;
}



