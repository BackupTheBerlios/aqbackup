/***************************************************************************
 $RCSfile: client.h,v $
                             -------------------
    cvs         : $Id: client.h,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
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


#ifndef AQBACKUP_CLIENT_H
#define AQBACKUP_CLIENT_H

#include <backup/entry.h>
#include <backup/interactor.h>

#include <chameleon/chameleon.h>
#include <chameleon/conf.h>
#include <chameleon/stringlist.h>



typedef struct _AQB_CLIENT AQB_CLIENT;


AQB_CLIENT *AQBClient_new();
void AQBClient_free(AQB_CLIENT *c);
AQB_CLIENT *AQBClient_dup(AQB_CLIENT *o);

void AQBClient_add(AQB_CLIENT *c, AQB_CLIENT **head);
void AQBClient_del(AQB_CLIENT *c, AQB_CLIENT **head);



int AQBClient_Init(AQB_CLIENT *c, CONFIGGROUP *cfg);
int AQBClient_Fini(AQB_CLIENT *c);

int AQBClient_Register(AQB_CLIENT *c, const char *name);
int AQBClient_Unregister(AQB_CLIENT *c);
int AQBClient_OpenRepository(AQB_CLIENT *c,
			     const char *hostname,
			     const char *repository,
			     int writeAccess);
int AQBClient_CreateRepository(AQB_CLIENT *c,
			       const char *hostname,
			       const char *repository);
int AQBClient_CloseRepository(AQB_CLIENT *c);
int AQBClient_OpenDir(AQB_CLIENT *c,
		      const char *dir);
int AQBClient_CloseDir(AQB_CLIENT *c,
		       int did);

int AQBClient_GetEntries(AQB_CLIENT *c,
			 int did,
                         time_t when,
			 AQB_ENTRY **entries);

int AQBClient_SetEntry(AQB_CLIENT *c,
		       int did,
		       AQB_ENTRY *entry);

int AQBClient_DeleteEntry(AQB_CLIENT *c,
			  int did,
			  AQB_ENTRY *entry);


int AQBClient_FileOpenOut(AQB_CLIENT *c,
			  int did,
			  AQB_ENTRY *entry);

int AQBClient_FileWrite(AQB_CLIENT *c,
			int fid,
			const char *data,
			unsigned int size);

int AQBClient_FileCloseOut(AQB_CLIENT *c,
			   int fid, const char *md5);

int AQBClient_FileOpenIn(AQB_CLIENT *c,
			 int did,
			 const char *name,
			 time_t when,
			 AQB_ENTRY **entry);

int AQBClient_FileRead(AQB_CLIENT *c,
		       int fid,
		       char *data,
		       unsigned int size);

int AQBClient_FileCloseIn(AQB_CLIENT *c,
			  int fid);

void AQBClient_SetInteractor(AQB_CLIENT *c,
                             AQB_INTERACTOR *ia);
AQB_INTERACTOR *AQBClient_GetInteractor(AQB_CLIENT *c);



#endif

