/***************************************************************************
 $RCSfile: entry.c,v $
                             -------------------
    cvs         : $Id: entry.c,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
    begin       : Sun May 25 2003
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

#include "entry.h"
#include "misc.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>



AQB_ENTRY_VERSION *AQBEntryVersion_new() {
  AQB_ENTRY_VERSION *e;

  e=(AQB_ENTRY_VERSION *)malloc(sizeof(AQB_ENTRY_VERSION));
  assert(e);
  memset(e, 0, sizeof(AQB_ENTRY_VERSION));
  return e;
}



void AQBEntryVersion_free(AQB_ENTRY_VERSION *e){
  if (e) {
    free(e->md5);
    free(e->storeName);
    free(e->owner);
    free(e->group);
    free(e->dstname);
    free(e);
  }
}


AQB_ENTRY_VERSION *AQBEntryVersion_dup(AQB_ENTRY_VERSION *o){
  AQB_ENTRY_VERSION *v;

  assert(o);
  v=AQBEntryVersion_new();
  v->status=o->status;
  v->revision=o->revision;
  if (o->md5)
    v->md5=strdup(o->md5);
  if (o->storeName)
    v->storeName=strdup(o->storeName);
  v->flags=o->flags;
  v->mode=o->mode;
  if (o->owner)
    v->owner=strdup(o->owner);
  if (o->group)
    v->group=strdup(o->owner);
  if (o->dstname)
    v->dstname=strdup(o->dstname);
  v->device=o->device;
  v->size=o->size;
  v->mtime=o->mtime;
  v->ctime=o->ctime;
  v->btime=o->btime;

  return v;
}



void AQBEntryVersion_add(AQB_ENTRY_VERSION *e, AQB_ENTRY_VERSION **head){
  AQLIST_ADD(AQB_ENTRY_VERSION, e, head);
}



void AQBEntryVersion_del(AQB_ENTRY_VERSION *e, AQB_ENTRY_VERSION **head){
  AQLIST_DEL(AQB_ENTRY_VERSION, e, head);
}



AQB_ENTRY_VERSION *AQBEntryVersion_FromConfig(CONFIGGROUP *group){
  AQB_ENTRY_VERSION *e;

  e=AQBEntryVersion_new();
  e->status=Config_GetIntValue(group, "status", 0, 0);
  e->revision=Config_GetIntValue(group, "revision", 0, 0);
  e->md5=strdup(Config_GetValue(group, "md5", "", 0));
  e->storeName=strdup(Config_GetValue(group, "storename", "", 0));
  e->flags=Config_GetIntValue(group, "flags", 0, 0);
  e->mode=Config_GetIntValue(group, "mode", 0, 0);
  e->owner=strdup(Config_GetValue(group,"owner", "", 0));
  e->group=strdup(Config_GetValue(group, "group", "", 0));
  e->dstname=strdup(Config_GetValue(group, "dstname", "", 0));
  e->device=Config_GetIntValue(group, "device", 0, 0);
  e->size=Config_GetIntValue(group, "size", 0, 0);
  e->mtime=Config_GetTimeValue(group, "mtime", 0, 0);
  e->ctime=Config_GetTimeValue(group, "ctime", 0, 0);
  e->btime=Config_GetTimeValue(group, "btime", 0, 0);
  return e;
}



int AQBEntryVersion_ToConfig(AQB_ENTRY_VERSION *e, CONFIGGROUP *group){
  if (Config_SetIntValue(group,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "status",
			 e->status))
    return 1;

  if (Config_SetIntValue(group,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "revision",
			 e->revision))
    return 1;

  if (Config_SetValue(group,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "md5",
		      e->md5))
    return 1;

  if (Config_SetValue(group,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "storename",
		      e->storeName))
    return 1;

  if (Config_SetIntValue(group,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "flags",
			 e->flags))
    return 1;

  if (Config_SetIntValue(group,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "mode",
			 e->mode))
    return 1;

  if (Config_SetValue(group,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "owner",
		      e->owner))
    return 1;

  if (Config_SetValue(group,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "group",
		      e->group))
    return 1;

  if (Config_SetValue(group,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "dstname",
		      e->dstname))
    return 1;

  if (Config_SetIntValue(group,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "device",
			 e->device))
    return 1;

  if (Config_SetIntValue(group,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "size",
			 e->size))
    return 1;

  if (Config_SetTimeValue(group,
			  CONFIGMODE_VARIABLE |
			  CONFIGMODE_NAMECREATE_VARIABLE |
			  CONFIGMODE_OVERWRITE_VARS,
			  "mtime",
			  e->mtime))
    return 1;

  if (Config_SetTimeValue(group,
			  CONFIGMODE_VARIABLE |
			  CONFIGMODE_NAMECREATE_VARIABLE |
			  CONFIGMODE_OVERWRITE_VARS,
			  "ctime",
			  e->ctime))
    return 1;

  if (Config_SetTimeValue(group,
			  CONFIGMODE_VARIABLE |
			  CONFIGMODE_NAMECREATE_VARIABLE |
			  CONFIGMODE_OVERWRITE_VARS,
			  "btime",
			  e->btime))
    return 1;

  return 0;
}



AQB_ENTRY_VERSION *AQBEntryVersion_FromMessage(IPCMESSAGE *m){
  ERRORCODE err;
  char *p;
  int i;
  AQB_ENTRY_VERSION *v;

  v=AQBEntryVersion_new();

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->status=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->revision=i;

  err=IPCMessage_NextStringParameter(m, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->md5=strdup(p);

  err=IPCMessage_NextStringParameter(m, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->storeName=strdup(p);

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->flags=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->mode=i;

  err=IPCMessage_NextStringParameter(m, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->owner=strdup(p);

  err=IPCMessage_NextStringParameter(m, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->group=strdup(p);

  err=IPCMessage_NextStringParameter(m, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->dstname=strdup(p);

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->device=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->size=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->mtime=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->ctime=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntryVersion_free(v);
    return 0;
  }
  v->btime=i;

  return v;
}



int AQBEntryVersion_ToMessage(AQB_ENTRY_VERSION *e, IPCMESSAGE *m){
  ERRORCODE err;

  err=IPCMessage_AddIntParameter(m, e->status);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->revision);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddStringParameter(m, e->md5);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddStringParameter(m, e->storeName);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->flags);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->mode);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddStringParameter(m, e->owner);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddStringParameter(m, e->group);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddStringParameter(m, e->dstname);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->device);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->size);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->mtime);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->ctime);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->btime);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  return 0;
}



AQB_ENTRY_VERSION *AQBEntryVersion_FromStat(const struct stat *s) {
  AQB_ENTRY_VERSION *v;
  struct passwd *pw;
  struct group *gr;

  v=AQBEntryVersion_new();
  v->status=EntryStatusUsed;
  v->mode=s->st_mode;

  pw=getpwuid(s->st_uid);
  if (pw) {
    v->owner=strdup(pw->pw_name);
  }

  gr=getgrgid(s->st_gid);
  if (gr) {
    v->group=strdup(gr->gr_name);
  }

  if (S_ISBLK(s->st_mode) || S_ISCHR(s->st_mode))
    v->device=s->st_rdev;
  v->size=s->st_size;
  v->mtime=s->st_mtime;
  v->ctime=s->st_ctime;

  return v;
}



void AQBEntryVersion_dump(FILE *f, AQB_ENTRY_VERSION *v) {
  assert(v);
  assert(f);

  fprintf(f, " Revision    : %u\n", v->revision);
  fprintf(f, " Status      : ");
  switch (v->status) {
  case EntryStatusUsed:
    fprintf(f, "used\n");
    break;
  case EntryStatusDeleted:
    fprintf(f, "deleted\n");
    break;
  default:
    fprintf(f, "<unknown>\n");
  } /* switch */

  fprintf(f, " Checksum    : ");
  if (v->md5)
    fprintf(f, "%s\n", v->md5);
  else
    fprintf(f, "<none>\n");

  fprintf(f, " Storename   : ");
  if (v->storeName)
    fprintf(f, "%s\n", v->storeName);
  else
    fprintf(f, "<none>\n");

  fprintf(f, " Mode        : %o\n",
	  v->mode);

  fprintf(f, " Owner       : ");
  if (v->owner)
    fprintf(f, "%s\n", v->owner);
  else
    fprintf(f, "<none>\n");

  fprintf(f, " Group       : ");
  if (v->group)
    fprintf(f, "%s\n", v->group);
  else
    fprintf(f, "<none>\n");

  fprintf(f, " Link To     : ");
  if (v->dstname)
    fprintf(f, "%s\n", v->dstname);
  else
    fprintf(f, "<no link>\n");

  fprintf(f, " Device      : %u:%u (%x)\n",
	  (unsigned char)((v->device>>8)&0xff),
	  (unsigned char)(v->device&0xff),
	  (unsigned int)(v->device));

  fprintf(f, " Size        : %u\n",
	  (unsigned int)(v->size));

  fprintf(f, " mtime       : %s\n",
	  ctime(&(v->mtime)));

  fprintf(f, " ctime       : %s\n",
	  ctime(&(v->ctime)));

  fprintf(f, " Backup time : %s\n",
	  ctime(&(v->btime)));

}



/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                              AQBEntry
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_ENTRY *AQBEntry_new() {
  AQB_ENTRY *e;

  e=(AQB_ENTRY *)malloc(sizeof(AQB_ENTRY));
  assert(e);
  memset(e, 0, sizeof(AQB_ENTRY));
  return e;
}



void AQBEntry_free(AQB_ENTRY *e){
  if (e) {
    AQB_ENTRY_VERSION *v;

    /* free all version entries */
    v=e->versions;
    while(v) {
      AQB_ENTRY_VERSION *next;

      next=v->next;
      AQBEntryVersion_free(v);
      v=next;
    } /* while */

    free(e->name);
    free(e->abspath);
    free(e);
  }
}


void AQBEntry_freeAll(AQB_ENTRY *e){
  while(e) {
    AQB_ENTRY *next;

    next=e->next;
    free(e);
    e=next;
  } /* while */
}



AQB_ENTRY *AQBEntry_dup(AQB_ENTRY *o){
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v, *vnew;

  assert(o);
  e=AQBEntry_new();
  if (o->name)
    e->name=strdup(o->name);
  if (o->abspath)
    e->abspath=strdup(o->abspath);
  e->nextRevision=o->nextRevision;

  e->id=o->id;

  v=o->versions;
  while(v) {
    vnew=AQBEntryVersion_dup(v);
    AQBEntryVersion_add(vnew, &(e->versions));
    v=v->next;
  } /* while */

  return e;
}



AQB_ENTRY *AQBEntry_dup_date(AQB_ENTRY *o, time_t when){
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v, *vnew, *latest;

  assert(o);
  assert(o->versions);
  v=o->versions;
  /* get latest entry after "when" */
  latest=0;
  while(v) {
    /* if a time is given, then only entries which are before the given
     * date are selected. If you want the most recent version you have to
     * specify 0 as the time.
     */
    if (difftime(when, v->btime)>0 || when==0) {
      /* current is a candidate, check if it is the latest */
      if (latest) {
	if (difftime(v->btime, latest->btime)>=0)
	  latest=v;
      }
      else
	latest=v;
    }
    v=v->next;
  } /* while */

  if (latest) {
    DBG_INFO("Returning revision %d of %s",
	     latest->revision,
	     o->name);
    e=AQBEntry_new();
    if (o->name)
      e->name=strdup(o->name);
    if (o->abspath)
      e->abspath=strdup(o->abspath);
    e->nextRevision=o->nextRevision;

    e->id=o->id;
    vnew=AQBEntryVersion_dup(latest);
    AQBEntryVersion_add(vnew, &(e->versions));
    return e;
  }
  else {
    DBG_INFO("Returning no revision of %s", o->name);
  }
  DBG_INFO("No version found");
  return 0;
}



void AQBEntry_add(AQB_ENTRY *e, AQB_ENTRY **head){
  AQLIST_ADD(AQB_ENTRY, e, head);
}



void AQBEntry_del(AQB_ENTRY *e, AQB_ENTRY **head){
  AQLIST_DEL(AQB_ENTRY, e, head);
}



int AQBEntry_ReadFile(AQB_ENTRY **ep, const char *name){
  CONFIGGROUP *group;
  CONFIGGROUP *currg;
  AQB_ENTRY *e;

  group=Config_new();
  if (Config_ReadFile(group,
		      name,
		      CONFIGMODE_REMOVE_STARTING_BLANKS |
		      CONFIGMODE_REMOVE_TRAILING_BLANKS |
		      CONFIGMODE_REMOVE_QUOTES          |
		      CONFIGMODE_ALLOW_GROUPS           |
		      CONFIGMODE_OVERWRITE_VARS         |
		      CONFIGMODE_NAMECREATE_GROUP)) {
    DBG_ERROR("Could not read file \"%s\"", name);
    Config_free(group);
    return 1;
  }

  /* read entry (including versions) */
  currg=group->groups;
  if (currg) {
    CONFIGGROUP *currv;

    e=AQBEntry_new();
    assert(currg->name);
    e->name=strdup(currg->name);
    e->id=Config_GetIntValue(currg, "fileid", 0, 0);
    e->nextRevision=Config_GetIntValue(currg, "nextrevision", 0, 0);

    /* read and add versions */
    currv=currg->groups;
    while(currv) {
      AQB_ENTRY_VERSION *v;

      v=AQBEntryVersion_FromConfig(currv);
      assert(v);
      AQBEntryVersion_add(v, &(e->versions));
      currv=currv->next;
    } /* while version */
    *ep=e;
  } /* if groups */
  else {
    *ep=0;
  }

  return 0;
}



int AQBEntry_WriteFile(AQB_ENTRY *e, const char *name){
  CONFIGGROUP *group;
  CONFIGGROUP *currg;
  CONFIGGROUP *currv;
  AQB_ENTRY_VERSION *v;
  int rv;

  group=Config_new();
  currg=Config_GetGroup(group, e->name, CONFIGMODE_NAMECREATE_GROUP);
  assert(currg);

  if (Config_SetIntValue(currg,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "nextrevision",
			 e->nextRevision)) {
    Config_free(group);
    return 1;
  }

  if (Config_SetIntValue(currg,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "fileid",
			 e->id)) {
    Config_free(group);
    return 1;
  }

  v=e->versions;
  while(v) {
    currv=Config_GetGroup(currg, "version", CONFIGMODE_NAMECREATE_GROUP);
    rv=AQBEntryVersion_ToConfig(v, currv);
    assert(rv==0);
    v=v->next;
  } /* while versions */

  rv=Config_WriteFile(group,
		      name,
		      CONFIGMODE_REMOVE_QUOTES |
		      CONFIGMODE_REMOVE_STARTING_BLANKS |
		      CONFIGMODE_REMOVE_TRAILING_BLANKS |
		      CONFIGMODE_ALLOW_GROUPS);
  Config_free(group);
  return rv;
}



AQB_ENTRY *AQBEntry_FromMessage(IPCMESSAGE *m){
  ERRORCODE err;
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v;
  char *p;
  int i;

  e=AQBEntry_new();

  err=IPCMessage_NextStringParameter(m, &p);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntry_free(e);
    return 0;
  }
  e->name=strdup(p);

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntry_free(e);
    return 0;
  }
  e->nextRevision=i;

  err=IPCMessage_NextIntParameter(m, &i);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    AQBEntry_free(e);
    return 0;
  }
  e->id=i;

  v=AQBEntryVersion_FromMessage(m);
  if (!v) {
    AQBEntry_free(e);
    return 0;
  }
  AQBEntryVersion_add(v, &(e->versions));
  return e;
}



int AQBEntry_ToMessage(AQB_ENTRY *e, AQB_ENTRY_VERSION *v, IPCMESSAGE *m){
  ERRORCODE err;

  assert(e);
  assert(v);
  assert(m);

  err=IPCMessage_AddStringParameter(m, e->name);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->nextRevision);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  err=IPCMessage_AddIntParameter(m, e->id);
  if (!Error_IsOk(err)) {
    DBG_ERROR_ERR(err);
    return 1;
  }

  return AQBEntryVersion_ToMessage(v, m);
}


AQB_ENTRY *AQBEntry_FromStat(const char *name,
			     const struct stat *s) {
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v;

  e=AQBEntry_new();
  e->name=strdup(name);
  v=AQBEntryVersion_FromStat(s);
  AQBEntryVersion_add(v, &(e->versions));

  return e;
}


void AQBEntry_dump(FILE *f, AQB_ENTRY *e) {
  AQB_ENTRY_VERSION *v;

  assert(e);
  assert(f);

  fprintf(f, "Name         : ");
  if (e->name)
    fprintf(f, "%s\n", e->name);
  else
    fprintf(f, "<none>\n");

  fprintf(f, "Id           : %u\n", e->id);
  fprintf(f, "NextRevision : %u\n", e->nextRevision);

  v=e->versions;
  while(v) {
    fprintf(f, "Version\n");
    AQBEntryVersion_dump(f, v);
    v=v->next;
  } /* while */
}



