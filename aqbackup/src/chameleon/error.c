/***************************************************************************
 $RCSfile: error.c,v $
                             -------------------
    cvs         : $Id: error.c,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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


/**
 * Maximum number of error types that are registerable.
 */
#define ERROR_MAX_TYPES 64

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chameleon/error.h"
#include "chameleon/debug.h"

// forward declaration
const char *Error_ErrorString(int c);


int error_is_initialized=0;
ERRORTYPEREGISTRATIONFORM error_error_descr= {
    Error_ErrorString,
    0,
    "ErrorManager"};
ERRORTYPEREGISTRATIONFORM* error_type_ptr[ERROR_MAX_TYPES];


void Error_ModuleInit() {
  int i;

  DBG_VERBOUS("Error_ModuleInit");
  if (!error_is_initialized) {
    // init all ptrs to zero
    for (i=0; i<ERROR_MAX_TYPES; i++)
      error_type_ptr[i]=(ERRORTYPEREGISTRATIONFORM*)0;
    error_type_ptr[0]=&error_error_descr;
    error_is_initialized=1;
  } // if not initialized
}


void Error_ModuleFini() {
    error_is_initialized=0;
}


int Error_RegisterType(ERRORTYPEREGISTRATIONFORM *tptr){
    int i;

    assert(tptr);
    // search free type
    for (i=0; i<ERROR_MAX_TYPES; i++)
	if (error_type_ptr[i]==(ERRORTYPEREGISTRATIONFORM*)0) {
	    // found one, allocate it and return index
	    error_type_ptr[i]=tptr;
	    tptr->typ=i;
#if DEBUGMODE>2
	    fprintf(stderr,"Registered type \"%s\" (%d)\n",
		    tptr->type_name, tptr->typ);
#endif
	    return 1;
	}
    // not found, flag error
#if DEBUGMODE>0
    fprintf(stderr,"Could not egister type \"%s\"\n",
	    tptr->type_name);
#endif
    return 0;
}


int Error_UnregisterType(ERRORTYPEREGISTRATIONFORM *tptr) {
    assert(tptr);
    if (tptr->typ>=ERROR_MAX_TYPES || tptr->typ<0)
	return 0;
    error_type_ptr[tptr->typ]=(ERRORTYPEREGISTRATIONFORM*)0;
    tptr->typ=-1;
    return 1;
}


int Error_FindType(const char *name){
    int i;

    // browse all types
    assert(name);
    for (i=0; i<ERROR_MAX_TYPES; i++) {
	// is type in use ?
	if (error_type_ptr[i]!=(ERRORTYPEREGISTRATIONFORM*)0) {
	    // compare typename to argument
	    if (strcmp(error_type_ptr[i]->type_name,
		       name)==0)
		// match, return type number
		return i;
	}
    } // for
    // not found
    return -1;
}


const char *Error_GetTypename(int t) {
  if (t>=ERROR_MAX_TYPES || t<0)
    return (const char*)0;

  if (error_type_ptr[t]==(ERRORTYPEREGISTRATIONFORM*)0)
    return (const char*)0;
  return error_type_ptr[t]->type_name;
}



ERRORCODE Error_New(int iscustom, int severity, int typ, int code) {
    ERRORCODE c;

    c=0;
    Error_SetSeverity(&c, severity);
    Error_SetType(&c, typ);
    Error_SetIsCustom(&c, iscustom);
    Error_SetCode(&c, code);

    return c;
}


int Error_IsOk(ERRORCODE c) {
    if (c==0)
	return 1;
    return (Error_GetSeverity(c)<ERROR_SEVERITY_ERR);
}


int Error_GetSeverity(ERRORCODE c){
    return (c>>30) & 3;
}


void Error_SetSeverity(ERRORCODE *c, int v){
    assert(c);
    (*c) &= 0x3fffffff;
    v = (v&3)<<30;
    (*c) |= v;
}


int Error_IsCustom(ERRORCODE c){
    return c & 0x20000000;
}


void Error_SetIsCustom(ERRORCODE *c, int iscustom){
    assert(c);
    if (iscustom)
	(*c) |= 0x20000000;
    else
	(*c) &= ~0x20000000;
}


int Error_GetType(ERRORCODE c){
    return (c>>16) & 0xfff;
}


void Error_SetType(ERRORCODE *c, int v){
    assert(c);
    (*c) &= ~0x0fff0000;
    v = (v&0xfff)<<16;
    (*c) |= v;
}


int Error_GetCode(ERRORCODE c){
  return (short)(c & 0xffff);
}


void Error_SetCode(ERRORCODE *c, int v){
    assert(c);
    (*c) &= ~0xffff;
    (*c) |= v&0xffff;
}


// internal function
int Error_ConcatStrings(char *dst, int dsize, const char *src) {
  if (!src)
    return 1;
  assert(dst);
  if ((strlen(dst)+strlen(src)+1)>=dsize)
    return 0;
  strcat(dst,src);
  return 1;
}


int Error_ToString(ERRORCODE c, char *buffer, int bsize) {
    char str[64]; // for number conversions
    const char *s;
    int i;

    assert(buffer);
    if (bsize<64)
	return 0;
    buffer[0]=0;

    // severity
    if (!Error_ConcatStrings(buffer,bsize," Severity: "))
	return 0;
    switch(Error_GetSeverity(c)) {
    case ERROR_SEVERITY_DEBUG:
	s="Debug";
	break;

    case ERROR_SEVERITY_INFO:
	s="Info";
	break;

    case ERROR_SEVERITY_WARN:
	s="Warning";
	break;

    case ERROR_SEVERITY_ERR:
	s="Error";
	break;

    default:
	s="Unknown";
	break;
    } // switch
    if (!Error_ConcatStrings(buffer,bsize,s))
	return 0;

    if (Error_IsCustom(c)) {
	// this is an custom error, so the normal rules do not apply
	// error type
	if (!Error_ConcatStrings(buffer,bsize," Custom Type: "))
	    return 0;
	sprintf(str,"%d (%04x)",
		Error_GetType(c),
		Error_GetType(c));
	if (!Error_ConcatStrings(buffer,bsize,str))
	    return 0;

	// error code
	if (!Error_ConcatStrings(buffer,bsize," Custom Code: "))
	    return 0;
	sprintf(str,"%d (%04x)",
		Error_GetCode(c),
		Error_GetCode(c));
	if (!Error_ConcatStrings(buffer,bsize,str))
	    return 0;
    }
    else {
	// error type
	if (!Error_ConcatStrings(buffer,bsize," Type: "))
	    return 0;
	s=Error_GetTypename(Error_GetType(c));
	if (s==(const char*)0) {
	    sprintf(str,"Unknown (%4x)",Error_GetType(c));
	    s=str;
	}
	if (!Error_ConcatStrings(buffer,bsize,s))
	    return 0;

        // error code
	if (!Error_ConcatStrings(buffer,bsize," Code: "))
	    return 0;
	// get message function
	i=Error_GetType(c);
	if (i>=ERROR_MAX_TYPES || i<0)
	    s=(const char*)0;
	else {
	  if (error_type_ptr[i]==(ERRORTYPEREGISTRATIONFORM*)0)
	    s=0;
	  else
	    s=error_type_ptr[i]->msgptr(Error_GetCode(c));
	}
	if (s==(const char*)0) {
	  if (!Error_ConcatStrings(buffer,bsize,"Unknown"))
	    return 0;
	}
	if (!Error_ConcatStrings(buffer,bsize,s))
	  return 0;
	sprintf(str," (%d)", Error_GetCode(c));
	s=str;
	if (!Error_ConcatStrings(buffer,bsize,s))
	  return 0;
    }
    // finished
    return 1;
}


const char *Error_ErrorString(int c) {
    const char *s;

    switch(c) {
    case ERROR_UNSPECIFIED:
	s="Undefined error";
        break;
    case ERROR_COULD_NOT_REGISTER:
	s="Could not register";
	break;
    case ERROR_COULD_NOT_UNREGISTER:
	s="Could not unregister";
	break;
    case ERROR_INVALID_BUFFERSIZE:
      s="Invalid buffer size";
      break;
    default:
	s=(const char*)0;
    } // switch
    return s;
}



