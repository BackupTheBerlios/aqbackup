/***************************************************************************
 $RCSfile: client_direct_p.h,v $
                             -------------------
    cvs         : $Id: client_direct_p.h,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
    begin       : Sat May 31 2003
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


#ifndef AQBACKUP_CLIENT_DIRECT_P_H
#define AQBACKUP_CLIENT_DIRECT_P_H

#include "server.h"
#include "client.h"
#include "client_direct.h"


typedef struct _AQB_CLIENT_DIRECT_DATA AQB_CLIENT_DIRECT_DATA;
struct _AQB_CLIENT_DIRECT_DATA {
  AQB_SERVER *server;

};


AQB_CLIENT_DIRECT_DATA *AQBClient_DirectData_new();
void AQBClient_DirectData_free(AQB_CLIENT_DIRECT_DATA *cd);


int AQBClientDirect_Init(AQB_CLIENT *c, CONFIGGROUP *cfg);
int AQBClientDirect_Fini(AQB_CLIENT *c);

int AQBClientDirect_Register(AQB_CLIENT *c, const char *name);
int AQBClientDirect_Unregister(AQB_CLIENT *c);
int AQBClientDirect_OpenRepository(AQB_CLIENT *c,
				   const char *hostname,
				   const char *repository,
				   int writeAccess);
int AQBClientDirect_CreateRepository(AQB_CLIENT *c,
				     const char *hostname,
				     const char *repository);
int AQBClientDirect_CloseRepository(AQB_CLIENT *c);
int AQBClientDirect_OpenDir(AQB_CLIENT *c,
			    const char *dir);
int AQBClientDirect_CloseDir(AQB_CLIENT *c,
			     int did);

int AQBClientDirect_GetEntries(AQB_CLIENT *c,
			       int did,
                               time_t when,
			       AQB_ENTRY **entries);
int AQBClientDirect_SetEntry(AQB_CLIENT *c,
			     int did,
			     AQB_ENTRY *entry);
int AQBClientDirect_DeleteEntry(AQB_CLIENT *c,
				int did,
				AQB_ENTRY *entry);

int AQBClientDirect_FileOpenOut(AQB_CLIENT *c,
				int did,
				AQB_ENTRY *entry);

int AQBClientDirect_FileWrite(AQB_CLIENT *c,
			      int fid,
			      const char *data,
			      unsigned int size);

int AQBClientDirect_FileCloseOut(AQB_CLIENT *c,
				 int fid, const char *md5);

int AQBClientDirect_FileOpenIn(AQB_CLIENT *c,
			       int did,
			       const char *name,
			       time_t when,
			       AQB_ENTRY **entry);

int AQBClientDirect_FileRead(AQB_CLIENT *c,
			     int fid,
			     char *data,
			     unsigned int size);

int AQBClientDirect_FileCloseIn(AQB_CLIENT *c,
				int fid);



void AQBClientDirect_FreePrivate(AQB_CLIENT *c);



#endif

