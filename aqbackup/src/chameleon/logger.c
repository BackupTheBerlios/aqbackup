/***************************************************************************
 $RCSfile: logger.c,v $
                             -------------------
    cvs         : $Id: logger.c,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Sun Dec 05 2003
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


#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#include <string.h>
#ifdef HAVE_TIME_H
# include <time.h>
#endif
#include <unistd.h>


static int _Logger_Enabled=1;
static LOGGER_LOGTYPE _Logger_LogType=LoggerTypeConsole;
static char _Logger_File[256];
static char _Logger_Ident[256];
static LOGGER_LEVEL _Logger_Level=LoggerLevelError;
static LOGGERFUNCTIONLOG _Logger_Function=0;


int Logger_Open(const char *ident,
		const char *file,
		LOGGER_LOGTYPE logtype,
		LOGGER_FACILITY facility){

  _Logger_LogType=logtype;

  Logger_SetIdent(ident);

  if (logtype==LoggerTypeFile) {
    /* logging to a file */
    if (file==0) {
      _Logger_File[0]=0;
      _Logger_LogType=LoggerTypeConsole;
      _Logger_Enabled=1;
      fprintf(stderr,"LOGGER: No filename given, will log to console.\n");
    }
    else {
      if ((strlen(file)+1)>sizeof(_Logger_File)) {
	_Logger_File[0]=0;
	_Logger_LogType=LoggerTypeConsole;
	_Logger_Enabled=1;
	fprintf(stderr,"LOGGER: Filename too long, will log to console.\n");
      }
      else {
	_Logger_LogType=LoggerTypeFile;
	_Logger_Enabled=1;
	strcpy(_Logger_File, file);
      }
    }
  }
#ifdef HAVE_SYSLOG_H
  else if (logtype==LoggerTypeSyslog) {
    /* caller wants to log via syslog */
    int fac;

    switch(facility) {
    case LoggerFacilityAuth:
      fac=LOG_AUTH;
      break;
    case LoggerFacilityDaemon:
      fac=LOG_DAEMON;
      break;
    case LoggerFacilityMail:
      fac=LOG_MAIL;
      break;
    case LoggerFacilityNews:
      fac=LOG_NEWS;
      break;
    case LoggerFacilityUser:
    default:
      fac=LOG_USER;
      break;
    }

    openlog(ident,
	    LOG_CONS |
	    LOG_PID,
	    fac);
    _Logger_LogType=LoggerTypeSyslog;
    _Logger_Enabled=1;
  } /* if syslog */
#endif /* ifdef HAVE_SYSLOG_H */

  else {
    /* console */
    _Logger_File[0]=0;
    _Logger_LogType=LoggerTypeConsole;
    _Logger_Enabled=1;
  }

  return Logger_Log(LoggerLevelDebug,"started");
}



void Logger_Close(){
  Logger_Log(LoggerLevelDebug,"stopped");
  _Logger_LogType=LoggerTypeConsole;
  _Logger_Enabled=0;
#ifdef HAVE_SYSLOG_H
  closelog();
#endif
}


int _Logger_CreateMessage(LOGGER_LEVEL priority, const char *s,
			  char *buffer, int bufsize) {
  int i;
#ifdef HAVE_TIME_H
  struct tm *t;
  time_t tt;
#endif

  if (strlen(s)+strlen(_Logger_Ident)+32>=bufsize) {
    fprintf(stderr," LOGGER: Logbuffer too small (1).\n");
    return 1;
  }


#ifdef HAVE_TIME_H
  tt=time(0);
  t=localtime(&tt);

# ifdef HAVE_SNPRINTF
  buffer[bufsize-1]=0;
#  ifdef HAVE_GETPID
  i=snprintf(buffer, bufsize-1,
	     "%d:%04d/%02d/%02d %02d-%02d-%02d:%s(%d):%s\n",priority,
	     t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	     t->tm_hour, t->tm_min, t->tm_sec,
	     _Logger_Ident, getpid(), s);
#  else
  i=snprintf(buffer, bufsize-1,
	     "%d:%04d/%02d/%02d %02d-%02d-%02d:%s:%s\n",priority,
	     t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	     t->tm_hour, t->tm_min, t->tm_sec,
	     _Logger_Ident, s);
#  endif /* HAVE_GETPID */
  if (i>=bufsize) {
    fprintf(stderr," LOGGER: Logbuffer too small (2).\n");
    return 1;
  }
# else   /* HAVE_SNPRINTF */
#  ifdef HAVE_GETPID
  sprintf(buffer,"%d:%04d/%02d/%02d %02d-%02d-%02d:%s(%d):%s\n",priority,
	  t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	  t->tm_hour, t->tm_min, t->tm_sec,
	  _Logger_Ident, getpid(), s);
#  else
  sprintf(buffer,"%d:%04d/%02d/%02d %02d-%02d-%02d:%s:%s\n",priority,
	  t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	  t->tm_hour, t->tm_min, t->tm_sec,
	  _Logger_Ident, s);
#  endif /* HAVE_GETPID */
# endif  /* HAVE_SNPRINTF */
#else    /* HAVE_TIME_H */
# ifdef HAVE_SNPRINTF
  buffer[bufsize-1]=0;
  i=snprintf(buffer, bufsize-1,
	     "%d:%s:%s\n",priority,
	     _Logger_Ident, s);
  if (i>=bufsize) {
    fprintf(stderr," LOGGER: Logbuffer too small (3).\n");
    return 1;
  }
# else   /* HAVE_SNPRINTF */
  sprintf(buffer,"%d:%s:%s\n",priority,
	  _Logger_Ident, s);
# endif  /* HAVE_SNPRINTF */
#endif   /* HAVE_TIME_H */
  return 0;
}


int _Logger_Log(LOGGER_LEVEL priority, const char *s){
  FILE *f;
  int pri;
  char buffer[300];
  int rv;

  if (priority>_Logger_Level)
    /* priority too low, don't log */
    return 0;

  switch(_Logger_LogType) {
  case LoggerTypeFile:
    rv=_Logger_CreateMessage(priority, s,
			     buffer, sizeof(buffer));
    if (rv)
      return rv;

    f=fopen(_Logger_File,"a+");
    if (f==0) {
      fprintf(stderr,
	      "LOGGER: Unable to open file \"%s\" (%s)\n",
	      _Logger_File,
	      strerror(errno));
      _Logger_LogType=LoggerTypeConsole;
      return 1;
    }

    rv=fprintf(f, "%s", buffer);
    if (rv==-1 || rv!=strlen(buffer)) {
      fprintf(stderr,
	      "LOGGER: Unable to write to file \"%s\" (%s)\n",
	      _Logger_File,
	      strerror(errno));
      fclose(f);
      _Logger_LogType=LoggerTypeConsole;
      return 1;
    }
    if (fclose(f)) {
      fprintf(stderr,
	      "LOGGER: Unable to close file \"%s\" (%s)\n",
	      _Logger_File,
	      strerror(errno));
      _Logger_LogType=LoggerTypeConsole;
      return 1;
    }
    break;

#ifdef HAVE_SYSLOG_H
  case LoggerTypeSyslog:
    switch(priority) {
    case LoggerLevelEmergency:
      pri=LOG_EMERG;
      break;
    case LoggerLevelAlert:
      pri=LOG_ALERT;
      break;
    case LoggerLevelCritical:
      pri=LOG_CRIT;
      break;
    case LoggerLevelError:
      pri=LOG_ERR;
      break;
    case LoggerLevelWarning:
      pri=LOG_WARNING;
      break;
    case LoggerLevelNotice:
      pri=LOG_NOTICE;
      break;
    case LoggerLevelInfo:
      pri=LOG_NOTICE;
      break;
    case LoggerLevelDebug:
    default:
      pri=LOG_DEBUG;
      break;
    } /* switch */
    syslog(pri,"%s",s);
    break;
#endif

  case LoggerTypeFunction:
    if (_Logger_Function==0) {
      fprintf(stderr,
	      "LOGGER: Logtype is \"Function\", but no function is set.\n");
      return 1;
    }
    rv=_Logger_CreateMessage(priority, s,
			     buffer, sizeof(buffer));
    if (rv)
      return rv;
    (_Logger_Function)(buffer);
    break;

  case LoggerTypeConsole:
  default:
    rv=_Logger_CreateMessage(priority, s,
			     buffer, sizeof(buffer));
    if (rv)
      return rv;

    fprintf(stderr, "%s", buffer);
    break;
  } /* switch */
  return 0;
}


int Logger_Log(LOGGER_LEVEL priority, const char *s){
  char buffer[256];
  const char *p;
  int rv;
  int i;

  if (!_Logger_Enabled)
    return 1;

  /* copy buffer, exchange all newlines by 0 */
  for (i=0; i<strlen(s)+1; i++) {
    if (s[i]=='\n') {
      buffer[i]=0;
    }
    else
      buffer[i]=s[i];
  }
  buffer[i]=0; /* add final 0 */

  /* now log each line */
  rv=0;
  p=buffer;
  while (*p) {
    rv|=_Logger_Log(priority, p);
    while(*p)
      p++;
    p++;
  }
  return rv;
}


void Logger_Enable(int f){
  _Logger_Enabled=f;
}


int Logger_IsEnabled(){
  return _Logger_Enabled;
}


void Logger_SetLevel(LOGGER_LEVEL l){
  _Logger_Level=l;
}

int Logger_GetLevel() {
  return _Logger_Level;
}


void Logger_SetIdent(const char *id){
  /* copy ident string */
  _Logger_Ident[0]=0;
  if (id) {
    if ((strlen(id)+1)>sizeof(_Logger_Ident)) {
      fprintf(stderr,"LOGGER: Identification too long, will not use it.\n");
    }
    else {
      strcpy(_Logger_Ident, id);
    }
  }
}



