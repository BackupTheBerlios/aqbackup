/***************************************************************************
 $RCSfile: converter.c,v $
                             -------------------
    cvs         : $Id: converter.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#include "converter.h"
#include "backup/misc.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>


#include <stdlib.h>
#include <string.h>
#include <assert.h>



CONVERTER *Converter_new(){
  CONVERTER *dm;

  dm=(CONVERTER *)malloc(sizeof(CONVERTER));
  assert(dm);
  memset(dm, 0, sizeof(CONVERTER));
  return dm;
}


void Converter_free(CONVERTER *dm){
  if (dm) {
    if (dm->freePrivatePtr)
      dm->freePrivatePtr(dm);
    free(dm->typename);
    free(dm);
  }
}



void Converter_add(CONVERTER *dm, CONVERTER **head){
  AQLIST_ADD(CONVERTER, dm, head);
}



void Converter_del(CONVERTER *dm, CONVERTER **head){
  AQLIST_DEL(CONVERTER, dm, head);
}



int Converter_Begin(CONVERTER *dm){
  assert(dm);
  assert(dm->beginPtr);
  dm->notAllDataTaken=0;
  dm->bytesReceived=0;
  dm->bytesSent=0;
  return dm->beginPtr(dm);
}


unsigned int Converter_HasData(CONVERTER *dm){
  assert(dm);
  assert(dm->hasDataPtr);
  if (dm->notAllDataTaken)
    /* converter is out of sync, so data is corrupted */
    return 0;
  return dm->hasDataPtr(dm);
}



unsigned int Converter_NeedsData(CONVERTER *dm){
  int rv;

  assert(dm);
  assert(dm->needsDataPtr);
  rv=dm->needsDataPtr(dm);
  DBG_DEBUG("Node \"%s\" needs %d bytes", dm->typename, rv);
  return rv;
}



int Converter_SetData(CONVERTER *dm,
			 const char *data,
		      unsigned int size){
  int rv;

  assert(dm);
  assert(dm->setDataPtr);
  rv=dm->setDataPtr(dm, data, size);
  if (rv>0)
    dm->bytesSent+=rv;
  DBG_INFO("Bytes sent to \"%s\": %d (total %d)",
	   dm->typename, rv, dm->bytesSent);
  return rv;
}


int Converter_GetData(CONVERTER *dm,
		      char *data,
		      unsigned int size){
  int rv;

  assert(dm);
  assert(dm->getDataPtr);
  if (dm->notAllDataTaken) {
    /* converter is out of sync, so data is corrupted */
    DBG_WARN("Converter out of sync (%s)",
	     dm->typename);
    return 0;
  }
  rv=dm->getDataPtr(dm, data, size);
  if (rv>0)
    dm->bytesReceived+=rv;
  DBG_INFO("Bytes received from \"%s\": %d (total %d)",
	   dm->typename, rv, dm->bytesReceived);
  return rv;
}


int Converter_Work(CONVERTER *dm){
  assert(dm);
  assert(dm->workPtr);
  return dm->workPtr(dm);
}


int Converter_End(CONVERTER *dm){
  int rv;

  assert(dm);
  assert(dm->endPtr);
  rv=Converter_HasData(dm);
  if (rv!=0) {
    DBG_WARN("Still %d bytes available", rv);
  }
  DBG_INFO("Statistics for converter \"%s\":\n"
	   " Bytes received: %d\n"
	   " Bytes sent    : %d",
	   dm->typename,
	   dm->bytesReceived,
	   dm->bytesSent);
  return dm->endPtr(dm);
}










int ConverterGroup_Begin(CONVERTER *dm){
  int rv;

  rv=0;
  while(dm) {
    CONVERTER *tee;

    /* first let the node itself begin */
    rv|=Converter_Begin(dm);
    tee=dm->tee;
    while(tee) {
      /* then let all tee nodes begin (RECURSION !) */
      rv|=ConverterGroup_Begin(tee);
      tee=tee->next;
    } /* while tee */
    dm=dm->next;
  } /* while */

  return rv;
}


unsigned int ConverterGroup_HasData(CONVERTER *dm){
  DBG_DEBUG("HasData");
  if (dm) {
    /* return the available data of the last in the chain
     * No TEE handling here, because all TEE nodes are just additional,
     * like MD5 sum calculator which work in parallel with compression
     */
    while(dm->next)
      dm=dm->next;
    return Converter_HasData(dm);
  } /* while */

  return 0;
}



unsigned int ConverterGroup_NeedsData(CONVERTER *dm){
  int size;

  DBG_DEBUG("NeedsData");
  size=0;
  if (dm) {
    CONVERTER *tee;
    int localsize;

    /* first the node itself */
    size=Converter_NeedsData(dm);
    tee=dm->tee;
    while(tee) {
      /* then all tee nodes */
      localsize=Converter_NeedsData(tee);
      if (localsize<size)
	size=localsize;
      tee=tee->next;
    } /* while tee */
  } /* if dm */

  return size;
}



int ConverterGroup_SetData(CONVERTER *dm,
			   const char *data,
			   unsigned int size){
  unsigned int i;
  unsigned int taken;

  assert(dm);

  DBG_DEBUG("SetData %d bytes (%x)", size, (unsigned int)dm);
  taken=i=0;
  if (size)
    i=ConverterGroup_NeedsData(dm);
  DBG_DEBUG("Needs data : %d (got %d)", i, size);
  if (i>=size || size==0) {
    CONVERTER *tee;
    int localsize;

    /* first the node itself */
    taken=Converter_SetData(dm, data, size);

    tee=dm->tee;
    while(tee) {
      int setNotAllTaken;

      setNotAllTaken=0;
      /* then all tee nodes */
      localsize=Converter_SetData(tee, data, size);
      if (localsize!=size) {
	DBG_ERROR("Uuups, one converter did not accept all the data !");
	setNotAllTaken=1;
      }
      if (setNotAllTaken)
	tee->notAllDataTaken++;
      tee=tee->next;
    } /* while tee */
  } /* if dm */

  if (taken!=size) {
    CONVERTER *dmn;

    DBG_ERROR("Uuups, this converter did not accept all the data,"
	      " will tell his brothers (%d!=%d, %d)", taken, size, i);
    dmn=dm;
    while(dmn) {
      dmn->notAllDataTaken++;
      dmn=dmn->next;
    }
  }

  DBG_DEBUG("Data taken: %d (from %d)", taken, size);
  return taken;
}


int ConverterGroup_GetData(CONVERTER *dm,
			   char *data,
			   unsigned int size){
  DBG_DEBUG("GetData");
  if (dm) {
    int rv;

    /* return the available data of the last in the chain
     * No TEE handling here, because all TEE nodes are just additional,
     * like MD5 sum calculator which work in parallel with compression
     */
    while(dm->next)
      dm=dm->next;
    rv=Converter_GetData(dm, data, size);
    DBG_DEBUG("Returning %d bytes from \"%s\"", rv, dm->typename);
    return rv;
  } /* while */

  return 0;
}



int ConverterGroup_End(CONVERTER *dm){
  int rv;

  rv=0;
  while(dm) {
    CONVERTER *tee;

    /* first let the node itself end */
    rv|=Converter_End(dm);
    tee=dm->tee;
    while(tee) {
      /* then let all tee nodes end (RECURSION !) */
      rv|=ConverterGroup_End(tee);
      tee=tee->next;
    } /* while tee */
    dm=dm->next;
  } /* while */

  return rv;
}



int ConverterGroup_HasAllTaken(CONVERTER *dm){
  assert(dm);
  return dm->notAllDataTaken==0;
}



void ConverterGroup_Append(CONVERTER *dm, CONVERTER *newc){
  Converter_add(newc, &(dm->next));
}



void ConverterGroup_Tee(CONVERTER *dm, CONVERTER *newc){
  Converter_add(newc, &(dm->tee));
}



void ConverterGroup_free(CONVERTER *dm){
  while(dm) {
    CONVERTER *dmnext, *tee;

    dmnext=dm->next;

    tee=dm->tee;
    while(tee) {
      CONVERTER *teenext;

      teenext=tee->next;
      Converter_free(tee);
      tee=teenext;
    } /* while */
    Converter_free(dm);
    dm=dmnext;
  } /* while */
}



int ConverterGroup_Work(CONVERTER *dm){
  int rv;

  DBG_DEBUG("Work (%x)", (unsigned int)dm);

  rv=0;
  while(dm) {
    CONVERTER *tee;
    CONVERTER *next;
    int hasData;
    int wantsData;

    next=dm->next;

    /* first let the node itself work */

    DBG_DEBUG("Checking node (%s)", dm->typename);
    hasData=Converter_HasData(dm);
    if (hasData<1) {
      int rv;

      if (dm->eof==0) {
	/* let the node work */
	DBG_DEBUG("Node has no data, working (%s)", dm->typename);
	rv=Converter_Work(dm);
	if (rv==CONVERTER_RESULT_EOF) {
	  DBG_DEBUG("Node has finished (%s)", dm->typename);
	  dm->eof=1;
	}
	else if (rv==CONVERTER_RESULT_ERROR) {
	  DBG_ERROR("Error on %s", dm->typename);
	  return rv;
	}
	hasData=Converter_HasData(dm);
      }
      else {
	DBG_DEBUG("Converter %s already finished, skip work",
		   dm->typename);
      }
    } /* if does not have data */

    if (hasData==0) {
      if (dm->eof==0) {
	DBG_DEBUG("Node still has no data (%s)", dm->typename);
	return CONVERTER_RESULT_OK;
      }
      else {
	if (next) {
	  DBG_DEBUG("End of stream, informing \"%s\"", next->typename);
	  ConverterGroup_SetData(next, 0, 0);
	}
	else {
	  DBG_DEBUG("Last one has finished");
	  return CONVERTER_RESULT_EOF;
	}
      }
    }

    if (next && hasData) {
      /* check for successors */
      wantsData=ConverterGroup_NeedsData(next);
      DBG_DEBUG("Next group wants %d bytes (%s)",
                 wantsData, next->typename);
      if (wantsData==0) {
	DBG_DEBUG("Next node does not need data, doing nothing (%s)",
		   next->typename);
      }
      else {
	char buffer[3333];
	int i;
	int j;

	i=wantsData;
	if (i>hasData)
	  i=hasData;
	if (i>sizeof(buffer))
	  i=sizeof(buffer);

	rv=Converter_GetData(dm, buffer, i);
	if (rv==0) {
	  DBG_DEBUG("Node did not return data (%s)",dm->typename);
	  return CONVERTER_RESULT_OK;
	}
	j=rv;
	rv=ConverterGroup_SetData(next, buffer, j);
	if (rv!=j) {
	  DBG_ERROR("Next node did not take all my data (%s: %d of %d)",
		    next->typename, rv, j);
	  return CONVERTER_RESULT_ERROR;
	}
      }
    } /* if next */

    /* let all TEEs work */
    DBG_DEBUG("Letting all TEEs work");
    tee=dm->tee;
    while(tee) {
      int localrv;

      /* then let all tee nodes work (RECURSION !) */
      localrv=ConverterGroup_Work(tee);
      if (localrv==CONVERTER_RESULT_ERROR) {
	DBG_WARN("Error in a side branch (%s)", tee->typename);
	return CONVERTER_RESULT_SIDE_ERROR;
      }
      tee=tee->next;
    } /* while tee */


    dm=next;
  } /* while */

  DBG_DEBUG("Work (%x) done", (unsigned int)dm);
  return CONVERTER_RESULT_OK;
}



unsigned int ConverterGroup_BytesReceived(CONVERTER *dm) {
  assert(dm);

  /* return the received data of the last in the chain
   * No TEE handling here, because all TEE nodes are just additional,
   * like MD5 sum calculator which work in parallel with compression
   */
  while(dm->next)
    dm=dm->next;
  return dm->bytesReceived;
}



unsigned int ConverterGroup_BytesSent(CONVERTER *dm) {
  assert(dm);

  /* return the available data of the first in the chain
   * No TEE handling here, because all TEE nodes are just additional,
   * like MD5 sum calculator which work in parallel with compression
   */
  return dm->bytesSent;
}




