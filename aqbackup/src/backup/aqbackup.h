/***************************************************************************
 $RCSfile: aqbackup.h,v $
                             -------------------
    cvs         : $Id: aqbackup.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
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


#ifndef AQBACKUP_AQBACKUP_H
#define AQBACKUP_AQBACKUP_H

#define AQBACKUP_DEFAULT_ZIPLEVEL 6

#define AQBACKUP_FLAGS_RECURSIVE          0x0001
#define AQBACKUP_FLAGS_IGNORE_ERRORS      0x0002
#define AQBACKUP_FLAGS_LOCAL_DELETE       0x0004
#define AQBACKUP_FLAGS_FORCE              0x0008
#define AQBACKUP_FLAGS_VERBOUS            0x0010
#define AQBACKUP_FLAGS_DONT_CHANGE_RIGHTS 0x0020
#define AQBACKUP_FLAGS_VERY_VERBOUS       0x0040
#define AQBACKUP_FLAGS_OVERWRITE          0x0080
#define AQBACKUP_FLAGS_DONT_CHANGE_OWNER  0x0100
#define AQBACKUP_FLAGS_DONT_CHANGE_TIMES  0x0200
#define AQBACKUP_FLAGS_DEEP_FIRST         0x0400


#include <backup/interactor.h>

#include <chameleon/chameleon.h>
#include <chameleon/conf.h>

#include <time.h>


typedef enum {
  AQBackupJobUnknown=0,
  AQBackupJobStore,
  AQBackupJobShowDiffs,
  AQBackupJobRestore,
} AQBACKUP_JOB_TYPE;

typedef struct _AQB_BACKUP_REPOSITORY AQB_BACKUP_REPOSITORY;
typedef struct _AQBACKUP AQBACKUP;


AQBACKUP *AQBackup_new();
void AQBackup_free(AQBACKUP *b);


int AQBackup_Init(AQBACKUP *b, CONFIGGROUP *cfg);
int AQBackup_Fini(AQBACKUP *b, CONFIGGROUP *cfg);

int AQBackup_Open(AQBACKUP *b, const char *dir, int writeAccess);
int AQBackup_Create(AQBACKUP *b,
		    CONFIGGROUP *cfg,
		    int remoteToo);

int AQBackup_Close(AQBACKUP *b);
int AQBackup_Abandon(AQBACKUP *b);

int AQBackup_HandleDir(AQBACKUP *b,
		       const char *dir,
		       AQBACKUP_JOB_TYPE job,
		       time_t t,
		       const char *destdir,
		       unsigned int flags);

int AQBackup_AddExclude(AQBACKUP *b, const char *d);
int AQBackup_RemoveExclude(AQBACKUP *b, const char *d);
int AQBackup_AddIgnore(AQBACKUP *b, const char *d, const char *s);
int AQBackup_RemoveIgnore(AQBACKUP *b, const char *d, const char *s);
int AQBackup_AddNozip(AQBACKUP *b, const char *d, const char *s);
int AQBackup_RemoveNozip(AQBACKUP *b, const char *d, const char *s);


void AQBackup_SetInteractor(AQBACKUP *b,
			    AQB_INTERACTOR *ia);
AQB_INTERACTOR *AQBackup_GetInteractor(AQBACKUP *b);





#endif

