/***************************************************************************
 $RCSfile: ipcmessagelayer.h,v $
                             -------------------
    cvs         : $Id: ipcmessagelayer.h,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
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


#ifndef CHAMELEON_IPCMESSAGELAYER_H
#define CHAMELEON_IPCMESSAGELAYER_H "$Id"

#include <chameleon/ipctransportlayer.h>
#include <chameleon/ipcmessage.h>

#ifdef __cplusplus
extern "C" {
#endif

CHIPCARD_API typedef struct IPCMESSAGELAYERSTRUCT IPCMESSAGELAYER;
CHIPCARD_API typedef ERRORCODE (*IPCMESSAGELAYER_FREEPTR)(IPCMESSAGELAYER *ml);


CHIPCARD_API typedef enum {
  StateIdle=0,
  StateConnecting,
  StateListening,
  StateReading,
  StateWriting,
  StateDisconnected,
  StateUnreachable
} IPCMESSAGELAYERSTATE;


CHIPCARD_API struct IPCMESSAGELAYERSTRUCT {
  int id;
  IPCTRANSPORTLAYERTABLE *transportLayer;
  IPCMESSAGELAYERSTATE status;
  IPCMESSAGE *outQueue;
  IPCMESSAGE *inQueue;
  char headerBuffer[IPCMESSAGE_HEADERSIZE];
  int readingHeader;
  int currentBytesLeft;
  char *currentPointer;
  int currentReadMessageSize;
  char *currentReadBuffer;
  IPCMESSAGE *currentWriteMessage;
  unsigned int msgReadCount;  /** statistical data */
  unsigned int msgReadBytes;  /** statistical data */
  unsigned int msgWriteCount; /** statistical data */
  unsigned int msgWriteBytes; /** statistical data */
  void *userData;
  int useEncryption;
  int persistent;
  int mark;
  IPCMESSAGELAYER_FREEPTR freeUserData_cb; /** callback: free userData */
  IPCMESSAGELAYER *next;
};



CHIPCARD_API IPCMESSAGELAYER *IPCMessageLayer_new();
CHIPCARD_API void IPCMessageLayer_free(IPCMESSAGELAYER *m);


CHIPCARD_API IPCTRANSPORTLAYERTABLE *IPCMessageLayer_GetTransportLayer(IPCMESSAGELAYER *m);
CHIPCARD_API void IPCMessageLayer_SetTransportLayer(IPCMESSAGELAYER *m,
						    IPCTRANSPORTLAYERTABLE *tl);
CHIPCARD_API int IPCMessageLayer_GetId(IPCMESSAGELAYER *m);
CHIPCARD_API void IPCMessageLayer_SetId(IPCMESSAGELAYER *m, int id);

CHIPCARD_API IPCMESSAGELAYERSTATE IPCMessageLayer_GetStatus(IPCMESSAGELAYER *m);
CHIPCARD_API void IPCMessageLayer_SetStatus(IPCMESSAGELAYER *m,
					    IPCMESSAGELAYERSTATE s);
ERRORCODE IPCMessageLayer_SendMessage(IPCMESSAGELAYER *m,
				      IPCMESSAGE *msg);
CHIPCARD_API IPCMESSAGE *IPCMessageLayer_NextMessage(IPCMESSAGELAYER *m);
CHIPCARD_API ERRORCODE IPCMessageLayer_Work(IPCMESSAGELAYER *m);

CHIPCARD_API void IPCMessageLayer_ResetStats(IPCMESSAGELAYER *m);
CHIPCARD_API unsigned int IPCMessageLayer_GetMessageReadCount(IPCMESSAGELAYER *m);
CHIPCARD_API unsigned int IPCMessageLayer_GetMessageWriteCount(IPCMESSAGELAYER *m);
CHIPCARD_API unsigned int IPCMessageLayer_GetMessageReadBytes(IPCMESSAGELAYER *m);
CHIPCARD_API unsigned int IPCMessageLayer_GetMessageWriteBytes(IPCMESSAGELAYER *m);

CHIPCARD_API int IPCMessageLayer_GetAutoRemove(IPCMESSAGELAYER *m);
CHIPCARD_API void IPCMessageLayer_SetAutoRemove(IPCMESSAGELAYER *m, int b);

CHIPCARD_API void *IPCMessageLayer_GetUserData(IPCMESSAGELAYER *m);
CHIPCARD_API void IPCMessageLayer_SetUserData(IPCMESSAGELAYER *m, void* u);

CHIPCARD_API ERRORCODE IPCMessageLayer_ShutDown(IPCMESSAGELAYER *m);

CHIPCARD_API void IPCMessageLayer_SetFreeUserDataCallback(IPCMESSAGELAYER *m,
							  IPCMESSAGELAYER_FREEPTR p);

CHIPCARD_API ERRORCODE IPCMessageLayer_IdleCheck(IPCMESSAGELAYER *m);

CHIPCARD_API TransportLayerType IPCMessageLayer_GetType(IPCMESSAGELAYER *m);

CHIPCARD_API int IPCMessageLayer_UsesEncryption(IPCMESSAGELAYER *m);
CHIPCARD_API void IPCMessageLayer_UseEncryption(IPCMESSAGELAYER *m,
						int b);
CHIPCARD_API void IPCMessageLayer_SetPersistence(IPCMESSAGELAYER *m,
						  int p);
CHIPCARD_API int IPCMessageLayer_IsPersistent(IPCMESSAGELAYER *m);

CHIPCARD_API void IPCMessageLayer_SetMark(IPCMESSAGELAYER *m,
					   int mark);
CHIPCARD_API int IPCMessageLayer_GetMark(IPCMESSAGELAYER *m);


#ifdef __cplusplus
}
#endif

#endif /* CHAMELEON_IPCMESSAGELAYER_H */


