/***************************************************************************
 $RCSfile: ipcmessagelayer.c,v $
 -------------------
 cvs         : $Id: ipcmessagelayer.c,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <chameleon/ipcmessagelayer.h>
#include <chameleon/chameleon.h>
#include <chameleon/debug.h>


#ifdef MEMORYTRACE
static int IPCMessageLayer_Counter=0;
#endif



IPCMESSAGELAYER *IPCMessageLayer_new(){
  IPCMESSAGELAYER *m;

  m=(IPCMESSAGELAYER*)malloc(sizeof(IPCMESSAGELAYER));
  assert(m);
  memset(m,0,sizeof(IPCMESSAGELAYER));
  m->id=-1;
#ifdef MEMORYTRACE
  IPCMessageLayer_Counter++;
  DBG_NOTICE("Created IPCMessageLayer, now %d",IPCMessageLayer_Counter);
#endif
  return m;
}


void IPCMessageLayer_free(IPCMESSAGELAYER *m){
  IPCMESSAGE *curr, *next;

  assert(m);

  /* free user data, if possible */
  if (m->userData && m->freeUserData_cb)
    m->freeUserData_cb(m);

  /* free currently used read buffer */
  if (m->currentReadBuffer)
    free(m->currentReadBuffer);

  /* free all messages in outqueue */
  curr=m->outQueue;
  while(curr) {
    next=curr->next;
    IPCMessage_free(curr);
    curr=next;
  } /* while */
  m->outQueue=0;

  /* free all messages in inqueue */
  curr=m->inQueue;
  while(curr) {
    next=curr->next;
    IPCMessage_free(curr);
    curr=next;
  } /* while */
  m->inQueue=0;

  /* free transport layer */
  if (m->transportLayer)
    free(m->transportLayer);

  free(m);
#ifdef MEMORYTRACE
  IPCMessageLayer_Counter--;
  DBG_NOTICE("Freed IPCMessageLayer, now %d",IPCMessageLayer_Counter);
#endif
}


IPCTRANSPORTLAYERTABLE *IPCMessageLayer_GetTransportLayer(IPCMESSAGELAYER *m){
  assert(m);
  return m->transportLayer;
}


void IPCMessageLayer_SetTransportLayer(IPCMESSAGELAYER *m,
				       IPCTRANSPORTLAYERTABLE *tl){
  assert(m);
  assert(tl);
  m->transportLayer=tl;
}


IPCMESSAGELAYERSTATE IPCMessageLayer_GetStatus(IPCMESSAGELAYER *m){
  assert(m);
  return m->status;
}


void IPCMessageLayer_SetStatus(IPCMESSAGELAYER *m,
			       IPCMESSAGELAYERSTATE s){
  assert(m);
  m->status=s;
}


ERRORCODE IPCMessageLayer_SendMessage(IPCMESSAGELAYER *m,
				      IPCMESSAGE *msg){
  IPCMESSAGE *curr;

  DBG_ENTER;
  assert(m);
  DBG_DEBUG("Status is %d",m->status);
  curr=m->outQueue;
  if (!curr) {
    m->outQueue=msg;
  }
  else {
    /* find last message in queue */
    while (curr->next)
      curr=curr->next;
    /* enqueue behind last message */
    curr->next=msg;
  }

  /* directly send message */
  if (m->status==StateIdle)
    m->status=StateWriting;

  return 0;
}


IPCMESSAGE *IPCMessageLayer_NextMessage(IPCMESSAGELAYER *m){
  IPCMESSAGE *curr;

  assert(m);
  curr=m->inQueue;
  if (curr) {
    m->inQueue=curr->next;
      curr->next=0;
  }
  return curr;
}


ERRORCODE IPCMessageLayer_IdleCheck(IPCMESSAGELAYER *m){
  ERRORCODE err;

  DBG_ENTER;
  if (m->outQueue) {
    DBG_VERBOUS("Changing to StateWriting");
    m->status=StateWriting;
  }
  else {
    /* nothing to write, but is there something to read ? */
    assert(m->transportLayer->canRead);
    /* check whether there is data available for reading */
    err=m->transportLayer->canRead(m->transportLayer);
    if (!Error_IsOk(err)) {
      /* was it just "timeout" ? */
      if (Error_GetType(err)!=Error_FindType("Socket") ||
	  Error_GetCode(err)!=SOCKET_ERROR_TIMEOUT) {
	/* no, it was something more serious, disconnect and
	 * return that error */
	DBG_DEBUG_ERR(err);
	IPCMessageLayer_ShutDown(m);
	DBG_LEAVE;
	return err;
      }
    }
    else {
      /* otherwise start reading next time */
      DBG_VERBOUS("Changing to StateReading");
      m->status=StateReading;
    } /* else of "if !outqueue empty" */
  }
  DBG_LEAVE;
  return 0;
}


ERRORCODE IPCMessageLayer_Work(IPCMESSAGELAYER *m){
  ERRORCODE err;

  DBG_ENTER;
  assert(m);
  assert(m->transportLayer);

  err=0;
  switch(m->status) {
  case StateIdle:
    DBG_VERBOUS("StateIdle");
    err=IPCMessageLayer_IdleCheck(m);
    if (!Error_IsOk(err)) {
      DBG_DEBUG_ERR(err);
      DBG_LEAVE;
      return err;
    }
    break;

  case StateConnecting:
    DBG_VERBOUS("StateConnecting");;
    assert(m->transportLayer->finishConnect);
    err=m->transportLayer->finishConnect(m->transportLayer);
    if (!Error_IsOk(err)) {
      m->status=StateUnreachable;
      DBG_DEBUG_ERR(err);
      DBG_LEAVE;
      return err;
    }
    DBG_VERBOUS("Connection established, going into idle mode");
    m->status=StateIdle;
    break;

  case StateListening:
    DBG_VERBOUS("StateListening");
    break;

  case StateReading:
    DBG_VERBOUS("StateReading");
    if (m->currentPointer==0) {
      /* start reading header */
      m->currentPointer=m->headerBuffer;
      m->currentBytesLeft=IPCMESSAGE_HEADERSIZE;
      m->readingHeader=1;
    }

    /* read bytes, if needed */
    if (m->currentBytesLeft) {
      int bsize;
      ERRORCODE err;

      DBG_VERBOUS("Still bytes to read");
      assert(m->transportLayer->read);
      bsize=m->currentBytesLeft;
      err=m->transportLayer->read(m->transportLayer,
				  m->currentPointer,
				  &bsize);
      if (!Error_IsOk(err)) {
	/* error in reading: We are about to disconnect the connection,
	 * because we won't read the rest of the message. This would
	 * leave the connection in an inconsistent state, since the next
	 * reading would deliver garbage data (the rest of this currently
	 * aborted message). So it is safer to disconnect the client now.
	 * The client should have delivered valid data, if he really liked
	 * to be served ;-)
	 */
        IPCMessageLayer_ShutDown(m);
	m->currentPointer=0;
	if (m->currentReadBuffer)
	  free(m->currentReadBuffer);
	return err;
      }
      if (bsize==0) {
	DBG_INFO("Peer disconnected while reading");
	assert(m->transportLayer->disconnect);
	m->transportLayer->disconnect(m->transportLayer);
	m->status=StateDisconnected;
	m->currentPointer=0;
	if (m->currentReadBuffer)
	  free(m->currentReadBuffer);
      }
      m->msgReadBytes+=bsize;
      m->currentBytesLeft-=bsize;
      m->currentPointer+=bsize;
    }

    /* check if the header is finished */
    if (m->currentBytesLeft==0 && m->readingHeader) {
      int size;
      /* finished header */

      m->currentPointer=0;

      DBG_VERBOUS("Finished reading header");
      size=(unsigned char)(m->headerBuffer[0]);
      size=size<<8;
      size+=(unsigned char)(m->headerBuffer[1]);
      /* check size */
      if (size<2 || size>IPCMESSAGE_MAXMSGSIZE) {
	IPCMessageLayer_ShutDown(m);
	m->currentPointer=0;
	DBG_LEAVE;
	return Error_New(0,
			 ERROR_SEVERITY_ERR,
			 Error_FindType("IPC"),
			 (size<2)?IPCMESSAGE_ERROR_COMMAND_CORRUPTED:
			 IPCMESSAGE_ERROR_COMMAND_TOO_BIG);
      }

      /* setup for reading the message data */
      m->readingHeader=0;
      m->currentReadBuffer=(char*)malloc(size);
      m->currentReadMessageSize=size;
      /* copy header into the new buffer */
      memmove(m->currentReadBuffer, m->headerBuffer,IPCMESSAGE_HEADERSIZE);
      m->currentPointer=m->currentReadBuffer+IPCMESSAGE_HEADERSIZE;
      m->currentBytesLeft=size-IPCMESSAGE_HEADERSIZE;
    } /* if readingHeader */

    /* check if the message is finished */
    if (m->currentBytesLeft==0 && m->readingHeader==0) {
      IPCMESSAGE *curr;
      IPCMESSAGE *newmsg;

      DBG_VERBOUS("Finished reading message");
      newmsg=IPCMessage_new();
      assert(newmsg);
      IPCMessage_SetBuffer(newmsg,
			   m->currentReadBuffer,
			   m->currentReadMessageSize);

#if DEBUGMODE>7
      fprintf(stderr,"Received message:\n");
      Chameleon_DumpString(IPCMessage_GetMessageBegin(newmsg),
			   IPCMessage_GetMessageSize(newmsg));
#endif
      /* enqueue message */
      curr=m->inQueue;
      if (!curr)
	m->inQueue=newmsg;
      else {
	while (curr->next)
	  curr=curr->next;
	/* enqueue behind last message */
	curr->next=newmsg;
      }

      /* reset read pointers */
      m->currentReadBuffer=0;
      m->currentBytesLeft=0;
      m->currentReadMessageSize=0;
      m->currentPointer=0;
      m->msgReadCount++;
      /* enter idle state */
      DBG_VERBOUS("Message read, going into idle mode");
      m->status=StateIdle;
    } /* if message finished */

    break;

  case StateWriting:
    DBG_VERBOUS("StateWriting");
    if (m->currentWriteMessage==0) {
      DBG_VERBOUS("Writing next message from queue");
      m->currentWriteMessage=m->outQueue;
      if (m->currentWriteMessage!=0) {
#if DEBUGMODE>7
	fprintf(stderr,"Sending message:\n");
	Chameleon_DumpString(IPCMessage_GetMessageBegin(m->currentWriteMessage),
			     IPCMessage_GetMessageSize(m->currentWriteMessage));
#endif
	/* dequeue message */
	IPCMessage_RemoveMessage(m->currentWriteMessage,
				 &(m->outQueue));

	/* get buffer size and pointer for sending */
	m->currentWriteMessage->next=0;
	m->currentBytesLeft=IPCMessage_GetMessageSize(m->currentWriteMessage);
	m->currentPointer=(char*)IPCMessage_GetMessageBegin(m->currentWriteMessage);
	assert(m->currentPointer);

	/* check size */
	if (m->currentBytesLeft<2) {
          DBG_WARN("Bytesleft smaller than 2 !");
	  /* bad message */
	  IPCMessage_free(m->currentWriteMessage);
	  m->currentWriteMessage=0;
	  m->currentBytesLeft=0;
	  m->currentPointer=0;
	  return Error_New(0,
			   ERROR_SEVERITY_ERR,
			   Error_FindType("IPC"),
			   IPCMESSAGE_ERROR_BAD_MESSAGE);
	}
      }
      else {
        DBG_WARN("Should not happen ?");
      }
    } /* if no message still in queue */

    if (m->currentBytesLeft) {
      /* still data to write */
      int bsize;

      DBG_VERBOUS("Still some bytes to write");
      assert(m->transportLayer->write);
      assert(m->currentPointer);
      /* actually write data (as much as we can, up to the whole msg) */
      bsize=m->currentBytesLeft;
      err=m->transportLayer->write(m->transportLayer,
				   m->currentPointer,
				   &bsize);
      if (!Error_IsOk(err)) {
	/* error in reading: We are about to disconnect the connection,
	 * because we won't read the rest of the message. This would
	 * leave the connection in an inconsistent state, since the next
	 * reading would deliver garbage data (the rest of this currently
	 * aborted message). So it is safer to disconnect the client now.
	 * The client should have delivered valid data, if he really liked
	 * to be served ;-)
	 */
        DBG_DEBUG_ERR(err);
	IPCMessageLayer_ShutDown(m);
        DBG_LEAVE;
	return err;
      }
      if (bsize==0) {
        DBG_INFO("Peer disconnected while writing");
	IPCMessageLayer_ShutDown(m);
      }

      m->currentBytesLeft-=bsize;
      m->currentPointer+=bsize;
      m->msgWriteBytes+=bsize;
      DBG_VERBOUS("Sending done.");
    }

    /* check if message is completely written */
    if (m->currentBytesLeft==0) {
      /* message written, delete it */
      DBG_VERBOUS("Message written");
      assert(m->currentWriteMessage);

      /* delete message */
      IPCMessage_free(m->currentWriteMessage);

      /* reset write pointers */
      m->currentWriteMessage=0;
      m->currentBytesLeft=0;
      m->currentPointer=0;
      m->msgWriteCount++;
      /* enter idle state */
      m->status=StateIdle;
      DBG_VERBOUS("I am idle again");
    }
    break;

  case StateDisconnected:
    DBG_VERBOUS("StateDisconnected");;
    break;

  default:
    DBG_VERBOUS("Default");;
    break;

  } /* switch */

  /* finished */
  return 0;
}


void IPCMessageLayer_ResetStats(IPCMESSAGELAYER *m){
  assert(m);
  m->msgReadCount=0;
  m->msgWriteCount=0;
  m->msgReadBytes=0;
  m->msgWriteBytes=0;
}


unsigned int IPCMessageLayer_GetMessageReadCount(IPCMESSAGELAYER *m){
  assert(m);
  return m->msgReadCount;
}


unsigned int IPCMessageLayer_GetMessageWriteCount(IPCMESSAGELAYER *m){
  assert(m);
  return m->msgWriteCount;
}


unsigned int IPCMessageLayer_GetMessageReadBytes(IPCMESSAGELAYER *m){
  assert(m);
  return m->msgReadBytes;
}


unsigned int IPCMessageLayer_GetMessageWriteBytes(IPCMESSAGELAYER *m){
  assert(m);
  return m->msgWriteBytes;
}


void *IPCMessageLayer_GetUserData(IPCMESSAGELAYER *m){
  DBG_ENTER;
  assert(m);
  DBG_LEAVE;
  return m->userData;
}


void IPCMessageLayer_SetUserData(IPCMESSAGELAYER *m, void* u){
  DBG_ENTER;
  assert(m);
  m->userData=u;
  DBG_LEAVE;
}


ERRORCODE IPCMessageLayer_ShutDown(IPCMESSAGELAYER *m){
  ERRORCODE err;

  DBG_ENTER;
  assert(m);
  assert(m->transportLayer);
  err=m->transportLayer->disconnect(m->transportLayer);
  m->status=StateDisconnected;
  if (!Error_IsOk(err)) {
    DBG_DEBUG_ERR(err);
  }
  DBG_LEAVE;
  return err;
}


void IPCMessageLayer_SetFreeUserDataCallback(IPCMESSAGELAYER *m,
					     IPCMESSAGELAYER_FREEPTR p){
  DBG_ENTER;
  assert(m);
  m->freeUserData_cb=p;
  DBG_LEAVE;
}


int IPCMessageLayer_GetId(IPCMESSAGELAYER *m){
  DBG_ENTER;
  assert(m);
  DBG_LEAVE;
  return m->id;
}


void IPCMessageLayer_SetId(IPCMESSAGELAYER *m, int id){
  DBG_ENTER;
  assert(m);
  m->id=id;
  DBG_LEAVE;
}


TransportLayerType IPCMessageLayer_GetType(IPCMESSAGELAYER *m){
  assert(m);
  assert(m->transportLayer);
  return IPC_TransportLayer_GetType(m->transportLayer);
}


int IPCMessageLayer_UsesEncryption(IPCMESSAGELAYER *m){
  assert(m);
  return m->useEncryption;
}


void IPCMessageLayer_UseEncryption(IPCMESSAGELAYER *m,
				   int b){
  assert(m);
  m->useEncryption=b;
}


void IPCMessageLayer_SetPersistence(IPCMESSAGELAYER *m,
				    int p){
  assert(m);
  m->persistent=p;
}


int IPCMessageLayer_IsPersistent(IPCMESSAGELAYER *m) {
  assert(m);
  return m->persistent;
}


void IPCMessageLayer_SetMark(IPCMESSAGELAYER *m,
			     int mark){
  assert(m);
  m->mark=mark;
}


int IPCMessageLayer_GetMark(IPCMESSAGELAYER *m){
  assert(m);
  return m->mark;
}



