/***************************************************************************
 $RCSfile: conf.h,v $
                             -------------------
    cvs         : $Id: conf.h,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
    begin       : Mon Dec 02 2002
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
 * @file chameleon/conf.h
 *
 * This file contains the configuration file parser and generator
 */

#ifndef CHAMELEON_CONF_H
#define CHAMELEON_CONF_H

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


#define CONFIGMODE_PATHMUSTEXIST          0x000001
#define CONFIGMODE_PATHMUSTNOTEXIST       0x000002
#define CONFIGMODE_PATHCREATE             0x000004

#define CONFIGMODE_NAMEMUSTEXIST          0x000008
#define CONFIGMODE_NAMEMUSTNOTEXIST       0x000010
#define CONFIGMODE_NAMECREATE_GROUP       0x000020
#define CONFIGMODE_NAMECREATE_VARIABLE    0x000040

#define CONFIGMODE_VARIABLE               0x000080
#define CONFIGMODE_OVERWRITE_VARS         0x000100
#define CONFIGMODE_OVERWRITE_GROUPS       0x000200
#define CONFIGMODE_REMOVE_QUOTES          0x000400
#define CONFIGMODE_REMOVE_STARTING_BLANKS 0x000800
#define CONFIGMODE_REMOVE_TRAILING_BLANKS 0x001000
#define CONFIGMODE_ALLOW_PATH_IN_VARS     0x002000
#define CONFIGMODE_ALLOW_GROUPS           0x004000
#define CONFIGMODE_EMPTY_GROUPS           0x008000


CHIPCARD_API typedef struct CONFIGVALUESTRUCT CONFIGVALUE;
CHIPCARD_API typedef struct CONFIGVARIABLESTRUCT CONFIGVARIABLE;
CHIPCARD_API typedef struct CONFIGGROUPSTRUCT CONFIGGROUP;


/**
 */
CHIPCARD_API struct CONFIGVALUESTRUCT {
  CONFIGVALUE *next;
  char *value;
};


/**
 */
CHIPCARD_API struct CONFIGVARIABLESTRUCT {
  CONFIGVARIABLE *next;
  char *name;
  CONFIGGROUP *parent;
  CONFIGVALUE *values;
};


/**
 */
CHIPCARD_API struct CONFIGGROUPSTRUCT {
  CONFIGGROUP *next;
  char *name;
  CONFIGGROUP *parent;
  CONFIGGROUP *groups;
  CONFIGVARIABLE *variables;
};


/**
 * Constructor. Please always use this constructor to create a configuration
 * structure.
 */
CHIPCARD_API CONFIGGROUP *Config_new();

/**
 * Destructor.
 */
CHIPCARD_API void Config_free(CONFIGGROUP *g);


/**
 * Get the value of a variable or a default value if the variable does not
 * exist or doesn't have a value.
 * @param root root to which the path is relative
 * @param path path and name of the variable (e.g. "global/settings/var1")
 * @param defaultValue this value is returned if the variable does not exist
 * or if the variable has no value
 * @param idx a variable may have multiple values, so you can choose here
 * which value you want (starting with 0)
 */
CHIPCARD_API const char *Config_GetValue(CONFIGGROUP *root,
					 const char *path,
					 const char *defaultValue,
					 int idx);

/**
 * Get an integer value. This internally calls @ref Config_GetValue, please
 * see there for details.
 */
CHIPCARD_API int Config_GetIntValue(CONFIGGROUP *root,
				    const char *path,
				    int defaultValue,
				    int idx);

/**
 * Get a time value. Format of the parsed string is:
 * "YEAR/MONTH/DAY-HOUR:MINUTE:SECOND"
 */
time_t Config_GetTimeValue(CONFIGGROUP *root,
			   const char *path,
			   time_t defaultValue,
			   int idx);

/**
 * Set the value of a variable in respect to the mode given.
 * @param root root to which the path is relative
 * @param mode this specifies how the value is set. This mode consists
 * of one or multiple "CONFIGMODE_XXX" values which may be ORed.
 * @param path path of the variable (e.g. "global/settings/var1")
 * @param value value to set
 */
CHIPCARD_API int Config_SetValue(CONFIGGROUP *root,
				 unsigned int mode,
				 const char *path,
				 const char *value);

/**
 * This is just a conveniance function which internally calls
 * @ref Config_SetValue. Please see there for details.
 */
CHIPCARD_API int Config_SetIntValue(CONFIGGROUP *root,
				    unsigned int mode,
				    const char *path,
				    int value);

int Config_SetTimeValue(CONFIGGROUP *root,
			unsigned int mode,
			const char *path,
			time_t value);

/**
 * Adds a value to a given variable.
 * @param root root to which the path is relative
 * @param mode this specifies how the value is set. This mode consists
 * of one or multiple "CONFIGMODE_XXX" values which may be ORed.
 * @param path path of the variable (e.g. "global/settings/var1")
 * @param value value to add
 */
CHIPCARD_API int Config_AddValue(CONFIGGROUP *root,
				 unsigned int mode,
				 const char *path,
				 const char *value);

/**
 * Clears a variable, so all it's values are destroyed.
 */
CHIPCARD_API int Config_ClearVariable(CONFIGGROUP *root,
				      unsigned int mode,
				      const char *path);

/**
 * This function parses a text line and stores variables/groups read
 * in the given root.
 * @return current group (this may have changed depending on the content
 * of the text line just parsed
 * @param root root to which all paths are relative
 * @param group current group within the root, this is the group all variables
 * parsed are relative to. Please note that group paths are relative to
 * the root, not to group !
 * @param mode specifies how to store the groups and variables. The mode
 * consists of one or multiple CONFIGMODE_XYZ values which may be logically
 * ORed.
 */
CHIPCARD_API CONFIGGROUP *Config_ParseLine(CONFIGGROUP *root,
					   CONFIGGROUP *group,
					   const char *s,
					   int mode);

/**
 * Reads a configuration file and parses it.
 * @param fname path and name of the file to read.
 * @param mode specifies how to store the groups and variables. The mode
 * consists of one or multiple CONFIGMODE_XYZ values which may be logically
 * ORed.
 */
CHIPCARD_API int Config_ReadFile(CONFIGGROUP *root,
				 const char *fname,
				 int mode);

/**
 * Creates a configuration file from the given configuration.
 * @param fname path and name of the file to create
 * @param mode specifies how to store the groups and variables. The mode
 * consists of one or multiple CONFIGMODE_XYZ values which may be logically
 * ORed.
 */
CHIPCARD_API int Config_WriteFile(CONFIGGROUP *root,
				  const char *fname,
				  int mode);

/**
 * Adds a variable to a given group.
 */
CHIPCARD_API void Config_AddVariable(CONFIGGROUP *p, CONFIGVARIABLE *v);


/**
 * Dumps the content of a configuration into an open file.
 * This is used for debugging purposes, where f is stderr.
 * @param f file to dump to (most cases "stderr")
 * @param root group to dump
 * @param ins number of spaces to prepend to each output line, this is used
 * to show hierarchies.
 */
CHIPCARD_API int Config_DumpGroup(FILE *f, CONFIGGROUP *root,
				  int ins);

/**
 * Lookup a group by its name.
 */
CHIPCARD_API CONFIGGROUP *Config_GetGroup(CONFIGGROUP *root,
					  const char *path,
					  unsigned int mode);


/**
 * Adds a group to another one. The new group becomes the child of the
 * existing one.
 * @param p parent (group to which to add the new group)
 * @param w new group to add
 */
CHIPCARD_API void Config_AddGroup(CONFIGGROUP *p, CONFIGGROUP *w);

/**
 * Adds all groups and variables which are the children of the given
 * group to the given parent by making deep copies.
 */
void Config_ImportTreeChildren(CONFIGGROUP *p, CONFIGGROUP *g);

/**
 * Unlinks a group from the configuration. This group will not be destroyed.
 * @todo Hmm, this function may change, since the parent pointer is already
 * stored within the group struct.
 * @param p parent of the group
 */
CHIPCARD_API void Config_UnlinkGroup(CONFIGGROUP *p, CONFIGGROUP *w);

/**
 * Clears a group, so all it's subgroups and variables are destroyed.
 */
CHIPCARD_API void Config_ClearGroup(CONFIGGROUP *g);


/**
 * Duplicates a group. The new group is a deep copy of the original.
 */
CHIPCARD_API CONFIGGROUP *Config_Duplicate(CONFIGGROUP *root);

/**
 * Imports a new group into an existing configuration (including all the
 * subgroups and variables of the given group)
 */
CHIPCARD_API void Config_ImportGroup(CONFIGGROUP *root, CONFIGGROUP *g);


/**
 * Just a conveniance function for case insensitive compares.
 * Well, meanwhile I now that there already are such functions ;-)
 * @return result of "s1-s2"
 */
CHIPCARD_API int Config_Compare(const char *s1, const char *s2);

#ifdef __cplusplus
}
#endif


#endif




