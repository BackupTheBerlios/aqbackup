/***************************************************************************
 $RCSfile: interactor.h,v $
                             -------------------
    cvs         : $Id: interactor.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
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


#ifndef AQBACKUP_INTERACTOR_H
#define AQBACKUP_INTERACTOR_H

#include <backup/entry.h>


typedef enum {
  CheckResultContinue=0,
  CheckResultAbort,
  CheckResultSkip
} INTERACTOR_CHECK_RESULT;


typedef enum {
  InteractorLevelMute=0,
  InteractorLevelVerbous,
  InteractorLevelVeryVerbous,
  InteractorLevelExtremelyVerbous
} INTERACTOR_LEVEL;


typedef struct _AQB_INTERACTOR AQB_INTERACTOR;


AQB_INTERACTOR *AQBInteractor_new();
void AQBInteractor_free(AQB_INTERACTOR *ia);


INTERACTOR_CHECK_RESULT AQBInteractor_EnterDir(AQB_INTERACTOR *ia,
					       const char *name);
void AQBInteractor_LeaveDir(AQB_INTERACTOR *ia,
			    const char *name);

void AQBInteractor_StartWriteFile(AQB_INTERACTOR *ia,
				  const AQB_ENTRY *e);
void AQBInteractor_WriteFile(AQB_INTERACTOR *ia,
			     unsigned int bytesWritten);
void AQBInteractor_StopWriteFile(AQB_INTERACTOR *ia);

void AQBInteractor_StartReadFile(AQB_INTERACTOR *ia,
				 const AQB_ENTRY *e);
void AQBInteractor_ReadFile(AQB_INTERACTOR *ia,
			    unsigned int bytesRead);
void AQBInteractor_StopReadFile(AQB_INTERACTOR *ia);

void AQBInteractor_LogStr(AQB_INTERACTOR *ia,
			  const char *s, INTERACTOR_LEVEL level);
void AQBInteractor_LogStr2(AQB_INTERACTOR *ia,
			   const char *s1,
                           const char *s2,
			   INTERACTOR_LEVEL level);

INTERACTOR_CHECK_RESULT AQBInteractor_CheckAbort(AQB_INTERACTOR *ia);

void AQBInteractor_Modifying(AQB_INTERACTOR *ia,
			     const AQB_ENTRY *e);
void AQBInteractor_Creating(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e);

/* deleting entry in repository, no local deleting ! */
void AQBInteractor_Deleting(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e);
void AQBInteractor_Updating(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e);

/* removing locally */
void AQBInteractor_Removing(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e);


void AQBInteractor_SetLevel(AQB_INTERACTOR *ia, INTERACTOR_LEVEL i);


#endif


