/***************************************************************************
 $RCSfile: debug.h,v $
 -------------------
 cvs         : $Id: debug.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
 begin       : Thu Nov 28 2002
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


#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <chameleon/logger.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined HAVE_FUNC && (DEBUGMODE>10))
# define DBG_ENTER fprintf(stderr,"Enter \""__func__"\" \n")
# define DBG_LEAVE fprintf(stderr,"Leave \""__func__"\" \n")
#else
# define DBG_ENTER
# define DBG_LEAVE
#endif

#ifdef HAVE_SNPRINTF

#define DBG_ERROR(format, args...) if (1){\
  char dbg_buffer[256]; \
  snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
  __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelError, dbg_buffer);};

#define DBG_ERROR_ERR(dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelError, dbg_buffer);};


#define DBG_WARN(format, args...) {\
  char dbg_buffer[256]; \
  snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
  __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelWarning, dbg_buffer);};

#define DBG_WARN_ERR(dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelWarning, dbg_buffer);};


#define DBG_NOTICE(format, args...) \
 if (Logger_GetLevel()>=LoggerLevelNotice) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelNotice, dbg_buffer);};

#define DBG_NOTICE_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelNotice) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelNotice, dbg_buffer);};


#define DBG_INFO(format, args...) \
 if (Logger_GetLevel()>=LoggerLevelInfo) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelInfo, dbg_buffer);};

#define DBG_INFO_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelInfo) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelInfo, dbg_buffer);};


#define DBG_DEBUG(format, args...) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};

#define DBG_DEBUG_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};


#define DBG_VERBOUS(format, args...) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};

#define DBG_VERBOUS_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 snprintf(dbg_buffer, sizeof(dbg_buffer)-1,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
  dbg_buffer[sizeof(dbg_buffer)-1]=0; \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};



#else



#define DBG_ERROR(format, args...) {\
 char dbg_buffer[256]; \
 sprintf(dbg_buffer, \
 __FILE__":%5d: " format  , __LINE__ , ## args); \
 Logger_Log(LoggerLevelError, dbg_buffer);};

#define DBG_ERROR_ERR(dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 sprintf(dbg_buffer, \
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
 Logger_Log(LoggerLevelError, dbg_buffer);};


#define DBG_WARN(format, args...) {\
 char dbg_buffer[256]; \
 sprintf(dbg_buffer,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
 Logger_Log(LoggerLevelWarning, dbg_buffer);};

#define DBG_WARN_ERR(dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 sprintf(dbg_buffer,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
 Logger_Log(LoggerLevelWarning, dbg_buffer);};


#define DBG_NOTICE(format, args...) {\
 char dbg_buffer[256]; \
 sprintf(dbg_buffer,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
 Logger_Log(LoggerLevelNotice, dbg_buffer);};

#define DBG_NOTICE_ERR(dbg_err) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 sprintf(dbg_buffer,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
 Logger_Log(LoggerLevelNotice, dbg_buffer);};


#define DBG_INFO(format, args...) \
 if (Logger_GetLevel()>=LoggerLevelInfo) {\
 char dbg_buffer[256]; \
 sprintf(dbg_buffer,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
 Logger_Log(LoggerLevelInfo, dbg_buffer);};

#define DBG_INFO_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelInfo) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 sprintf(dbg_buffer,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
 Logger_Log(LoggerLevelInfo, dbg_buffer);};


#define DBG_DEBUG(format, args...) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 sprintf(dbg_buffer,\
 __FILE__":%5d: " format  , __LINE__ , ## args); \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};

#define DBG_DEBUG_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 sprintf(dbg_buffer,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};


#define DBG_VERBOUS(format, args...) \
  if (Logger_GetLevel()>=LoggerLevelDebug) { \
  char dbg_buffer[256]; \
  sprintf(dbg_buffer,\
  __FILE__":%5d: " format  , __LINE__ , ## args); \
  Logger_Log(LoggerLevelDebug, dbg_buffer);};

#define DBG_VERBOUS_ERR(dbg_err) \
 if (Logger_GetLevel()>=LoggerLevelDebug) {\
 char dbg_buffer[256]; \
 char dbg_errbuff[256]; \
 Error_ToString(dbg_err,dbg_errbuff, sizeof(dbg_errbuff)); \
 sprintf(dbg_buffer,\
 __FILE__":%5d: %s" , __LINE__ , dbg_errbuff); \
 Logger_Log(LoggerLevelDebug, dbg_buffer);};

#endif /* HAVE_SNPRINTF */

#ifdef __cplusplus
}
#endif


#endif


