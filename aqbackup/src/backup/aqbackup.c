/***************************************************************************
 $RCSfile: aqbackup.c,v $
                             -------------------
    cvs         : $Id: aqbackup.c,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
    begin       : Tue Jun 03 2003
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


#include "aqbackup_p.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>
#include <chameleon/directory.h>
#include <chameleon/stringlist.h>
#include <converter/converter_md5.h>
#include <converter/converter_zip.h>
#include <converter/converter_unzip.h>
#include <converter/converter_filein.h>
#include <converter/converter_fileout.h>
#include <converter/converter_dummy.h>
#include "misc.h"
#include "client_direct.h"
#include "interactor.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <utime.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                          AQBackupRepository
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQB_BACKUP_REPOSITORY *AQBackupRepository_new() {
  AQB_BACKUP_REPOSITORY *r;

  r=(AQB_BACKUP_REPOSITORY *)malloc(sizeof(AQB_BACKUP_REPOSITORY));
  assert(r);
  memset(r, 0, sizeof(AQB_BACKUP_REPOSITORY));
  r->excludes=StringList_new();
  r->ignores=StringList_new();
  r->nozips=StringList_new();
  return r;
}



void AQBackupRepository_free(AQB_BACKUP_REPOSITORY *r){
  if (r) {
    free(r->baseDir);
    free(r->user);
    free(r->passwd);
    StringList_free(r->excludes);
    StringList_free(r->ignores);
    StringList_free(r->nozips);
    Config_free(r->config);
    free(r);
  }
}



void AQBackupRepository_add(AQB_BACKUP_REPOSITORY *r, AQB_BACKUP_REPOSITORY **head){
  AQLIST_ADD(AQB_BACKUP_REPOSITORY, r, head);
}



void AQBackupRepository_del(AQB_BACKUP_REPOSITORY *r, AQB_BACKUP_REPOSITORY **head){
  AQLIST_DEL(AQB_BACKUP_REPOSITORY, r, head);
}


AQB_BACKUP_REPOSITORY *AQBackupRepository_fromConfig(CONFIGGROUP *cfg) {
  AQB_BACKUP_REPOSITORY *r;
  int i;
  const char *p;
  CONFIGGROUP *group;

  r=AQBackupRepository_new();

  /* copy client configuration (or create empty one) */
  group=Config_GetGroup(cfg, "client", CONFIGMODE_PATHMUSTEXIST);

  r->config=Config_new();
  if (group)
    Config_ImportTreeChildren(r->config, group);

  p=Config_GetValue(cfg, "basedir", 0,0);
  if (!p) {
    DBG_ERROR("No base dir in repository");
    AQBackupRepository_free(r);
    return 0;
  }
  r->baseDir=strdup(p);

  p=Config_GetValue(cfg, "name", 0,0);
  if (!p) {
    DBG_ERROR("No name in repository");
    AQBackupRepository_free(r);
    return 0;
  }
  r->name=strdup(p);

  p=Config_GetValue(cfg, "user", 0,0);
  if (!p) {
    DBG_ERROR("No user in repository");
    AQBackupRepository_free(r);
    return 0;
  }
  r->user=strdup(p);

  p=Config_GetValue(cfg, "passwd", 0,0);
  if (!p) {
    DBG_WARN("No passwd in repository");
  }
  else {
    r->passwd=strdup(p);
  }

  r->zipLevel=Config_GetIntValue(cfg, "ziplevel",
				 AQBACKUP_DEFAULT_ZIPLEVEL,
				 0);

  p=Config_GetValue(cfg, "type", "direct", 0);
  if (strcasecmp(p, "direct")==0) {
    r->typ=RepositoryTypeDirect;
  }
  else if (strcasecmp(p, "local")==0) {
    r->typ=RepositoryTypeLocal;
  }
  else if (strcasecmp(p, "net")==0) {
    r->typ=RepositoryTypeNet;
  }
  else {
    DBG_ERROR("Unknown repository type \"%s\"", p);
    AQBackupRepository_free(r);
    return 0;
  }

  /* read excludes */
  i=0;
  while((p=Config_GetValue(cfg, "exclude",0,i++))) {
    StringList_AppendEntry(r->excludes, StringListEntry_new(p, 0));
  }

  /* read ignores */
  i=0;
  while((p=Config_GetValue(cfg, "ignore",0,i++))) {
    StringList_AppendEntry(r->ignores, StringListEntry_new(p, 0));
  }

  /* read nozips */
  i=0;
  while((p=Config_GetValue(cfg, "nozip",0,i++))) {
    StringList_AppendEntry(r->nozips, StringListEntry_new(p, 0));
  }

  return r;
}



int AQBackupRepository_toConfig(AQB_BACKUP_REPOSITORY *r,
				CONFIGGROUP *cfg){
  STRINGLISTENTRY *se;
  const char *p;
  CONFIGGROUP *group;

  assert(r);

  if (Config_SetValue(cfg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "basedir",
		      r->baseDir)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  if (Config_SetValue(cfg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "name",
		      r->name)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  if (Config_SetValue(cfg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "user",
		      r->user)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  if (Config_SetValue(cfg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "passwd",
		      r->passwd)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  if (Config_SetIntValue(cfg,
			 CONFIGMODE_VARIABLE |
			 CONFIGMODE_NAMECREATE_VARIABLE |
			 CONFIGMODE_OVERWRITE_VARS,
			 "ziplevel",
			 r->zipLevel)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  /* set type specific data */
  switch(r->typ) {
  case RepositoryTypeDirect: p="direct"; break;
  case RepositoryTypeLocal:  p="local"; break;
  case RepositoryTypeNet:    p="net"; break;
  default:
    DBG_ERROR("Unknown repository type %d", r->typ);
    break;
  } /* switch */

  if (Config_SetValue(cfg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "type",
		      p)) {
    DBG_ERROR("Could not set value");
    Config_free(cfg);
    return 1;
  }

  /* save excludes */
  se=r->excludes->first;
  while(se) {
    if (Config_SetValue(cfg,
			CONFIGMODE_VARIABLE,
			"exclude",
			se->data)) {
      DBG_ERROR("Could not set value");
      Config_free(cfg);
      return 1;
    }
    se=se->next;
  }

  /* save ignores */
  se=r->ignores->first;
  while(se) {
    if (Config_SetValue(cfg,
			CONFIGMODE_VARIABLE,
			"ignore",
			se->data)) {
      DBG_ERROR("Could not set value");
      Config_free(cfg);
      return 1;
    }
    se=se->next;
  }

  /* save nozips */
  se=r->ignores->first;
  while(se) {
    if (Config_SetValue(cfg,
			CONFIGMODE_VARIABLE,
			"nozip",
			se->data)) {
      DBG_ERROR("Could not set value");
      Config_free(cfg);
      return 1;
    }
    se=se->next;
  }

  /* copy client configuration */
  group=Config_GetGroup(cfg, "client", CONFIGMODE_OVERWRITE_GROUPS);
  if (!group) {
    DBG_ERROR("Could not create group");
    Config_free(cfg);
    return 1;
  }
  Config_ImportTreeChildren(group, r->config);

  return 0;
}




/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                          AQBackup
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




AQBACKUP *AQBackup_new() {
  AQBACKUP *b;

  b=(AQBACKUP *)malloc(sizeof(AQBACKUP));
  assert(b);
  memset(b, 0, sizeof(AQBACKUP));
  return b;
}



void AQBackup_free(AQBACKUP *b){
  if (b) {
    AQB_BACKUP_REPOSITORY *r;

    /* free repositories */
    r=b->repositories;
    while(r) {
      AQB_BACKUP_REPOSITORY *rn;

      rn=r->next;
      AQBackupRepository_free(r);
      r=rn;
    } /* while repository */

    AQBClient_free(b->currClient);
    AQBInteractor_free(b->interactor);
    free(b->hostName);

    free(b);
  }
}



int AQBackup_Init(AQBACKUP *b, CONFIGGROUP *cfg) {
  CONFIGGROUP *group, *group2;
  const char *p;

  assert(b);
  assert(cfg);

  /* get hostname */
  p=Config_GetValue(cfg,
		    "hostname",
		    0,
		    0);
  if (!p) {
    char hnbuffer[300];

    /* get hostname from system */
    if (gethostname(hnbuffer, sizeof(hnbuffer))) {
      DBG_ERROR("Error on gethostname(): %s",
		strerror(errno));
      return 1;
    }
    b->hostName=strdup(hnbuffer);
  }
  else {
    b->hostName=strdup(p);
  }

  /* read repositories */
  group=Config_GetGroup(cfg,
			"repositories",
			CONFIGMODE_NAMEMUSTEXIST);
  if (!group) {
    DBG_NOTICE("No repositories in configuration");
  }
  else {
    group2=group->groups;
    while(group2) {
      CONFIGGROUP *currgroup;
      AQB_BACKUP_REPOSITORY *r;

      currgroup=group2;
      group2=group2->next;

      r=AQBackupRepository_fromConfig(currgroup);
      if (!r) {
	DBG_ERROR("Error reading repository");
        return 1;
      }
      AQBackupRepository_add(r, &(b->repositories));
      DBG_INFO("Repository \"%s\" added", r->name);
    } /* while */
  }

  return 0;
}



int AQBackup_Fini(AQBACKUP *b, CONFIGGROUP *cfg){
  CONFIGGROUP *group, *group2;
  unsigned int nextNum;
  AQB_BACKUP_REPOSITORY *r;

  assert(b);
  assert(cfg);

  nextNum=0;
  if (Config_SetValue(cfg,
		      CONFIGMODE_VARIABLE |
		      CONFIGMODE_NAMECREATE_VARIABLE |
		      CONFIGMODE_OVERWRITE_VARS,
		      "hostname",
		      b->hostName)) {
    DBG_ERROR("Could not set value");
    return 1;
  }

  group=Config_GetGroup(cfg,
			"repositories",
			0);
  if (!group) {
    DBG_ERROR("Could not create group");
    return 1;
  }

  r=b->repositories;
  while(r) {
    char numbuff[32];

    sprintf(numbuff, "%d", ++nextNum);
    group2=Config_GetGroup(group,
			   numbuff,
			   0);
    if (!group2) {
      DBG_ERROR("Could not create group");
      return 1;
    }
    if (AQBackupRepository_toConfig(r, group2)) {
      DBG_ERROR("Error storing configuration of repository");
      return 1;
    }
    r=r->next;
  } /* while */


  return 0;
}


int AQBackup__GetMD5(const char *filename,
		     char *buffer,
		     unsigned int maxsize) {
  CONVERTER *dm;
  FILE *fin;
  int ineof, outeof;
  int bytesin;
  int bytesout;
  char outbuffer[300];
  char inbuffer[1024];
  int rv;

  dm=ConverterMd5_new();
  fin=fopen(filename, "r");
  if (!fin) {
    DBG_ERROR("Could not open infile (%s)\n",
	      strerror(errno));
    Converter_free(dm);
    return 1;
  }

  rv=Converter_Begin(dm);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    Converter_free(dm);
    return 1;
  }

  ineof=0;
  outeof=0;
  while (!outeof) {
    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=Converter_NeedsData(dm);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);
	rv=fread(inbuffer, 1, bytesin, fin);
	if (rv==0) {
	  ineof=1;
	  rv=Converter_SetData(dm, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read from infile (%s)\n",
		    strerror(errno));
	  Converter_free(dm);
	  return 1;
	}
	else {
	  DBG_INFO("Setting data");
	  rv=Converter_SetData(dm, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  Converter_free(dm);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=Converter_Work(dm);
    if (ineof && rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	Converter_free(dm);
	return 1;
      }
    }

    /* check if there already is data */
    bytesout=Converter_HasData(dm);
    if (bytesout) {
      if (bytesout>sizeof(outbuffer)) {
	DBG_ERROR("Buffer too small (needs %d bytes)", bytesout);
	Converter_free(dm);
	return 1;
      }
      DBG_INFO("Getting data");
      rv=Converter_GetData(dm, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
	Converter_free(dm);
	return 1;
      }
      if (!Text_ToHex(outbuffer, rv, buffer, maxsize)) {
	Converter_free(dm);
	return 1;
      }
    }
  } /* while */

  fclose(fin);

  return 0;
}




AQB_ENTRY *AQBackup_File2Entry(const char *filename){
  struct stat st;
  const char *p;
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v;
  char buffer[300];

  DBG_DEBUG("File2Entry \"%s\"", filename);
  if (stat(filename, &st)) {
    DBG_ERROR("Error on stat(%s): %s",
	      filename, strerror(errno));
    return 0;
  }

  /* if link, then stat the link itself rather than the destination file */
  if (!S_ISLNK(st.st_mode)) {
    if (lstat(filename, &st)) {
      DBG_ERROR("Error on lstat(%s): %s",
		filename, strerror(errno));
      return 0;
    }
  }

  p=strrchr(filename, '/');
  if (!p)
    p=filename;
  else {
    p++;
  }

  /* escape filename */
  p=Text_Escape(p, buffer, sizeof(buffer));
  if (!p) {
    DBG_ERROR("Error escaping filename");
    return 0;
  }
  e=AQBEntry_FromStat(p, &st);
  if (!e) {
    DBG_ERROR("Could not create entry");
    return 0;
  }
  free(e->abspath);
  e->abspath=strdup(filename);

  /* check whether this is a link */
  v=e->versions;
  assert(v);
  if (S_ISLNK(st.st_mode)) {
    /* ok, this is a link, so try to get the target */
    int rv;
    char lnkbuffer[300];

    rv=readlink(filename, lnkbuffer, sizeof(lnkbuffer)-1);
    if (rv<0) {
      DBG_ERROR("Error on readlink(%s): %s",
		filename, strerror(errno));
      AQBEntry_free(e);
      return 0;
    }
    lnkbuffer[rv]=0;
    v->dstname=strdup(lnkbuffer);
  }

  return e;
}



int AQBackup_Dir2Entries(AQB_ENTRY **head, const char *dirname,
			 int nonexistentisok){
  DIRECTORYDATA *dd;
  int rv;
  char buffer[300];
  char fbuffer[300];

  DBG_DEBUG("Checking dir %s", dirname);
  dd=Directory_new();
  rv=Directory_Open(dd, dirname);
  if (rv) {
    DBG_ERROR("Could not open dir \"%s\"", dirname);
    Directory_free(dd);
    if (nonexistentisok)
      return 0;
    return 1;
  }

  *head=0;
  while(Directory_Read(dd, buffer, sizeof(buffer))==0) {
    AQB_ENTRY *e;

    DBG_DEBUG("Checking entry %s", buffer);
    /* skip special entries */
    if (strlen(buffer)==1) {
      if (strcmp(buffer, ".")==0)
	continue;
    }
    if (strlen(buffer)==2) {
      if (strcmp(buffer, "..")==0)
	continue;
    }

    if ((strlen(dirname)+
	 strlen(buffer)+10)>sizeof(fbuffer)) {
      DBG_ERROR("Buffer too small");
      Directory_free(dd);
      return 1;
    }
    strcpy(fbuffer,  (char*)dirname);
    strcat(fbuffer, "/");
    strcat(fbuffer, buffer);

    e=AQBackup_File2Entry(fbuffer);
    if (!e) {
      DBG_ERROR("Could not create entry for \"%s\"", fbuffer);
      Directory_free(dd);
      return 1;
    }
    DBG_DEBUG("Adding entry for \"%s\"", fbuffer);
    AQBEntry_add(e, head);
  } /* while */

  Directory_Close(dd);
  Directory_free(dd);

  return 0;
}



int AQBackup__CompareEntries(AQB_ENTRY *el, AQB_ENTRY *er,
			     AQB_BACKUP_TIME_COMPARE_MODE cmode) {
  int changed;
  int contentchanged;
  AQB_ENTRY_VERSION *vr, *vl;

  changed=0;
  contentchanged=0;
  vr=er->versions;
  assert(vr);
  vl=el->versions;
  assert(vl);

  if (vl->status!=vr->status) {
    DBG_DEBUG("Status changed");
    if ((vr->status==EntryStatusDeleted || vl->status==EntryStatusDeleted) &&
	S_ISREG(vl->mode) &&
	!S_ISLNK(vl->mode)) {
      /* need to send/pull the whole file, it was deleted */
      contentchanged++;
    }
    else
      changed++;
  }

  /* compare times */
  if (!S_ISLNK(vl->mode)) {
    /* we cannot set the times of a link, so no need to compare the times */
    switch (cmode) {
    case TimeCompareLocalNewer:
      if (vl->mtime>vr->mtime) {
	DBG_DEBUG("local mtime is newer");
	changed++;
      }
      if (vl->ctime>vr->ctime) {
	DBG_DEBUG("local ctime is newer");
	changed++;
      }
      break;

    case TimeCompareRemoteNewer:
      if (vr->mtime>vl->mtime) {
	DBG_DEBUG("remote mtime is newer");
	changed++;
      }
      if (vr->ctime>vl->ctime) {
	DBG_DEBUG("remote ctime is newer");
	changed++;
      }
      break;

    case TimeCompareNone:
      break;

    case TimeCompareForceChange:
      DBG_DEBUG("change forced");
      changed++;
      break;

    case TimeCompareDiff:
    default:
      if (vl->mtime!=vr->mtime) {
	DBG_DEBUG("mtime differs");
	changed++;
      }
      if (vl->ctime!=vr->ctime) {
	DBG_DEBUG("ctime differs");
	changed++;
      }
      break;
    } /* switch */
  }

  if (vl->mode!=vr->mode) {
    DBG_DEBUG("Mode differs");
    changed++;
  }

  /* device changed ? */
  if (vl->device!=vr->device) {
    DBG_DEBUG("Device differs");
    changed++;
  }

  /* owner changed or newly provided ? */
  if (Text_Compare(vl->owner, vr->owner, 0)!=0 && vl->owner) {
    DBG_DEBUG("Owner differs");
    changed++;
  }

  /* group changed or newly provided ? */
  if (Text_Compare(vl->group, vr->group, 0)!=0 && vl->group) {
    DBG_DEBUG("Group differs");
    changed++;
  }

  /* symlink target changed ? */
  if (Text_Compare(vl->dstname, vr->dstname, 0)!=0) {
    DBG_DEBUG("Symlink target differs");
    changed++;
  }

  if (S_ISREG(vl->mode) && !S_ISLNK(vl->mode)) {
    /* only check regular files for content changes */
    if (vl->size!=vr->size) {
      DBG_DEBUG("Size differs");
      contentchanged++;
    }
  }

  if (!S_ISLNK(vl->mode) && S_ISREG(vl->mode) && changed) {
    /* only check regular files for content changes */
    /* md5 checksum changed or newly provided ? */
    char md5[64];
    int needMD5;

    needMD5=0;
    if (!vl->md5)
      needMD5=1;
    else if (strlen(vl->md5)==0)
      needMD5=1;

    if (needMD5) {
      int rv;

      DBG_INFO("Reading md5 for %s", el->abspath);
      rv=AQBackup__GetMD5(el->abspath, md5, sizeof(md5));
      if (rv) {
	DBG_ERROR("Error getting MD5");
	return -1;
      }
      free(vl->md5);
      vl->md5=strdup(md5);
    }

    /* now compare the MD5 */
    if (Text_Compare(vl->md5, vr->md5, 0)!=0 && vl->md5) {
      DBG_DEBUG("Md5 checksum differs");
      contentchanged++;
    }
  } /* if file and something changed */

  if (changed || contentchanged) {
    DBG_NOTICE("%s has changed", el->abspath);
  }
  if (contentchanged)
    return 2;
  if (changed)
    return 1;

  return 0;
}



int AQBackup_GetDiffEntriesForBackup(AQB_CLIENT *c,
				     int did,
				     const char *abspath,
                                     time_t when,
				     STRINGLIST *dirlist,
				     AQB_ENTRY **modifiedEntries,
				     AQB_ENTRY **modifiedFiles,
				     AQB_ENTRY **deletedEntries,
				     unsigned int flags) {
  AQB_ENTRY *localEntries;
  AQB_ENTRY *remoteEntries;
  AQB_ENTRY *el, *er, *enext;
  int rv;

  assert(c);
  assert(abspath);
  assert(dirlist);
  assert(modifiedEntries);
  assert(modifiedFiles);
  assert(deletedEntries);

  /* init */
  localEntries=0;
  remoteEntries=0;

  /* get local entries */
  rv=AQBackup_Dir2Entries(&localEntries, abspath, 0);
  if (rv) {
    DBG_ERROR("Could not get local entries");
    AQBEntry_freeAll(localEntries);
    return 1;
  }

  /* get remote entries
   */
  rv=AQBClient_GetEntries(c, did, when, &remoteEntries);
  if (rv) {
    DBG_ERROR("Could not get local entries");
    AQBEntry_freeAll(localEntries);
    AQBEntry_freeAll(remoteEntries);
    return 1;
  }

  /* add all directories to the stringlist */
  el=localEntries;
  while(el) {
    AQB_ENTRY_VERSION *v;

    v=el->versions;
    assert(v);
    if (S_ISDIR(v->mode)) {
      char unescapebuffer[300];

      /* add directory */
      if (!Text_Unescape(el->name, unescapebuffer, sizeof(unescapebuffer))) {
	DBG_ERROR("Could not unescape name");
	AQBEntry_freeAll(localEntries);
	AQBEntry_freeAll(remoteEntries);
	return 1;
      }
      else {
	char dirbuffer[300];

	if ((strlen(abspath)+strlen(unescapebuffer)+2)>sizeof(dirbuffer)) {
	  DBG_ERROR("Buffer too small");
	  AQBEntry_freeAll(localEntries);
	  AQBEntry_freeAll(remoteEntries);
	  return 1;
	}
	strcpy(dirbuffer, abspath);
	strcat(dirbuffer, "/");
	strcat(dirbuffer, unescapebuffer);
        if (flags & AQBACKUP_FLAGS_DEEP_FIRST)
	  StringList_InsertString(dirlist, dirbuffer,0,1); /* doubleChecked*/
        else
	  StringList_AppendString(dirlist, dirbuffer,0,1); /* doubleChecked*/
      }
    }
    el=el->next;
  } /* while */

  /* check for entries to be deleted in repository */
  DBG_DEBUG("Checking for entries to be deleted");
  er=remoteEntries;
  while(er) {
    char *absname;
    unsigned int absnamesize;
    char unescapebuffer[300];

    if (!Text_Unescape(er->name, unescapebuffer, sizeof(unescapebuffer))) {
      DBG_ERROR("Could not unescape name");
      AQBEntry_freeAll(localEntries);
      AQBEntry_freeAll(remoteEntries);
      return 1;
    }

    absnamesize=strlen(abspath)+strlen(unescapebuffer)+2;
    absname=(char*)malloc(absnamesize);
    assert(absname);
    strcpy(absname, abspath);
    strcat(absname, "/");
    strcat(absname, unescapebuffer);
    er->abspath=absname;

    enext=er->next;

    /* search for this entry in repository */
    el=localEntries;
    while(el) {
      if (strcmp(er->name, el->name)==0)
	break;
      el=el->next;
    } /* while remote */

    if (!el) {
      AQB_ENTRY_VERSION *v;

      v=er->versions;
      assert(v);

      /* entry not in repository, so move it to "deleted" list */
      DBG_DEBUG("Entry \"%s\" is deleted", er->name);
      AQBEntry_del(er, &remoteEntries);
      if (v->status!=EntryStatusDeleted)
	AQBEntry_add(er, deletedEntries);
      else
	/* was already deleted in repository, so ignre this entry */
	AQBEntry_free(er);
    }
    er=enext;
  } /* while */

  /* check for entries to be updated */
  DBG_DEBUG("Checking for entries to be updated");
  el=localEntries;
  while(el) {
    enext=el->next;
    DBG_DEBUG("Verifying \"%s\"", el->name);
    /* search for this entry in repository */
    er=remoteEntries;
    while(er) {
      if (strcmp(el->name, er->name)==0)
	break;
      er=er->next;
    } /* while remote */

    if (!er) {
      AQB_ENTRY_VERSION *vl;
      vl=el->versions;
      assert(vl);

      /* entry not in repository, so move it to "modified" list */
      if (S_ISREG(vl->mode) && !S_ISLNK(vl->mode)) {
	/* regular file, so move to "modified files" list */
	DBG_DEBUG("File \"%s\" is new", el->name);
	AQBEntry_del(el, &localEntries);
	AQBEntry_add(el, modifiedFiles);
      }
      else {
	/* otherwise only update the entry */
	DBG_DEBUG("Entry \"%s\" is new", el->name);
	AQBEntry_del(el, &localEntries);
	AQBEntry_add(el, modifiedEntries);
      }
    }
    else {
      if (!(flags & AQBACKUP_FLAGS_FORCE)) {
	/* entry is in the repository, check if it needs modification */
	int change;

	change=AQBackup__CompareEntries(el, er, TimeCompareDiff);
	if (change==-1) {
	  if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	    DBG_ERROR("Error comparing two entries");
	    return 1;
	  }
	  /* remove bad entry */
	  AQBEntry_del(el, &localEntries);
	  AQBEntry_free(el);

	}
	else if (change==2) {
	  /* content changed, so move it to "modified files" list */
	  DBG_DEBUG("File \"%s\" is modified", el->name);
	  AQBEntry_del(el, &localEntries);
	  AQBEntry_add(el, modifiedFiles);
	}
	else if (change==1) {
	  /* entry changed, so move it to "modified entries" list */
	  DBG_DEBUG("Entry \"%s\" is modified", el->name);
	  AQBEntry_del(el, &localEntries);
	  AQBEntry_add(el, modifiedEntries);
	}
      }
      else {
        /* user wants to FORCE storage of files/entries */
	AQB_ENTRY_VERSION *vl;
	vl=el->versions;
        assert(vl);

	/* file, so move it to "modified" list */
	if (S_ISREG(vl->mode) && !S_ISLNK(vl->mode)) {
	  /* regular file, so move to "modified files" list */
	  DBG_DEBUG("File \"%s\" is new", el->name);
	  AQBEntry_del(el, &localEntries);
	  AQBEntry_add(el, modifiedFiles);
	}
	else {
	  /* otherwise only update the entry */
	  DBG_DEBUG("Entry \"%s\" is new", el->name);
	  AQBEntry_del(el, &localEntries);
	  AQBEntry_add(el, modifiedEntries);
	}
      }
    }
    el=enext;
  } /* while */

  /* free the remaing entries, they have not changed */
  AQBEntry_freeAll(localEntries);
  AQBEntry_freeAll(remoteEntries);

  DBG_DEBUG("Checking for entries done");
  return 0;
}



int AQBackup_GetDiffEntriesForRestore(AQB_CLIENT *c,
				      int did,
				      const char *abspath,
				      time_t when,
				      STRINGLIST *dirlist,
				      AQB_ENTRY **modifiedEntries,
				      AQB_ENTRY **modifiedFiles,
				      AQB_ENTRY **newEntries,
				      AQB_ENTRY **deletedEntries,
				      unsigned int flags) {
  AQB_ENTRY *localEntries;
  AQB_ENTRY *remoteEntries;
  AQB_ENTRY *el, *er, *enext;
  int rv;

  assert(c);
  assert(abspath);
  assert(dirlist);
  assert(modifiedEntries);
  assert(modifiedFiles);
  assert(deletedEntries);
  assert(newEntries);

  /* init */
  localEntries=0;
  remoteEntries=0;

  /* get local entries */
  rv=AQBackup_Dir2Entries(&localEntries, abspath, 1);
  if (rv) {
    DBG_ERROR("Could not get local entries");
    AQBEntry_freeAll(localEntries);
    return 1;
  }

  /* get remote entries */
  rv=AQBClient_GetEntries(c, did, when, &remoteEntries);
  if (rv) {
    DBG_ERROR("Could not get local entries");
    AQBEntry_freeAll(localEntries);
    AQBEntry_freeAll(remoteEntries);
    return 1;
  }

  /* remove all remote entries which are deleted */
  er=remoteEntries;
  while(er) {
    AQB_ENTRY *en;
    AQB_ENTRY_VERSION *v;

    en=er->next;
    v=er->versions;
    assert(v);
    if (v->status==EntryStatusDeleted) {
      AQBEntry_del(er, &remoteEntries);
      AQBEntry_free(er);
    }
    else {
      /* set abspath */
      char *absname;
      unsigned int absnamesize;
      char unescapebuffer[300];

      /* add directory */
      if (!Text_Unescape(er->name, unescapebuffer, sizeof(unescapebuffer))) {
	DBG_ERROR("Could not unescape name");
	AQBEntry_freeAll(localEntries);
	AQBEntry_freeAll(remoteEntries);
	return 1;
      }

      absnamesize=strlen(abspath)+strlen(unescapebuffer)+2;
      absname=(char*)malloc(absnamesize);
      assert(absname);
      strcpy(absname, abspath);
      strcat(absname, "/");
      strcat(absname, unescapebuffer);
      er->abspath=absname;
    }
    er=en;
  } /* while */

  /* add all remote directories to the stringlist */
  er=remoteEntries;
  while(er) {
    AQB_ENTRY_VERSION *v;

    v=er->versions;
    assert(v);

    if (S_ISDIR(v->mode)) {
      char unescapebuffer[300];

      /* add directory */
      if (!Text_Unescape(er->name, unescapebuffer, sizeof(unescapebuffer))) {
	DBG_ERROR("Could not unescape name");
	AQBEntry_freeAll(localEntries);
	AQBEntry_freeAll(remoteEntries);
	return 1;
      }
      else {
	char dirbuffer[300];

	if ((strlen(abspath)+strlen(unescapebuffer)+2)>sizeof(dirbuffer)) {
	  DBG_ERROR("Buffer too small");
	  AQBEntry_freeAll(localEntries);
	  AQBEntry_freeAll(remoteEntries);
	  return 1;
	}
	strcpy(dirbuffer, abspath);
	strcat(dirbuffer, "/");
	strcat(dirbuffer, unescapebuffer);
	if (flags & AQBACKUP_FLAGS_DEEP_FIRST)
	  StringList_InsertString(dirlist, dirbuffer,0,1); /* doubleChecked*/
        else
	  StringList_AppendString(dirlist, dirbuffer,0,1); /* doubleChecked*/
      }
    }
    er=er->next;
  } /* while */

  /* check for entries to be locally deleted */
  DBG_DEBUG("Checking for entries to be locally deleted");
  el=localEntries;
  while(el) {
    enext=el->next;

    /* search for this entry in repository */
    er=remoteEntries;
    while(er) {
      if (strcmp(el->name, er->name)==0)
	break;
      er=er->next;
    } /* while remote */

    if (!er) {
      /* entry not existent, so move it to "deleted" list */
      DBG_DEBUG("Entry \"%s\" is deleted", el->name);
      AQBEntry_del(el, &localEntries);
      AQBEntry_add(el, deletedEntries);
    }
    el=enext;
  } /* while */

  /* check for entries to be updated */
  DBG_DEBUG("Checking for entries to be updated");
  er=remoteEntries;
  while(er) {
    enext=er->next;
    DBG_DEBUG("Verifying \"%s\"", er->name);
    /* search for this entry in repository */
    el=localEntries;
    while(el) {
      if (strcmp(er->name, el->name)==0)
	break;
      el=el->next;
    } /* while remote */

    if (!el) {
      AQB_ENTRY_VERSION *vr;
      vr=er->versions;

      /* entry not existent (locally), so move it to "modified" list */
      if (S_ISREG(vr->mode) && !S_ISLNK(vr->mode)) {
	/* regular file, so move to "modified files" list */
	DBG_DEBUG("Remote file \"%s\" is newer", er->name);
	AQBEntry_del(er, &remoteEntries);
	AQBEntry_add(er, modifiedFiles);
      }
      else {
	/* otherwise only update the entry */
	DBG_DEBUG("Remote entry \"%s\" is new", er->name);
	AQBEntry_del(er, &remoteEntries);
	AQBEntry_add(er, newEntries);
      }
    }
    else {
      /* entry is in the repository, check if it needs modification */
      if (flags & AQBACKUP_FLAGS_FORCE) {
	AQB_ENTRY_VERSION *vtmp;

	vtmp=er->versions;
	if (S_ISREG(vtmp->mode) && !S_ISLNK(vtmp->mode)) {
	  DBG_DEBUG("Entry \"%s\" is new", el->name);
	  AQBEntry_del(er, &remoteEntries);
	  AQBEntry_add(er, modifiedFiles);
	}
	else {
	  DBG_DEBUG("Entry \"%s\" is new", el->name);
	  AQBEntry_del(er, &remoteEntries);
	  AQBEntry_add(er, newEntries);
	}
      }
      else {
	int change;

	change=AQBackup__CompareEntries(el, er, TimeCompareRemoteNewer);
	if (change==2) {
	  /* content changed, so move it to "modified files" list */
	  DBG_DEBUG("File \"%s\" is modified", el->name);
	  AQBEntry_del(er, &remoteEntries);
	  AQBEntry_add(er, modifiedFiles);
	}
	else if (change==1) {
	  /* entry changed, so move it to "modified entries" list */
	  DBG_DEBUG("Entry \"%s\" is modified", er->name);
	  AQBEntry_del(er, &remoteEntries);
	  AQBEntry_add(er, modifiedEntries);
	}
      }
    }
    er=enext;
  } /* while */

  /* free the remaing entries, they have not changed */
  AQBEntry_freeAll(localEntries);
  AQBEntry_freeAll(remoteEntries);

  DBG_DEBUG("Checking for entries done");
  return 0;
}



int AQBackup__FindInStringList(STRINGLIST *sl,
			       const char *p) {
  STRINGLISTENTRY *se;

  assert(sl);
  assert(p);

  se=sl->first;
  while(se) {
    DBG_DEBUG("Check: \"%s\" == \"%s\" ?",
	      p, se->data);
    if (-1!=Text_ComparePattern(p, se->data, 1)) {
      DBG_DEBUG("Yes: \"%s\" == \"%s\" !",
		p, se->data);
      return 1;
    }
    else {
      DBG_DEBUG("No: \"%s\" != \"%s\" !",
		p, se->data);
    }
    se=se->next;
  } /* while */
  return 0;

}



AQB_BACKUP_REPOSITORY *AQBackup__FindMatchingRepository(AQBACKUP *b,
							const char *dir) {
  AQB_BACKUP_REPOSITORY *r, *best;
  int highestMatches;
  int matches;
  const char *p1, *p2;

  assert(b);

  highestMatches=0;
  best=0;
  r=b->repositories;
  while(r) {
    matches=0;
    p1=dir;
    p2=r->baseDir;

    /* count matches */
    for(;;) {
      if (*p1 && *p2) {
	if (*p1!=*p2) {
	  matches=0;
	  break;
	}
	else
	  matches++;
      }
      else if (*p2==0) {
	/* baseDir ends, if dir has a "/" at that pos or ends too we got it */
	if (*p1=='/' || *p1==0)
	  break;
	else {
	  matches=0;
	  break;
	}
      }
      else {
	matches=0;
	break;
      }
      p1++;
      p2++;
    } /* for */

    if (matches) {
      if (best==0) {
	best=r;
	highestMatches=matches;
      }
      else {
	if (matches>highestMatches) {
	  best=r;
	  highestMatches=matches;
	}
      }
    }
    r=r->next;
  } /* while */

  if (best==0) {
    DBG_INFO("No repository found for dir \"%s\"", dir);
    return 0;
  }

  return best;
}



int AQBackup_Open(AQBACKUP *b, const char *dir, int writeAccess) {
  AQB_BACKUP_REPOSITORY *r;
  const char *relpath;
  AQB_CLIENT *c;
  int cid;
  int rid;

  assert(b);
  assert(dir);

  if (b->currClient || b->currRepository) {
    DBG_ERROR("Repository already open");
    return 1;
  }

  /* find matching repository */
  DBG_DEBUG("Looking for matching repository for \"%s\"",
	    dir);
  r=AQBackup__FindMatchingRepository(b, dir);
  if (!r) {
    DBG_ERROR("No repository for \"%s\" found", dir);
    return 1;
  }

  if (strlen(dir)>strlen(r->baseDir))
    relpath=&(dir[strlen(r->baseDir)]);
  else
    relpath="/";
  DBG_DEBUG("Relative path is \"%s\"",
	    relpath);

  /* create client for this repository */
  DBG_DEBUG("Getting appropriate client");
  switch(r->typ) {
  case RepositoryTypeDirect:
    c=AQBClientDirect_new();
    break;

  case RepositoryTypeLocal:
  case RepositoryTypeNet:
  default:
    DBG_ERROR("Not yet implemented, sorry.");
    return 1;
    break;
  } /* switch */

  /* we now do have a valid client object, init it */
  DBG_DEBUG("Initializing client");
  if (AQBClient_Init(c, r->config)) {
    DBG_ERROR("Could not init client for \"%s\"",
	      r->name);
    AQBClient_free(c);
    return 1;
  }
  b->currClient=c;
  b->currRepository=r;

  DBG_DEBUG("Registering client");
  cid=AQBClient_Register(c, r->user);
  if (cid==0) {
    DBG_ERROR("Could not register client.");
    return 1;
  }
  DBG_INFO("Client registered as %d", cid);

  DBG_DEBUG("Opening repository");
  rid=AQBClient_OpenRepository(c,
			       b->hostName,
			       r->name,
			       writeAccess);
  if (rid==0) {
    DBG_ERROR("Could not open repository.");
    return 1;
  }
  DBG_INFO("Opened repository as %d", rid);

  return 0;
}



int AQBackup__Create(AQBACKUP *b,
		     AQB_BACKUP_REPOSITORY *r) {
  AQB_CLIENT *c;
  int cid;
  int rid;

  assert(b);
  assert(r);

  if (b->currClient || b->currRepository) {
    DBG_ERROR("Repository already open");
    return 1;
  }

  /* create client for this repository */
  DBG_DEBUG("Getting appropriate client");
  switch(r->typ) {
  case RepositoryTypeDirect:
    c=AQBClientDirect_new();
    break;

  case RepositoryTypeLocal:
  case RepositoryTypeNet:
  default:
    DBG_ERROR("Not yet implemented, sorry.");
    return 1;
    break;
  } /* switch */

  /* we now do have a valid client object, init it */
  DBG_DEBUG("Initializing client");
  if (AQBClient_Init(c, r->config)) {
    DBG_ERROR("Could not init client for \"%s\"",
	      r->name);
    AQBClient_free(c);
    return 1;
  }
  b->currClient=c;
  b->currRepository=r;

  DBG_DEBUG("Registering client");
  cid=AQBClient_Register(c, r->user);
  if (cid==0) {
    DBG_ERROR("Could not register client.");
    return 1;
  }
  DBG_INFO("Client registered as %d", cid);

  DBG_DEBUG("Opening repository");
  rid=AQBClient_CreateRepository(c,
				 b->hostName,
				 r->name);
  if (rid==0) {
    DBG_ERROR("Could not create repository.");
    return 1;
  }
  DBG_INFO("Created repository as %d", rid);

  return 0;
}



int AQBackup__StoreFile(AQBACKUP *b,
			int did,
			const char *dir,
			AQB_ENTRY *e,
			unsigned int flags) {
  FILE *ftest;
  int fid;
  char fullname[300];
  char unescapebuffer[300];
  char *p;
  int rv;
  int compressed;
  AQB_ENTRY_VERSION *v;
  CONVERTER *dmgroup;
  CONVERTER *dm;
  CONVERTER *dmMD5;
  int outeof;
  int bytesout;
  char outbuffer[2048];
  unsigned int bytesReceivedOld;

  v=e->versions;
  assert(v);

  /* unescape file name from entry */
  p=Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer));
  if (p==0) {
    DBG_ERROR("Could not unescape");
    return 1;
  }

  if (AQBackup__FindInStringList(b->currRepository->nozips, unescapebuffer)) {
    compressed=0;
  }
  else {
    if (v->size>AQBACKUP_COMPRESS_TRIGGER_SIZE) {
      v->flags|=ENTRY_VERSION_FLAGS_COMPRESSED;
      compressed=1;
    }
    else
      compressed=0;
  }


  /* create local path and name */
  strcpy(fullname, dir);
  strcat(fullname, "/");
  strcat(fullname, unescapebuffer);

  /* locally open file */
  ftest=fopen(fullname, "rb");
  if (ftest==0) {
    DBG_ERROR("Error on fopen(%s): %s",
	      fullname, strerror(errno));
    return 1;
  }
  fclose(ftest);

  /* remotely open file */
  DBG_INFO("Opening file \"%s\"", unescapebuffer);
  fid=AQBClient_FileOpenOut(b->currClient, did, e);
  if (fid==0) {
    DBG_ERROR("Could not open file for writing.");
    return 1;
  }

  /* compress file on the fly if wanted */
  AQBInteractor_StartWriteFile(b->interactor, e);

  dmgroup=ConverterFileIn_new(fullname);
  if (compressed)
    dm=ConverterZip_new(b->currRepository->zipLevel);
  else
    dm=ConverterDummy_new();
  ConverterGroup_Append(dmgroup, dm);
  dmMD5=ConverterMd5_new();
  ConverterGroup_Tee(dm, dmMD5);

  rv=ConverterGroup_Begin(dmgroup);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    ConverterGroup_free(dmgroup);
    AQBInteractor_StopWriteFile(b->interactor);
    return 1;
  }

  outeof=0;
  bytesReceivedOld=0;
  while (!outeof) {
    unsigned int bytesReceived;

    /* process data */
    DBG_INFO("Processing data");
    rv=ConverterGroup_Work(dmgroup);
    if (rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	ConverterGroup_free(dmgroup);
	AQBInteractor_StopWriteFile(b->interactor);
	return 1;
      }
    }

    /* check if there already is data */
    bytesReceived=0;
    while ((bytesout=ConverterGroup_HasData(dmgroup))) {
      DBG_INFO("Data available :%d", bytesout);
      if (bytesout>sizeof(outbuffer)) {
	DBG_INFO("Reading less than available bytes (%d of %d)",
		 sizeof(outbuffer), bytesout);
	bytesout=sizeof(outbuffer);
      }
      DBG_DEBUG("Getting data");
      rv=ConverterGroup_GetData(dmgroup, outbuffer, bytesout);
      if (rv<1) {
	DBG_ERROR("Error getting data");
	ConverterGroup_free(dmgroup);
	AQBInteractor_StopWriteFile(b->interactor);
	return 1;
      }
      bytesReceived=rv;

      rv=AQBClient_FileWrite(b->currClient, fid,
			     outbuffer, rv);
      if (rv) {
	DBG_ERROR("Could not write file");
	ConverterGroup_free(dmgroup);
	AQBInteractor_StopWriteFile(b->interactor);
	return 1;
      }
    }

    if (bytesReceived) {
      bytesReceivedOld+=bytesReceived;
      AQBInteractor_WriteFile(b->interactor,
			      bytesReceivedOld);
    }
  } /* while */

  /* get the MD5 sum */
  if ((bytesout=Converter_HasData(dmMD5))) {
    char md5buffer[64];

    DBG_INFO("Data available :%d", bytesout);
    if (bytesout>sizeof(outbuffer)) {
      DBG_INFO("Reading less than available bytes (%d of %d)",
	       sizeof(outbuffer), bytesout);
      bytesout=sizeof(outbuffer);
    }
    DBG_DEBUG("Getting data");
    rv=Converter_GetData(dmMD5, outbuffer, bytesout);
    if (rv<1) {
      DBG_ERROR("Error getting data");
      ConverterGroup_free(dmgroup);
      AQBInteractor_StopWriteFile(b->interactor);
      return 1;
    }
    if (!Text_ToHex(outbuffer, rv, md5buffer, sizeof(md5buffer))) {
      DBG_ERROR("Error getting md5 sum");
      ConverterGroup_free(dmgroup);
      AQBEntry_free(e);
      AQBInteractor_StopWriteFile(b->interactor);
      return 1;
    }
    free(v->md5);
    v->md5=strdup(md5buffer);
  }

  ConverterGroup_End(dmgroup);
  ConverterGroup_free(dmgroup);

  DBG_INFO("File written");
  rv=AQBClient_FileCloseOut(b->currClient, fid, v->md5);
  AQBInteractor_StopWriteFile(b->interactor);
  if (rv) {
    DBG_ERROR("Could not close file");
    return 1;
  }

  DBG_DEBUG("File \"%s\" written", fullname);
  return 0;
}



int AQBackup__StoreDir(AQBACKUP *b,
		       int did,
		       const char *abspath,
		       STRINGLIST *sl,
		       unsigned int flags) {
  AQB_ENTRY *modEntries;
  AQB_ENTRY *modFiles;
  AQB_ENTRY *delEntries;
  AQB_ENTRY *e;
  int rv;

  /* get diff between local and remote */
  modEntries=0;
  modFiles=0;
  modEntries=0;
  delEntries=0;
  rv=AQBackup_GetDiffEntriesForBackup(b->currClient,
				      did,
				      abspath,
                                      0,
				      sl,
				      &modEntries,
				      &modFiles,
				      &delEntries,
				      flags);
  if (rv) {
    DBG_ERROR("Could not get diffs");
    return 1;
  }

  /* update modified entries */
  e=modEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      DBG_DEBUG("Updating entry \"%s\"", e->name);
      AQBInteractor_Updating(b->interactor, e);
      rv=AQBClient_SetEntry(b->currClient, did, e);
      if (rv) {
	DBG_ERROR("Could not update entry.");
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  AQBEntry_freeAll(modEntries);
	  AQBEntry_freeAll(modFiles);
	  AQBEntry_freeAll(delEntries);
	  return 1;
	}
      }
    }
    e=e->next;
  } /* while */

  /* delete modified entries */
  e=delEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      DBG_DEBUG("Deleting entry \"%s\"", e->name);
      AQBInteractor_Deleting(b->interactor, e);
      rv=AQBClient_DeleteEntry(b->currClient, did, e);
      if (rv) {
	DBG_ERROR("Could not delete entry.");
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  AQBEntry_freeAll(modEntries);
	  AQBEntry_freeAll(modFiles);
	  AQBEntry_freeAll(delEntries);
	  return 1;
	}
      }
    }
    e=e->next;
  } /* while */

  /* store modified files */
  e=modFiles;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      AQB_ENTRY_VERSION *v;

      DBG_NOTICE("Storing file in \"%s\"", e->name);

      v=e->versions;
      rv=AQBackup__StoreFile(b, did, abspath, e, flags);
      if (rv) {
	DBG_ERROR("Error storing file in \"%s\"", e->name);
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  AQBEntry_freeAll(modEntries);
	  AQBEntry_freeAll(modFiles);
	  AQBEntry_freeAll(delEntries);
	  return 1;
	}
      }
    }
    e=e->next;
  } /* while */

  /* cleanup */
  AQBEntry_freeAll(modEntries);
  AQBEntry_freeAll(modFiles);
  AQBEntry_freeAll(delEntries);
  return 0;
}



int AQBackup__ShowDiffs(AQBACKUP *b,
			int did,
			const char *abspath,
                        time_t when,
			STRINGLIST *sl,
			unsigned int flags) {
  AQB_ENTRY *modEntries;
  AQB_ENTRY *modFiles;
  AQB_ENTRY *delEntries;
  AQB_ENTRY *e;
  int rv;

  /* get diff between local and remote */
  modEntries=0;
  modFiles=0;
  modEntries=0;
  delEntries=0;
  rv=AQBackup_GetDiffEntriesForBackup(b->currClient,
				      did,
				      abspath,
                                      when,
				      sl,
				      &modEntries,
				      &modFiles,
				      &delEntries,
				      flags);
  if (rv) {
    DBG_ERROR("Could not get diffs");
    return 1;
  }

  /* update modified entries */
  e=modEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer,
		      sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      AQBInteractor_LogStr2(b->interactor,
			    "M ",
			    e->abspath, InteractorLevelMute);
    }
    e=e->next;
  } /* while */

  /* deleted entries */
  e=delEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      AQBInteractor_LogStr2(b->interactor,
			    "D ",
			    e->abspath, InteractorLevelMute);
    }
    e=e->next;
  } /* while */

  /* store modified files */
  e=modFiles;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      AQBInteractor_LogStr2(b->interactor,
			    "C ",
			    e->abspath, InteractorLevelMute);
    }
    e=e->next;
  } /* while */

  /* cleanup */
  AQBEntry_freeAll(modEntries);
  AQBEntry_freeAll(modFiles);
  AQBEntry_freeAll(delEntries);
  return 0;
}



int AQBackup_Close(AQBACKUP *b){
  int rv;

  assert(b);
  assert(b->currClient);
  assert(b->currRepository);

  rv=AQBClient_CloseRepository(b->currClient);
  if (rv) {
    DBG_ERROR("Could not close current repository");
    return 1;
  }
  b->currRepository=0;

  rv=AQBClient_Unregister(b->currClient);
  if (rv) {
    DBG_ERROR("Could not close current repository");
    return 1;
  }
  b->currClient=0;

  return 0;
}


int AQBackup_Abandon(AQBACKUP *b){
  int rv;

  assert(b);

  if (b->currClient) {
    rv=AQBClient_CloseRepository(b->currClient);
    if (rv) {
      DBG_ERROR("Could not close current repository");
    }
    b->currRepository=0;

    rv=AQBClient_Unregister(b->currClient);
    if (rv) {
      DBG_ERROR("Could not close current repository");
      return 1;
    }
    b->currClient=0;
  }

  return 0;
}



int AQBackup__CreateDirEnt(const char *absname,
			   AQB_ENTRY *e,
			   unsigned int flags) {
  AQB_ENTRY_VERSION *v;
  char *p;
  char pbuff[300];

  DBG_DEBUG("Creating dir entry \"%s\"", absname);

  if (access(absname, F_OK)==0) {
    DBG_ERROR("File \"%s\" already exists", absname);
    return 1;
  }

  v=e->versions;
  assert(v);

  if (strlen(absname)>sizeof(pbuff)) {
    DBG_ERROR("Buffer too small");
    return 1;
  }
  strcpy(pbuff, absname);
  p=strrchr(pbuff, '/');
  if (p) {
    *p=0;
    if (pbuff[0]) {
      DBG_DEBUG("Creating path \"%s\"", pbuff);
      if (Path_Create("", pbuff)) {
	DBG_ERROR("Error creating path \"%s\"", pbuff);
	return 1;
      }
    }
  }

  if (S_ISDIR(v->mode)) {
    /* create directory */
    if (mkdir(absname, v->mode)) {
      DBG_ERROR("Error on mkdir(%s): %s",
		absname, strerror(errno));
      return 1;
    }
  }
  else if (S_ISCHR(v->mode) || S_ISBLK(v->mode)) {
    /* create device */
    if (mknod(absname, v->mode, v->device)) {
      DBG_ERROR("Error on mknod(%s): %s",
		absname, strerror(errno));
      return 1;
    }
  }
  else if (S_ISLNK(v->mode)) {
    /* create symlink */
    if (symlink(v->dstname, absname)) {
      DBG_ERROR("Error on symlink(%s): %s",
		absname, strerror(errno));
      return 1;
    }
  }
  else if (S_ISREG(v->mode)) {
    /* regular file, do nothing */
    return 0;
  }
  else if (S_ISFIFO(v->mode)) {
    /* FIFO */
    if (mknod(absname, v->mode, v->device)) {
      DBG_ERROR("Error on mknod(%s): %s",
		absname, strerror(errno));
      return 1;
    }
  }

  /* now set rights, times, owner etc if allowed to */
  if (AQBackup__ModifyDirEnt(absname, e, flags)) {
    DBG_ERROR("Could not modify \"%s\"", absname);
    return 1;
  }

  return 0;
}



int AQBackup__ModifyDirEnt(const char *absname,
			   AQB_ENTRY *e,
			   unsigned int flags) {
  AQB_ENTRY_VERSION *v;
  struct utimbuf ut;
  int uid;
  int gid;

  v=e->versions;
  assert(v);

  if (!(flags & AQBACKUP_FLAGS_DONT_CHANGE_OWNER)) {
    /* get owner uid */
    uid=-1;
    if (v->owner) {
      if (v->owner[0]!=0) {
	struct passwd *pw;

	pw=getpwnam(v->owner);
	if (pw)
	  uid=pw->pw_uid;
      }
    }

    /* get group */
    gid=-1;
    if (v->group) {
      if (v->group[0]!=0) {
	struct group *gr;

	gr=getgrnam(v->group);
	if (gr)
	  gid=gr->gr_gid;
      }
    }

    /* set uid and gid */
    if ((gid!=-1 || uid!=-1) && getuid()==0) {
      if (S_ISLNK(v->mode)) {
	if (lchown(absname, uid, gid)) {
	  DBG_ERROR("Error on lchown(%s): %s",
		    absname, strerror(errno));
	  return 1;
	}
      }
      else {
	if (chown(absname, uid, gid)) {
	  DBG_ERROR("Error on chown(%s): %s",
		    absname, strerror(errno));
	  return 1;
	}
      }
    }
  }

  /* set mode (not the file type specific bits) */
  if (!(flags & AQBACKUP_FLAGS_DONT_CHANGE_RIGHTS)) {
    if (!S_ISLNK(v->mode)) {
      if (chmod(absname, (v->mode) & 07777)) {
	DBG_ERROR("Error on chmod(%s): %s",
		  absname, strerror(errno));
	return 1;
      }
    }
  }

  /* set times (last to do, since other ops may change times again) */
  if (!(flags & AQBACKUP_FLAGS_DONT_CHANGE_TIMES)) {
    if (!S_ISLNK(v->mode)) {
      ut.actime=0;
      ut.modtime=v->mtime;
      if (utime(absname, &ut)) {
	DBG_ERROR("Error on utime(%s): %s",
		  absname, strerror(errno));
	return 1;
      }
    }
  }

  return 0;
}



int AQBackup__RestoreFile(AQBACKUP *b,
			  int did,
			  const char *destdir,
			  const char *ename,
			  time_t when,
			  unsigned int flags) {
  int fid;
  char fullname[300];
  char unescapebuffer[300];
  char md5buffer[64];
  char *p;
  int rv;
  int compressed;
  AQB_ENTRY *e;
  AQB_ENTRY_VERSION *v;
  CONVERTER *dmgroup;
  CONVERTER *dm;
  CONVERTER *dmMD5;
  int ineof, outeof;
  int bytesin;
  int bytesout;
  char inbuffer[1024];
  unsigned int bytesReceived;
  unsigned int bytesReceivedOld;

  /* unescape file name from entry */
  p=Text_Unescape(ename, unescapebuffer, sizeof(unescapebuffer));
  if (p==0) {
    DBG_ERROR("Could not unescape");
    return 1;
  }

  /* create local path and name */
  fullname[0]=0;
  strcat(fullname, destdir);
  if (fullname[strlen(fullname)-1]!='/')
    strcat(fullname, "/");
  strcat(fullname, unescapebuffer);

  if (Path_Create("", destdir)) {
    DBG_ERROR("Could not create path \"%s\"", destdir);
    return 1;
  }

  /* locally open file */
  DBG_DEBUG("Will open for write: %s\n",
	    fullname);
  if (access(fullname, F_OK)==0) {
    if (!(flags & AQBACKUP_FLAGS_OVERWRITE)) {
      DBG_ERROR("File \"%s\" already exists", fullname);
      return 1;
    }
  }

  /* remotely open file */
  DBG_INFO("Opening file \"%s\"", unescapebuffer);
  fid=AQBClient_FileOpenIn(b->currClient, did, ename, when, &e);
  if (fid==0) {
    DBG_ERROR("Could not open file for writing.");
    return 1;
  }

  free(e->abspath);
  e->abspath=strdup(fullname);
  v=e->versions;
  assert(v);

  compressed=(v->flags & ENTRY_VERSION_FLAGS_COMPRESSED);

  /* create converters */
  if (compressed)
    dmgroup=ConverterUnzip_new();
  else
    dmgroup=ConverterDummy_new();
  dm=ConverterFileOut_new(fullname, "w+b");
  ConverterGroup_Append(dmgroup, dm);
  dmMD5=ConverterMd5_new();
  ConverterGroup_Tee(dm, dmMD5);

  AQBInteractor_StartReadFile(b->interactor, e);
  if (compressed) {
    DBG_INFO("Decompressing \"%s\"", fullname);
  }
  else {
    DBG_INFO("Copying \"%s\"", fullname);
  }
  rv=ConverterGroup_Begin(dmgroup);
  if (rv!=0) {
    DBG_ERROR("Error: %d", rv);
    ConverterGroup_free(dmgroup);
    AQBEntry_free(e);
    AQBInteractor_StopReadFile(b->interactor);
    return 1;
  }

  ineof=0;
  outeof=0;
  bytesReceivedOld=0;
  while (!outeof) {
    unsigned int bytesReceivedNew;

    /* check if input data is needed */
    DBG_INFO("Data needed ?");
    if (!ineof) {
      bytesin=ConverterGroup_NeedsData(dmgroup);
      DBG_INFO("Data needed :%d", bytesin);
      if (bytesin) {
	if (bytesin>sizeof(inbuffer))
	  bytesin=sizeof(inbuffer);

	rv=AQBClient_FileRead(b->currClient, fid, inbuffer,
			      bytesin);
	if (rv==0) {
	  ineof=1;
	  rv=ConverterGroup_SetData(dmgroup, 0, 0);
	}
	else if (rv<0) {
	  DBG_ERROR("Could not read file\n");
	  ConverterGroup_free(dmgroup);
	  AQBEntry_free(e);
	  AQBInteractor_StopReadFile(b->interactor);
	  return 1;
	}
	else {
	  DBG_DEBUG("Setting data");
	  rv=ConverterGroup_SetData(dmgroup, inbuffer, rv);
	}
	if (rv<0) {
	  DBG_ERROR("Error setting data (%d)", rv);
	  ConverterGroup_free(dmgroup);
	  AQBEntry_free(e);
	  AQBInteractor_StopReadFile(b->interactor);
	  return 1;
	}
      }
    }

    /* now process data */
    DBG_INFO("Processing data");
    rv=ConverterGroup_Work(dmgroup);
    if (rv==CONVERTER_RESULT_EOF)
      outeof=1;
    else {
      if (rv!=CONVERTER_RESULT_OK) {
	DBG_ERROR("Error processing data (%d)", rv);
	ConverterGroup_free(dmgroup);
	AQBEntry_free(e);
	AQBInteractor_StopReadFile(b->interactor);
	return 1;
      }
    }
    bytesReceivedNew=ConverterGroup_BytesReceived(dmgroup);
    if (bytesReceivedNew!=bytesReceivedOld)
      AQBInteractor_ReadFile(b->interactor,
			     bytesReceivedNew);
    bytesReceivedOld=bytesReceivedNew;
  } /* while */

  /* get the MD5 sum */
  md5buffer[0]=0;
  if ((bytesout=Converter_HasData(dmMD5))) {
    DBG_INFO("Data available :%d", bytesout);
    if (bytesout>sizeof(inbuffer)) {
      DBG_INFO("Reading less than available bytes (%d of %d)",
	       sizeof(inbuffer), bytesout);
      bytesout=sizeof(inbuffer);
    }
    DBG_DEBUG("Getting data");
    rv=Converter_GetData(dmMD5, inbuffer, bytesout);
    if (rv<1) {
      DBG_ERROR("Error getting data");
      ConverterGroup_free(dmgroup);
      AQBEntry_free(e);
      AQBInteractor_StopReadFile(b->interactor);
      return 1;
    }
    if (!Text_ToHex(inbuffer, rv, md5buffer, sizeof(md5buffer))) {
      DBG_ERROR("Error getting md5 sum");
      ConverterGroup_free(dmgroup);
      AQBEntry_free(e);
      AQBInteractor_StopReadFile(b->interactor);
      return 1;
    }
  }

  bytesReceived=ConverterGroup_BytesReceived(dmgroup);
  ConverterGroup_End(dmgroup);
  ConverterGroup_free(dmgroup);

  DBG_INFO("File written");
  rv=AQBClient_FileCloseIn(b->currClient, fid);
  if (rv) {
    DBG_ERROR("Could not close file");
    AQBEntry_free(e);
    AQBInteractor_StopReadFile(b->interactor);
    return 1;
  }

  /* check size */
  if (bytesReceived!=(unsigned int)(v->size)) {
    DBG_ERROR("Bad file size for %s (is %d, should be %d)",
	      fullname, bytesReceived, (unsigned int)(v->size));
    AQBEntry_free(e);
    AQBInteractor_StopReadFile(b->interactor);
    return 1;
  }

  /* check md5 */
  if (strcasecmp(md5buffer, v->md5)==0) {
    DBG_DEBUG("Integrity of \"%s\" ok.",
	      fullname);
  }
  else {
    DBG_ERROR("MD5 checksums do not match for \"%s\":\n"
	      "Is       : %s\n"
	      "Should be: %s\n",
	      fullname,
	      md5buffer,
	      v->md5
	     );
    AQBEntry_free(e);
    AQBInteractor_StopReadFile(b->interactor);
    return 1;
  }

  /* adjust rights and times */
  rv=AQBackup__ModifyDirEnt(fullname, e, flags);
  AQBEntry_free(e);
  if (rv) {
    DBG_ERROR("Could not modify file \"%s\"", fullname);
    AQBInteractor_StopReadFile(b->interactor);
    return 1;
  }

  AQBInteractor_StopReadFile(b->interactor);
  DBG_DEBUG("File \"%s\" restored", fullname);
  return 0;
}



int AQBackup__RestoreDir(AQBACKUP *b,
			 int did,
			 const char *abspath,
			 const char *destdir,
			 STRINGLIST *sl,
                         time_t when,
			 unsigned int flags,
			 AQB_ENTRY **chmodEntries) {
  AQB_ENTRY *modEntries;
  AQB_ENTRY *modFiles;
  AQB_ENTRY *newEntries;
  AQB_ENTRY *delEntries;
  AQB_ENTRY *e;
  int rv;

  /* get diff between local and remote */
  modEntries=0;
  modFiles=0;
  modEntries=0;
  delEntries=0;
  newEntries=0;
  rv=AQBackup_GetDiffEntriesForRestore(b->currClient,
				       did,
				       abspath,
                                       when,
				       sl,
				       &modEntries,
				       &modFiles,
				       &newEntries,
				       &delEntries,
				       flags);
  if (rv) {
    DBG_ERROR("Could not get diffs");
    return 1;
  }

  /* update modified entries */
  DBG_DEBUG("Checking for modified entries");
  e=modEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(newEntries);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      char destbuffer[300];
      AQB_ENTRY_VERSION *v;

      DBG_DEBUG("Updating entry \"%s\"", e->name);
      if ((strlen(destdir)+
	   strlen(unescapebuffer)+2)>sizeof(destbuffer)) {
	DBG_ERROR("Buffer too small");
	AQBEntry_freeAll(modEntries);
	AQBEntry_freeAll(modFiles);
	AQBEntry_freeAll(newEntries);
	AQBEntry_freeAll(delEntries);
	return 1;
      }
      strcpy(destbuffer, destdir);
      strcat(destbuffer, "/");
      strcat(destbuffer, unescapebuffer);

      v=e->versions;
      assert(v);
      if (S_ISDIR(v->mode)) {
	AQB_ENTRY *ne;

	/* modify later, if all subdirectories are done */
	ne=AQBEntry_dup(e);
	free(ne->name);
	ne->name=strdup(destbuffer);
	ne->next=*chmodEntries;
	*chmodEntries=ne;
      }
      else {
	/* modify now */
	AQBInteractor_Modifying(b->interactor, e);
	rv=AQBackup__ModifyDirEnt(destbuffer, e, flags);
	if (rv) {
	  DBG_ERROR("Could not update entry.");
	  if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	    AQBEntry_freeAll(modEntries);
	    AQBEntry_freeAll(modFiles);
	    AQBEntry_freeAll(newEntries);
	    AQBEntry_freeAll(delEntries);
	    return 1;
	  }
	}
      }
    }
    e=e->next;
  } /* while */

  /* create new entries */
  DBG_DEBUG("Checking for new entries");
  e=newEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(newEntries);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      char destbuffer[300];
      AQB_ENTRY_VERSION *v;

      DBG_DEBUG("Creating entry \"%s\"", e->name);
      if ((strlen(destdir)+
	   strlen(unescapebuffer)+2)>sizeof(destbuffer)) {
	DBG_ERROR("Buffer too small");
	AQBEntry_freeAll(modEntries);
	AQBEntry_freeAll(modFiles);
	AQBEntry_freeAll(newEntries);
	AQBEntry_freeAll(delEntries);
	return 1;
      }
      strcpy(destbuffer, destdir);
      if (destbuffer[strlen(destbuffer)-1]!='/')
	strcat(destbuffer, "/");
      strcat(destbuffer, unescapebuffer);

      DBG_DEBUG("Updating entry \"%s\"", e->name);
      AQBInteractor_Creating(b->interactor, e);

      AQBInteractor_Creating(b->interactor, e);
      rv=AQBackup__CreateDirEnt(destbuffer, e, flags);
      if (rv) {
	DBG_ERROR("Could not update entry.");
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  AQBEntry_freeAll(modEntries);
	  AQBEntry_freeAll(modFiles);
	  AQBEntry_freeAll(newEntries);
	  AQBEntry_freeAll(delEntries);
	  return 1;
	}
      }

      /* directories are modified again later, because changing the content
       * of a directory might change its times again.
       */
      v=e->versions;
      assert(v);
      if (S_ISDIR(v->mode)) {
	AQB_ENTRY *ne;

	/* modify later, if all subdirectories and entries are done */
	ne=AQBEntry_dup(e);
	free(ne->name);
	ne->name=strdup(destbuffer);
	ne->next=*chmodEntries;
	*chmodEntries=ne;
      }
    }
    e=e->next;
  } /* while */

  /* delete modified entries */
  DBG_DEBUG("Checking for deleted entries");
  e=delEntries;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(newEntries);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      if (flags & AQBACKUP_FLAGS_LOCAL_DELETE) {
	AQBInteractor_Removing(b->interactor, e);
	DBG_DEBUG("Locally deleting entry \"%s\"", e->name);

	//rv=AQBClient_DeleteEntry(b->currClient, did, e);
        /* TODO */
	rv=0;

	if (rv) {
	  DBG_ERROR("Could not delete entry.");
	  if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	    AQBEntry_freeAll(modEntries);
	    AQBEntry_freeAll(modFiles);
	    AQBEntry_freeAll(newEntries);
	    AQBEntry_freeAll(delEntries);
	    return 1;
	  }
	}
      }
    }
    e=e->next;
  } /* while */

  /* store modified files */
  DBG_DEBUG("Checking for modified files");
  e=modFiles;
  while(e) {
    char unescapebuffer[300];

    if (Text_Unescape(e->name, unescapebuffer, sizeof(unescapebuffer))==0) {
      DBG_ERROR("Could not escape name \"%s\" !",
		e->name);
      AQBEntry_freeAll(modEntries);
      AQBEntry_freeAll(modFiles);
      AQBEntry_freeAll(newEntries);
      AQBEntry_freeAll(delEntries);
      return 1;
    }
    if (AQBackup__FindInStringList(b->currRepository->ignores,
				   unescapebuffer)) {
      DBG_DEBUG("Ignoring entry \"%s\"", unescapebuffer);
    }
    else {
      DBG_NOTICE("Restoring file in \"%s\"", e->name);

      rv=AQBackup__RestoreFile(b, did, destdir, e->name,
			       when, flags);
      if (rv) {
	DBG_INFO("Error restoring file in \"%s\"", e->name);
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  AQBEntry_freeAll(modEntries);
	  AQBEntry_freeAll(modFiles);
	  AQBEntry_freeAll(newEntries);
	  AQBEntry_freeAll(delEntries);
	  return 1;
	}
      }
    }
    e=e->next;
  } /* while */

  /* cleanup */
  AQBEntry_freeAll(modEntries);
  AQBEntry_freeAll(modFiles);
  AQBEntry_freeAll(newEntries);
  AQBEntry_freeAll(delEntries);

  return 0;
}



int AQBackup_HandleDir(AQBACKUP *b,
		       const char *dir,
		       AQBACKUP_JOB_TYPE job,
		       time_t when,
		       const char *destdir,
		       unsigned int flags) {
  const char *relpath;
  AQB_CLIENT *c;
  int did;
  AQB_BACKUP_REPOSITORY *r;
  STRINGLIST *sl;
  STRINGLISTENTRY *se;
  int workResult;
  int rv;
  char destbuffer[300];
  AQB_ENTRY *chmodEntries;
  int hasDestDir;

  assert(b);
  assert(b->currClient);
  assert(b->currRepository);

  chmodEntries=0;
  r=b->currRepository;
  c=b->currClient;
  sl=StringList_new();

  hasDestDir=0;
  if (destdir)
    if (strlen(destdir))
      hasDestDir=1;

  StringList_AppendEntry(sl, StringListEntry_new(dir,0));

  while((se=sl->first)) {
    /* check if this dir is in the exclude list */
    if (AQBackup__FindInStringList(r->excludes, se->data)) {
      AQBInteractor_LogStr2(b->interactor, "Excluding ", se->data,
			    InteractorLevelVerbous);
      DBG_INFO("Excluding dir \"%s\"", se->data);
    }
    else {
      /* get relative path */
      if (strlen(se->data)>strlen(r->baseDir))
	relpath=&(se->data[strlen(r->baseDir)]);
      else
	relpath="/";
      DBG_DEBUG("Relative path is \"%s\"",
		relpath);

      if (hasDestDir) {
	const char *p;

	/* create destination path */
	if (strlen(se->data)>strlen(dir))
	  p=&(se->data[strlen(dir)]);
	else
	  p="/";
	destbuffer[0]=0;
	strcat(destbuffer, destdir);
	strcat(destbuffer, p);
      }
      else
	strcpy(destbuffer, se->data);

      /* open directory */
      switch(AQBInteractor_EnterDir(b->interactor, destbuffer)) {
      case CheckResultContinue:
	DBG_DEBUG("Check: Continue %s", destbuffer);
	break;
      case CheckResultSkip:
	DBG_NOTICE("Skip %s", destbuffer);
	StringList_free(sl);
	return 0;
      case CheckResultAbort:
      default:
	DBG_NOTICE("Abort %s", destbuffer);
	StringList_free(sl);
	AQBInteractor_LogStr(b->interactor, "Aborting",
			     InteractorLevelMute);
	return 1;
      } /* switch */
      DBG_DEBUG("Opening directory");
      did=AQBClient_OpenDir(c, relpath);
      if (did==0) {
	DBG_ERROR("Could not open directory \"%s\"", relpath);
	StringList_free(sl);
	return 1;
      }

      /* do something */
      switch(job) {
      case AQBackupJobStore:
	workResult=AQBackup__StoreDir(b, did, se->data, sl, flags);
	break;

      case AQBackupJobShowDiffs:
	workResult=AQBackup__ShowDiffs(b, did, se->data, when, sl, flags);
	break;

      case AQBackupJobRestore:
	workResult=AQBackup__RestoreDir(b,
					did,
					se->data,
					destbuffer,
					sl,
					when,
					flags,
					&chmodEntries);
	break;

      default:
	DBG_ERROR("Unknown job %d", job);
	workResult=1;
	break;
      } /* switch */

      /* close directory */
      AQBInteractor_LeaveDir(b->interactor, destbuffer);
      rv=AQBClient_CloseDir(c, did);
      if (rv) {
	DBG_ERROR("Could not close dir");
	StringList_free(sl);
	return 1;
      }

      /* evaluate workResult */
      if (workResult) {
	DBG_INFO("Error handling dir \"%s\"", se->data);
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  StringList_free(sl);
	  return 1;
	}
      }
    }

    StringList_RemoveEntry(sl, se);
    StringListEntry_free(se);

    if (!(flags & AQBACKUP_FLAGS_RECURSIVE))
      break;
  } /* while */

  /* check whether there are some entries we have to chmod now */
  if (chmodEntries) {
    AQB_ENTRY *e;

    /* now modify times of directories again */
    DBG_NOTICE("Modifying directories again");
    e=chmodEntries;
    while(e) {
      /* name has been replaced by the absolute name */
      AQBInteractor_Modifying(b->interactor, e);
      rv=AQBackup__ModifyDirEnt(e->name, e, flags);
      if (rv) {
	DBG_ERROR("Could not update entry.");
	if (!(flags & AQBACKUP_FLAGS_IGNORE_ERRORS)) {
	  AQBEntry_freeAll(chmodEntries);
	  workResult=1;
	  break;
	}
      }
      e=e->next;
    } /* while */
    AQBEntry_freeAll(chmodEntries);
  }

  DBG_DEBUG("Done.");

  StringList_free(sl);
  return 0;
}


int AQBackup_Create(AQBACKUP *b,
		    CONFIGGROUP *cfg,
		    int remoteToo) {
  AQB_BACKUP_REPOSITORY *r, *rt;
  int rv;

  assert(b);
  assert(cfg);

  r=AQBackupRepository_fromConfig(cfg);
  if (!r) {
    DBG_ERROR("Could not create repository");
    return 1;
  }

  /* check whether this repository already exists */
  rt=AQBackup__FindMatchingRepository(b, r->baseDir);
  if (rt) {
    DBG_ERROR("Repository for path \"%s\" already exists (%s)",
	      r->baseDir,
	      rt->baseDir);
    AQBackupRepository_free(r);
    return 1;
  }

  AQBackupRepository_add(r, &(b->repositories));
  if (remoteToo) {
    /* try to create the repository remotely */
    DBG_NOTICE("Creating repository remotely");
    rv=AQBackup__Create(b,r);
  }
  else {
    /* just try to open an remote existing repository */
    rv=AQBackup_Open(b, r->baseDir, 1);
  }

  if (rv) {
    DBG_ERROR("Could not open/create repository");
    AQBackupRepository_del(r, &(b->repositories));
    AQBackupRepository_free(r);
    return 1;
  }

  DBG_NOTICE("Repository added for path \"%s\"", r->baseDir);

  return 0;
}


int AQBackup_AddExclude(AQBACKUP *b, const char *d) {
  AQB_BACKUP_REPOSITORY *r;

  assert(b);
  assert(d);

  r=AQBackup__FindMatchingRepository(b, d);
  if (!r) {
    DBG_ERROR("No repository for path \"%s\" found", d);
    return 1;
  }

  StringList_AppendString(r->excludes,
			  d,
			  0,
			  1);
  return 0;
}



int AQBackup_RemoveExclude(AQBACKUP *b, const char *d) {
  AQB_BACKUP_REPOSITORY *r;
  STRINGLISTENTRY *se;

  assert(b);
  assert(d);

  r=AQBackup__FindMatchingRepository(b, d);
  if (!r) {
    DBG_ERROR("No repository for path \"%s\" found", d);
    return 1;
  }

  assert(r->excludes);
  se=r->excludes->first;
  while(se) {
    if (strcmp(se->data, d)==0) {
      StringList_RemoveEntry(r->excludes, se);
      StringListEntry_free(se);
      break;
    }
    se=se->next;
  } /* while */

  if (!se) {
    DBG_WARN("Directory \"%s\" not found in excludes", d);
  }

  return 0;
}


int AQBackup_AddIgnore(AQBACKUP *b, const char *d, const char *s) {
  AQB_BACKUP_REPOSITORY *r;

  assert(b);
  assert(s);

  r=AQBackup__FindMatchingRepository(b, d);
  if (!r) {
    DBG_ERROR("No repository for path \"%s\" found", d);
    return 1;
  }

  StringList_AppendString(r->ignores,
			  s,
			  0,
			  1);
  return 0;
}



int AQBackup_RemoveIgnore(AQBACKUP *b, const char *d, const char *s) {
  AQB_BACKUP_REPOSITORY *r;
  STRINGLISTENTRY *se;

  assert(b);
  assert(s);

  r=AQBackup__FindMatchingRepository(b, d);
  if (!r) {
    DBG_ERROR("No repository for path \"%s\" found", d);
    return 1;
  }

  assert(r->ignores);
  se=r->ignores->first;
  while(se) {
    if (strcmp(se->data, d)==0) {
      StringList_RemoveEntry(r->ignores, se);
      StringListEntry_free(se);
      break;
    }
    se=se->next;
  } /* while */

  if (!se) {
    DBG_WARN("Item \"%s\" not found in ignores", s);
  }

  return 0;
}



int AQBackup_AddNozip(AQBACKUP *b, const char *d, const char *s) {
  AQB_BACKUP_REPOSITORY *r;

  assert(b);
  assert(s);

  r=AQBackup__FindMatchingRepository(b, d);
  if (!r) {
    DBG_ERROR("No repository for path \"%s\" found", d);
    return 1;
  }

  StringList_AppendString(r->nozips,
			  s,
			  0,
			  1);
  return 0;
}



int AQBackup_RemoveNozip(AQBACKUP *b, const char *d, const char *s) {
  AQB_BACKUP_REPOSITORY *r;
  STRINGLISTENTRY *se;

  assert(b);
  assert(s);

  r=AQBackup__FindMatchingRepository(b, d);
  if (!r) {
    DBG_ERROR("No repository for path \"%s\" found", d);
    return 1;
  }

  assert(r->nozips);
  se=r->nozips->first;
  while(se) {
    if (strcmp(se->data, d)==0) {
      StringList_RemoveEntry(r->nozips, se);
      StringListEntry_free(se);
      break;
    }
    se=se->next;
  } /* while */

  if (!se) {
    DBG_WARN("Item \"%s\" not found in nozips", s);
  }

  return 0;
}



void AQBackup_SetInteractor(AQBACKUP *b,
			    AQB_INTERACTOR *ia){
  assert(b);
  b->interactor=ia;
}



AQB_INTERACTOR *AQBackup_GetInteractor(AQBACKUP *b){
  assert(b);
  return b->interactor;
}






