/***************************************************************************
 $RCSfile: server.h,v $
                             -------------------
    cvs         : $Id: server.h,v 1.1 2003/06/07 21:07:46 aquamaniac Exp $
    begin       : Thue May 21 2003
    copyright   : (C) 2003 by Martin Preuss
    email       : openhbci@aquamaniac.de

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


#ifndef AQBACKUP_SERVER_H
#define AQBACKUP_SERVER_H


#include <backup/entry.h>
#include <stdio.h>
#include <chameleon/chameleon.h>
#include <chameleon/stringlist.h>


#define AQBACKUP_MINIMUM_AUTHLEVEL 0


typedef struct _AQB_FILE AQB_FILE;
typedef struct _AQB_DIRECTORY AQB_DIRECTORY;
typedef struct _AQB_SERVER_CLIENT AQB_SERVER_CLIENT;
typedef struct _AQB_SERVER_USER AQB_SERVER_USER;
typedef struct _AQB_REPOSITORY AQB_REPOSITORY;
typedef struct _AQB_SERVER AQB_SERVER;


struct _AQB_FILE {
  AQB_FILE *next;
  unsigned int id;
  char *path;
  unsigned int bytesWritten;
  unsigned int bytesRead;
  unsigned int did;
  unsigned int fileId;
  unsigned int fileRevision;
  FILE *file;
};


AQB_FILE *AQBFile_new();
void AQBFile_free(AQB_FILE *f);
void AQBFile_add(AQB_FILE *f, AQB_FILE **head);
void AQBFile_del(AQB_FILE *f, AQB_FILE **head);


struct _AQB_DIRECTORY {
  AQB_DIRECTORY *next;
  int modified;
  char *entriesPath;

  unsigned int id;
  AQB_REPOSITORY *repository;
  char *name;
  unsigned int usageCounter;

  AQB_ENTRY *entries;
};

AQB_DIRECTORY *AQBDirectory_new();
void AQBDirectory_free(AQB_DIRECTORY *d);
void AQBDirectory_add(AQB_DIRECTORY *d, AQB_DIRECTORY **head);
void AQBDirectory_del(AQB_DIRECTORY *d, AQB_DIRECTORY **head);



struct _AQB_SERVER_CLIENT {
  AQB_SERVER_CLIENT *next;
  unsigned int id;
  int repositoryId;
  char *name;
  int mayCreate;
  int authLevel;
  int readOk;
  int writeOk;
};


AQB_SERVER_CLIENT *AQBServerClient_new();
void AQBServerClient_free(AQB_SERVER_CLIENT *c);
void AQBServerClient_add(AQB_SERVER_CLIENT *c, AQB_SERVER_CLIENT **head);
void AQBServerClient_del(AQB_SERVER_CLIENT *c, AQB_SERVER_CLIENT **head);


struct _AQB_SERVER_USER {
  AQB_SERVER_USER *next;
  char *name;
  char *passwd;
  int mayCreate;
};

AQB_SERVER_USER *AQBServerUser_new();
void AQBServerUser_free(AQB_SERVER_USER *c);
void AQBServerUser_add(AQB_SERVER_USER *c, AQB_SERVER_USER **head);
void AQBServerUser_del(AQB_SERVER_USER *c, AQB_SERVER_USER **head);



struct _AQB_REPOSITORY {
  AQB_REPOSITORY *next;
  char *hostName;
  char *name;
  char *baseDir;
  unsigned int id;
  unsigned int nextFolderId;
  unsigned int nextFileId;
  STRINGLIST *readers;
  STRINGLIST *writers;

  unsigned int writeLockedBy;
  unsigned int usageCounter;
  AQB_DIRECTORY *folders;
  AQB_FILE *files;
  unsigned int nextFileRuntimeId;
};

AQB_REPOSITORY *AQBRepository_new();
void AQBRepository_free(AQB_REPOSITORY *r);
void AQBRepository_add(AQB_REPOSITORY *r, AQB_REPOSITORY **head);
void AQBRepository_del(AQB_REPOSITORY *r, AQB_REPOSITORY **head);



struct _AQB_SERVER {
  unsigned int nextRepositoryId;
  unsigned int nextClientId;
  AQB_REPOSITORY *repositories;
  char *baseDir;
  unsigned int minimumAuthLevel;
  AQB_SERVER_CLIENT *clients;
  AQB_SERVER_USER *users;
};

AQB_SERVER *AQBServer_new();
void AQBServer_free(AQB_SERVER *s);

int AQBServer_Init(AQB_SERVER *s, CONFIGGROUP *cfg);
int AQBServer_Fini(AQB_SERVER *s);

int AQBServer_ClientRegister(AQB_SERVER *s, const char *name);
int AQBServer_ClientUnregister(AQB_SERVER *s, int cid);


int AQBServer_OpenRepository(AQB_SERVER *s,
			     int cid,
			     const char *hostname,
			     const char *repository,
			     int writeAccess);


int AQBServer_CreateRepository(AQB_SERVER *s,
			       int cid,
			       const char *hostname,
			       const char *repository);


int AQBServer_CloseRepository(AQB_SERVER *s,
			      int cid,
			      int rid);

int AQBServer_OpenDir(AQB_SERVER *s,
		      int cid,
		      int rid,
		      const char *dir);
int AQBServer_CloseDir(AQB_SERVER *s,
		       int cid,
		       int did);

int AQBServer_GetEntries(AQB_SERVER *s,
			 int cid,
			 int did,
                         time_t when,
			 AQB_ENTRY **entries);

int AQBServer_SetEntry(AQB_SERVER *s,
		       int cid,
		       int did,
		       AQB_ENTRY *entry);


int AQBServer_DeleteEntry(AQB_SERVER *s,
			  int cid,
			  int did,
			  AQB_ENTRY *entry);


int AQBServer_FileOpenOut(AQB_SERVER *s,
			  int cid,
			  int did,
			  AQB_ENTRY *entry);

int AQBServer_FileWrite(AQB_SERVER *s,
			int cid,
			int fid,
			const char *data,
			unsigned int size);

int AQBServer_FileCloseOut(AQB_SERVER *s,
			   int cid,
			   int fid,
			   const char *md5);

int AQBServer_FileOpenIn(AQB_SERVER *s,
			 int cid,
			 int did,
			 const char *name,
			 time_t when,
			 AQB_ENTRY **entry);

int AQBServer_FileRead(AQB_SERVER *s,
		       int cid,
		       int fid,
		       char *data,
		       unsigned int size);

int AQBServer_FileCloseIn(AQB_SERVER *s,
			  int cid,
			  int fid);




#endif




