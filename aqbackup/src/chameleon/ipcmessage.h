/***************************************************************************
 $RCSfile: ipcmessage.h,v $
                             -------------------
    cvs         : $Id: ipcmessage.h,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
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


#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include "chameleon/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPCMESSAGE_HEADERSIZE 2

#define IPCMESSAGE_MAXMSGSIZE 4096

#define IPCMESSAGE_ERROR_TYPE              "IPC"
#define IPCMESSAGE_ERROR_COMMAND_END         1
#define IPCMESSAGE_ERROR_COMMAND_FULL        2
#define IPCMESSAGE_ERROR_COMMAND_EMPTY       3
#define IPCMESSAGE_ERROR_COMMAND_CORRUPTED   4
#define IPCMESSAGE_ERROR_COMMAND_TOO_BIG     5
#define IPCMESSAGE_ERROR_BAD_MESSAGE         6
#define IPCMESSAGE_ERROR_NO_MESSAGE          7
#define IPCMESSAGE_ERROR_NO_MESSAGELAYER     8
#define IPCMESSAGE_ERROR_BUFFER_TOO_SMALL    9
#define IPCMESSAGE_ERROR_NO_TRANSPORTLAYER   10
#define IPCMESSAGE_ERROR_NO_SOCKETNAME       11

struct IPCMESSAGESTRUCT;


CHIPCARD_API struct IPCMESSAGESTRUCT {
  int ownPointer;
  char *ptr;
  int bsize;
  int size;
  int pos;
  struct IPCMESSAGESTRUCT *next;
};

CHIPCARD_API typedef struct IPCMESSAGESTRUCT IPCMESSAGE;


CHIPCARD_API ERRORCODE IPCMessage_ModuleInit();
CHIPCARD_API ERRORCODE IPCMessage_ModuleFini();


CHIPCARD_API IPCMESSAGE *IPCMessage_new();
CHIPCARD_API void IPCMessage_free(IPCMESSAGE *cmd);


/**
 * This function sets the buffer and makes the IPCMessage own it.
 * If the pointer is 0 then an appropriate buffer will be malloc'ed.
 */
CHIPCARD_API ERRORCODE IPCMessage_SetBuffer(IPCMESSAGE *cmd,
					    char *ptr,
					    int size);

/**
 * This function sets the buffer and does not make the IPCMessage own it !
 */
CHIPCARD_API ERRORCODE IPCMessage_UseBuffer(IPCMESSAGE *cmd,
					    char *ptr,
					    int size);

CHIPCARD_API ERRORCODE IPCMessage_GetBuffer(const IPCMESSAGE *cmd,
					    const char **ptr,
					    int *size);

CHIPCARD_API const char *IPCMessage_GetMessageBegin(IPCMESSAGE *cmd);

CHIPCARD_API int IPCMessage_GetMessageSize(IPCMESSAGE *cmd);


CHIPCARD_API ERRORCODE IPCMessage_FirstParameter(IPCMESSAGE *cmd,
						 char **ptr,
						 int *size);
CHIPCARD_API ERRORCODE IPCMessage_FirstIntParameter(IPCMESSAGE *cmd,
						    int *param);
CHIPCARD_API ERRORCODE IPCMessage_FirstStringParameter(IPCMESSAGE *cmd,
						       char **param);

CHIPCARD_API ERRORCODE IPCMessage_NextParameter(IPCMESSAGE *cmd,
						char **ptr,
						int *size);
CHIPCARD_API ERRORCODE IPCMessage_NextIntParameter(IPCMESSAGE *cmd,
						   int *param);
CHIPCARD_API ERRORCODE IPCMessage_NextStringParameter(IPCMESSAGE *cmd,
						      char **param);
CHIPCARD_API ERRORCODE IPCMessage_Parameter(IPCMESSAGE *cmd,
					    int idx,
					    char **ptr,
					    int *size);
CHIPCARD_API ERRORCODE IPCMessage_IntParameter(IPCMESSAGE *cmd,
					       int idx,
					       int *param);
CHIPCARD_API ERRORCODE IPCMessage_StringParameter(IPCMESSAGE *cmd,
						  int idx,
						  char **param);


CHIPCARD_API ERRORCODE IPCMessage_AddParameter(IPCMESSAGE *cmd,
					       const char *ptr,
					       int size);
CHIPCARD_API ERRORCODE IPCMessage_AddIntParameter(IPCMESSAGE *cmd,
						  int param);
CHIPCARD_API ERRORCODE IPCMessage_AddStringParameter(IPCMESSAGE *cmd,
						     const char *param);


CHIPCARD_API ERRORCODE IPCMessage_BuildMessage(IPCMESSAGE *cmd);


CHIPCARD_API void IPCMessage_AddMessage(IPCMESSAGE *cmd, IPCMESSAGE **head);
CHIPCARD_API void IPCMessage_RemoveMessage(IPCMESSAGE *cmd, IPCMESSAGE **head);



#ifdef __cplusplus
}
#endif

#endif



