/***************************************************************************
 $RCSfile: backupserver.c,v $
                             -------------------
    cvs         : $Id: backupserver.c,v 1.2 2003/06/11 13:18:35 aquamaniac Exp $
    begin       : Sat Jun 07 2003
    copyright   : (C) 2003 by Martin Preuss
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
#include "backupserver_p.h"
#include "backup/errors.h"

#include <ctserver.h>
#include <chameleon/conf.h>
#include <chameleon/debug.h>


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>



BACKUPSERVERDATA *BackupServerData_new(){
  BACKUPSERVERDATA *bd;

  bd=(BACKUPSERVERDATA *)malloc(sizeof(BACKUPSERVERDATA));
  assert(bd);
  memset(bd, 0, sizeof(BACKUPSERVERDATA));
  bd->bserver=AQBServer_new();
  return bd;
}



void BackupServerData_free(BACKUPSERVERDATA *bd){
  if (bd) {
    AQBServer_free(bd->bserver);
    free(bd);
  }
}



BACKUPSERVERPEERDATA *BackupServerPeerData_new(){
  BACKUPSERVERPEERDATA *bpd;

  bpd=(BACKUPSERVERPEERDATA *)malloc(sizeof(BACKUPSERVERPEERDATA));
  assert(bpd);
  memset(bpd, 0, sizeof(BACKUPSERVERPEERDATA));
  return bpd;
}



void BackupServerPeerData_free(BACKUPSERVERPEERDATA *bpd){
  if (bpd) {
    free(bpd->clientName);
    free(bpd);
  }
}



void BackupServerPeerData_freeUserData(void *d){
  BackupServerPeerData_free((BACKUPSERVERPEERDATA *)d);
}




/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *           Handler for the various supported command messages
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



ERRORCODE BackupServer__SendErrorMessage(CTSERVERDATA *sd,
					 IPCMESSAGELAYER *ml,
					 IPCMESSAGE *req,
					 ERRORCODE errcode) {
  ERRORCODE err;
  IPCMESSAGE *msg;
  BACKUPSERVERDATA *rsd;
  int newerrcode;
  char errorbuffer[256];
  int requestid;
  int i;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

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



ERRORCODE BackupServer__HandlePing(CTSERVERDATA *sd,
				   IPCMESSAGELAYER *ml,
				   IPCMESSAGE *req) {
  ERRORCODE err;
  IPCMESSAGE *msg;
  int requestid;
  int i;
  BACKUPSERVERDATA *rsd;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

  /* check version */
  err=IPCMessage_IntParameter(req, 1, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if ((i&0xff00)!=(BACKUPSERVICE_MSGCODE_RQ_PING_VERSION&0xff00)) {
    DBG_ERROR("Bad message version");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_MESSAGE_VERSION);
  }

  /* get request id */
  err=IPCMessage_NextIntParameter(req, &requestid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }

  /* create response */
  msg=CTService_Message_Create(BACKUPSERVICE_MSGCODE_RP_PING,
			       BACKUPSERVICE_MSGCODE_RP_PING_VERSION,
			       ++(sd->nextMessageId),
			       requestid,
                               256);
  if (!msg) {
    DBG_ERROR("Could not create message");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_MESSAGE);
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



ERRORCODE BackupServer__HandleRegister(CTSERVERDATA *sd,
				       IPCMESSAGELAYER *ml,
				       IPCMESSAGE *req) {
  ERRORCODE err;
  IPCMESSAGE *msg;
  int requestid;
  int i;
  BACKUPSERVERDATA *rsd;
  BACKUPSERVERPEERDATA *spd;
  char *p;
  int result;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

  /* check version */
  err=IPCMessage_IntParameter(req, 1, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if ((i&0xff00)!=(BACKUPSERVICE_MSGCODE_RQ_REGISTER_VERSION&0xff00)) {
    DBG_ERROR("Bad message version");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_MESSAGE_VERSION);
  }

  /* get request id */
  err=IPCMessage_NextIntParameter(req, &requestid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }

  /* get client id */
  err=IPCMessage_StringParameter(req, 4, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }

  spd=CTService_GetPeerUserData(ml);
  assert(spd);

  if (spd->cid!=0) {
    if (!spd->clientName) {
      DBG_ERROR("Client without name is already registered, please check");
      result=0;
    }
    else {
      if (strcmp(spd->clientName, p)==0) {
	DBG_NOTICE("Client \"%s\" already registered", p);
	result=1;
      }
      else {
	DBG_NOTICE("Client \"%s\" still registered, please unregeister first",
		   spd->clientName);
	result=0;
      }
    }
  }

  spd->cid=AQBServer_ClientRegister(rsd->bserver, p);
  if (spd==0) {
    DBG_NOTICE("Error registering client \"%s\"",p);
    result=0;
  }
  else {
    spd->clientName=strdup(p);
    result=1;
  }

  /* create response */
  msg=CTService_Message_Create(BACKUPSERVICE_MSGCODE_RP_REGISTER,
			       BACKUPSERVICE_MSGCODE_RP_REGISTER_VERSION,
			       ++(sd->nextMessageId),
			       requestid,
                               256);
  if (!msg) {
    DBG_ERROR("Could not create message");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_MESSAGE);
  }

  /* add result */
  err=IPCMessage_AddIntParameter(msg, result);
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



ERRORCODE BackupServer__HandleUnregister(CTSERVERDATA *sd,
					 IPCMESSAGELAYER *ml,
					 IPCMESSAGE *req) {
  ERRORCODE err;
  IPCMESSAGE *msg;
  int requestid;
  int i;
  BACKUPSERVERDATA *rsd;
  BACKUPSERVERPEERDATA *spd;
  char *p;
  int result;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

  /* check version */
  err=IPCMessage_IntParameter(req, 1, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }
  if ((i&0xff00)!=(BACKUPSERVICE_MSGCODE_RQ_UNREGISTER_VERSION&0xff00)) {
    DBG_ERROR("Bad message version");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(CTSERVICE_ERROR_TYPE),
		     CTSERVICE_ERROR_BAD_MESSAGE_VERSION);
  }

  /* get request id */
  err=IPCMessage_NextIntParameter(req, &requestid);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }

  /* get client id */
  err=IPCMessage_StringParameter(req, 4, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    DBG_LEAVE;
    return err;
  }

  spd=CTService_GetPeerUserData(ml);
  assert(spd);

  if (spd->cid!=0) {
    if (!spd->clientName) {
      DBG_ERROR("Client without name is already registered, please check");
      result=0;
    }
    else {
      if (strcmp(spd->clientName, p)==0) {
	DBG_NOTICE("Client \"%s\" already registered", p);
	result=1;
      }
      else {
	DBG_NOTICE("Client \"%s\" still registered, please unregeister first",
		   spd->clientName);
	result=0;
      }
    }
  }

  result=AQBServer_ClientUnregister(rsd->bserver, spd->cid);
  if (result) {
    DBG_NOTICE("Error unregistering client \"%s\"", spd->clientName);
  }

  /* create response */
  msg=CTService_Message_Create(BACKUPSERVICE_MSGCODE_RP_UNREGISTER,
			       BACKUPSERVICE_MSGCODE_RP_UNREGISTER_VERSION,
			       ++(sd->nextMessageId),
			       requestid,
                               256);
  if (!msg) {
    DBG_ERROR("Could not create message");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType(IPCMESSAGE_ERROR_TYPE),
		     IPCMESSAGE_ERROR_NO_MESSAGE);
  }

  /* add result */
  err=IPCMessage_AddIntParameter(msg, result);
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







ERRORCODE BackupServer_RequestHandler(CTSERVERDATA *sd,
				      IPCMESSAGELAYER *ml,
				      IPCMESSAGE *msg) {
  ERRORCODE err;
  int msgCode;
  IPCTRANSPORTLAYERTABLE *tlt;
  char addrbuffer[256];
  BACKUPSERVERDATA *rsd;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

  /* get peer address and port */
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
    /* no address, so use the id */
#ifdef HAVE_SNPRINTF
    snprintf(addrbuffer,sizeof(addrbuffer),
	     "Id %d", IPCMessageLayer_GetId(ml));
#else
    sprintf(addrbuffer,"Id %d", IPCMessageLayer_GetId(ml));
#endif
  }

  err=IPCMessage_FirstIntParameter(msg, &msgCode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return err;
  }
  switch (msgCode) {
  case BACKUPSERVICE_MSGCODE_RQ_PING:
    DBG_NOTICE("REQUEST: Ping (%s)",addrbuffer);
    err=BackupServer__HandlePing(sd, ml, msg);
    break;
  case BACKUPSERVICE_MSGCODE_RQ_REGISTER:
    DBG_NOTICE("REQUEST: Register (%s)",addrbuffer);
    err=BackupServer__HandleRegister(sd, ml, msg);
    break;
  case BACKUPSERVICE_MSGCODE_RQ_UNREGISTER:
    DBG_NOTICE("REQUEST: Unregister (%s)",addrbuffer);
    err=BackupServer__HandleUnregister(sd, ml, msg);
    break;
  default:
    DBG_ERROR("Message code %d not supported (%s)",
	      msgCode,
	      addrbuffer);
    err=Error_New(0,
		  ERROR_SEVERITY_ERR,
		  Error_FindType(CTSERVICE_ERROR_TYPE),
		  CTSERVICE_ERROR_BAD_MESSAGE_CODE);
    break;
  } /* switch */

  if (!Error_IsOk(err)) {
    ERRORCODE localerr;

    localerr=BackupServer__SendErrorMessage(sd,
					    ml,
					    msg,
					    err);
    if (!Error_IsOk(localerr)) {
      DBG_ERROR_ERR(localerr);
    }
  }

  DBG_LEAVE;
  return err;
}



ERRORCODE BackupServer_ClientUp(CTSERVERDATA *sd,
				IPCMESSAGELAYER *ml){
  return 0;
}



ERRORCODE BackupServer_ClientDown(CTSERVERDATA *sd,
				  IPCMESSAGELAYER *ml){
  return 0;
}




int BackupServer_Init(CTSERVERDATA *sd, CONFIGGROUP *root) {
  BACKUPSERVERDATA *rsd;
  ERRORCODE err;
  int rv;

  DBG_INFO("Initializing reader server");
  assert(sd);

  /* initialize server */
  err=CTServer_Init(sd, root);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return err;
  }

  /* create new private data */
  rsd=BackupServerData_new();
  assert(rsd);

  /* ok, server is running, now setup Backupservice */
  rv=AQBServer_Init(rsd->bserver, root);
  if (rv) {
    DBG_INFO("Error initializing backup server");
    BackupServerData_free(rsd);
    return BACKUP_ERROR_SERVER_INIT;
  }

  /* set private data and callbacks */
  CTServer_SetPrivateData(sd, rsd);
  CTServer_SetRequestHandler(sd, BackupServer_RequestHandler);
  CTServer_SetClientUpHandler(sd, BackupServer_ClientUp);
  CTServer_SetClientDownHandler(sd, BackupServer_ClientDown);


  DBG_INFO("Backup server initialized");
  return 0;
}



ERRORCODE BackupServer_Fini(CTSERVERDATA *sd){
  ERRORCODE err1, err2;
  BACKUPSERVERDATA *rsd;

  DBG_ENTER;
  DBG_INFO("Deinitializing reader server");
  assert(sd);
  rsd=CTServer_GetPrivateData(sd);
  assert(rsd);

  err1=CTServer_Fini(sd);
  if (!Error_IsOk(err1)) {
    DBG_ERROR_ERR(err1);
  }

  CTServer_SetPrivateData(sd,0);
  BackupServerData_free(rsd);

  DBG_INFO("Reader server deinitialized.");
  DBG_LEAVE;
  if (!Error_IsOk(err1))
    return err1;
  if (!Error_IsOk(err2))
    return err2;

  return 0;
}



ERRORCODE BackupServer_Work(CTSERVERDATA *sd,
			    int timeout,
			    int maxmsg){
  ERRORCODE err;
  BACKUPSERVERDATA *rsd;

  DBG_ENTER;
  assert(sd);
  rsd=(BACKUPSERVERDATA*)CTServer_GetPrivateData(sd);
  assert(rsd);

  err=CTServer_Work(sd, timeout, maxmsg);
  if (!Error_IsOk(err)) {
    DBG_DEBUG_ERR(err);
  }


  /* do whatever is necessary */

  DBG_LEAVE;
  return err;
}
