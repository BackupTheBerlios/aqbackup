/***************************************************************************
 $RCSfile: client_p.h,v $
                             -------------------
    cvs         : $Id: client_p.h,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
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


#ifndef AQBACKUP_CLIENT_P_H
#define AQBACKUP_CLIENT_P_H

#include "client.h"


typedef int (*AQB_CLIENT_INIT_PTR)(AQB_CLIENT *c, CONFIGGROUP *cfg);
typedef int (*AQB_CLIENT_FINI_PTR)(AQB_CLIENT *c);

typedef int (*AQB_CLIENT_REGISTER_PTR)(AQB_CLIENT *c, const char *name);
typedef int (*AQB_CLIENT_UNREGISTER_PTR)(AQB_CLIENT *c);
typedef int (*AQB_CLIENT_OPENREPOSITORY_PTR)(AQB_CLIENT *c,
					     const char *hostname,
					     const char *repository,
					     int writeAccess);
typedef int (*AQB_CLIENT_CREATEREPOSITORY_PTR)(AQB_CLIENT *c,
					       const char *hostname,
					       const char *repository);
typedef int (*AQB_CLIENT_CLOSEREPOSITORY_PTR)(AQB_CLIENT *c);
typedef int (*AQB_CLIENT_OPENDIR_PTR)(AQB_CLIENT *c,
				      const char *dir);
typedef int (*AQB_CLIENT_CLOSEDIR_PTR)(AQB_CLIENT *c,
				       int did);

typedef int (*AQB_CLIENT_GETENTRIES_PTR)(AQB_CLIENT *c,
					 int did,
                                         time_t when,
					 AQB_ENTRY **entries);
typedef int (*AQB_CLIENT_SETENTRY_PTR)(AQB_CLIENT *c,
				       int did,
				       AQB_ENTRY *entry);
typedef int (*AQB_CLIENT_DELETEENTRY_PTR)(AQB_CLIENT *c,
					  int did,
					  AQB_ENTRY *entry);
typedef int (*AQB_CLIENT_FILEOPENOUT_PTR)(AQB_CLIENT *c,
					  int did,
					  AQB_ENTRY *entry);
typedef int (*AQB_CLIENT_FILEWRITE_PTR)(AQB_CLIENT *c,
					int fid,
					const char *data,
					unsigned int size);
typedef int (*AQB_CLIENT_FILECLOSEOUT_PTR)(AQB_CLIENT *c,
					   int fid, const char *md5);

typedef int (*AQB_CLIENT_FILEOPENIN_PTR)(AQB_CLIENT *c,
					 int did,
					 const char *name,
					 time_t when,
					 AQB_ENTRY **entry);
typedef int (*AQB_CLIENT_FILEREAD_PTR)(AQB_CLIENT *c,
				       int fid,
				       char *data,
				       unsigned int size);
typedef int (*AQB_CLIENT_FILECLOSEIN_PTR)(AQB_CLIENT *c,
					  int fid);



typedef void (*AQB_CLIENT_FREEPRIVATE_PTR)(AQB_CLIENT *c);



struct _AQB_CLIENT {
  AQB_CLIENT *next;
  AQB_INTERACTOR *interactor; /* not owned ! It is owned by AQB_BACKUP */

  AQB_CLIENT_INIT_PTR initPtr;
  AQB_CLIENT_FINI_PTR finiPtr;
  AQB_CLIENT_REGISTER_PTR registerPtr;
  AQB_CLIENT_UNREGISTER_PTR unregisterPtr;
  AQB_CLIENT_OPENREPOSITORY_PTR openRepositoryPtr;
  AQB_CLIENT_CREATEREPOSITORY_PTR createRepositoryPtr;
  AQB_CLIENT_CLOSEREPOSITORY_PTR closeRepositoryPtr;
  AQB_CLIENT_OPENDIR_PTR openDirPtr;
  AQB_CLIENT_CLOSEDIR_PTR closeDirPtr;
  AQB_CLIENT_GETENTRIES_PTR getEntriesPtr;
  AQB_CLIENT_SETENTRY_PTR setEntryPtr;
  AQB_CLIENT_DELETEENTRY_PTR deleteEntryPtr;
  AQB_CLIENT_FILEOPENOUT_PTR fileOpenoutPtr;
  AQB_CLIENT_FILEWRITE_PTR fileWritePtr;
  AQB_CLIENT_FILECLOSEOUT_PTR fileCloseoutPtr;
  AQB_CLIENT_FILEOPENIN_PTR fileOpeninPtr;
  AQB_CLIENT_FILEREAD_PTR fileReadPtr;
  AQB_CLIENT_FILECLOSEIN_PTR fileCloseinPtr;

  AQB_CLIENT_FREEPRIVATE_PTR freePrivatePtr;

  int cid;
  int rid;
  void *privateData;
};



#endif


