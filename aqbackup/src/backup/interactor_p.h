/***************************************************************************
 $RCSfile: interactor_p.h,v $
                             -------------------
    cvs         : $Id: interactor_p.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
    begin       : Fri Jun 06 2003
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


#ifndef AQBACKUP_INTERACTOR_P_H
#define AQBACKUP_INTERACTOR_P_H


#include <backup/interactor.h>
#include <backup/entry.h>


typedef INTERACTOR_CHECK_RESULT
  (*AQB_INTERACTOR_ENTERDIR_PTR)(AQB_INTERACTOR *ia,
				 const char *name);
typedef void (*AQB_INTERACTOR_LEAVEDIR_PTR)(AQB_INTERACTOR *ia,
					    const char *name);

typedef void (*AQB_INTERACTOR_STARTWRITEFILE_PTR)(AQB_INTERACTOR *ia,
						  const AQB_ENTRY *e);
typedef void (*AQB_INTERACTOR_WRITEFILE_PTR)(AQB_INTERACTOR *ia,
					     unsigned int bytesWritten);
typedef void (*AQB_INTERACTOR_STOPWRITEFILE_PTR)(AQB_INTERACTOR *ia);

typedef void (*AQB_INTERACTOR_STARTREADFILE_PTR)(AQB_INTERACTOR *ia,
						 const AQB_ENTRY *e);
typedef void (*AQB_INTERACTOR_READFILE_PTR)(AQB_INTERACTOR *ia,
					    unsigned int bytesRead);
typedef void (*AQB_INTERACTOR_STOPREADFILE_PTR)(AQB_INTERACTOR *ia);

typedef void (*AQB_INTERACTOR_LOGSTR_PTR)(AQB_INTERACTOR *ia,
					  const char *s,
					  INTERACTOR_LEVEL level);

typedef void (*AQB_INTERACTOR_MODIFYING_PTR)(AQB_INTERACTOR *ia,
					     const AQB_ENTRY *e);
typedef void (*AQB_INTERACTOR_CREATING_PTR)(AQB_INTERACTOR *ia,
					    const AQB_ENTRY *e);
typedef void (*AQB_INTERACTOR_DELETING_PTR)(AQB_INTERACTOR *ia,
					    const AQB_ENTRY *e);
typedef void (*AQB_INTERACTOR_REMOVING_PTR)(AQB_INTERACTOR *ia,
					    const AQB_ENTRY *e);
typedef void (*AQB_INTERACTOR_UPDATING_PTR)(AQB_INTERACTOR *ia,
					    const AQB_ENTRY *e);


typedef void (*AQB_INTERACTOR_FREEDATA_PTR)(AQB_INTERACTOR *ia);

typedef INTERACTOR_CHECK_RESULT
  (*AQB_INTERACTOR_CHECKABORT_PTR)(AQB_INTERACTOR *ia);


struct _AQB_INTERACTOR {
  AQB_INTERACTOR_ENTERDIR_PTR enterDirPtr;
  AQB_INTERACTOR_LEAVEDIR_PTR leaveDirPtr;

  AQB_INTERACTOR_STARTWRITEFILE_PTR startWriteFilePtr;
  AQB_INTERACTOR_WRITEFILE_PTR writeFilePtr;
  AQB_INTERACTOR_STOPWRITEFILE_PTR stopWriteFilePtr;

  AQB_INTERACTOR_STARTREADFILE_PTR startReadFilePtr;
  AQB_INTERACTOR_READFILE_PTR readFilePtr;
  AQB_INTERACTOR_STOPREADFILE_PTR stopReadFilePtr;

  AQB_INTERACTOR_LOGSTR_PTR logStrPtr;

  AQB_INTERACTOR_CHECKABORT_PTR checkAbortPtr;
  AQB_INTERACTOR_MODIFYING_PTR modifyingPtr;
  AQB_INTERACTOR_CREATING_PTR creatingPtr;
  AQB_INTERACTOR_DELETING_PTR deletingPtr;
  AQB_INTERACTOR_REMOVING_PTR removingPtr;
  AQB_INTERACTOR_UPDATING_PTR updatingPtr;

  AQB_INTERACTOR_FREEDATA_PTR freeDataPtr;
  void *privateData;
  INTERACTOR_LEVEL level;
};





#endif


