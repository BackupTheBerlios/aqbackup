/***************************************************************************
 $RCSfile: converter_fileout_p.h,v $
                             -------------------
    cvs         : $Id: converter_fileout_p.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_FILEOUT_P_H
#define AQBACKUP_CONVERTER_FILEOUT_P_H

#include "converter_fileout.h"

#include <stdio.h>

#define CONVERTER_FILEOUT_CHUNKSIZE 2048


typedef struct _CONVERTER_FILEOUT_DATA CONVERTER_FILEOUT_DATA;
struct _CONVERTER_FILEOUT_DATA {
  int eof;
  char *filename;
  char *mode;
  FILE *file;
};


CONVERTER_FILEOUT_DATA *ConverterFileOutData_new(const char *fname,
						 const char *mode);
void ConverterFileOutData_free(CONVERTER_FILEOUT_DATA *dzd);

void ConverterFileOut_FreePrivate(CONVERTER *dm);


int ConverterFileOut_Begin(CONVERTER *dm);
unsigned int ConverterFileOut_HasData(CONVERTER *dm);
unsigned int ConverterFileOut_NeedsData(CONVERTER *dm);
int ConverterFileOut_SetData(CONVERTER *dm,
			 const char *data,
			 unsigned int size);
int ConverterFileOut_GetData(CONVERTER *dm,
			 char *data,
			 unsigned int size);
int ConverterFileOut_Work(CONVERTER *dm);
int ConverterFileOut_End(CONVERTER *dm);


#endif


