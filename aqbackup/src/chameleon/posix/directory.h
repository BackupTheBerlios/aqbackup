/***************************************************************************
 $RCSfile: directory.h,v $
                             -------------------
    cvs         : $Id: directory.h,v 1.1 2003/06/07 21:07:50 aquamaniac Exp $
    begin       : Tue Dec 03 2002
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

/**
 * @file chameleon/directory.h
 * @short This file contains the directory handling module
 */


#ifndef CH_DIRECTORY_H
#define CH_DIRECTORY_H


#include <sys/types.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

CHIPCARD_API typedef struct DIRECTORYDATASTRUCT DIRECTORYDATA;


CHIPCARD_API struct DIRECTORYDATASTRUCT {
  DIR *handle;
};


/**
 * Constructor.
 */
CHIPCARD_API DIRECTORYDATA *Directory_new();

/**
 * Destructor
 */
CHIPCARD_API void Directory_free(DIRECTORYDATA *d);



/**
 * Opens a directory. This allows calling "Directory_Read" to succeed.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref Directory_new().
 * @param n path and name of the directory to open
 */
CHIPCARD_API int Directory_Open(DIRECTORYDATA *d, const char *n);

/**
 * Closes a previously opened directory.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref Directory_new().
 */
CHIPCARD_API int Directory_Close(DIRECTORYDATA *d);

/**
 * Reads the next entry from a directory and stores the name of that
 * entry in the given buffer. The entry returned is relative to the
 * open directory.
 * @author Martin Preuss<martin@libchipcard.de>
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref Directory_new().
 * @param buffer pointer to a buffer to receive the name
 * @param len size of the buffer
 */
CHIPCARD_API int Directory_Read(DIRECTORYDATA *d,
				char *buffer,
				int len);

/**
 * Rewinds the internal pointers, so that the next call to
 * @author Martin Preuss<martin@libchipcard.de>
 * @ref Directory_Read will return the first entry of the open directory.
 * @return 0 if ok, !=0 on error
 * @param d pointer to a directory data structure. This should be created
 * by calling @ref Directory_new().
 */
CHIPCARD_API int Directory_Rewind(DIRECTORYDATA *d);


#ifdef __cplusplus
}
#endif


#endif /* CH_DIRECTORY_H */




