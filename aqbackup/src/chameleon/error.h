/***************************************************************************
 $RCSfile: error.h,v $
                             -------------------
    cvs         : $Id: error.h,v 1.1 2003/06/07 21:07:49 aquamaniac Exp $
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

/**
 * @file chameleon/error.h
 * @short This file contains the module for error handling.
 */

#ifndef MOD_ERROR_H
#define MOD_ERROR_H

/**
 * @defgroup mod_error Error module
 * @short This module does all error handling
 * @author Martin Preuss<martin@libchipcard.de>
 *
 * All errors are grouped into error types. There is no predefined error
 * type, all modules, which want to take advantage of this module should
 * register their own error type.
 * When registering an error type, this module learns about the new error
 * type:
 * <ul>
 *  <li>name (like <i>Socket</i>, <i>InetAddress</i>, <i>Time</i> etc.)</li>
 *  <li>function to create human readable error messages for this type</li>
 * </ul>
 */
// @{

/*
 * Allow this to be used from C and C++
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * A function of this type returns a descriptive string for the given
 * error. So it is supposed to translate a 16 bit error value into a human
 * readable text (please note that this code is PART of a ERRORCODE, not the
 * whole ERRORCODE itself !).
 */
CHIPCARD_API typedef const char* (*ERRORMESSAGEPTR)(int c);


/**
 * @struct
 * When registering an error type this form is needed.
 */
CHIPCARD_API typedef struct {
    /**
     * Pointer to the function that returns a descriptive error message
     * for a given error code (must be set by the caller).
     */
    ERRORMESSAGEPTR msgptr;
    /**
     * If the registration applied this field holds the number assigned
     * to this code. The calling module is then guaranteed to have this
     * code for itself (i.e. this number will not assigned to any other
     * module). So when returning an ERRORCODE specific to the module
     * you can use this number as the "type".
     */
    int typ;

    /**
     * This holds the name of the error type. It should be human readable
     * and quite descriptive (like the "Socket" module will use "Socket" as
     * a type name). This way you can alway resolve the number of a known
     * error type by given its name. This field must be set by the caller.
     * This string must be zero terminated (standard c-string).
     */
    char type_name[16];

} ERRORTYPEREGISTRATIONFORM;


/**
 * The error code is a debug level code.
 */
#define ERROR_SEVERITY_DEBUG   0
/**
 * The error code is a information level code.
 */
#define ERROR_SEVERITY_INFO    1
/**
 * The error code is a warning level code.
 */
#define ERROR_SEVERITY_WARN 2
/**
 * The error code is a error level code.
 */
#define ERROR_SEVERITY_ERR   3

/**
 * This class is used for ERROR-related errors ;-)
 */
#define ERROR_TYPE_ERROR 0

#define ERROR_UNSPECIFIED          1
#define ERROR_COULD_NOT_REGISTER   2
#define ERROR_COULD_NOT_UNREGISTER 3
#define ERROR_INVALID_BUFFERSIZE   4


/**
 * An error code is 32 bits long and has the following bit format:
 * <table BORDER="1">
 * <tr><td><b>Bit(s)</b></td><td><b>Meaning</b></td></tr>
 * <tr><td>31-30</td> <td>Severity (0-3)</td></tr>
 * <tr><td>29</td>    <td>Custom error</td></tr>
 * <tr><td>28</td>    <td>Reserved</td></tr>
 * <tr><td>27-16</td> <td>Type (specifies the type of the error, such as
 *           "ERROR_CLASS_SOCKET", "ERROR_CLASS_TIME" etc)</td></tr>
 * <tr><td>15-0</td>  <td>Code (16 bit error code)</td></tr>
 * </table>
 */
CHIPCARD_API typedef unsigned long ERRORCODE;


/**
 * @name Initialization
 */
//@{
/**
 * Initializes this module.
 */
CHIPCARD_API void Error_ModuleInit();

/**
 * Deinitializes this module.
 */
CHIPCARD_API void Error_ModuleFini();

//@}


/**
 * @name Error Type Registration
 * These functions are used to allocate error types. Each module should
 * allocate a type to allow descriptive error messages.
 * Since function pointers are involved you should carefully register and
 * unregister the error types.
 */
//@{

/**
 * Register an error type. This function is called by other modules
 * to register a unique type value for itself.
 * @return 0 on error (success otherwise)
 */
CHIPCARD_API int Error_RegisterType(ERRORTYPEREGISTRATIONFORM *tptr);

/**
 * When removing a module it should always unregister its assigned error
 * type to avoid segfaultes (due to pointers pointing to nowhere).
 * @return 0 on error (success otherwise)
 */
CHIPCARD_API int Error_UnregisterType(ERRORTYPEREGISTRATIONFORM *tptr);
//@}

/**
 * @name Error Type Lookup
 * These functions lookup error types by name or by number.
 */
//@{

/**
 * This function returns the type number for the given type name.
 * @return type number (-1 on error)
 */
CHIPCARD_API int Error_FindType(const char *name);

/**
 * Returns the name of the type referenced by the its type number.
 * This function is used when composing a human readable error string.
 * @return name of the type (0 on error)
 * @param t type number
 */
CHIPCARD_API const char *Error_GetTypename(int t);
//@}


/**
 * @name Getters And Setters
 * These functions get and set parts of an ERRORCODE.
 */
//@{
/**
 * Creates an error code based on the arguments given
 * @return error code based on the given arguments
 * @param iscustom if !=0, then this is a custom code. Such an error code
 * can be used by applications. No module of this library will produce
 * errors with this set, so you can use this to take advantage of the
 * ERRORCODE management even without registering you own error type.
 * @param severity severity level of this code (might be simply a debug code,
 * a warning, an information or an error)
 * @param typ type of this code. As you read above all errors are grouped
 * into error types.
 * @param code error code. This needs only to be unique within the error
 * type (in fact that was the reason to introduce the "error type")
 */
CHIPCARD_API ERRORCODE Error_New(int iscustom, int severity, int typ, int code);

/**
 * Checks whether the code really contains an error. If it contains a
 * debug/info/warn code then it will be treaten as "ok".
 * @return !=0 if ok, 0 if it really is an error
 */
CHIPCARD_API int Error_IsOk(ERRORCODE c);

/**
 * Returns the severity of the error
 */
CHIPCARD_API int Error_GetSeverity(ERRORCODE c);

/**
 * Sets the severity level.
 */
CHIPCARD_API void Error_SetSeverity(ERRORCODE *c, int v);

/**
 * Checks whether this is a custom error code (which means that the error
 * type and code has to be handled differently. Such a code might be used
 * in programs).
 * @return !=0 if it is a custom code, 0 otherwise
 */
CHIPCARD_API int Error_IsCustom(ERRORCODE c);

/**
 * Sets the custom flag.
 */
CHIPCARD_API void Error_SetIsCustom(ERRORCODE *c, int iscustom);

/**
 * Returns the error type.
 */
CHIPCARD_API int Error_GetType(ERRORCODE c);

/**
 * Sets the error type.
 */
CHIPCARD_API void Error_SetType(ERRORCODE *c, int v);

/**
 * Returns the 16 bit error code value.
 */
CHIPCARD_API int Error_GetCode(ERRORCODE c);

/**
 * Sets the error code.
 */
CHIPCARD_API void Error_SetCode(ERRORCODE *c, int v);
//@}


/**
 * @name Verbosity
 * Composing error messages.
 */
//@{
/**
 * Composes a human readable error string which dumps all parts of an
 * ERRORCODE.
 * @return 1 detailed error message created. If some parts of the ERRORCODE
 * could not be translated (e.g. unassigned type number etc) a 0 is returned.
 * @param buffer pointer to a buffer to receive the message
 * @param bsize size of that buffer in bytes
 */
CHIPCARD_API int Error_ToString(ERRORCODE c, char *buffer, int bsize);
//@}

#ifdef __cplusplus
}
#endif

//@} group mod_error


#endif // MOD_ERROR_H


