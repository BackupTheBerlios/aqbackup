/***************************************************************************
 $RCSfile: stringlist.c,v $
 -------------------
 cvs         : $Id: stringlist.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __declspec
# if BUILDING_CHIPCARD_DLL
#  define CHIPCARD_API __declspec (dllexport)
# else /* Not BUILDING_CHIPCARD_DLL */
#  define CHIPCARD_API __declspec (dllimport)
# endif /* Not BUILDING_CHIPCARD_DLL */
#else
# define CHIPCARD_API
#endif


#include "chameleon.h"
#include "stringlist.h"
#include "debug.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>


STRINGLIST *StringList_new(){
  STRINGLIST *sl;

  sl=(STRINGLIST *)malloc(sizeof(STRINGLIST));
  assert(sl);
  memset(sl,0,sizeof(STRINGLIST));
  return sl;
}


void StringList_free(STRINGLIST *sl){
  STRINGLISTENTRY *curr, *next;

  if (sl) {
    curr=sl->first;
    while(curr) {
      next=curr->next;
      StringListEntry_free(curr);
      curr=next;
    } /* while */
    free(sl);
  }
}


STRINGLISTENTRY *StringListEntry_new(const char *s, int take){
  STRINGLISTENTRY *sl;

  sl=(STRINGLISTENTRY *)malloc(sizeof(STRINGLISTENTRY));
  assert(sl);
  memset(sl,0,sizeof(STRINGLISTENTRY));
  if (s) {
    if (take)
      sl->data=s;
    else
      sl->data=strdup(s);
  }
  return sl;
}


void StringListEntry_ReplaceString(STRINGLISTENTRY *e,
				   const char *s,
				   int take){
  assert(e);
  if (e->data)
    free((void*)(e->data));
  if (take)
    e->data=s;
  else
    e->data=strdup(s);
}



void StringListEntry_free(STRINGLISTENTRY *sl){
  if (sl) {
    if (sl->data)
      free((void*)(sl->data));
    free(sl);
  }
}


void StringList_AppendEntry(STRINGLIST *sl,
			    STRINGLISTENTRY *se){
  STRINGLISTENTRY *curr;

  DBG_ENTER;
  assert(sl);
  assert(se);

  curr=sl->first;
  if (!curr) {
    sl->first=se;
  }
  else {
    while(curr->next) {
      curr=curr->next;
    }
    curr->next=se;
  }
  sl->count++;
  DBG_LEAVE;
}


void StringList_RemoveEntry(STRINGLIST *sl,
			    STRINGLISTENTRY *se){
  STRINGLISTENTRY *curr;

  DBG_ENTER;
  assert(sl);
  assert(se);

  curr=sl->first;
  if (curr) {
    if (curr==se) {
      sl->first=curr->next;
      if (sl->count)
	sl->count--;
    }
    else {
      while(curr->next!=se) {
	curr=curr->next;
      }
      if (curr) {
	curr->next=se->next;
        if (sl->count)
	  sl->count--;
      }
    }
  }
  DBG_LEAVE;
}


void StringList_Clear(STRINGLIST *sl){
  STRINGLISTENTRY *se, *next;

  assert(sl);
  se=sl->first;
  sl->first=0;
  while (se) {
    next=se->next;
    StringListEntry_free(se);
    se=next;
  } /* while */
}


int StringList_AppendString(STRINGLIST *sl,
			    const char *s,
			    int take,
			    int checkDouble){
  STRINGLISTENTRY *se;

  if (checkDouble) {
    se=sl->first;
    while(se) {
      if (strcmp(se->data, s)==0) {
	if (take)
	  free((char*)s);
	return 0;
      }
      se=se->next;
    } /* while */
  } /* if checkdouble */

  se=StringListEntry_new(s, take);
  StringList_AppendEntry(sl, se);
  return 1;
}


int StringList_InsertString(STRINGLIST *sl,
			    const char *s,
			    int take,
			    int checkDouble){
  STRINGLISTENTRY *se;

  if (checkDouble) {
    se=sl->first;
    while(se) {
      if (strcmp(se->data, s)==0) {
	if (take)
	  free((char*)s);
	return 0;
      }
      se=se->next;
    } /* while */
  } /* if checkdouble */

  se=StringListEntry_new(s, take);
  se->next=sl->first;
  sl->first=se;
  return 1;
}





