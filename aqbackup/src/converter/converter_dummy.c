/***************************************************************************
 $RCSfile: converter_dummy.c,v $
                             -------------------
    cvs         : $Id: converter_dummy.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#include "converter_dummy_p.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


CONVERTER_DUMMY_DATA *ConverterDummyData_new(){
  CONVERTER_DUMMY_DATA *dzd;

  dzd=(CONVERTER_DUMMY_DATA *)malloc(sizeof(CONVERTER_DUMMY_DATA));
  assert(dzd);
  memset(dzd, 0, sizeof(CONVERTER_DUMMY_DATA));
  return dzd;
}


void ConverterDummyData_free(CONVERTER_DUMMY_DATA *dzd){
  if (dzd) {
    free(dzd);
  }
}



int ConverterDummy_Begin(CONVERTER *dm){
  CONVERTER_DUMMY_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  dzd->nextPosForGetData=0;
  dzd->eof=0;
  dzd->bytesinbuffer=0;

  return CONVERTER_RESULT_OK;
}



unsigned int ConverterDummy_HasData(CONVERTER *dm){
  CONVERTER_DUMMY_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  return dzd->bytesinbuffer-dzd->nextPosForGetData;
}



unsigned int ConverterDummy_NeedsData(CONVERTER *dm){
  CONVERTER_DUMMY_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  if (dzd->eof)
    return 0;

  if (dzd->nextPosForGetData<dzd->bytesinbuffer)
    return 0;

  return CONVERTER_DUMMY_CHUNKSIZE;
}



int ConverterDummy_SetData(CONVERTER *dm,
			    const char *data,
			    unsigned int size){
  CONVERTER_DUMMY_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  if (dzd->eof)
    return 0;

  if (size==0) {
    dzd->eof=1;
    DBG_NOTICE("EOF set");
    return 0;
  }

  if (dzd->nextPosForGetData<dzd->bytesinbuffer) {
    DBG_DEBUG("Still data in the buffer");
    return 0;
  }

  if (size>CONVERTER_DUMMY_CHUNKSIZE)
    size=CONVERTER_DUMMY_CHUNKSIZE;

  if (size) {
    memmove(dzd->outbuffer, data, size);
    dzd->nextPosForGetData=0;
    dzd->bytesinbuffer=size;
  }
  return size;
}



int ConverterDummy_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size){
  CONVERTER_DUMMY_DATA *dzd;
  unsigned int i;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  i=dzd->bytesinbuffer-dzd->nextPosForGetData;
  if (size>i)
    size=i;
  if (size) {
    assert(data);
    memmove(data, dzd->outbuffer+dzd->nextPosForGetData, size);
    dzd->nextPosForGetData+=size;
  }
  return size;
}



int ConverterDummy_Work(CONVERTER *dm){
  CONVERTER_DUMMY_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  if (dzd->eof)
    return CONVERTER_RESULT_EOF;

  return CONVERTER_RESULT_OK;
}



int ConverterDummy_End(CONVERTER *dm){
  CONVERTER_DUMMY_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_DUMMY_DATA *)(dm->privateData);

  return CONVERTER_RESULT_OK;
}



void ConverterDummy_FreePrivate(CONVERTER *dm){
  assert(dm);
  if (dm->privateData)
    ConverterDummyData_free((CONVERTER_DUMMY_DATA*)(dm->privateData));
}


CONVERTER *ConverterDummy_new(){
  CONVERTER *dm;
  CONVERTER_DUMMY_DATA *dzd;

  dm=Converter_new();
  dzd=ConverterDummyData_new();
  dm->privateData=dzd;

  /* set function pointers */
  dm->beginPtr=ConverterDummy_Begin;
  dm->endPtr=ConverterDummy_End;
  dm->hasDataPtr=ConverterDummy_HasData;
  dm->needsDataPtr=ConverterDummy_NeedsData;
  dm->setDataPtr=ConverterDummy_SetData;
  dm->getDataPtr=ConverterDummy_GetData;
  dm->workPtr=ConverterDummy_Work;
  dm->freePrivatePtr=ConverterDummy_FreePrivate;
  dm->typename=strdup("Dummy");
  return dm;
}






