/***************************************************************************
 $RCSfile: stringlist.h,v $
 -------------------
 cvs         : $Id: stringlist.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
 begin       : Thu Apr 03 2003
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

#ifndef CHAMELEON_STRINGLIST_H
#define CHAMELEON_STRINGLIST_H


#ifdef __cplusplus
extern "C" {
#endif


CHIPCARD_API typedef struct STRINGLISTENTRYSTRUCT STRINGLISTENTRY;
struct STRINGLISTENTRYSTRUCT {
  STRINGLISTENTRY *next;
  const char *data;
};


CHIPCARD_API typedef struct STRINGLISTSTRUCT STRINGLIST;
struct STRINGLISTSTRUCT {
  STRINGLISTENTRY *first;
  unsigned int count;
};


CHIPCARD_API STRINGLIST *StringList_new();
CHIPCARD_API void StringList_free(STRINGLIST *sl);
CHIPCARD_API void StringList_Clear(STRINGLIST *sl);

CHIPCARD_API STRINGLISTENTRY *StringListEntry_new(const char *s, int take);
CHIPCARD_API void StringListEntry_ReplaceString(STRINGLISTENTRY *e,
						 const char *s,
						 int take);
CHIPCARD_API void StringListEntry_free(STRINGLISTENTRY *sl);
CHIPCARD_API void StringList_AppendEntry(STRINGLIST *sl,
					 STRINGLISTENTRY *se);
CHIPCARD_API void StringList_RemoveEntry(STRINGLIST *sl,
					 STRINGLISTENTRY *se);

/**
 * Appends a string.
 * @return 0 if not appended, !=0 if appended
 * @param take if true then the StringList takes over ownership of the string
 * @param checkDouble if true the the string will only be appended if it
 * does not already exist
 */
CHIPCARD_API int StringList_AppendString(STRINGLIST *sl,
					 const char *s,
					 int take,
					 int checkDouble);

CHIPCARD_API int StringList_InsertString(STRINGLIST *sl,
					 const char *s,
					 int take,
					 int checkDouble);


#ifdef __cplusplus
}
#endif

#endif


