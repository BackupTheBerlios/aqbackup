/***************************************************************************
 $RCSfile: ipcservicelayer.h,v $
                             -------------------
    cvs         : $Id: ipcservicelayer.h,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
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


#ifndef CHAMELEON_IPCSERVICELAYER_H
#define CHAMELEON_IPCSERVICELAYER_H "$Id: ipcservicelayer.h,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $"

#include <chameleon/ipcmessagelayer.h>
#include <chameleon/ipcmessage.h>
#include <chameleon/error.h>

#ifdef __cplusplus
extern "C" {
#endif

CHIPCARD_API typedef struct IPCSERVICELAYERSTRUCT IPCSERVICELAYER;


CHIPCARD_API struct IPCSERVICELAYERSTRUCT {
  int refCount;
  IPCMESSAGELAYER *messageLayers;
  IPCMESSAGELAYER *nextMessageLayer;
};




/**
 * Creates a service layer with a reference count of 1.
 */
CHIPCARD_API IPCSERVICELAYER *IPCServiceLayer_new();

/**
 * Tells Chameleon that another object is using this service layer.
 * This increments the internal reference counter.
 * When you call @ref IPCServiceLayer_free the reference counter is
 * decremented. If then this counter reaches zero, the service layer
 * really gets freed.
 */
CHIPCARD_API void IPCServiceLayer_share(IPCSERVICELAYER *sl);

/**
 * Free the service layer, but only if the reference counter becomes 0.
 */
CHIPCARD_API void IPCServiceLayer_free(IPCSERVICELAYER *sl);


CHIPCARD_API ERRORCODE IPCServiceLayer_SendMessage(IPCSERVICELAYER *sl,
						   IPCMESSAGELAYER *ml,
						   IPCMESSAGE *msg);

/**
 * If mark is 0, then every message layer matches. Otherwise
 * Only those messagelayers are scanned for new messages, whoes mark
 * equals the given one.
 */
CHIPCARD_API ERRORCODE IPCServiceLayer_NextMessage(IPCSERVICELAYER *sl,
						    IPCMESSAGELAYER **ml,
						    IPCMESSAGE **msg,
						    int mark);

CHIPCARD_API void IPCServiceLayer_AddMessageLayer(IPCSERVICELAYER *sl,
						   IPCMESSAGELAYER *ml);

CHIPCARD_API void IPCServiceLayer_UnlinkMessageLayer(IPCSERVICELAYER *sl,
						      IPCMESSAGELAYER *ml);

CHIPCARD_API ERRORCODE IPCServiceLayer_Work(IPCSERVICELAYER *sl, int timeout);

CHIPCARD_API void IPCServiceLayer_RemoveDisconnected(IPCSERVICELAYER *sl);


CHIPCARD_API void IPCServiceLayer_ShutDown(IPCSERVICELAYER *sl);


CHIPCARD_API IPCMESSAGELAYER *IPCServiceLayer_FindMessageLayer(IPCSERVICELAYER *sl,
								int id);


#ifdef __cplusplus
}
#endif

#endif



