/***************************************************************************
 $RCSfile: converter_zip_p.h,v $
                             -------------------
    cvs         : $Id: converter_zip_p.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_ZIP_P_H
#define AQBACKUP_CONVERTER_ZIP_P_H

#include "converter_zip.h"

#include <zlib.h>

#define CONVERTER_ZIP_DEFAULT_LEVEL 6


typedef struct _CONVERTER_ZIP_DATA CONVERTER_ZIP_DATA;
struct _CONVERTER_ZIP_DATA {
  z_stream *zstream;
  int level;
  int eof;
  char inbuffer[2048];
  char outbuffer[2048];
  int nextPosForGetData;
};


CONVERTER_ZIP_DATA *ConverterZipData_new();
void ConverterZipData_free(CONVERTER_ZIP_DATA *dzd);

void ConverterZip_FreePrivate(CONVERTER *dm);


int ConverterZip_Begin(CONVERTER *dm);
unsigned int ConverterZip_HasData(CONVERTER *dm);
unsigned int ConverterZip_NeedsData(CONVERTER *dm);
int ConverterZip_SetData(CONVERTER *dm,
			 const char *data,
			 unsigned int size);
int ConverterZip_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size);
int ConverterZip_Work(CONVERTER *dm);
int ConverterZip_End(CONVERTER *dm);


#endif


