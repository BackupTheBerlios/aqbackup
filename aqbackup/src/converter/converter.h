/***************************************************************************
 $RCSfile: converter.h,v $
                             -------------------
    cvs         : $Id: converter.h,v 1.1 2003/06/07 21:07:52 aquamaniac Exp $
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


#ifndef AQBACKUP_CONVERTER_H
#define AQBACKUP_CONVERTER_H


#define CONVERTER_RESULT_OK    0
#define CONVERTER_RESULT_ERROR (-1)
#define CONVERTER_RESULT_EOF   (-2)
#define CONVERTER_RESULT_SIDE_ERROR (-3)


typedef struct _CONVERTER CONVERTER;


typedef int (*CONVERTER_BEGIN_PTR)(CONVERTER *dm);
typedef unsigned int (*CONVERTER_HASDATA_PTR)(CONVERTER *dm);
typedef unsigned int (*CONVERTER_NEEDSDATA_PTR)(CONVERTER *dm);
typedef int (*CONVERTER_SETDATA_PTR)(CONVERTER *dm,
					 const char *data,
					 unsigned int size);
typedef int (*CONVERTER_GETDATA_PTR)(CONVERTER *dm,
					 char *data,
					 unsigned int size);
typedef int (*CONVERTER_WORK_PTR)(CONVERTER *dm);
typedef int (*CONVERTER_END_PTR)(CONVERTER *dm);
typedef void (*CONVERTER_FREEPRIVATE_PTR)(CONVERTER *dm);


struct _CONVERTER {
  CONVERTER *next;
  /** all converters listed here will get data in parallel */
  CONVERTER *tee;
  int notAllDataTaken;
  int eof;
  char *typename;
  unsigned int bytesReceived;
  unsigned int bytesSent;

  CONVERTER_BEGIN_PTR beginPtr;
  CONVERTER_END_PTR endPtr;
  CONVERTER_HASDATA_PTR hasDataPtr;
  CONVERTER_NEEDSDATA_PTR needsDataPtr;
  CONVERTER_SETDATA_PTR setDataPtr;
  CONVERTER_GETDATA_PTR getDataPtr;
  CONVERTER_WORK_PTR workPtr;
  CONVERTER_FREEPRIVATE_PTR freePrivatePtr;

  void *privateData;
};


CONVERTER *Converter_new();
void Converter_free(CONVERTER *dm);

void Converter_add(CONVERTER *dm, CONVERTER **head);
void Converter_del(CONVERTER *dm, CONVERTER **head);


/**
 * Allocates some structures needed for processing data
 */
int Converter_Begin(CONVERTER *dm);

/**
 * Checks whether there is data available in the output buffer.
 */
unsigned int Converter_HasData(CONVERTER *dm);

/**
 * Checks whether input data is needed by the modifier.
 */
unsigned int Converter_NeedsData(CONVERTER *dm);

/**
 * Sets new data. Please use this only if @ref Converter_NeedsData
 * returns a non-zero value ! Please use the value returned by that
 * function (or less).
 * @param dm pointer to the modifiers internal data
 * @param data pointer to the data to be set
 * @param size number of bytes to be set. If 0 then no further data input
 * will be accepted and the end of the input stream is assumed
 * @return Error code or number of bytes taken
 */
int Converter_SetData(CONVERTER *dm,
		      const char *data,
		      unsigned int size);
int Converter_GetData(CONVERTER *dm,
		      char *data,
		      unsigned int size);

/**
 * Makes the data modifier process the input data.
 * Please note that the return value is special here:
 * As long as you did not flag the end of input stream (by calling
 * @ref Converter_SetData with zero bytes) you should take a return
 * value other than CONVERTER_RESULT_OK as an error.
 * However, if you did flag the end of input stream, you should call this
 * method here as long as the return value is CONVERTER_RESULT_OK and
 * stop only after CONVERTER_RESULT_EOF (or CONVERTER_RESULT_ERROR)
 * is returned.
 */
int Converter_Work(CONVERTER *dm);

/**
 * Frees some data structures needed for processing data.
 */
int Converter_End(CONVERTER *dm);







/**
 * Allocates some structures needed for processing data
 */
int ConverterGroup_Begin(CONVERTER *dm);

/**
 * Checks whether there is data available in the output buffer.
 */
unsigned int ConverterGroup_HasData(CONVERTER *dm);

/**
 * Checks whether input data is needed by the modifier.
 */
unsigned int ConverterGroup_NeedsData(CONVERTER *dm);

/**
 * Sets new data. Please use this only if @ref ConverterGroup_NeedsData
 * returns a non-zero value ! Please use the value returned by that
 * function (or less).
 * @param dm pointer to the modifiers internal data
 * @param data pointer to the data to be set
 * @param size number of bytes to be set. If 0 then no further data input
 * will be accepted and the end of the input stream is assumed
 * @return Error code or number of bytes taken
 */
int ConverterGroup_SetData(CONVERTER *dm,
		      const char *data,
		      unsigned int size);
int ConverterGroup_GetData(CONVERTER *dm,
		      char *data,
		      unsigned int size);

/**
 * Makes the data modifier process the input data.
 * Please note that the return value is special here:
 * As long as you did not flag the end of input stream (by calling
 * @ref ConverterGroup_SetData with zero bytes) you should take a return
 * value other than CONVERTER_RESULT_OK as an error.
 * However, if you did flag the end of input stream, you should call this
 * method here as long as the return value is CONVERTER_RESULT_OK and
 * stop only after CONVERTER_RESULT_EOF (or CONVERTER_RESULT_ERROR)
 * is returned.
 */
int ConverterGroup_Work(CONVERTER *dm);

/**
 * Frees some data structures needed for processing data.
 */
int ConverterGroup_End(CONVERTER *dm);


void ConverterGroup_Append(CONVERTER *dm, CONVERTER *newc);
void ConverterGroup_Tee(CONVERTER *dm, CONVERTER *newc);
void ConverterGroup_free(CONVERTER *dm);

unsigned int ConverterGroup_BytesReceived(CONVERTER *dm);
unsigned int ConverterGroup_BytesSent(CONVERTER *dm);


#endif


