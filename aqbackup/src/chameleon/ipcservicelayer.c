/***************************************************************************
 $RCSfile: ipcservicelayer.c,v $
                             -------------------
    cvs         : $Id: ipcservicelayer.c,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
    begin       : Fri Nov 08 2002
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


#include <chameleon/ipcservicelayer.h>
#include <chameleon/ipcmessagelayer.h>
#include <chameleon/ipctransportlayer.h>
#include <chameleon/inetsocket.h>
#include <chameleon/debug.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


IPCSERVICELAYER *IPCServiceLayer_new(){
  IPCSERVICELAYER *sl;

  DBG_ENTER;
  sl=(IPCSERVICELAYER*)malloc(sizeof(IPCSERVICELAYER));
  assert(sl);
  memset(sl,0,sizeof(IPCSERVICELAYER));
  sl->refCount=1;
  DBG_LEAVE;
  return sl;
}


void IPCServiceLayer_share(IPCSERVICELAYER *sl){
  assert(sl);
  (sl->refCount)++;
}


void IPCServiceLayer_free(IPCSERVICELAYER *sl){
  IPCMESSAGELAYER *curr, *next;

  DBG_ENTER;
  if (sl) {
    if (--(sl->refCount)<1) {
      /* free all messagelayers */
      curr=sl->messageLayers;
      while(curr) {
	next=curr->next;
	IPCMessageLayer_free(curr);
	curr=next;
      } /* while */

      /* free this */
      free(sl);
    }
  }
  DBG_LEAVE;
}


ERRORCODE IPCServiceLayer_SendMessage(IPCSERVICELAYER *sl,
				      IPCMESSAGELAYER *ml,
				      IPCMESSAGE *msg){
  DBG_ENTER;
  assert(sl);
  assert(ml);
  assert(msg);
  DBG_LEAVE;
  return IPCMessageLayer_SendMessage(ml,msg);
}


ERRORCODE IPCServiceLayer_NextMessage(IPCSERVICELAYER *sl,
				      IPCMESSAGELAYER **ml,
				      IPCMESSAGE **msg,
				      int mark){
  IPCMESSAGELAYER *curr;
  IPCMESSAGELAYER *prev;
  IPCMESSAGE *nextmsg;

  DBG_ENTER;
  assert(sl);
  assert(ml);
  assert(msg);
  curr=sl->nextMessageLayer;
  if (!curr)
    curr=sl->messageLayers;
  if (!curr) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType("IPC"),
		     IPCMESSAGE_ERROR_NO_MESSAGELAYER);
  }

  /* check each message layer only once. If one message layer returns
   * a message the next time this function will check the next message layer.
   * This way no message layer can occupy thie service layer completely.
   */
  nextmsg=0;
  prev=0;
  while (!nextmsg && curr) {
    if (mark==0 || curr->mark==mark) {
      nextmsg=IPCMessageLayer_NextMessage(curr);
    }
    prev=curr;
    curr=curr->next;
  } /* while */
  sl->nextMessageLayer=curr;
  if (!nextmsg) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     Error_FindType("IPC"),
		     IPCMESSAGE_ERROR_NO_MESSAGE);
  }
  *ml=prev;
  *msg=nextmsg;

  DBG_DEBUG("Have a message");
  DBG_LEAVE;
  return 0;
}


void IPCServiceLayer_AddMessageLayer(IPCSERVICELAYER *sl,
				     IPCMESSAGELAYER *ml){
  IPCMESSAGELAYER *curr;

  DBG_ENTER;
  assert(sl);
  assert(ml);
  curr=sl->messageLayers;
  if (!curr) {
    sl->messageLayers=ml;
  }
  else {
    /* find last messagelayer in queue */
    while (curr->next)
      curr=curr->next;
    /* enqueue behind last message */
    curr->next=ml;
  }
  DBG_LEAVE;
}


void IPCServiceLayer_UnlinkMessageLayer(IPCSERVICELAYER *sl,
					IPCMESSAGELAYER *ml){
  IPCMESSAGELAYER *curr;

  DBG_ENTER;
  assert(sl);
  assert(ml);
  curr=sl->messageLayers;
  if (!curr) {
    DBG_LEAVE;
    return;
  }

  /* find predecessor */
  while(curr->next && curr->next!=ml)
    curr=curr->next;

  /* unlink*/
  if (curr->next==ml)
    curr->next=ml->next;
  ml->next=0;
  DBG_LEAVE;
}


ERRORCODE IPCServiceLayer_Work(IPCSERVICELAYER *sl, int timeout){
  ERRORCODE err;
  IPCMESSAGELAYER *curr;
  SOCKETSET rset, wset;
  IPCTRANSPORTLAYERTABLE *currtl;
  struct SOCKETSTRUCT *currsock;
  int status;
  int i;
  int ndone;

  DBG_ENTER;
  assert(sl);
  err=SocketSet_Create(&rset);
  if (!Error_IsOk(err)) {
    DBG_LEAVE;
    return err;
  }
  err=SocketSet_Create(&wset);
  if (!Error_IsOk(err)) {
    DBG_LEAVE;
    return err;
  }

  /* get all readable/writeable sockets */
  i=0;
  ndone=0;
  curr=sl->messageLayers;
  if (!curr) {
    DBG_WARN("No message layers.");
  }
  while(curr) {
    status=IPCMessageLayer_GetStatus(curr);
    if (status==StateIdle) {
      err=IPCMessageLayer_IdleCheck(curr);
      if (!Error_IsOk(err)) {
	DBG_DEBUG_ERR(err);
        return err;
      }
      status=IPCMessageLayer_GetStatus(curr);
    }

    if (status!=StateDisconnected) {
      currtl=IPCMessageLayer_GetTransportLayer(curr);
      assert(currtl);
      currsock=currtl->getSocket(currtl);
      if (currsock) {
	DBG_VERBOUS("Have a socket.");
	if (status==StateIdle ||
	    status==StateReading ||
	    status==StateListening) {
	  DBG_VERBOUS("Have a socket for reading.");
	  err=SocketSet_AddSocket(&rset, currsock);
	  if (!Error_IsOk(err)) {
	    DBG_LEAVE;
	    return err;
	  }
	  i++;
	}
	else if (status==StateWriting ||
		 status==StateConnecting) {
	  err=SocketSet_AddSocket(&wset, currsock);
	  DBG_VERBOUS("Have a socket for writing.");
	  if (!Error_IsOk(err)) {
	    DBG_LEAVE;
	    return err;
	  }
	  i++;
	}
      } /* if currsock */
      else {
	DBG_VERBOUS("Don't have a socket, working now");
        ndone++;
	err=IPCMessageLayer_Work(curr);
	if (!Error_IsOk(err)) {
          DBG_DEBUG_ERR(err);
	} /* if error */
      } /* if no socket */
    } /* if not disconnected */
    curr=curr->next;
  } /* while */

  if (!i) {
    if (ndone) {
      /* no socket, but something done, so sleep now to not overload the
       * CPU usage */
      DBG_VERBOUS("Sleeping");
      Socket_Select(0, 0, 0, timeout);
      DBG_LEAVE;
      return 0;
    }
    else {
      /* nothing done, so sleep now to not overload the CPU usage */
      DBG_LEAVE;
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       Error_FindType("IPC"),
		       IPCMESSAGE_ERROR_NO_TRANSPORTLAYER);
    }
  }

  DBG_VERBOUS("Selecting");
  err=Socket_Select(&rset, &wset, 0, timeout);
  if (!Error_IsOk(err)) {
    if (Error_GetType(err)!=Error_FindType("Socket") ||
	Error_GetCode(err)!=SOCKET_ERROR_TIMEOUT) {
      DBG_DEBUG_ERR(err);
      DBG_LEAVE;
      return err;
    }
  }

  /* now handle all messagelayers whose socket changed */
  curr=sl->messageLayers;
  while(curr) {
    currtl=IPCMessageLayer_GetTransportLayer(curr);
    status=IPCMessageLayer_GetStatus(curr);
    if (status!=StateDisconnected) {
      assert(currtl);
      currsock=currtl->getSocket(currtl);
      if (currsock) {
	if (SocketSet_HasSocket(&rset, currsock) ||
	    SocketSet_HasSocket(&wset, currsock)) {
	  DBG_VERBOUS("Socket state changed");
	  /* check if the message layer is listening */
	  if (IPCMessageLayer_GetStatus(curr)==StateListening) {
	    /* accept incoming connection */
	    struct IPCTRANSPORTLAYERTABLESTRUCT *newtl;

	    DBG_VERBOUS("Socket changed while listening, "
			"will accept new client");
	    err=currtl->accept(currtl,
			       &newtl);
	    if (Error_IsOk(err)) {
	      /* create new messagelayer */
	      IPCMESSAGELAYER *newml;

	      assert(newtl);
	      newml=IPCMessageLayer_new();
	      IPCMessageLayer_SetMark(newml, IPCMessageLayer_GetMark(curr));
	      assert(newml);
	      IPCMessageLayer_SetTransportLayer(newml, newtl);
	      IPCServiceLayer_AddMessageLayer(sl, newml);
              DBG_INFO("Accepted new connection");
	    }
	    else {
              /* other error */
	      DBG_DEBUG_ERR(err);
	    } /* if error */
	  } /* if listening */
	  else {
	    err=IPCMessageLayer_Work(curr);
	    if (!Error_IsOk(err)) {
	      DBG_DEBUG_ERR(err);
	    } /* if error */
	  }
	} /* if socket has changed */
      } /* if message layer has a socket */
      else {
	/* check if the message layer is listening */
	if (IPCMessageLayer_GetStatus(curr)==StateListening) {
	  /* accept incoming connection */
	  struct IPCTRANSPORTLAYERTABLESTRUCT *newtl;

	  err=currtl->accept(currtl,
			     &newtl);
	  if (Error_IsOk(err)) {
	    /* create new messagelayer */
	    IPCMESSAGELAYER *newml;

	    DBG_INFO("Creating new message layer");
	    newml=IPCMessageLayer_new();
	    assert(newml);
	    IPCMessageLayer_SetTransportLayer(newml, newtl);
	    IPCServiceLayer_AddMessageLayer(sl, newml);
	  }
	  else {
	    DBG_DEBUG_ERR(err);
	  } /* if error */
	} /* if listening */
      } /* if no socket */

    } /* if state!=StateDisconnected */
    curr=curr->next;
  } /* while */

  DBG_LEAVE;
  return 0;
}


void IPCServiceLayer_RemoveDisconnected(IPCSERVICELAYER *sl){
  IPCMESSAGELAYER *curr, *prev, *next;

  DBG_ENTER;
  curr=sl->messageLayers;
  prev=0;
  while(curr) {
    next=curr->next;
    if (IPCMessageLayer_GetStatus(curr)==StateDisconnected) {
      DBG_INFO("Removing a client");
      /* unlink */
      if (prev)
	prev->next=curr->next;
      else
	sl->messageLayers=curr->next;
      /* free */
      IPCMessageLayer_free(curr);
    } /* if disconnected */
    prev=curr;
    curr=next;
  } /* while */
  DBG_LEAVE;
}


void IPCServiceLayer_ShutDown(IPCSERVICELAYER *sl){
  IPCMESSAGELAYER *curr;
  IPCMESSAGELAYER *next;
  ERRORCODE err;

  DBG_ENTER;
  assert(sl);
  curr=sl->messageLayers;
  while(curr) {
    next=curr->next;
    err=IPCMessageLayer_ShutDown(curr);
    if (!Error_IsOk(err)) {
      DBG_DEBUG_ERR(err);
    }
    IPCMessageLayer_free(curr);
    curr=next;
  }
  sl->messageLayers=0;
  DBG_LEAVE;
}


IPCMESSAGELAYER *IPCServiceLayer_FindMessageLayer(IPCSERVICELAYER *sl,
						  int id){
  IPCMESSAGELAYER *curr;

  DBG_ENTER;
  assert(sl);
  curr=sl->messageLayers;
  while(curr) {
    if (IPCMessageLayer_GetId(curr)==id) {
      DBG_LEAVE;
      return curr;
    }
    curr=curr->next;
  }

  DBG_LEAVE;
  return 0;
}






