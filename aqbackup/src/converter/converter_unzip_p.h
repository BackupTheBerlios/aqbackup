/***************************************************************************
 $RCSfile: converter_unzip_p.h,v $
                             -------------------
    cvs         : $Id: converter_unzip_p.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_UNZIP_P_H
#define AQBACKUP_CONVERTER_UNZIP_P_H

#include "converter_unzip.h"

#include <zlib.h>

#define CONVERTER_ZIP_DEFAULT_LEVEL 6


typedef struct _CONVERTER_UNZIP_DATA CONVERTER_UNZIP_DATA;
struct _CONVERTER_UNZIP_DATA {
  z_stream *zstream;
  int eof;
  char inbuffer[1024];
  char outbuffer[2024];
  int nextPosForGetData;
  int nextPosForSetData;
};


CONVERTER_UNZIP_DATA *ConverterUnzipData_new();
void ConverterUnzipData_free(CONVERTER_UNZIP_DATA *dzd);

void ConverterUnzip_FreePrivate(CONVERTER *dm);


int ConverterUnzip_Begin(CONVERTER *dm);
unsigned int ConverterUnzip_HasData(CONVERTER *dm);
unsigned int ConverterUnzip_NeedsData(CONVERTER *dm);
int ConverterUnzip_SetData(CONVERTER *dm,
			 const char *data,
			 unsigned int size);
int ConverterUnzip_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size);
int ConverterUnzip_Work(CONVERTER *dm);
int ConverterUnzip_End(CONVERTER *dm);


#endif


