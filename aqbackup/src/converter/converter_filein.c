/***************************************************************************
 $RCSfile: converter_filein.c,v $
                             -------------------
    cvs         : $Id: converter_filein.c,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#include "converter_filein_p.h"

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>


CONVERTER_FILEIN_DATA *ConverterFileInData_new(const char *fname){
  CONVERTER_FILEIN_DATA *dzd;

  dzd=(CONVERTER_FILEIN_DATA *)malloc(sizeof(CONVERTER_FILEIN_DATA));
  assert(dzd);
  memset(dzd, 0, sizeof(CONVERTER_FILEIN_DATA));
  if (fname)
    dzd->filename=strdup(fname);
  return dzd;
}


void ConverterFileInData_free(CONVERTER_FILEIN_DATA *dzd){
  if (dzd) {
    free(dzd->filename);
    free(dzd);
  }
}



int ConverterFileIn_Begin(CONVERTER *dm){
  CONVERTER_FILEIN_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);

  dzd->nextPosForGetData=0;
  if (dzd->filename) {
    dzd->file=fopen(dzd->filename, "rb");
    if (!dzd->file) {
      DBG_ERROR("Error on fopen(%s): %s",
		dzd->filename, strerror(errno));
      return CONVERTER_RESULT_ERROR;
    }
  }
  else {
    DBG_INFO("Using stdin");
    dzd->file=stdin;
  }

  dzd->eof=0;

  return CONVERTER_RESULT_OK;
}



unsigned int ConverterFileIn_HasData(CONVERTER *dm){
  CONVERTER_FILEIN_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);
  assert(dzd->file);

  return dzd->bytesinbuffer-dzd->nextPosForGetData;
}



unsigned int ConverterFileIn_NeedsData(CONVERTER *dm){
  CONVERTER_FILEIN_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);
  assert(dzd->file);

  return 0;
}



int ConverterFileIn_SetData(CONVERTER *dm,
			    const char *data,
			    unsigned int size){
  CONVERTER_FILEIN_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);
  assert(dzd->file);

  return 0;
}



int ConverterFileIn_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size){
  CONVERTER_FILEIN_DATA *dzd;
  unsigned int i;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);
  assert(dzd->file);

  i=sizeof(dzd->outbuffer)-dzd->nextPosForGetData;
  if (size>i)
    size=i;
  if (size) {
    assert(data);
    memmove(data, dzd->outbuffer+dzd->nextPosForGetData, size);
    dzd->nextPosForGetData+=size;
  }
  DBG_INFO("Returning %d bytes", size);
  return size;
}



int ConverterFileIn_Work(CONVERTER *dm){
  CONVERTER_FILEIN_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);
  assert(dzd->file);

  if (dzd->eof)
    return CONVERTER_RESULT_EOF;

  if (dzd->nextPosForGetData>=dzd->bytesinbuffer) {
    int rv;

    rv=fread(dzd->outbuffer,1, sizeof(dzd->outbuffer), dzd->file);
    if (rv<0) {
      DBG_ERROR("Error on fread(%s): %s",
		dzd->filename, strerror(errno));
      return CONVERTER_RESULT_ERROR;
    }
    else if (rv==0) {
      dzd->eof=1;
    }
    else {
      dzd->bytesinbuffer=rv;
      dzd->nextPosForGetData=0;
    }
  }

  return CONVERTER_RESULT_OK;
}



int ConverterFileIn_End(CONVERTER *dm){
  CONVERTER_FILEIN_DATA *dzd;

  assert(dm);
  assert(dm->privateData);
  dzd=(CONVERTER_FILEIN_DATA *)(dm->privateData);
  assert(dzd->file);

  if (dzd->filename) {
    int rv;

    rv=fclose(dzd->file);
    dzd->file=0;
    if (rv) {
      DBG_ERROR("Error on fclose(%s): %s",
		dzd->filename, strerror(errno));
      return CONVERTER_RESULT_ERROR;
    }
  }
  dzd->file=0;

  return CONVERTER_RESULT_OK;
}



void ConverterFileIn_FreePrivate(CONVERTER *dm){
  assert(dm);
  if (dm->privateData)
    ConverterFileInData_free((CONVERTER_FILEIN_DATA*)(dm->privateData));
}


CONVERTER *ConverterFileIn_new(const char *fname){
  CONVERTER *dm;
  CONVERTER_FILEIN_DATA *dzd;

  dm=Converter_new();
  dzd=ConverterFileInData_new(fname);
  dm->privateData=dzd;

  /* set function pointers */
  dm->beginPtr=ConverterFileIn_Begin;
  dm->endPtr=ConverterFileIn_End;
  dm->hasDataPtr=ConverterFileIn_HasData;
  dm->needsDataPtr=ConverterFileIn_NeedsData;
  dm->setDataPtr=ConverterFileIn_SetData;
  dm->getDataPtr=ConverterFileIn_GetData;
  dm->workPtr=ConverterFileIn_Work;
  dm->freePrivatePtr=ConverterFileIn_FreePrivate;

  dm->typename=strdup("FileIn");

  return dm;
}






