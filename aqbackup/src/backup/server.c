/***************************************************************************
 $RCSfile: server.c,v $
                             -------------------
    cvs         : $Id: server.c,v 1.1 2003/06/07 21:07:46 aquamaniac Exp $
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "server.h"
#include "server_p.h"
#include "misc.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>
#include <chameleon/directory.h>


/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                AQBFile
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_FILE *AQBFile_new(){
  AQB_FILE *f;

  f=(AQB_FILE *)malloc(sizeof(AQB_FILE));
  assert(f);
  memset(f, 0, sizeof(AQB_FILE));
  return f;
}



void AQBFile_free(AQB_FILE *f){
  if (f) {
    free(f->path);
    free(f);
  }
}



void AQBFile_add(AQB_FILE *f, AQB_FILE **head){
  AQLIST_ADD(AQB_FILE, f, head);
}



void AQBFile_del(AQB_FILE *f, AQB_FILE **head){
  AQLIST_DEL(AQB_FILE, f, head);
}




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                AQBDirectory
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_DIRECTORY *AQBDirectory_new(){
  AQB_DIRECTORY *d;

  d=(AQB_DIRECTORY *)malloc(sizeof(AQB_DIRECTORY));
  assert(d);
  memset(d, 0, sizeof(AQB_DIRECTORY));
  return d;
}



void AQBDirectory_free(AQB_DIRECTORY *d){
  if (d) {
    AQB_ENTRY *e;

    /* free entries */
    e=d->entries;
    while(e) {
      AQB_ENTRY *ne;

      ne=e->next;
      AQBEntry_free(e);
      e=ne;
    } /* while entries */

    free(d->name);
    free(d->entriesPath);
    free(d);
  }
}



void AQBDirectory_add(AQB_DIRECTORY *d, AQB_DIRECTORY **head){
  AQLIST_ADD(AQB_DIRECTORY, d, head);
}



void AQBDirectory_del(AQB_DIRECTORY *d, AQB_DIRECTORY **head){
  AQLIST_DEL(AQB_DIRECTORY, d, head);
}




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                AQBServerClient
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_SERVER_CLIENT *AQBServerClient_new(){
  AQB_SERVER_CLIENT *c;

  c=(AQB_SERVER_CLIENT *)malloc(sizeof(AQB_SERVER_CLIENT));
  assert(c);
  memset(c, 0, sizeof(AQB_SERVER_CLIENT));
  return c;
}



void AQBServerClient_free(AQB_SERVER_CLIENT *c){
  if (c) {
    free(c->name);
    free(c);
  }
}



void AQBServerClient_add(AQB_SERVER_CLIENT *c, AQB_SERVER_CLIENT **head){
  AQLIST_ADD(AQB_SERVER_CLIENT, c, head);
}



void AQBServerClient_del(AQB_SERVER_CLIENT *c, AQB_SERVER_CLIENT **head){
  AQLIST_DEL(AQB_SERVER_CLIENT, c, head);
}




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                AQBServerUser
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_SERVER_USER *AQBServerUser_new(){
  AQB_SERVER_USER *u;

  u=(AQB_SERVER_USER *)malloc(sizeof(AQB_SERVER_USER));
  assert(u);
  memset(u, 0, sizeof(AQB_SERVER_USER));
  return u;
}



void AQBServerUser_free(AQB_SERVER_USER *u){
  if (u) {
    free(u->name);
    free(u);
  }
}



void AQBServerUser_add(AQB_SERVER_USER *u, AQB_SERVER_USER **head){
  AQLIST_ADD(AQB_SERVER_USER, u, head);
}



void AQBServerUser_del(AQB_SERVER_USER *u, AQB_SERVER_USER **head){
  AQLIST_DEL(AQB_SERVER_USER, u, head);
}




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                AQBRepository
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_REPOSITORY *AQBRepository_new(){
  AQB_REPOSITORY *r;

  r=(AQB_REPOSITORY *)malloc(sizeof(AQB_REPOSITORY));
  assert(r);
  memset(r, 0, sizeof(AQB_REPOSITORY));
  r->readers=StringList_new();
  r->writers=StringList_new();
  return r;
}



void AQBRepository_free(AQB_REPOSITORY *r){
  if (r) {
    AQB_DIRECTORY *d;
    AQB_FILE *f;

    /* free directories */
    d=r->folders;
    while(d) {
      AQB_DIRECTORY *nd;

      nd=d->next;
      AQBDirectory_free(d);
      d=nd;
    } /* while entries */

    /* free files */
    f=r->files;
    while(f) {
      AQB_FILE *nf;

      nf=f->next;
      AQBFile_free(f);
      f=nf;
    } /* while files */

    free(r->name);
    free(r->hostName);
    free(r->baseDir);
    StringList_free(r->readers);
    StringList_free(r->writers);
    free(r);
  }
}



void AQBRepository_add(AQB_REPOSITORY *r, AQB_REPOSITORY **head){
  AQLIST_ADD(AQB_REPOSITORY, r, head);
}



void AQBRepository_del(AQB_REPOSITORY *r, AQB_REPOSITORY **head){
  AQLIST_DEL(AQB_REPOSITORY, r, head);
}




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                AQBServer
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


AQB_SERVER *AQBServer_new(){
  AQB_SERVER *s;

  s=(AQB_SERVER *)malloc(sizeof(AQB_SERVER));
  assert(s);
  memset(s, 0, sizeof(AQB_SERVER));
  s->minimumAuthLevel=AQBACKUP_MINIMUM_AUTHLEVEL;
  return s;
}



void AQBServer_free(AQB_SERVER *s){
  if (s) {

    AQB_SERVER_CLIENT *c;
    AQB_REPOSITORY *r;
    AQB_SERVER_USER *u;

    /* free clients */
    c=s->clients;
    while(c) {
      AQB_SERVER_CLIENT *nc;

      nc=c->next;
      AQBServerClient_free(c);
      c=nc;
    } /* while clients */

    /* free repositories */
    r=s->repositories;
    while(r) {
      AQB_REPOSITORY *nr;

      nr=r->next;
      AQBRepository_free(r);
      r=nr;
    } /* while repositories */

    u=s->users;
    while(u) {
      AQB_SERVER_USER *nu;

      nu=u->next;
      AQBServerUser_free(u);
      u=nu;
    } /* while clients */

    free(s->baseDir);
    free(s);
  }
}



int AQBServer_Init(AQB_SERVER *s, CONFIGGROUP *cfg){
  CONFIGGROUP *group, *group2;

  DBG_DEBUG("Initializing server");
  s->baseDir=strdup(Config_GetValue(cfg,
				    "basedir",
				    AQBACKUP_REPOSITORY_DIR,
				    0));

  group=Config_GetGroup(cfg,
			"users",
			CONFIGMODE_NAMEMUSTEXIST);
  if (!group) {
    DBG_WARN("No users in configuration");
  }
  else {
    group2=group->groups;
    while(group2) {
      CONFIGGROUP *currgroup;
      AQB_SERVER_USER *u;
      const char *t;

      currgroup=group2;
      group2=group2->next;

      u=AQBServerUser_new();

      t=Config_GetValue(currgroup, "name", 0, 0);
      if (!t) {
	DBG_ERROR("User section without name");
	AQBServerUser_free(u);
	return 1;
      }
      u->name=strdup(t);

      t=Config_GetValue(currgroup, "passwd", 0, 0);
      if (!t) {
	DBG_ERROR("User section without passwd");
	AQBServerUser_free(u);
	return 1;
      }
      u->passwd=strdup(t);

      u->mayCreate=Config_GetIntValue(currgroup, "maycreate",0,0);
      AQBServerUser_add(u, &(s->users));
      DBG_INFO("User \"%s\" added", u->name);
    } /* while */
  }

  DBG_DEBUG("Initializing server - done");
  return 0;
}



int AQBServer_Fini(AQB_SERVER *s){
  return 0;
}



int AQBServer_ClientRegister(AQB_SERVER *s, const char *name){
  AQB_SERVER_CLIENT *c;
  AQB_SERVER_USER *u;

  assert(s);
  assert(name);

  u=s->users;
  while(u) {
    if (strcmp(u->name, name)==0)
      break;
    u=u->next;
  } /* while user */

  if (!u) {
    DBG_ERROR("User \"%s\" unknown", name);
    return 0;
  }

  c=AQBServerClient_new();
  c->id=++(s->nextClientId);
  c->name=strdup(name);
  c->mayCreate=u->mayCreate;
  c->authLevel=0; /* just to be sure ;-) */

  AQBServerClient_add(c, &(s->clients));
  DBG_NOTICE("Client registered");
  return c->id;
}



int AQBServer_ClientUnregister(AQB_SERVER *s, int cid){
  AQB_SERVER_CLIENT *c;

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  /* unregister repository */
  if (c->repositoryId)
    AQBServer_CloseRepository(s, c->id, c->repositoryId);

  AQBServerClient_del(c, &(s->clients));
  AQBServerClient_free(c);

  DBG_NOTICE("Client unregistered");
  return 0;
}



int AQBServer_CreateRepository(AQB_SERVER *s,
			       int cid,
			       const char *hostname,
			       const char *repository) {
  char buffer[300];
  char buffer2[300];
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;

  assert(s);
  /* get client */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("Client \"%d\" not found", cid);
    return 0;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 0;
  }

  if (c->mayCreate==0) {
    DBG_ERROR("Client \"%s\" is not allowed to create repositories",
	      c->name);
    return 0;
  }

  r=s->repositories;
  while(r) {
    if (strcmp(r->hostName, hostname)==0 &&
	strcmp(r->name, repository)==0)
      break;
    r=r->next;
  } /* while repositories */

  if (r) {
    DBG_ERROR("Repository \"%s:%s\" already exists.",
	      hostname, repository);
    return 0;
  }

  if ((strlen(s->baseDir)+
       strlen(hostname)+
       10)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 0;
  }

  /* create dir */
  strcpy(buffer, s->baseDir);
  strcat(buffer,"/");
  strcat(buffer, hostname);
  if (mkdir(buffer,S_IRWXU)) {
    if (errno!=EEXIST) {
      DBG_ERROR("Could not create dir \"%s\": %s",
		buffer, strerror(errno));
      return 0;
    }
  }

  strcat(buffer,"/");
  strcat(buffer, repository);
  if (mkdir(buffer,S_IRWXU)) {
    DBG_ERROR("Could not create dir \"%s\": %s",
	      buffer, strerror(errno));
    return 0;
  }

  /* create repository */
  r=AQBRepository_new();
  r->hostName=strdup(hostname);
  r->name=strdup(repository);
  r->baseDir=strdup(buffer);
  r->id=++(s->nextRepositoryId);
  r->writeLockedBy=cid;
  StringList_AppendEntry(r->writers, StringListEntry_new(c->name, 0));
  r->usageCounter++;
  c->readOk=1;
  c->writeOk=1;
  AQBRepository_add(r, &(s->repositories));

  /* create administrative folders */
  strcpy(buffer2, buffer);
  /* admin dir */
  strcat(buffer,"/");
  strcat(buffer, "admin");
  if (mkdir(buffer,S_IRWXU)) {
    DBG_ERROR("Could not create dir \"%s\": %s",
	      buffer, strerror(errno));
    return 0;
  }
  strcpy(buffer, buffer2);

  /* data dir */
  strcat(buffer,"/");
  strcat(buffer, "data");
  if (mkdir(buffer,S_IRWXU)) {
    DBG_ERROR("Could not create dir \"%s\": %s",
	      buffer, strerror(errno));
    return 0;
  }


  c->repositoryId=r->id;
  DBG_INFO("Repository created");
  return r->id;
}



AQB_REPOSITORY *AQBServer__LoadRepository(AQB_SERVER *s,
					  const char *hostname,
					  const char *repository) {
  CONFIGGROUP *cfg;
  char buffer[300];
  AQB_REPOSITORY *r;
  const char *p;
  int i;

  /* create repository struct */
  if ((strlen(s->baseDir)+
       strlen(hostname)+
       32)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 0;
  }

  /* create dirname */
  strcpy(buffer, s->baseDir);
  strcat(buffer,"/");
  strcat(buffer, hostname);
  strcat(buffer,"/");
  strcat(buffer, repository);

  r=AQBRepository_new();
  r->hostName=strdup(hostname);
  r->name=strdup(repository);
  r->baseDir=strdup(buffer);

  /* read configuration */
  strcat(buffer,"/repository.conf");
  cfg=Config_new();
  if (Config_ReadFile(cfg,
		      buffer,
		      CONFIGMODE_REMOVE_STARTING_BLANKS |
		      CONFIGMODE_REMOVE_TRAILING_BLANKS |
		      CONFIGMODE_REMOVE_QUOTES          |
		      CONFIGMODE_ALLOW_GROUPS           |
		      CONFIGMODE_NAMECREATE_GROUP)) {
    DBG_ERROR("Could not read file \"%s\"", buffer);
    Config_free(cfg);
    AQBRepository_free(r);
    return 0;
  }
  r->nextFileId=Config_GetIntValue(cfg, "nextfileid", 0, 0);

  /* read readers */
  i=0;
  while((p=Config_GetValue(cfg, "reader",0,i++))) {
    StringList_AppendEntry(r->readers, StringListEntry_new(p, 0));
    DBG_DEBUG("Added reader \"%s\"", p);
  }

  /* read writers */
  i=0;
  while((p=Config_GetValue(cfg, "writer",0,i++))) {
    StringList_AppendEntry(r->writers, StringListEntry_new(p, 0));
    DBG_DEBUG("Added writer \"%s\"", p);
  }

  Config_free(cfg);

  return r;
}


int AQBServer__SaveRepository(AQB_SERVER *s,
			      AQB_REPOSITORY *r) {
  CONFIGGROUP *cfg;
  char buffer[300];
  STRINGLISTENTRY *se;

  if ((strlen(r->baseDir)+32)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 1;
  }
  strcpy(buffer, r->baseDir);
  strcat(buffer,"/repository.conf");

  cfg=Config_new();
  if (Config_SetIntValue(cfg,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "nextfileid",
			 r->nextFileId)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  /* save readers */
  se=r->readers->first;
  while(se) {
    if (Config_SetValue(cfg,
			CONFIGMODE_VARIABLE |
			CONFIGMODE_NAMECREATE_VARIABLE,
			"reader",
			se->data)) {
      DBG_ERROR("Could not set value");
      Config_free(cfg);
      return 1;
    }
    se=se->next;
  }

  /* save writers */
  se=r->writers->first;
  while(se) {
    if (Config_SetValue(cfg,
			CONFIGMODE_VARIABLE |
			CONFIGMODE_NAMECREATE_VARIABLE,
			"writer",
			se->data)) {
      DBG_ERROR("Could not set value");
      Config_free(cfg);
      return 1;
    }
    se=se->next;
  }

  if (Config_WriteFile(cfg,
		       buffer,
		       CONFIGMODE_REMOVE_STARTING_BLANKS |
		       CONFIGMODE_REMOVE_TRAILING_BLANKS |
		       CONFIGMODE_REMOVE_QUOTES          |
		       CONFIGMODE_ALLOW_GROUPS           |
		       CONFIGMODE_OVERWRITE_VARS         |
			 CONFIGMODE_NAMECREATE_GROUP)) {
    DBG_ERROR("Could not write file \"%s\"", buffer);
    Config_free(cfg);
    return 1;
  }
    else {
      DBG_INFO("Written file \"%s\"", buffer);
    }
  Config_free(cfg);

  return 0;
}



int AQBServer__CheckRepositoryUser(AQB_REPOSITORY *r, AQB_SERVER_CLIENT *c){
  STRINGLISTENTRY *se;

  assert(r);
  assert(c);
  assert(c->name);
  assert(r->readers);
  assert(r->writers);

  c->writeOk=0;
  c->readOk=0;

  se=r->writers->first;
  DBG_DEBUG("Checking writers");
  while(se) {
    DBG_DEBUG("Checking user \"%s\"", se->data);
    if (strcmp(se->data, c->name)==0) {
      c->writeOk=1;
      c->readOk=1;
      return 0;
    }
  } /* while writers */

  DBG_DEBUG("Checking readers");
  se=r->readers->first;
  while(se) {
    DBG_DEBUG("Checking user \"%s\"", se->data);
    if (strcmp(se->data, c->name)==0) {
      c->readOk=1;
      return 0;
    }
  } /* while readers */

  return 1;
}


int AQBServer_OpenRepository(AQB_SERVER *s,
			     int cid,
			     const char *hostname,
			     const char *repository,
			     int writeAccess) {
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;

  assert(s);

  /* get client */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("Client \"%d\" not found", cid);
    return 0;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 0;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (strcmp(r->hostName, hostname)==0 &&
	strcmp(r->name, repository)==0)
      break;
    r=r->next;
  } /* while repositories */

  if (!r) {
    r=AQBServer__LoadRepository(s, hostname, repository);
    if (!r) {
      DBG_ERROR("Error loading repository \"%s:%s\"",
		hostname, repository);
      return 0;
    }

    /* assign id to new repository and add to list */
    r->id=++(s->nextRepositoryId);
    AQBRepository_add(r, &(s->repositories));
  }

  if (AQBServer__CheckRepositoryUser(r, c)) {
    DBG_ERROR("User \"%s\" has no rigths for repository \"%s:%s\"",
	      c->name, hostname, repository);
    return 0;
  }

  if (writeAccess) {
    if (!c->writeOk) {
      DBG_ERROR("User \"%s\" has no write access to repository \"%s:%s\"",
		c->name, hostname, repository);
      return 0;
    }
    if (r->writeLockedBy) {
      DBG_ERROR("Repository locked");
      return 0;
    }
    r->writeLockedBy=cid;
  }
  else {
    if (!c->readOk) {
      DBG_ERROR("User \"%s\" has no read access to repository \"%s:%s\"",
		c->name, hostname, repository);
      return 0;
    }
  }

  r->usageCounter++;
  c->repositoryId=r->id;

  DBG_INFO("Repository open");
  return r->id;
}



int AQBServer_CloseRepository(AQB_SERVER *s,
			      int cid,
			      int rid){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==rid)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", rid);
    return 1;
  }

  if (r->writeLockedBy && r->writeLockedBy==cid) {
    r->writeLockedBy=0;
  }

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 1;
  }

  /* detach repository from client */
  DBG_DEBUG("C: %d R: %d", c->repositoryId, r->id);
  if (c->repositoryId==r->id) {
    if (r->usageCounter)
      r->usageCounter--;
    c->repositoryId=0;
  }

  /* check whether the repository is still in use */
  if (r->usageCounter<1) {
    if (AQBServer__SaveRepository(s, r)) {
      DBG_ERROR("Error saving repository");
      return 1;
    }
    AQBRepository_del(r, &(s->repositories));
    AQBRepository_free(r);
  }
  else {
    DBG_DEBUG("Repository still in use (%d)",
	      r->usageCounter);
  }

  DBG_INFO("Repository closed");
  return 0;
}


char *AQBServer__EscapePath(const char *path,
			    char *buffer,
			    int maxsize) {
  char elbuffer[128];
  char escbuffer[128];
  char *p;

  *buffer=0;
  while(*path) {
    const char *next;

    /* skip all leading '/' */
    if (*path=='/') {
      path++;
      continue;
    }

    /* get next element */
    p=Text_GetWord(path,
		   "/\\",
		   elbuffer,
		   sizeof(elbuffer),
		   TEXT_FLAGS_NEED_DELIMITER |
		   TEXT_FLAGS_NULL_IS_DELIMITER,
		   &next);
    if (!p) {
      DBG_ERROR("Error in path element \"%s\"", path);
      return 0;
    }

    /* escape path element */
    p=Text_Escape(elbuffer,
		  escbuffer,
		  sizeof(escbuffer));
    if (!p) {
      DBG_ERROR("Error in path element \"%s\"", path);
      return 0;
    }

    /* add element to buffer */
    if ((strlen(buffer)+
	 strlen(escbuffer)+10)>maxsize) {
      DBG_ERROR("Buffer too small");
      return 0;
    }
    strcat(buffer,"/");
    strcat(buffer, "d.");
    strcat(buffer, escbuffer);

    path=next;
  } /* while */

  return buffer;
}



int AQBServer_OpenDir(AQB_SERVER *s,
		      int cid,
                      int rid,
		      const char *dir){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_DIRECTORY *d;
  char buffer[300];
  char pathbuffer[300];

  assert(s);

  /* get client */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("Client \"%d\" not found", cid);
    return 0;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 0;
  }


  /* get repository */
  r=s->repositories;
  while(r) {
    if (r->id==rid)
      break;
    r=r->next;
  } /* while repos */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", rid);
    return 0;
  }

  /* escape path */
  if (AQBServer__EscapePath(dir,
			    pathbuffer,
			    sizeof(pathbuffer))==0) {
    DBG_ERROR("Could not escape path \"%s\"", dir);
    return 0;
  }
  DBG_DEBUG("Escaped path to \"%s\"", pathbuffer);

  /* create full path */
  if ((strlen(r->baseDir)+10)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 0;
  }
  strcpy(buffer, r->baseDir);
  strcat(buffer, "/admin");

  if (r->writeLockedBy==cid) {
    /* we are in write mode, so create path if needed */
    DBG_INFO("Creating dir \"%s\" in repository \"%s:%s\"",
	     pathbuffer, r->hostName, r->name);
    if (Path_Create(buffer, pathbuffer)) {
      DBG_ERROR("Could not create dir \"%s\" in repository \"%s:%s\"",
		pathbuffer, r->hostName, r->name);
      return 0;
    }
  }
  else {
    /* read mode, so the folder has to exist. Verify this. */
    DIRECTORYDATA *cd;
    int rv;
    char localbuffer[300];

    /* create full directory path */
    if ((strlen(buffer)+
	 strlen(pathbuffer)+10)>sizeof(localbuffer)) {
      DBG_ERROR("Buffer too small");
      return 0;
    }
    strcpy(localbuffer, buffer);
    strcat(localbuffer, "/");
    strcat(localbuffer, pathbuffer);

    /* check whether we can open the directory */
    cd=Directory_new();
    rv=Directory_Open(cd, localbuffer);
    if (rv==0)
      rv=Directory_Close(cd);
    Directory_free(cd);
    if (rv) {
      DBG_ERROR("Could not open dir \"%s\" in repository \"%s:%s\"",
		dir, r->hostName, r->name);
    }
    return 0;
  }

  /* create directory structure */
  d=AQBDirectory_new();
  d->name=strdup(pathbuffer);

  strcat(buffer, "/");
  strcat(buffer, pathbuffer);
  d->entriesPath=strdup(buffer);

  d->id=++(r->nextFolderId);
  d->repository=r;
  d->usageCounter++;
  AQBDirectory_add(d, &(r->folders));
  return d->id;
}



int AQBServer_CloseDir(AQB_SERVER *s,
		       int cid,
		       int did){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_DIRECTORY *d;
  AQB_ENTRY *e;
  int rv;

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 1;
  }

  /* find directory by id */
  d=r->folders;
  while(d) {
    if (d->id==did)
      break;
    d=d->next;
  } /* while folder */
  if (!d) {
    DBG_ERROR("No dir by id \"%d\"", did);
    return 1;
  }

  /* detach folder from repository */
  if (d->usageCounter)
    d->usageCounter--;


  /* write all modified entries */
  e=d->entries;
  rv=0;
  while(e) {
    int localrv;

    if (e->modified>0) {
      localrv=AQBServer__WriteEntry(s, d, e);
      if (localrv)
	rv=localrv;
    }
    e=e->next;
  } /* while entries */

  /* check whether the directory is still in use */
  if (d->usageCounter<1) {
    AQBDirectory_del(d, &(r->folders));
    AQBDirectory_free(d);
  }
  else {
    DBG_DEBUG("Directory still in use (%d)",
	      d->usageCounter);
  }

  if (rv) {
    DBG_ERROR("Could not save entries !");
    return 1;
  }

  DBG_INFO("Directory closed");
  return 0;
}



AQB_ENTRY *AQBServer__ReadEntry(AQB_SERVER *s,
				AQB_DIRECTORY *d,
				const char *name) {
  char buffer[300];
  FILE *f;
  AQB_ENTRY *e;

  if ((strlen(d->entriesPath)+
       strlen(name)+10)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 0;
  }

  strcpy(buffer, d->entriesPath);
  strcat(buffer,"/");
  strcat(buffer, name);

  f=fopen(buffer, "r");
  if (f) {
    /* file exists, so read it */
    int rv;

    fclose(f);
    DBG_INFO("Reading entries from \"%s\"", buffer);

    /* now read all entries from the file */
    rv=AQBEntry_ReadFile(&e, buffer);
    if (rv) {
      DBG_ERROR("Could not read entry file");
      return 0;
    }
  }
  else {
    DBG_INFO("Entry \"%s\" does not exist", buffer);
    return 0;
  }

  return e;
}



int AQBServer__WriteEntry(AQB_SERVER *s,
			  AQB_DIRECTORY *d,
			  AQB_ENTRY *e) {
  int rv;
  char buffer[300];

  assert(e);

  if ((strlen(d->entriesPath)+
       strlen(e->name)+10)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 1;
  }

  strcpy(buffer, d->entriesPath);
  strcat(buffer, "/e.");
  strcat(buffer, e->name);

  DBG_INFO("Writing entry to \"%s\"", buffer);

  /* now write entry to the file */
  rv=AQBEntry_WriteFile(e, buffer);
  if (rv) {
    DBG_ERROR("Could not write entry file");
    return 1;
  }

  e->modified=0;

  return 0;
}



int AQBServer__ReadEntries(AQB_SERVER *s, AQB_DIRECTORY *d) {
  DIRECTORYDATA *dd;
  int rv;
  char buffer[300];
  AQB_ENTRY *e;

  /* open directory */
  dd=Directory_new();
  rv=Directory_Open(dd, d->entriesPath);
  if (rv) {
    Directory_free(dd);
    return 1;
  }

  /* read all entries except those which begin with a '.' */
  while(Directory_Read(dd, buffer, sizeof(buffer))==0) {
    if (strlen(buffer)>1) {
      if (strncmp(buffer, "e.", 2)==0) {
        DBG_DEBUG("Reading entry file \"%s\"", buffer);
	e=AQBServer__ReadEntry(s, d, buffer);
	if (e)
	  AQBServer__AddEntryToDir(s, d, e);
      }
    }
  }

  /* close directory */
  rv=Directory_Close(dd);
  Directory_free(dd);
  if (rv) {
    return 1;
  }

  return 0;
}



int AQBServer_GetEntries(AQB_SERVER *s,
			 int cid,
			 int did,
                         time_t when,
			 AQB_ENTRY **entries){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_DIRECTORY *d;
  AQB_ENTRY *e, *enew;
  int rv;

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 1;
  }

  /* find directory by id */
  d=r->folders;
  while(d) {
    if (d->id==did)
      break;
    d=d->next;
  } /* while folder */
  if (!d) {
    DBG_ERROR("No dir by id \"%d\"", did);
    return 1;
  }

  /* now read all entries */
  rv=AQBServer__ReadEntries(s, d);
  if (rv) {
    DBG_ERROR("Could not read entries");
    return 1;
  }

  /* duplicate entries */
  e=d->entries;
  while(e) {
    enew=AQBEntry_dup_date(e, when);
    if (enew)
      AQBEntry_add(enew, entries);
    e=e->next;
  } /* while entries */

  return 0;
}



AQB_ENTRY *AQBServer__GetEntry(AQB_SERVER *s,
			       AQB_DIRECTORY *d,
			       const char *name) {
  AQB_ENTRY *e;

  /* check whether we already have an entry for this file */
  e=d->entries;
  while(e) {
    if (strcmp(e->name, name)==0)
      break;
    e=e->next;
  } /* while */
  if (!e) {
    /* not in memory, so try to load the entry */
    e=AQBServer__ReadEntry(s, d, name);
    if (e) {
      /* add entry to memory */
      DBG_INFO("Loaded entry \"%s\"", name);
      AQBEntry_add(e, &(d->entries));
    }
  }

  return e;
}



int AQBServer__AddEntryToDir(AQB_SERVER *s,
			     AQB_DIRECTORY *d,
			     AQB_ENTRY *enew) {
  AQB_ENTRY *e;

  assert(s);
  assert(d);
  assert(enew);

  /* check whether we already have an entry for this file */
  e=d->entries;
  while(e) {
    if (strcmp(e->name, enew->name)==0)
      break;
    e=e->next;
  } /* while */

  if (!e) {
    DBG_INFO("Adding entry \"%s\"", enew->name);
    AQBEntry_add(enew, &(d->entries));
    return 0;
  }
  else
    return 1;
}



int AQBServer__SetEntry(AQB_SERVER *s,
			int cid,
			int did,
			AQB_ENTRY *entry,
			int deleteIt){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_DIRECTORY *d;
  AQB_ENTRY *e, *enew;
  AQB_ENTRY_VERSION *v, *vnew;

  assert(entry);
  assert(entry->versions);

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 1;
  }

  /* find directory by id */
  d=r->folders;
  while(d) {
    if (d->id==did)
      break;
    d=d->next;
  } /* while folder */
  if (!d) {
    DBG_ERROR("No dir by id \"%d\"", did);
    return 1;
  }

  /* check whether we already have an entry for this file */
  e=AQBServer__GetEntry(s, d, entry->name);
  if (!e) {
    if (!deleteIt) {
      /* add whole entry, assigning new ids */
      enew=AQBEntry_dup(entry);
      enew->id=++(r->nextFileId);
      v=enew->versions;
      v->revision=++(enew->nextRevision);
      v->btime=time(0);
      AQBEntry_add(enew, &(d->entries));
      DBG_INFO("New entry added for \"%s\"", enew->name);

      /* save this new entry immediately */
      if (AQBServer__WriteEntry(s, d, enew)) {
	DBG_ERROR("Could not save entry");
	return 1;
      }
    } /* if !deleteIt */
    else {
      DBG_WARN("Entry \"%s\" does not exist, so not deleted",
	       entry->name);
    }
  }
  else {
    AQB_ENTRY_VERSION *vtmp;

    /* do not delete if it already is deleted */
    vtmp=e->versions;
    assert(vtmp);
    while(vtmp->next)
      vtmp=vtmp->next;

    if (vtmp->status!=EntryStatusDeleted || !deleteIt) {
      /* add version only */
      vnew=AQBEntryVersion_dup(entry->versions);
      vnew->revision=++(e->nextRevision);
      vnew->btime=time(0);

      /* copy storefilename and flags from the last entry */
      if (vnew->storeName)
	free(vnew->storeName);
      if (vtmp->storeName)
	vnew->storeName=strdup(vtmp->storeName);
      vnew->flags=vtmp->flags;

      if (!deleteIt) {
	vnew->status=EntryStatusUsed;
      }
      else {
	vnew->status=EntryStatusDeleted;
      }

      /* add entry */
      AQBEntryVersion_add(vnew, &(e->versions));
      DBG_INFO("New version added for \"%s\"", entry->name);

      /* save this modified entry immediately */
      if (AQBServer__WriteEntry(s, d, e)) {
	DBG_ERROR("Could not save entry");
	return 1;
      }
    }
    else {
      DBG_NOTICE("Entry \"%s\" already IS deleted", entry->name);
    }
  }

  return 0;
}



int AQBServer_SetEntry(AQB_SERVER *s,
		       int cid,
		       int did,
		       AQB_ENTRY *entry){
  return AQBServer__SetEntry(s, cid, did, entry, 0);
}



int AQBServer_DeleteEntry(AQB_SERVER *s,
			  int cid,
			  int did,
			  AQB_ENTRY *entry){
  return AQBServer__SetEntry(s, cid, did, entry, 1);
}



char *AQBServer__MakeStoragePath(unsigned int fileid,
				 unsigned int filever,
				 char *buffer,
				 unsigned int maxsize) {
  char *p;
  char hbuffer[64];
  char wbuffer[4];

  if (maxsize<64) {
    DBG_ERROR("Buffer too small");
    return 0;
  }

  strcpy(buffer, "data/");

  /* create string from fileid */
  wbuffer[0]=(fileid>>24) & 0xff;
  wbuffer[1]=(fileid>>16) & 0xff;
  wbuffer[2]=(fileid>>8) & 0xff;
  wbuffer[3]=fileid & 0xff;

  p=Text_ToHexGrouped(wbuffer, sizeof(wbuffer),
		      hbuffer,
                      sizeof(hbuffer),
		      2, '/',
		      0); /* don't skip leading zeroes */
  if (!p) {
    DBG_ERROR("Conversion error");
    return 0;
  }
  strcat(buffer, p);
  strcat(buffer, "-");

  /* create string from filever */
  wbuffer[0]=(filever>>24) & 0xff;
  wbuffer[1]=(filever>>16) & 0xff;
  wbuffer[2]=(filever>>8) & 0xff;
  wbuffer[3]=filever & 0xff;

  p=Text_ToHexGrouped(wbuffer, sizeof(wbuffer),
		      hbuffer,
                      sizeof(hbuffer),
		      2, '/',
		      1); /* skip leading zeroes */
  if (!p) {
    DBG_ERROR("Conversion error");
    return 0;
  }
  strcat(buffer, p);
  strcat(buffer, ".data");

  return buffer;
}



int AQBServer__FileClose(AQB_SERVER *s,
			 int cid,
			 int fid) {
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_FILE *f;

  assert(s);

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 1;
  }

  /* find file by id */
  f=r->files;
  while(f) {
    if (f->id==fid)
      break;
    f=f->next;
  } /* while folder */
  if (!f) {
    DBG_ERROR("No file by id \"%d\"", fid);
    return 1;
  }

  /* close file */
  if (fclose(f->file)) {
    DBG_ERROR("Error on fclose(%s): %s",
	      f->path, strerror(errno));
    return 1;
  }

  DBG_INFO("Read %d bytes from file \"%s\"",
	   f->bytesRead, f->path);

  /* free file entry */
  AQBFile_del(f, &(r->files));
  AQBFile_free(f);

  return 0;
}



int AQBServer_FileOpenOut(AQB_SERVER *s,
			  int cid,
			  int did,
			  AQB_ENTRY *entry){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_DIRECTORY *d;
  AQB_ENTRY *e, *enew, *etouse;
  AQB_ENTRY_VERSION *v, *vnew, *vtouse;
  AQB_FILE *f;
  char *p;
  char storebuffer[300];
  char fullname[300];
  int j;

  assert(entry);
  assert(entry->versions);

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 0;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 0;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 0;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 0;
  }

  /* find directory by id */
  d=r->folders;
  while(d) {
    if (d->id==did)
      break;
    d=d->next;
  } /* while folder */
  if (!d) {
    DBG_ERROR("No dir by id \"%d\"", did);
    return 0;
  }

  /* check whether we already have an entry for this file */
  e=AQBServer__GetEntry(s, d, entry->name);
  if (!e) {
    /* add whole entry, assigning new ids */
    enew=AQBEntry_dup(entry);
    enew->id=++(r->nextFileId);
    v=enew->versions;
    v->revision=++(enew->nextRevision);
    v->btime=time(0);
    free(v->storeName);
    v->storeName=0;

    /* add entry */
    enew->modified++;
    AQBEntry_add(enew, &(d->entries));
    DBG_INFO("New entry added for \"%s\"", enew->name);

    etouse=enew;
    vtouse=v;
  }
  else {
    /* add version only */
    vnew=AQBEntryVersion_dup(entry->versions);
    vnew->revision=++(e->nextRevision);
    vnew->btime=time(0);

    /* add entry */
    e->modified++;
    AQBEntryVersion_add(vnew, &(e->versions));
    DBG_INFO("New version added for \"%s\"", entry->name);

    etouse=e;
    vtouse=vnew;
  }

  /* create storeName */
  j=AQB_SERVER_MAX_TRY_NAME;
  while(j-->0) {
    FILE *fp;

    p=AQBServer__MakeStoragePath(etouse->id,
				 vtouse->revision,
				 storebuffer,
				 sizeof(storebuffer));
    if (!p) {
      DBG_ERROR("Could not create storage path");
      return 0;
    }

    if ((strlen(r->baseDir)+
	 strlen(storebuffer)+10)>sizeof(fullname)) {
      DBG_ERROR("Buffer too small");
      return 0;
    }
    strcpy(fullname, r->baseDir);
    strcat(fullname, "/");
    strcat(fullname, storebuffer);

    fp=fopen(fullname, "r");
    if (!fp) {
      /* check whether this is really free */
      if (errno==ENOENT)
	break;
    }
    else
      fclose(fp);
    /* increment revision number to not overwrite existing files */
    vtouse->revision=++(etouse->nextRevision);
    etouse->modified++;
  } /* while */

  /* store this name */
  free(vtouse->storeName);
  vtouse->storeName=strdup(storebuffer);
  etouse->modified++;
  DBG_DEBUG("Storage name and path is \"%s\"", vtouse->storeName);

  /* save repository, for safety reasons */
  if (AQBServer__SaveRepository(s, r)) {
    DBG_ERROR("Error saving repository");
    return 0;
  }

  /* save this modified entry */
  if (AQBServer__WriteEntry(s, d, etouse)) {
    DBG_ERROR("Could not save entry");
    return 0;
  }

  /* try to create this path */
  p=strrchr(storebuffer, '/');
  /* we should have a p here, meaning at least one "/" must be in there */
  assert(p);
  /* temporarily cut off the file name */
  *p=0;
  if (Path_Create(r->baseDir, storebuffer)) {
    DBG_ERROR("Could not create storage path");
    return 0;
  }
  DBG_DEBUG("Storage path \"%s\" created", fullname);
  /* restore full name again */
  *p='/';

  /* now create a AQB_FILE for this */
  f=AQBFile_new();
  f->path=strdup(fullname);
  f->did=did;
  f->fileId=etouse->id;
  f->fileRevision=vtouse->revision;
  f->file=fopen(f->path, "wb");
  if (!f->file) {
    DBG_ERROR("Error on fopen(%s): %s",
	      f->path, strerror(errno));
    AQBFile_free(f);
    return 0;
  }
  DBG_DEBUG("File \"%s\" open", f->path);
  f->id=++(r->nextFileRuntimeId);

  /* add file to directory */
  AQBFile_add(f, &(r->files));

  /* that's it */
  return f->id;
}



int AQBServer_FileWrite(AQB_SERVER *s,
			int cid,
			int fid,
			const char *data,
			unsigned int size) {
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_FILE *f;
  int rv;

  assert(s);

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 1;
  }

  /* find file by id */
  f=r->files;
  while(f) {
    if (f->id==fid)
      break;
    f=f->next;
  } /* while folder */
  if (!f) {
    DBG_ERROR("No file by id \"%d\"", fid);
    return 1;
  }

  rv=fwrite(data, size, 1, f->file);
  if (rv==0) {
    DBG_ERROR("Error on fwrite(%s): %s",
	      f->path, strerror(errno));
    return 1;
  }
  else if (rv<0) {
    DBG_ERROR("Error on fwrite(%s): %s",
	      f->path, strerror(errno));
    return 1;
  }

  f->bytesWritten+=size;
  DBG_DEBUG("Written %d bytes (total %d)", size, f->bytesWritten);

  return 0;
}



int AQBServer_FileCloseOut(AQB_SERVER *s,
			   int cid,
			   int fid,
			   const char *md5) {
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_FILE *f;
  AQB_DIRECTORY *d;
  AQB_ENTRY *e;

  assert(s);

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 1;
  }

  /* find file by id */
  f=r->files;
  while(f) {
    if (f->id==fid)
      break;
    f=f->next;
  } /* while folder */
  if (!f) {
    DBG_ERROR("No file by id \"%d\"", fid);
    return 1;
  }

  /* close file */
  if (fclose(f->file)) {
    DBG_ERROR("Error on fclose(%s): %s",
	      f->path, strerror(errno));
    return 1;
  }

  DBG_INFO("Written %d bytes to file \"%s\"",
	   f->bytesWritten, f->path);

  if (md5) {
    if (strlen(md5)) {
      /* find directory by id */
      d=r->folders;
      while(d) {
	if (d->id==f->did)
	  break;
	d=d->next;
      } /* while folder */
      if (!d) {
	DBG_ERROR("No dir by id \"%d\"", f->did);
	AQBFile_del(f, &(r->files));
	AQBFile_free(f);
	return 1;
      }

      /* find entry and revision */
      e=d->entries;
      while(e) {
	if (e->id==f->fileId) {
	  AQB_ENTRY_VERSION *v;

	  v=e->versions;
	  while (v) {
	    if (v->revision==f->fileRevision)
	      break;
            v=v->next;
	  } /* while */
	  if (!v) {
	    e=0;
	    break;
	  }
	  /* store new MD5 digest */
	  free(v->md5);
	  v->md5=strdup(md5);
	  e->modified++;
	  break;
	}
	e=e->next;
      } /* while */
      if (!e) {
	DBG_ERROR("Entry not found, this should never happen...");
	return 1;
      }
    }
  } /* if md5 */
  else {
    DBG_WARN("No md5 given !");
    getchar();
  }
  /* free file entry */
  AQBFile_del(f, &(r->files));
  AQBFile_free(f);

  return 0;
}



int AQBServer_FileOpenIn(AQB_SERVER *s,
			 int cid,
			 int did,
			 const char *name,
			 time_t when,
			 AQB_ENTRY **entry){
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_DIRECTORY *d;
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v;
  AQB_FILE *f;
  char storebuffer[300];
  char fullname[300];

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return 0;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return 0;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return 0;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return 0;
  }

  /* find directory by id */
  d=r->folders;
  while(d) {
    if (d->id==did)
      break;
    d=d->next;
  } /* while folder */
  if (!d) {
    DBG_ERROR("No dir by id \"%d\"", did);
    return 0;
  }

  /* check whether we have an entry for this file */
  e=AQBServer__GetEntry(s, d, name);
  if (!e) {
    DBG_ERROR("No entry for \"%s\"", name);
    return 1;
  }

  /* find appropriate version */
  e=AQBEntry_dup_date(e, when);
  if (!e) {
    DBG_ERROR("No version found");
    return 1;
  }
  v=e->versions;
  assert(v);

  /* create storeName */
  assert(v->storeName);
  if (strlen(v->storeName)>=sizeof(storebuffer)) {
    DBG_ERROR("Buffer too small");
    AQBEntry_free(e);
    return 0;
  }
  strcpy(storebuffer, v->storeName);

  if ((strlen(r->baseDir)+
       strlen(storebuffer)+10)>sizeof(fullname)) {
    DBG_ERROR("Buffer too small");
    AQBEntry_free(e);
    return 0;
  }
  strcpy(fullname, r->baseDir);
  strcat(fullname, "/");
  strcat(fullname, storebuffer);

  /* now create a AQB_FILE for this */
  f=AQBFile_new();
  f->path=strdup(fullname);
  f->did=did;
  f->fileId=e->id;
  f->fileRevision=v->revision;
  f->file=fopen(f->path, "rb");
  if (!f->file) {
    DBG_ERROR("Error on fopen(%s): %s",
	      f->path, strerror(errno));
    AQBFile_free(f);
    AQBEntry_free(e);
    return 0;
  }
  DBG_DEBUG("File \"%s\" open", f->path);
  f->id=++(r->nextFileRuntimeId);

  /* add file to directory */
  AQBFile_add(f, &(r->files));

  /* that's it */
  *entry=e;
  return f->id;
}



int AQBServer_FileRead(AQB_SERVER *s,
		       int cid,
		       int fid,
		       char *data,
		       unsigned int size) {
  AQB_REPOSITORY *r;
  AQB_SERVER_CLIENT *c;
  AQB_FILE *f;
  int rv;

  assert(s);

  /* find client by id */
  c=s->clients;
  while(c) {
    if (c->id==cid)
      break;
    c=c->next;
  } /* while client */
  if (!c) {
    DBG_ERROR("No client by id \"%d\"", cid);
    return -1;
  }

  if (c->authLevel<s->minimumAuthLevel) {
    DBG_ERROR("Client \"%s\" is not authenticated", c->name);
    return -1;
  }

  if (c->repositoryId==0) {
    DBG_ERROR("No repository open for client %d", cid);
    return -1;
  }

  /* find repository */
  r=s->repositories;
  while(r) {
    if (r->id==c->repositoryId)
      break;
    r=r->next;
  } /* while repositories */
  if (!r) {
    DBG_ERROR("Repository \"%d\" not found", c->repositoryId);
    return -1;
  }

  /* find file by id */
  f=r->files;
  while(f) {
    if (f->id==fid)
      break;
    f=f->next;
  } /* while folder */
  if (!f) {
    DBG_ERROR("No file by id \"%d\"", fid);
    return -1;
  }

  rv=fread(data, 1, size, f->file);
  if (rv==0) {
    DBG_INFO("EOF met");
    return 0;
  }
  else if (rv<0) {
    DBG_ERROR("Error on fread(%s): %s",
	      f->path, strerror(errno));
    return -1;
  }

  f->bytesRead+=rv;
  DBG_DEBUG("Read %d bytes (total %d)", size, f->bytesRead);

  return rv;
}



int AQBServer_FileCloseIn(AQB_SERVER *s,
			  int cid,
			  int fid) {
  return AQBServer__FileClose(s, cid, fid);
}







