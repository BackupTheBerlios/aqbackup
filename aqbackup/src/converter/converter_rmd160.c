/***************************************************************************
 $RCSfile: converter_rmd160.c,v $
                             -------------------
    cvs         : $Id: converter_rmd160.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#include "converter_rmd160_p.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>


CONVERTER_RMD160_DATA *ConverterRmd160Data_new(){
  CONVERTER_RMD160_DATA *dzd;

  dzd=(CONVERTER_RMD160_DATA *)malloc(sizeof(CONVERTER_RMD160_DATA));
  assert(dzd);
  memset(dzd, 0, sizeof(CONVERTER_RMD160_DATA));
  return dzd;
}


void ConverterRmd160Data_free(CONVERTER_RMD160_DATA *dzd){
  if (dzd) {
    free(dzd->ctx);
    free(dzd);
  }
}



int ConverterRmd160_Begin(CONVERTER *dm){
  CONVERTER_RMD160_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);

  /* create context */
  dzd->ctx=(RIPEMD160_CTX*)malloc(sizeof(RIPEMD160_CTX));
  assert(dzd->ctx);
  memset(dzd->ctx, 0, sizeof(RIPEMD160_CTX));

  RIPEMD160_Init(dzd->ctx);

  return CONVERTER_RESULT_OK;
}



unsigned int ConverterRmd160_HasData(CONVERTER *dm){
  CONVERTER_RMD160_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (dzd->done)
    return sizeof(dzd->rmd)-dzd->nextPosForGetData;
  return 0;
}



unsigned int ConverterRmd160_NeedsData(CONVERTER *dm){
  CONVERTER_RMD160_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (dzd->eof)
    return 0;
  return 1024;
}



int ConverterRmd160_SetData(CONVERTER *dm,
			    const char *data,
			    unsigned int size){
  CONVERTER_RMD160_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);
  assert(dzd->ctx);

  /* check if eof was set */
  if (dzd->eof) {
    DBG_ERROR("Eof already set, will not accept data");
    return 0;
  }

  /* check whether EOF is to be set (indicated by size=0) */
  if (size==0) {
    dzd->eof=1;
    dzd->done=1;
    DBG_DEBUG("EOF set");
    RIPEMD160_Final(dzd->rmd, dzd->ctx);
    return 0;
  }
  else {
    RIPEMD160_Update(dzd->ctx,
		     data,
		     size);
    return size;
  }
}



int ConverterRmd160_GetData(CONVERTER *dm,
			    char *data,
			    unsigned int size){
  CONVERTER_RMD160_DATA *dzd;
  unsigned int i;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (!(dzd->done)) {
    DBG_ERROR("No data");
    return CONVERTER_RESULT_ERROR;
  }

  i=sizeof(dzd->rmd)-dzd->nextPosForGetData;
  if (size>i)
    size=i;
  if (size) {
    assert(data);
    memmove(data, dzd->rmd+dzd->nextPosForGetData, size);
  }
  dzd->nextPosForGetData+=size;
  return size;
}



int ConverterRmd160_Work(CONVERTER *dm){
  CONVERTER_RMD160_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);
  assert(dzd->ctx);

  if (dzd->done)
    return CONVERTER_RESULT_EOF;
  return CONVERTER_RESULT_OK;
}



int ConverterRmd160_End(CONVERTER *dm){
  CONVERTER_RMD160_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_RMD160_DATA *)(dm->privateData);
  assert(dzd->ctx);

  free(dzd->ctx);
  dzd->ctx=0;
  return CONVERTER_RESULT_OK;
}



void ConverterRmd160_FreePrivate(CONVERTER *dm){
  assert(dm);
  if (dm->privateData)
    ConverterRmd160Data_free((CONVERTER_RMD160_DATA*)(dm->privateData));
}


CONVERTER *ConverterRmd160_new(int level){
  CONVERTER *dm;
  CONVERTER_RMD160_DATA *dzd;

  dm=Converter_new();
  dzd=ConverterRmd160Data_new();
  dm->privateData=dzd;

  /* set function pointers */
  dm->beginPtr=ConverterRmd160_Begin;
  dm->endPtr=ConverterRmd160_End;
  dm->hasDataPtr=ConverterRmd160_HasData;
  dm->needsDataPtr=ConverterRmd160_NeedsData;
  dm->setDataPtr=ConverterRmd160_SetData;
  dm->getDataPtr=ConverterRmd160_GetData;
  dm->workPtr=ConverterRmd160_Work;
  dm->freePrivatePtr=ConverterRmd160_FreePrivate;

  dm->typename=strdup("Rmd160");
  return dm;
}






