/***************************************************************************
 $RCSfile: interactor.c,v $
                             -------------------
    cvs         : $Id: interactor.c,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "interactor_p.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>



AQB_INTERACTOR *AQBInteractor_new(){
  AQB_INTERACTOR *i;

  i=(AQB_INTERACTOR *)malloc(sizeof(AQB_INTERACTOR));
  assert(i);
  memset(i, 0, sizeof(AQB_INTERACTOR));
  return i;
}


void AQBInteractor_free(AQB_INTERACTOR *ia){
  if (ia) {
    if (ia->freeDataPtr)
      ia->freeDataPtr(ia);
    free(ia);
  }
}



INTERACTOR_CHECK_RESULT AQBInteractor_EnterDir(AQB_INTERACTOR *ia,
					       const char *name){
  assert(ia);
  if (ia->enterDirPtr)
    return ia->enterDirPtr(ia, name);
  else {
    if (ia->level==InteractorLevelVeryVerbous)
      fprintf(stdout, "Dir      %s\n", name);
    else if (ia->level>=InteractorLevelExtremelyVerbous)
      fprintf(stdout, "Enter    %s\n", name);
    return CheckResultContinue;
  }
}


void AQBInteractor_LeaveDir(AQB_INTERACTOR *ia,
			    const char *name){
  assert(ia);
  if (ia->leaveDirPtr)
    ia->leaveDirPtr(ia, name);
  else {
    if (ia->level>=InteractorLevelExtremelyVerbous)
      fprintf(stdout, "Leave    %s\n", name);
  }
}



void AQBInteractor_StartWriteFile(AQB_INTERACTOR *ia,
				  const AQB_ENTRY *e){
  assert(ia);
  if (ia->startWriteFilePtr)
    ia->startWriteFilePtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Write   %s\n", e->abspath);
  }
}



void AQBInteractor_WriteFile(AQB_INTERACTOR *ia,
			     unsigned int bytesWritten){
  assert(ia);
  if (ia->writeFilePtr)
    ia->writeFilePtr(ia, bytesWritten);
  else {
    if (ia->level>=InteractorLevelExtremelyVerbous)
      fprintf(stdout, " Written %d bytes\r", bytesWritten);
  }
}



void AQBInteractor_StopWriteFile(AQB_INTERACTOR *ia){
  assert(ia);
  if (ia->stopWriteFilePtr)
    ia->stopWriteFilePtr(ia);
  else {
    if (ia->level>=InteractorLevelExtremelyVerbous)
      fprintf(stdout,"\n");
    if (ia->level>=InteractorLevelVeryVerbous)
      fprintf(stdout, " Writing finished.\n");
  }
}



void AQBInteractor_StartReadFile(AQB_INTERACTOR *ia,
				 const AQB_ENTRY *e){
  assert(ia);
  if (ia->startReadFilePtr)
    ia->startReadFilePtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Read    %s\n", e->abspath);
  }
}



void AQBInteractor_ReadFile(AQB_INTERACTOR *ia,
			    unsigned int bytesRead){
  assert(ia);
  if (ia->readFilePtr)
    ia->readFilePtr(ia, bytesRead);
  else {
    if (ia->level>=InteractorLevelExtremelyVerbous)
      fprintf(stdout, " Read    %d bytes\n", bytesRead);
  }
}



void AQBInteractor_StopReadFile(AQB_INTERACTOR *ia) {
  assert(ia);
  if (ia->stopReadFilePtr)
    ia->stopReadFilePtr(ia);
  else {
    if (ia->level>=InteractorLevelVeryVerbous)
      fprintf(stdout, " Reading finished.\n");
  }
}



void AQBInteractor_LogStr(AQB_INTERACTOR *ia,
			  const char *s,
			  INTERACTOR_LEVEL i){
  assert(ia);
  if (ia->logStrPtr)
    ia->logStrPtr(ia, s, i);
  else {
    if (ia->level>=i)
      fprintf(stdout, "%s",s);
  }
}



void AQBInteractor_LogStr2(AQB_INTERACTOR *ia,
			   const char *s1,
			   const char *s2,
			   INTERACTOR_LEVEL i){
  char *buffer;
  unsigned int size;

  assert(ia);
  assert(s1);
  assert(s2);

  size=strlen(s1)+strlen(s2)+1;
  buffer=(char*)malloc(size);
  assert(buffer);
  strcpy(buffer, s1);
  strcat(buffer, s2);

  if (ia->logStrPtr)
    ia->logStrPtr(ia, buffer, i);
  else {
    if (ia->level>=i)
      fprintf(stdout, "%s\n",buffer);
  }
  free(buffer);
}



void AQBInteractor_SetLevel(AQB_INTERACTOR *ia, INTERACTOR_LEVEL i){
  assert(ia);
  ia->level=i;
}



INTERACTOR_CHECK_RESULT AQBInteractor_CheckAbort(AQB_INTERACTOR *ia){
  assert(ia);
  if (ia->checkAbortPtr)
    return ia->checkAbortPtr(ia);
  return CheckResultContinue;
}


void AQBInteractor_Modifying(AQB_INTERACTOR *ia,
			     const AQB_ENTRY *e){
  assert(ia);
  if (ia->modifyingPtr)
    ia->modifyingPtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Modify  %s\n", e->abspath);
  }
}



void AQBInteractor_Creating(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e){
  assert(ia);
  if (ia->creatingPtr)
    ia->creatingPtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Create  %s\n", e->abspath);
  }
}



void AQBInteractor_Deleting(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e){
  assert(ia);
  if (ia->deletingPtr)
    ia->deletingPtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Delete  %s\n", e->abspath);
  }
}



void AQBInteractor_Removing(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e){
  assert(ia);
  if (ia->removingPtr)
    ia->removingPtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Remove  %s\n", e->abspath);
  }
}


void AQBInteractor_Updating(AQB_INTERACTOR *ia,
			    const AQB_ENTRY *e){
  assert(ia);
  if (ia->updatingPtr)
    ia->updatingPtr(ia, e);
  else {
    if (ia->level>=InteractorLevelVerbous)
      fprintf(stdout, " Update  %s\n", e->abspath);
  }
}






