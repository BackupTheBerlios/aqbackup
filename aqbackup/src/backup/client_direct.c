/***************************************************************************
 $RCSfile: client_direct.c,v $
                             -------------------
    cvs         : $Id: client_direct.c,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "client_p.h"
#include "client_direct_p.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


AQB_CLIENT_DIRECT_DATA *AQBClient_DirectData_new(){
  AQB_CLIENT_DIRECT_DATA *cdd;

  cdd=(AQB_CLIENT_DIRECT_DATA *)malloc(sizeof(AQB_CLIENT_DIRECT_DATA));
  assert(cdd);
  memset(cdd, 0, sizeof(AQB_CLIENT_DIRECT_DATA));
  cdd->server=AQBServer_new();
  return cdd;

}


void AQBClient_DirectData_free(AQB_CLIENT_DIRECT_DATA *cdd){
  if (cdd) {
    AQBServer_free(cdd->server);
    free(cdd);
  }
}



int AQBClientDirect_Init(AQB_CLIENT *c, CONFIGGROUP *cfg){
  AQB_CLIENT_DIRECT_DATA *cdd;
  const char *servercfgfile;
  CONFIGGROUP *servercfg;
  int rv;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  servercfgfile=Config_GetValue(cfg,"servercfg",AQBACKUPD_CFGFILE,0);

  servercfg=Config_new();
  rv=Config_ReadFile(servercfg,
		     servercfgfile,
		     CONFIGMODE_ALLOW_GROUPS |
		     CONFIGMODE_OVERWRITE_VARS |
		     CONFIGMODE_REMOVE_QUOTES |
		     CONFIGMODE_REMOVE_STARTING_BLANKS |
		     CONFIGMODE_REMOVE_TRAILING_BLANKS
		    );
  if (rv) {
    DBG_ERROR("Could not read server configuration file.");
    Config_free(servercfg);
    return 1;
  }

  rv=AQBServer_Init(cdd->server, servercfg);
  Config_free(servercfg);
  return rv;
}



int AQBClientDirect_Fini(AQB_CLIENT *c){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);
  return AQBServer_Fini(cdd->server);
}



int AQBClientDirect_Register(AQB_CLIENT *c, const char *name){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  c->cid=AQBServer_ClientRegister(cdd->server, name);
  return c->cid;
}



int AQBClientDirect_Unregister(AQB_CLIENT *c){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_ClientUnregister(cdd->server, c->cid);
}



int AQBClientDirect_OpenRepository(AQB_CLIENT *c,
				   const char *hostname,
				   const char *repository,
				   int writeAccess){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  c->rid=AQBServer_OpenRepository(cdd->server,
				  c->cid,
				  hostname,
				  repository,
				  writeAccess);
  return c->rid;
}



int AQBClientDirect_CreateRepository(AQB_CLIENT *c,
				     const char *hostname,
				     const char *repository){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  c->rid=AQBServer_CreateRepository(cdd->server,
				    c->cid,
				    hostname,
				    repository);
  return c->rid;
}



int AQBClientDirect_CloseRepository(AQB_CLIENT *c){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_CloseRepository(cdd->server, c->cid, c->rid);
}



int AQBClientDirect_OpenDir(AQB_CLIENT *c,
			    const char *dir){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_OpenDir(cdd->server, c->cid, c->rid, dir);
}



int AQBClientDirect_CloseDir(AQB_CLIENT *c,
			     int did){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_CloseDir(cdd->server, c->cid, did);
}



int AQBClientDirect_GetEntries(AQB_CLIENT *c,
			       int did,
                               time_t when,
			       AQB_ENTRY **entries){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_GetEntries(cdd->server, c->cid, did, when, entries);
}



int AQBClientDirect_SetEntry(AQB_CLIENT *c,
			     int did,
			     AQB_ENTRY *entry){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_SetEntry(cdd->server, c->cid, did, entry);
}



int AQBClientDirect_DeleteEntry(AQB_CLIENT *c,
				int did,
				AQB_ENTRY *entry){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_DeleteEntry(cdd->server, c->cid, did, entry);
}



int AQBClientDirect_FileOpenOut(AQB_CLIENT *c,
				int did,
				AQB_ENTRY *entry){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_FileOpenOut(cdd->server, c->cid, did, entry);
}



int AQBClientDirect_FileWrite(AQB_CLIENT *c,
			      int fid,
			      const char *data,
			      unsigned int size){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_FileWrite(cdd->server, c->cid, fid, data, size);
}



int AQBClientDirect_FileCloseOut(AQB_CLIENT *c,
				 int fid,
				 const char *md5){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_FileCloseOut(cdd->server, c->cid, fid, md5);
}



int AQBClientDirect_FileOpenIn(AQB_CLIENT *c,
			       int did,
			       const char *name,
			       time_t when,
			       AQB_ENTRY **entry){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_FileOpenIn(cdd->server, c->cid, did, name, when, entry);
}



int AQBClientDirect_FileRead(AQB_CLIENT *c,
			     int fid,
			     char *data,
			     unsigned int size){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_FileRead(cdd->server, c->cid, fid, data, size);
}



int AQBClientDirect_FileCloseIn(AQB_CLIENT *c,
				int fid){
  AQB_CLIENT_DIRECT_DATA *cdd;

  assert(c);
  assert(c->privateData);
  cdd=(AQB_CLIENT_DIRECT_DATA*)(c->privateData);
  assert(cdd->server);

  return AQBServer_FileCloseIn(cdd->server, c->cid, fid);
}



void AQBClientDirect_FreePrivate(AQB_CLIENT *c) {
  assert(c);
  if (c->privateData) {
    AQBClient_DirectData_free((AQB_CLIENT_DIRECT_DATA*)(c->privateData));
  }
}


AQB_CLIENT *AQBClientDirect_new(){
  AQB_CLIENT *c;
  AQB_CLIENT_DIRECT_DATA *cdd;

  c=AQBClient_new();
  cdd=AQBClient_DirectData_new();
  c->privateData=cdd;
  c->initPtr=AQBClientDirect_Init;
  c->finiPtr=AQBClientDirect_Fini;
  c->registerPtr=AQBClientDirect_Register;
  c->unregisterPtr=AQBClientDirect_Unregister;
  c->openRepositoryPtr=AQBClientDirect_OpenRepository;
  c->createRepositoryPtr=AQBClientDirect_CreateRepository;
  c->closeRepositoryPtr=AQBClientDirect_CloseRepository;
  c->openDirPtr=AQBClientDirect_OpenDir;
  c->closeDirPtr=AQBClientDirect_CloseDir;
  c->getEntriesPtr=AQBClientDirect_GetEntries;
  c->setEntryPtr=AQBClientDirect_SetEntry;
  c->deleteEntryPtr=AQBClientDirect_DeleteEntry;
  c->fileOpenoutPtr=AQBClientDirect_FileOpenOut;
  c->fileCloseoutPtr=AQBClientDirect_FileCloseOut;
  c->fileWritePtr=AQBClientDirect_FileWrite;
  c->fileOpeninPtr=AQBClientDirect_FileOpenIn;
  c->fileCloseinPtr=AQBClientDirect_FileCloseIn;
  c->fileReadPtr=AQBClientDirect_FileRead;

  c->freePrivatePtr=AQBClientDirect_FreePrivate;

  return c;
}







