/***************************************************************************
 $RCSfile: entry.h,v $
                             -------------------
    cvs         : $Id: entry.h,v 1.1 2003/06/07 21:07:46 aquamaniac Exp $
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


#ifndef AQBACKUP_ENTRY_H
#define AQBACKUP_ENTRY_H

#include <chameleon/chameleon.h>
#include <chameleon/conf.h>
#include <chameleon/ipcmessage.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define ENTRY_VERSION_FLAGS_COMPRESSED 0x0001


typedef enum {
  EntryStatusUnknown=0,
  EntryStatusUsed,
  EntryStatusDeleted
} AQB_ENTRY_VERSION_STATUS;


typedef struct _AQB_ENTRY_VERSION AQB_ENTRY_VERSION;
struct _AQB_ENTRY_VERSION {
  AQB_ENTRY_VERSION *next;
  AQB_ENTRY_VERSION_STATUS status;
  unsigned int revision;
  char *md5;
  char *storeName;
  unsigned int flags;

  mode_t mode;
  char *owner;
  char *group;
  char *dstname; /* for symlinks */
  dev_t device;
  off_t size;
  time_t mtime;
  time_t ctime;
  time_t btime;
};


AQB_ENTRY_VERSION *AQBEntryVersion_new();
void AQBEntryVersion_free(AQB_ENTRY_VERSION *e);
AQB_ENTRY_VERSION *AQBEntryVersion_dup(AQB_ENTRY_VERSION *o);

void AQBEntryVersion_add(AQB_ENTRY_VERSION *e, AQB_ENTRY_VERSION **head);
void AQBEntryVersion_del(AQB_ENTRY_VERSION *e, AQB_ENTRY_VERSION **head);

AQB_ENTRY_VERSION *AQBEntryVersion_FromConfig(CONFIGGROUP *group);
int AQBEntryVersion_ToConfig(AQB_ENTRY_VERSION *e, CONFIGGROUP *group);

AQB_ENTRY_VERSION *AQBEntryVersion_FromMessage(IPCMESSAGE *m);
int AQBEntryVersion_ToMessage(AQB_ENTRY_VERSION *e, IPCMESSAGE *m);

AQB_ENTRY_VERSION *AQBEntryVersion_FromStat(const struct stat *s);

void AQBEntryVersion_dump(FILE *f, AQB_ENTRY_VERSION *v);



/*___________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                              AQBEntry
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */




typedef struct _AQB_ENTRY AQB_ENTRY;
struct _AQB_ENTRY {
  AQB_ENTRY *next;
  int modified;

  char *name;
  unsigned int nextRevision;
  unsigned int id;
  AQB_ENTRY_VERSION *versions;

  /* only at runtime */
  char *abspath;
};


AQB_ENTRY *AQBEntry_new();
void AQBEntry_free(AQB_ENTRY *e);
void AQBEntry_freeAll(AQB_ENTRY *e);
AQB_ENTRY *AQBEntry_dup(AQB_ENTRY *o);

/**
 * Duplicates the given entry, if it contains a version BEFORE the
 * given time. Otherwise 0 is returned. If "when" is 0, then the most
 * recent version will be returned.
 * Please note that "time" refers to the time when the file was stored by
 * aqbackup, not the creation or modification time of the file itself.
 */
AQB_ENTRY *AQBEntry_dup_date(AQB_ENTRY *o, time_t when);

void AQBEntry_add(AQB_ENTRY *e, AQB_ENTRY **head);
void AQBEntry_del(AQB_ENTRY *e, AQB_ENTRY **head);

AQB_ENTRY *AQBEntry_FromConfig(CONFIGGROUP *group);
int AQBEntry_ToConfig(AQB_ENTRY *e, CONFIGGROUP *group);


int AQBEntry_ReadFile(AQB_ENTRY **entry, const char *name);
int AQBEntry_WriteFile(AQB_ENTRY *entry, const char *name);


AQB_ENTRY *AQBEntry_FromMessage(IPCMESSAGE *m);
int AQBEntry_ToMessage(AQB_ENTRY *e, AQB_ENTRY_VERSION *v, IPCMESSAGE *m);


AQB_ENTRY *AQBEntry_FromStat(const char *name,
			     const struct stat *s);

void AQBEntry_dump(FILE *f, AQB_ENTRY *e);


#endif



