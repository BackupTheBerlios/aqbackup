/***************************************************************************
 $RCSfile: logger.h,v $
                             -------------------
    cvs         : $Id: logger.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
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

#ifndef CH_LOGGER_H
#define CH_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LOGGERFUNCTIONLOG)(const char *s);


CHIPCARD_API typedef enum {
  LoggerTypeConsole,
  LoggerTypeFile,
  LoggerTypeSyslog,
  LoggerTypeFunction
} LOGGER_LOGTYPE;


CHIPCARD_API typedef enum {
  LoggerFacilityAuth=0,
  LoggerFacilityDaemon,
  LoggerFacilityMail,
  LoggerFacilityNews,
  LoggerFacilityUser
} LOGGER_FACILITY;


CHIPCARD_API typedef enum {
  LoggerLevelEmergency=0,
  LoggerLevelAlert,
  LoggerLevelCritical,
  LoggerLevelError,
  LoggerLevelWarning,
  LoggerLevelNotice,
  LoggerLevelInfo,
  LoggerLevelDebug
} LOGGER_LEVEL;



/**
 * Sets up logging. It automatically enables logging.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param ident this string is prepended to each message logged to identify
 * the logging program
 * @param file name of the file to log to. If this is empty and syslog is
 * available, then all messages are logged via syslog. If syslog is not
 * available, all messages are logged to the console.
 * @param logtype how to log (via syslog, to a file, to the console etc)
 * @param facility what kind of program the log message comes from
 */
CHIPCARD_API int Logger_Open(const char *ident,
			     const char *file,
			     LOGGER_LOGTYPE logtype,
			     LOGGER_FACILITY facility);

/**
 * Shuts down logging. Automatically disables logging.
 * @author Martin Preuss<martin@libchipcard.de>
 */
CHIPCARD_API void Logger_Close();

/**
 * Log a message.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param priority priority of the message
 * @param s string to log. This string is cut at all occurences of a newline
 * character thus splitting it into multiple log lines if necessary
 */
CHIPCARD_API int Logger_Log(LOGGER_LEVEL priority, const char *s);

/**
 * Enables or disables logging.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param f if 0 then logging is disabled, otherwise it is enabled
 */
CHIPCARD_API void Logger_Enable(int f);

/**
 * Checks whether logging is enabled.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if disabled, 1 otherwise
 */
CHIPCARD_API int Logger_IsEnabled();

/**
 * Sets the logger level. All messages with a priority up to the given one
 * will be logged, all others will not.
 * @author Martin Preuss<martin@libchipcard.de>
 * @param l maximum level to be logged
 */
CHIPCARD_API void Logger_SetLevel(LOGGER_LEVEL l);

/**
 * Returns the current log level.
 * @author Martin Preuss<martin@libchipcard.de>
 */
CHIPCARD_API int Logger_GetLevel();


/**
 * Set ident string. This string is prepended to every log message and
 * should contain the name of the running program.
 * @author Martin Preuss<martin@libchipcard.de>
 */
CHIPCARD_API void Logger_SetIdent(const char *id);


/**
 * Set logging function. This function is used to log messages in mode
 * LoggerTypeFunction.
 * @author Martin Preuss<martin@libchipcard.de>
 */
CHIPCARD_API LOGGERFUNCTIONLOG Logger_SetLogFunction(LOGGERFUNCTIONLOG fn);

#ifdef __cplusplus
}
#endif


#endif /* #ifndef CH_LOGGER_H */


