/***************************************************************************
 $RCSfile: ctservice.h,v $
 -------------------
 cvs         : $Id: ctservice.h,v 1.2 2003/06/11 13:18:35 aquamaniac Exp $
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


#ifndef CTSERVICE_H
#define CTSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <chameleon/chameleon.h>
#include <chameleon/cryp.h>
#include <chameleon/ipcmessage.h>
#include <chameleon/ipcmessagelayer.h>
#include <chameleon/conf.h>

#define CTSERVICE_MSGCODE_BF                       0x10001
#define CTSERVICE_MSGCODE_BF_VERSION               0x00100

#define CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS         0x10002
#define CTSERVICE_MSGCODE_RQ_EXCHANGE_KEYS_VERSION 0x00101
#define CTSERVICE_MSGCODE_RP_EXCHANGE_KEYS         0x10003
#define CTSERVICE_MSGCODE_RP_EXCHANGE_KEYS_VERSION 0x00100

#define CTSERVICE_MSGCODE_RQ_SESSION_KEY           0x10004
#define CTSERVICE_MSGCODE_RQ_SESSION_KEY_VERSION   0x00100
#define CTSERVICE_MSGCODE_RP_SESSION_KEY           0x10005
#define CTSERVICE_MSGCODE_RP_SESSION_KEY_VERSION   0x00100

#define CTSERVICE_MSGCODE_RP_ERROR                 0x10006
#define CTSERVICE_MSGCODE_RP_ERROR_VERSION         0x00100

#define CTSERVICE_MSGCODE_RQ_OPEN                  0x10007
#define CTSERVICE_MSGCODE_RQ_OPEN_VERSION          0x00100
#define CTSERVICE_MSGCODE_RP_OPEN                  0x10008
#define CTSERVICE_MSGCODE_RP_OPEN_VERSION          0x00100


#define CTSERVICE_ERROR_TYPE "CTService"
#define CTSERVICE_SUCCESS                   0
#define CTSERVICE_ERROR_DRIVER              1
#define CTSERVICE_ERROR_INVALID             2
#define CTSERVICE_ERROR_BUFFER              3
#define CTSERVICE_ERROR_NO_REQUEST          5
#define CTSERVICE_ERROR_NO_MESSAGE          6
#define CTSERVICE_ERROR_BAD_CHANNEL_STATUS  7
#define CTSERVICE_ERROR_BAD_MESSAGE_VERSION 8
#define CTSERVICE_ERROR_BAD_MESSAGE_CODE    9
#define CTSERVICE_ERROR_BAD_BUFFERSIZE      10
#define CTSERVICE_ERROR_NO_COMMANDS         11
#define CTSERVICE_ERROR_NO_CONFIG           12
#define CTSERVICE_ERROR_UNREACHABLE         13
#define CTSERVICE_ERROR_NO_MESSAGELAYER     14
#define CTSERVICE_ERROR_NO_CLIENT           15
#define CTSERVICE_ERROR_REMOTE              16
#define CTSERVICE_ERROR_NO_TRANSPORT_LAYER  17
#define CTSERVICE_ERROR_BAD_CONFIG          18
#define CTSERVICE_ERROR_SYSTEM_ERROR        19
#define CTSERVICE_ERROR_EXPIRED             20
#define CTSERVICE_ERROR_INTERRUPTED         21
#define CTSERVICE_ERROR_INTERNAL            22
#define CTSERVICE_ERROR_NO_SERVICE          23

/* TODO: Remove this from CTService ! */
#define CTSERVICE_ERROR_CARD_REMOVED        100
#define CTSERVICE_ERROR_CARD_LOCKED         101

#define CTSERVICE_DEFAULT_PORT 32891
#define CTSERVICE_DEFAULT_ACCESS "777"


typedef struct CTSERVICEREQUESTSTRUCT CTSERVICEREQUEST;
typedef struct CTSERVICEDATASTRUCT CTSERVICEDATA;

typedef void (*CTSERVICE_FREEUSERDATA_PTR)(void *d);


struct CTSERVICEREQUESTSTRUCT {
  CTSERVICEREQUEST *next;
  int requestId;
  int serviceId;
  int persistent;
  int abandoned; /** when response arrives this request gets autodeleted */
  int responseCount;
  IPCMESSAGE *message;
  IPCMESSAGE *responses;
};


typedef enum {
  ChannelClosed=0,
  ChannelOpening,
  ChannelOpen
} CTSERVICE_CHANNEL_STATE;


struct CTSERVICEDATASTRUCT {
  CRYP_RSAKEY *tempKey;
  CRYP_BFKEY *sessionKey;
  CTSERVICE_CHANNEL_STATE channelState;
  int nextSignId;
  int lastPeerSignId;
  void *userData;
  CTSERVICE_FREEUSERDATA_PTR freeUserDataPtr;
  CTSERVICEREQUEST *requests;
};


ERRORCODE CTService_ModuleInit();
ERRORCODE CTService_ModuleFini();



CTSERVICEDATA *CTService_PeerData_new();
void CTService_PeerData_free(CTSERVICEDATA *pd);


CTSERVICEREQUEST *CTService_Request_new();
void CTService_Request_free(CTSERVICEREQUEST *rq);

IPCMESSAGE *CTService_Request_NextResponse(CTSERVICEREQUEST *rq);
IPCMESSAGE *CTService_Request_PeekResponse(CTSERVICEREQUEST *rq);


/**
 * This function encrypts the given message.
 * @return encrypted message
 * @param pd pointer to service data
 * @param msg pointer to the raw message (will not be taken over!)
 */
IPCMESSAGE *CTService_EncryptMessage(CTSERVICEDATA *pd,
				     IPCMESSAGE *msg);
/**
 * This function decrypts the given message.
 * @return encrypted message
 * @param pd pointer to service data
 * @param msg pointer to the encrypted message (will not be taken over!)
 */
IPCMESSAGE *CTService_DecryptMessage(CTSERVICEDATA *pd,
				     IPCMESSAGE *msg);


void CTService_Request_AddRequest(CTSERVICEREQUEST *req,
				  CTSERVICEREQUEST **head);
void CTService_Request_RemoveRequest(CTSERVICEREQUEST *req,
				     CTSERVICEREQUEST **head);
CTSERVICEREQUEST *CTService_Request_FindRequest(int id,
						CTSERVICEREQUEST **head);


IPCMESSAGE *CTService_Message_Create(int msgCode,
				     int msgVersion,
				     int msgId,
				     int msgReply,
				     int msgSize);

CTSERVICEREQUEST *CTService_Request_Create(int serviceid,
					   int msgCode,
					   int msgVersion,
					   int msgId,
					   int msgReply,
					   int msgSize);

void *CTService_GetPeerUserData(IPCMESSAGELAYER *ml);
void CTService_SetPeerUserData(IPCMESSAGELAYER *ml, void *d);

CTSERVICE_FREEUSERDATA_PTR
  CTService_SetFreeUserDataPtr(IPCMESSAGELAYER *ml,
			       CTSERVICE_FREEUSERDATA_PTR newfn);

ERRORCODE CTService_CheckMsgCodeAndVersion(IPCMESSAGE *msg,
					   int msgCode,
					   int msgVersion);

unsigned int Debug_CreateKeyFingerprint(CRYP_RSAKEY *key1);
unsigned int Debug_DataFingerPrint(const char *data, int size);

#ifdef __cplusplus
}
#endif


#endif /* CTSERVICE_H */







