/***************************************************************************
 $RCSfile: ipcmessage.c,v $
                             -------------------
    cvs         : $Id: ipcmessage.c,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
    begin       : Fri Oct 11 2002
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

#include <stdio.h>
#include <chameleon/chameleon.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "chameleon/ipcmessage.h"
#include "chameleon/debug.h"



#ifdef MEMORYTRACE
static int IPCMessage__Counter=0;
#endif


// forward declaration
const char *IPCMessage_ErrorString(int c);


int ipcmessage_is_initialized=0;
ERRORTYPEREGISTRATIONFORM ipcmessage_error_descr= {
  IPCMessage_ErrorString,
  0,
  IPCMESSAGE_ERROR_TYPE};


ERRORCODE IPCMessage_ModuleInit(){
  if (!ipcmessage_is_initialized) {
    if (!Error_RegisterType(&ipcmessage_error_descr))
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ERROR_TYPE_ERROR,
		       ERROR_COULD_NOT_REGISTER);
    ipcmessage_is_initialized=1;
  }
  return 0;
}


ERRORCODE IPCMessage_ModuleFini(){
  if (ipcmessage_is_initialized) {
    ipcmessage_is_initialized=0;
    if (!Error_UnregisterType(&ipcmessage_error_descr))
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ERROR_TYPE_ERROR,
		       ERROR_COULD_NOT_UNREGISTER);
  }
  return 0;
}


IPCMESSAGE *IPCMessage_new(){
  IPCMESSAGE *cmd;

  cmd=(IPCMESSAGE*)malloc(sizeof(IPCMESSAGE));
  assert(cmd);
  memset(cmd,0,sizeof(IPCMESSAGE));
#ifdef MEMORYTRACE
  IPCMessage__Counter++;
  DBG_NOTICE("IPCMessage created (now %d)",
	     IPCMessage__Counter);
#endif
  return cmd;
}


void IPCMessage_free(IPCMESSAGE *cmd){

  if (cmd) {
    if (cmd->ownPointer && cmd->ptr)
      free(cmd->ptr);
    free(cmd);

#ifdef MEMORYTRACE
    IPCMessage__Counter--;
    DBG_NOTICE("IPCMessage freed (now %d)",IPCMessage__Counter);
#endif
  }
}



ERRORCODE IPCMessage_GetSize(const char *ptr, int *pos, int bsize, int *size) {
  unsigned int j;

  j=(unsigned char)ptr[(*pos)++];
  if (j==255) {
    /* 255 is an escape byte, it indicates that the following
     * two bytes hold the size in BigEndian */
    if (bsize<*pos+2)
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ipcmessage_error_descr.typ,
		       IPCMESSAGE_ERROR_COMMAND_FULL);
    j=(unsigned char)ptr[(*pos)++];
    j=j<<8;
    j+=(unsigned char)ptr[(*pos)++];
  }
  *size=j;
  return 0;
}


ERRORCODE IPCMessage_SetSize(char *ptr, int *pos, int bsize, int size) {
  if ((size<255 && bsize<*pos+1) ||
      (size>254 && bsize<*pos+3))
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_FULL);
  if (size<255)
    ptr[(*pos)++]=(unsigned char)size;
  else {
    ptr[(*pos)++]=(unsigned char)255;
    ptr[(*pos)++]=(unsigned char)(size >> 8);
    ptr[(*pos)++]=(unsigned char)(size & 255);
  }

  return 0;
}


ERRORCODE IPCMessage_SetBuffer(IPCMESSAGE *cmd,
			       char *ptr,
			       int bsize){
  assert(cmd);
  cmd->ownPointer=1;
  cmd->ptr=ptr;
  cmd->bsize=bsize;
  if (ptr==0 && bsize!=0) {
    /* buffer not given, but size is, so malloc the buffer */
    cmd->ptr=(char*)malloc(bsize);
    assert(cmd->ptr);
  }
  cmd->size=0;
  cmd->pos=0;
  return 0;
}


ERRORCODE IPCMessage_UseBuffer(IPCMESSAGE *cmd,
			       char *ptr,
			       int bsize){
  assert(cmd);
  cmd->ownPointer=0;
  cmd->ptr=ptr;
  cmd->bsize=bsize;
  cmd->size=0;
  cmd->pos=0;
  return 0;
}


ERRORCODE IPCMessage_GetBuffer(const IPCMESSAGE *cmd,
			       const char **ptr,
			       int *bsize){
  assert(cmd);
  *ptr=cmd->ptr;
  *bsize=cmd->bsize;
  return 0;
}


const char *IPCMessage_GetMessageBegin(IPCMESSAGE *cmd){
  assert(cmd);
  return cmd->ptr;
}


int IPCMessage_GetMessageSize(IPCMESSAGE *cmd) {
  assert(cmd);
  if (cmd->size==0) {
    int i;

    if (cmd->ptr==0) {
      DBG_WARN("No buffer");
      return 0;
    }
    if (cmd->bsize<2) {
      DBG_WARN("bsize too small");
      return 0;
    }

    // read the size
    i=(unsigned char)cmd->ptr[0];
    i=i<<8;
    i+=(unsigned char)cmd->ptr[1];
    if (i>cmd->bsize) {
      DBG_WARN("Size is bigger than buffer size");
      return 0;
    }
    cmd->size=i;
  }
  return cmd->size;
}


ERRORCODE IPCMessage_FirstParameter(IPCMESSAGE *cmd,
				    char **ptr,
				    int *size){
  int i;

  DBG_ENTER;
  assert(cmd);

  cmd->pos=0;
  if (!cmd->ptr) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_EMPTY);
  }

  // initially read full command header (IPCMESSAGE_HEADERSIZE bytes)
  if (cmd->bsize<IPCMESSAGE_HEADERSIZE){
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_END);
  }
  // read the size
  i=(unsigned char)cmd->ptr[0];
  i=i<<8;
  i+=(unsigned char)cmd->ptr[1];

  // check size for plausability
  if (i>cmd->bsize) {
    DBG_WARN("Size is bigger than buffer size");
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_CORRUPTED);
  }
  cmd->size=i;

  // set to begin of parameters
  cmd->pos=IPCMESSAGE_HEADERSIZE;

  // go and get the command
  DBG_LEAVE;
  return IPCMessage_NextParameter(cmd,ptr,size);
}


ERRORCODE IPCMessage_NextParameter(IPCMESSAGE *cmd,
				   char **ptr,
				   int *size){
  ERRORCODE err;

  DBG_ENTER;
  assert(cmd);

  DBG_DEBUG("Next Param (ptr=%08x, bsize=%d, size=%d, pos=%d)",
	    (unsigned int)cmd->ptr,
	    cmd->bsize,
	    cmd->size,
	    cmd->pos);

  // check if there is a command
  if (!cmd->ptr) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_EMPTY);
  }

  /* check for end of command */
  /* if (cmd->pos>=(cmd->size)+IPCMESSAGE_HEADERSIZE) { */
  if (cmd->pos>=(cmd->size)) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_END);
  }

  // read param size
  err=IPCMessage_GetSize(cmd->ptr,
			 &cmd->pos,
			 cmd->bsize,
			 size);
  if (!Error_IsOk(err)) {
    DBG_LEAVE;
    return err;
  }

  // set return value
  *ptr=&(cmd->ptr[cmd->pos]);
  // set to begin of next param
  cmd->pos+=*size;

  // check pos and size for plausability
  if (cmd->pos>cmd->bsize) {
    *ptr=0;
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_CORRUPTED);
  }

#if DEBUGMODE>5
  fprintf(stderr,"Parameter read:\n");
  Chameleon_DumpString(*ptr,*size);
#endif

    DBG_LEAVE;
  return 0;
}


ERRORCODE IPCMessage_Parameter(IPCMESSAGE *cmd,
			       int idx,
			       char **ptr,
			       int *size){
  char *p;
  int s;
  ERRORCODE err;

  err=IPCMessage_FirstParameter(cmd,&p, &s);
  if (!Error_IsOk(err)) {
    DBG_DEBUG_ERR(err);
    return err;
  }
  while(idx--) {
    err=IPCMessage_NextParameter(cmd,&p, &s);
    if (!Error_IsOk(err)) {
      DBG_DEBUG_ERR(err);
      return err;
    }
  } /* while */
  *ptr=p;
  *size=s;
  return 0;
}


ERRORCODE IPCMessage_IntParameter(IPCMESSAGE *cmd,
				  int idx,
				  int *param){
  ERRORCODE err;
  char *p;
  int s;

  if (idx==0)
    return IPCMessage_FirstIntParameter(cmd, param);
  idx--;
  err=IPCMessage_Parameter(cmd, idx, &p, &s);
  if (!Error_IsOk(err)) {
    DBG_DEBUG_ERR(err);
    return err;
  }
  return IPCMessage_NextIntParameter(cmd, param);
}


ERRORCODE IPCMessage_StringParameter(IPCMESSAGE *cmd,
				     int idx,
				     char **param){
  ERRORCODE err;
  char *p;
  int s;

  if (idx==0)
    return IPCMessage_FirstStringParameter(cmd, param);
  idx--;
  err=IPCMessage_Parameter(cmd, idx, &p, &s);
  if (!Error_IsOk(err)) {
    DBG_DEBUG_ERR(err);
    return err;
  }
  return IPCMessage_NextStringParameter(cmd, param);
}




ERRORCODE IPCMessage_AddParameter(IPCMESSAGE *cmd,
				  const char *ptr,
				  int size){
  int paramSize;
  ERRORCODE err;

  assert(cmd);
  if (size) {
    assert(ptr);
  }

#if DEBUGMODE>5
  fprintf(stderr,"Adding parameter:\n");
  Chameleon_DumpString(ptr,size);
  fprintf(stderr,"Status of IPCMESSAGE:\n");
  fprintf(stderr," ptr=%08x\n",(unsigned int)cmd->ptr);
  fprintf(stderr," bsize=%d\n",cmd->bsize);
  fprintf(stderr," size=%d\n",cmd->size);
  fprintf(stderr," pos=%d\n",cmd->pos);
#endif

  // check if there is a buffer
  if (!cmd->ptr)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_EMPTY);

  if (cmd->pos==0) {
    /* first command, preset pos. We keep space for IPCMESSAGE_HEADERSIZE
     * bytes of size  */
    cmd->pos=IPCMESSAGE_HEADERSIZE;
    cmd->size=IPCMESSAGE_HEADERSIZE;
  }
  // check for end of buffer
  if (size<255)
    paramSize=1;
  else
    paramSize=3;
  paramSize+=size;
  if (cmd->pos+paramSize>=cmd->bsize)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_FULL);

  /* store size */
  err=IPCMessage_SetSize(cmd->ptr, &cmd->pos, cmd->bsize, size);
  if (!Error_IsOk(err))
    return err;

  /* store data */
  if (size)
    memmove(&(cmd->ptr[cmd->pos]),ptr,size);
  cmd->pos+=size;
  cmd->size+=paramSize;

  return 0;
}


ERRORCODE IPCMessage_AddIntParameter(IPCMESSAGE *cmd,
				     int param) {
  char buffer[4];
  unsigned int uparam;

#if DEBUGMODE>5
  fprintf(stderr,"Adding int parameter: %d\n", param);
#endif
  uparam=param;
  buffer[0]=(unsigned char)((uparam>>24) & 0xff);
  buffer[1]=(unsigned char)((uparam>>16) & 0xff);
  buffer[2]=(unsigned char)((uparam>>8) & 0xff);
  buffer[3]=(unsigned char)(uparam & 0xff);
  return IPCMessage_AddParameter(cmd, buffer, 4);
}


ERRORCODE IPCMessage_AddStringParameter(IPCMESSAGE *cmd,
					const char *param) {
#if DEBUGMODE>5
  fprintf(stderr,"Adding int parameter: %d\n", param);
#endif

  if (!param)
    return IPCMessage_AddParameter(cmd, "", 1);
  return IPCMessage_AddParameter(cmd, param, strlen(param)+1);
}



ERRORCODE IPCMessage_FirstIntParameter(IPCMESSAGE *cmd,
				     int *param) {
  char *ptr;
  int size;
  ERRORCODE err;
  unsigned int result;
  int i;

  err=IPCMessage_FirstParameter(cmd,&ptr, &size);
  if (!Error_IsOk(err))
      return err;
  for (i=0; i<size; i++) {
      result=result<<8;
      result+=(unsigned char)(ptr[i]);
  } // for

  *param=result;
  return 0;
}


ERRORCODE IPCMessage_FirstStringParameter(IPCMESSAGE *cmd,
					  char **param) {
  char *ptr;
  int size;
  ERRORCODE err;

  err=IPCMessage_FirstParameter(cmd,&ptr, &size);
  if (!Error_IsOk(err))
    return err;
  if (size==0)
    ptr="";
  else {
    if (ptr[size-1]!=0) {
      DBG_ERROR("String without trailing 0 detected.\n");
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ipcmessage_error_descr.typ,
		       IPCMESSAGE_ERROR_BAD_MESSAGE);
    }
  }

  *param=ptr;
  return 0;
}


ERRORCODE IPCMessage_NextIntParameter(IPCMESSAGE *cmd,
				    int *param) {
  char *ptr;
  int size;
  ERRORCODE err;
  unsigned int result;
  int i;

  err=IPCMessage_NextParameter(cmd,&ptr, &size);
  if (!Error_IsOk(err))
      return err;
  for (i=0; i<size; i++) {
      result=result<<8;
      result+=(unsigned char)(ptr[i]);
  } // for

  *param=result;
  return 0;
}


ERRORCODE IPCMessage_NextStringParameter(IPCMESSAGE *cmd,
					 char **param) {
  char *ptr;
  int size;
  ERRORCODE err;

  err=IPCMessage_NextParameter(cmd,&ptr, &size);
  if (!Error_IsOk(err))
    return err;
  if (size==0)
    ptr="";
  else {
    if (ptr[size-1]!=0) {
      DBG_ERROR("String without trailing 0 detected.\n");
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ipcmessage_error_descr.typ,
		       IPCMESSAGE_ERROR_BAD_MESSAGE);
    }
  }

  *param=ptr;
  return 0;
}


ERRORCODE IPCMessage_BuildMessage(IPCMESSAGE *cmd) {
  assert(cmd);

  DBG_ENTER;
  // check if there is a buffer
  if (!cmd->ptr) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_EMPTY);
  }

  /* check if buffer is filled */
  if (cmd->size==0) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_EMPTY);
  }

  /* store size and code inside buffer */
  if (cmd->bsize<4) {
    DBG_LEAVE;
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     ipcmessage_error_descr.typ,
		     IPCMESSAGE_ERROR_COMMAND_FULL);
  }

  /* store size */
  cmd->ptr[0]=(unsigned char)(cmd->size >> 8);
  cmd->ptr[1]=(unsigned char)(cmd->size & 255);

  /* reset data to be used for next message */
  cmd->size=0;
  cmd->pos=0;

  /* done */
  DBG_LEAVE;
  return 0;
}


void IPCMessage_AddMessage(IPCMESSAGE *cmd, IPCMESSAGE **head) {
  IPCMESSAGE *curr;

  DBG_ENTER;
  assert(cmd);
  assert(head);

  curr=*head;
  if (!curr) {
    *head=cmd;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=cmd;
  }
  DBG_LEAVE;
}


void IPCMessage_RemoveMessage(IPCMESSAGE *cmd, IPCMESSAGE **head) {
  IPCMESSAGE *curr;

  DBG_ENTER;
  assert(cmd);
  assert(head);

  curr=*head;
  if (curr) {
    if (curr==cmd) {
      *head=curr->next;
    }
    else {
      /* find predecessor */
      while(curr->next!=cmd) {
	curr=curr->next;
      } /* while */
      if (curr)
	curr->next=cmd->next;
    }
  }
  DBG_LEAVE;
}


const char *IPCMessage_ErrorString(int c){
  const char *s;

  switch(c) {
  case IPCMESSAGE_ERROR_COMMAND_END:
    s="Message end reached";
    break;
  case IPCMESSAGE_ERROR_COMMAND_FULL:
    s="Message buffer full";
    break;
  case IPCMESSAGE_ERROR_COMMAND_EMPTY:
    s="Message buffer empty";
    break;
  case IPCMESSAGE_ERROR_COMMAND_CORRUPTED:
    s="Message buffer corrupted";
    break;
  case IPCMESSAGE_ERROR_COMMAND_TOO_BIG:
    s="Message to be received is too big";
    break;
  case IPCMESSAGE_ERROR_BAD_MESSAGE:
    s="Bad IPC message in queue";
    break;
  case IPCMESSAGE_ERROR_BUFFER_TOO_SMALL:
    s="Buffer too small";
    break;
  case IPCMESSAGE_ERROR_NO_TRANSPORTLAYER:
    s="No transport layer";
    break;
  case IPCMESSAGE_ERROR_NO_MESSAGELAYER:
    s="No message layer";
    break;
  case IPCMESSAGE_ERROR_NO_MESSAGE:
    s="No message";
    break;
  case IPCMESSAGE_ERROR_NO_SOCKETNAME:
    s="Could not create temporary name for socket";
    break;
  default:
    s=(const char*)0;
  } // switch
  return s;
}


