/***************************************************************************
  $RCSfile: libloader.c,v $
                             -------------------
    cvs         : $Id: libloader.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Fri Nov 22 2002
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

#include <chameleon/libloader.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef OS_WIN32
# ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
# endif
#else
# include <windows.h>
#endif


const char *LibLoader_ErrorString(int c);


int libloader_is_initialized=0;
ERRORTYPEREGISTRATIONFORM libloader_error_descr= {
  LibLoader_ErrorString,
  0,
  LIBLOADER_TYPE};



ERRORCODE LibLoader_ModuleInit(){
  if (!libloader_is_initialized) {
    if (!Error_RegisterType(&libloader_error_descr))
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ERROR_TYPE_ERROR,
		       ERROR_COULD_NOT_REGISTER);
    libloader_is_initialized=1;
  }
  return 0;
}


ERRORCODE LibLoader_ModuleFini(){
  if (libloader_is_initialized) {
    libloader_is_initialized=0;
    if (!Error_UnregisterType(&libloader_error_descr))
      return Error_New(0,
		       ERROR_SEVERITY_ERR,
		       ERROR_TYPE_ERROR,
		       ERROR_COULD_NOT_UNREGISTER);
  }
  return 0;
}


CHLIBLOADERHANDLE LibLoader_new(){
  CHLIBLOADERHANDLE h;

  h=(CHLIBLOADERHANDLE)malloc(sizeof(CHLIBLOADERTABLE));
  assert(h);
  memset(h,0,sizeof(CHLIBLOADERTABLE));
  return h;
}


void LibLoader_free(CHLIBLOADERHANDLE h){
  assert(h);
  free(h);
}


ERRORCODE LibLoader_OpenLibrary(CHLIBLOADERHANDLE h, const char *name){
  assert(h);
#if DEBUGMODE>10
  fprintf(stderr,"LibLoader_OpenLibrary\n");
#endif

#if defined (OS_WIN32)
  h->handle=(void*)LoadLibrary(name);
  if (!h->handle) {
# if DEBUGMODE>0
    fprintf(stderr,
	    "LIBCHIPCARD: Error loading library \"%s\"\n",
	    name);
# endif
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_COULD_NOT_LOAD);
  }
#else // for linux and others
  h->handle=dlopen(name,RTLD_LAZY);
  if (!h->handle) {
    fprintf(stderr,
	    "LIBCHIPCARD: Error loading library \"%s\": %s\n",
	    name, dlerror());
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_COULD_NOT_LOAD);
  }
#endif
  return 0;
}


ERRORCODE LibLoader_CloseLibrary(CHLIBLOADERHANDLE h){
  assert(h);
#if DEBUGMODE>10
  fprintf(stderr,"LibLoader_CloseLibrary\n");
#endif

  if (!h->handle)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_NOT_OPEN);
#ifndef OS_WIN32
  /* hmm, linux does not like this with libtowitoko.so and qlcsetup */
  //result=(dlclose(_handle)==0);
  if (dlclose(h->handle)!=0) {
    fprintf(stderr,
	    "LIBCHIPCARD: Error unloading library: %s\n",
	    dlerror());
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_COULD_NOT_CLOSE);
  }
#else
  if (!FreeLibrary((HINSTANCE)h->handle))
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_COULD_NOT_CLOSE);
#endif
  h->handle=0;
  return 0;
}


ERRORCODE LibLoader_Resolve(CHLIBLOADERHANDLE h, const char *name, void **p){
  assert(h);
  assert(name);
  assert(p);

#if DEBUGMODE>10
  fprintf(stderr,"LibLoader_Resolve\n");
#endif

  if (!h->handle)
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_NOT_OPEN);
#ifndef OS_WIN32
  *p=dlsym(h->handle,name);
  if (!*p) {
    fprintf(stderr,
	    "LIBCHIPCARD: Error resolving symbol \"%s\": %s\n",
	    name, dlerror());
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_COULD_RESOLVE);
  }
#else
  *p=(void*)GetProcAddress((HINSTANCE)h->handle,name);
  if (!*p) {
    fprintf(stderr,
	    "LIBCHIPCARD: Error resolving symbol \"%s\"\n",
	    name);
    return Error_New(0,
		     ERROR_SEVERITY_ERR,
		     libloader_error_descr.typ,
		     LIBLOADER_ERROR_COULD_RESOLVE);
  }
#endif

#if DEBUGMODE>2
  fprintf(stderr,"Resolve \"%s\": %08x\n",
	  name,(int)*p);
#endif
  return 0;
}


const char *LibLoader_ErrorString(int c){
  const char *s;

  switch(c) {
  case 0:
    s="Success";
    break;

  case LIBLOADER_ERROR_COULD_NOT_LOAD:
    s="Could not load library";
    break;
  case LIBLOADER_ERROR_NOT_OPEN:
    s="Library not open";
    break;
  case LIBLOADER_ERROR_COULD_NOT_CLOSE:
    s="Could not close library";
    break;
  case LIBLOADER_ERROR_COULD_RESOLVE:
    s="Could not resolve symbol";
    break;
  default:
    s=(const char*)0;
  } // switch
  return s;
}










