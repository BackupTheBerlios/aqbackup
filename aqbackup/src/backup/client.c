/***************************************************************************
 $RCSfile: client.c,v $
                             -------------------
    cvs         : $Id: client.c,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
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


#include "client_p.h"
#include "misc.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>
#include <chameleon/directory.h>
#include <chameleon/stringlist.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                          AQBClient
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_CLIENT *AQBClient_new() {
  AQB_CLIENT *c;

  c=(AQB_CLIENT *)malloc(sizeof(AQB_CLIENT));
  assert(c);
  memset(c, 0, sizeof(AQB_CLIENT));
  return c;
}



void AQBClient_free(AQB_CLIENT *c){
  if (c) {
    if (c->freePrivatePtr)
      c->freePrivatePtr(c);
    free(c);
  }
}



void AQBClient_add(AQB_CLIENT *c, AQB_CLIENT **head){
  AQLIST_ADD(AQB_CLIENT, c, head);
}



void AQBClient_del(AQB_CLIENT *c, AQB_CLIENT **head){
  AQLIST_DEL(AQB_CLIENT, c, head);
}





int AQBClient_Init(AQB_CLIENT *c, CONFIGGROUP *cfg){
  assert(c);
  assert(c->initPtr);
  return c->initPtr(c, cfg);
}



int AQBClient_Fini(AQB_CLIENT *c){
  assert(c);
  assert(c->finiPtr);
  return c->finiPtr(c);
}


int AQBClient_Register(AQB_CLIENT *c, const char *name){
  assert(c);
  assert(c->registerPtr);
  return c->registerPtr(c, name);
}



int AQBClient_Unregister(AQB_CLIENT *c){
  assert(c);
  assert(c->unregisterPtr);
  return c->unregisterPtr(c);
}



int AQBClient_OpenRepository(AQB_CLIENT *c,
			     const char *hostname,
			     const char *repository,
			     int writeAccess){
  assert(c);
  assert(c->openRepositoryPtr);
  return c->openRepositoryPtr(c,
			      hostname,
			      repository,
                              writeAccess);
}


int AQBClient_CreateRepository(AQB_CLIENT *c,
			       const char *hostname,
			       const char *repository){
  assert(c);
  assert(c->createRepositoryPtr);
  return c->createRepositoryPtr(c,
				hostname,
				repository);
}



int AQBClient_CloseRepository(AQB_CLIENT *c){
  assert(c);
  assert(c->closeRepositoryPtr);
  return c->closeRepositoryPtr(c);
}


int AQBClient_OpenDir(AQB_CLIENT *c,
		      const char *dir){
  assert(c);
  assert(c->openDirPtr);
  return c->openDirPtr(c, dir);
}


int AQBClient_CloseDir(AQB_CLIENT *c,
		       int did) {
  assert(c);
  assert(c->closeDirPtr);
  return c->closeDirPtr(c, did);
}



int AQBClient_GetEntries(AQB_CLIENT *c,
			 int did,
                         time_t when,
			 AQB_ENTRY **entries){
  assert(c);
  assert(c->getEntriesPtr);
  return c->getEntriesPtr(c, did, when, entries);
}


int AQBClient_SetEntry(AQB_CLIENT *c,
		       int did,
		       AQB_ENTRY *entry){
  assert(c);
  assert(c->setEntryPtr);
  return c->setEntryPtr(c, did, entry);
}


int AQBClient_DeleteEntry(AQB_CLIENT *c,
			  int did,
			  AQB_ENTRY *entry){
  assert(c);
  assert(c->deleteEntryPtr);
  return c->deleteEntryPtr(c, did, entry);
}


int AQBClient_FileOpenOut(AQB_CLIENT *c,
			  int did,
			  AQB_ENTRY *entry){
  assert(c);
  assert(c->fileOpenoutPtr);
  return c->fileOpenoutPtr(c, did, entry);
}


int AQBClient_FileWrite(AQB_CLIENT *c,
			int fid,
			const char *data,
			unsigned int size){
  assert(c);
  assert(c->fileWritePtr);
  return c->fileWritePtr(c, fid, data, size);
}



int AQBClient_FileCloseOut(AQB_CLIENT *c,
			   int fid, const char *md5){
  assert(c);
  assert(c->fileCloseoutPtr);
  return c->fileCloseoutPtr(c, fid, md5);
}


int AQBClient_FileOpenIn(AQB_CLIENT *c,
			 int did,
			 const char *name,
			 time_t when,
			 AQB_ENTRY **entry){
  assert(c);
  assert(c->fileOpeninPtr);
  return c->fileOpeninPtr(c, did, name, when, entry);
}



int AQBClient_FileRead(AQB_CLIENT *c,
		       int fid,
		       char *data,
		       unsigned int size){
  assert(c);
  assert(c->fileReadPtr);
  return c->fileReadPtr(c, fid, data, size);
}



int AQBClient_FileCloseIn(AQB_CLIENT *c,
			  int fid){
  assert(c);
  assert(c->fileCloseinPtr);
  return c->fileCloseinPtr(c, fid);
}



void AQBClient_SetInteractor(AQB_CLIENT *c,
			     AQB_INTERACTOR *ia){
  assert(c);
  c->interactor=ia;
}



AQB_INTERACTOR *AQBClient_GetInteractor(AQB_CLIENT *c){
  assert(c);
  return c->interactor;
}














