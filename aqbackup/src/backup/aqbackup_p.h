/***************************************************************************
 $RCSfile: aqbackup_p.h,v $
                             -------------------
    cvs         : $Id: aqbackup_p.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
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


#ifndef AQBACKUP_AQBACKUP_P_H
#define AQBACKUP_AQBACKUP_P_H


#define AQBACKUP_COMPRESS_TRIGGER_SIZE 64

#include "aqbackup.h"
#include "client.h"

#include <chameleon/chameleon.h>
#include <chameleon/stringlist.h>
#include <chameleon/conf.h>


typedef enum {
  TimeCompareDiff=0,
  TimeCompareLocalNewer,
  TimeCompareRemoteNewer,
  TimeCompareNone,
  TimeCompareForceChange
} AQB_BACKUP_TIME_COMPARE_MODE;


typedef enum {
  RepositoryTypeUnknown=0,
  RepositoryTypeDirect,
  RepositoryTypeLocal,
  RepositoryTypeNet
} AQB_BACKUP_REPOSITORY_TYPE;


struct _AQB_BACKUP_REPOSITORY {
  AQB_BACKUP_REPOSITORY *next;

  AQB_BACKUP_REPOSITORY_TYPE typ;
  char *baseDir;
  char *name;
  char *user;
  char *passwd;

  STRINGLIST *excludes;
  STRINGLIST *ignores;
  STRINGLIST *nozips;
  int zipLevel;

  CONFIGGROUP *config;
};


AQB_BACKUP_REPOSITORY *AQBackupRepository_new();
void AQBackupRepository_free(AQB_BACKUP_REPOSITORY *c);

void AQBackupRepository_add(AQB_BACKUP_REPOSITORY *c,
			    AQB_BACKUP_REPOSITORY **head);
void AQBackupRepository_del(AQB_BACKUP_REPOSITORY *c,
			    AQB_BACKUP_REPOSITORY **head);
AQB_BACKUP_REPOSITORY *AQBackupRepository_fromConfig(CONFIGGROUP *cfg);
int AQBackupRepository_toConfig(AQB_BACKUP_REPOSITORY *r,
				CONFIGGROUP *cfg);


struct _AQBACKUP {
  AQB_BACKUP_REPOSITORY *repositories;
  char *hostName;

  /* runtime vars */
  AQB_CLIENT *currClient;
  AQB_BACKUP_REPOSITORY *currRepository;
  AQB_INTERACTOR *interactor;
};


int AQBackup__GetMD5(const char *filename,
		     char *buffer,
		     unsigned int maxsize);



/**
 * This function reads all information about the given file
 * and creates an entry from it.
 */
AQB_ENTRY *AQBackup_File2Entry(const char *filename);

/**
 * This functions scans a directory and creates an AQBEntry for every
 * file found.
 */
int AQBackup_Dir2Entries(AQB_ENTRY **head, const char *dirname,
			 int nonexistentisok);


/**
 * This function scans the given directory and compares it against
 * the directory stored within a repository.
 * If a file exists locally but not in the repository (or it exists locally
 * and in the repository but has been modified) then its entry
 * will be added to modifiedFiles (if it is a regular file, otherwise
 * it will be added to modifiedEntries).
 * IF a file exists in the repository but not locally it will be added to
 * deletedEntries.
 * This function also returns a list of local subdirectories below the
 * given one.
 */
int AQBackup_GetDiffEntriesForBackup(AQB_CLIENT *c,
				     int did,
				     const char *abspath,
                                     time_t when,
				     STRINGLIST *dirlist,
				     AQB_ENTRY **modifiedEntries,
				     AQB_ENTRY **modifiedFiles,
				     AQB_ENTRY **deletedEntries,
				     unsigned int flags);

int AQBackup_GetDiffEntriesForRestore(AQB_CLIENT *c,
				      int did,
				      const char *abspath,
				      time_t when,
				      STRINGLIST *dirlist,
				      AQB_ENTRY **modifiedEntries,
				      AQB_ENTRY **modifiedFiles,
				      AQB_ENTRY **newEntries,
				      AQB_ENTRY **deletedEntries,
				      unsigned int flags);


int AQBackup__FindInStringList(STRINGLIST *sl,
			       const char *p);
AQB_BACKUP_REPOSITORY *AQBackup__FindMatchingRepository(AQBACKUP *b,
							const char *dir);
int AQBackup__StoreFile(AQBACKUP *b,
			int did,
			const char *dir,
			AQB_ENTRY *e,
			unsigned int flags);

int AQBackup__StoreDir(AQBACKUP *b,
		       int did,
		       const char *relpath,
		       STRINGLIST *sl,
		       unsigned int flags);
int AQBackup__ShowDiffs(AQBACKUP *b,
			int did,
			const char *abspath,
                        time_t when,
			STRINGLIST *sl,
			unsigned int flags);

int AQBackup__CompareEntries(AQB_ENTRY *el, AQB_ENTRY *er,
			     AQB_BACKUP_TIME_COMPARE_MODE cmode);

int AQBackup__ModifyDirEnt(const char *absname,
			   AQB_ENTRY *e,
			   unsigned int flags);

int AQBackup__RestoreFile(AQBACKUP *b,
			  int did,
			  const char *destdir,
			  const char *fname,
			  time_t when,
			  unsigned int flags);

int AQBackup__RestoreDir(AQBACKUP *b,
			 int did,
			 const char *abspath,
			 const char *destdir,
			 STRINGLIST *sl,
                         time_t when,
			 unsigned int flags,
			 AQB_ENTRY **chmodEntries);

int AQBackup__CreateDirEnt(const char *absname,
			   AQB_ENTRY *e,
			   unsigned int flags);

int AQBackup__Create(AQBACKUP *b,
		     AQB_BACKUP_REPOSITORY *r);


#endif



