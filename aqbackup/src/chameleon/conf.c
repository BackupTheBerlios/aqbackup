/***************************************************************************
 $RCSfile: conf.c,v $
                             -------------------
    cvs         : $Id: conf.c,v 1.1 2003/06/07 21:07:48 aquamaniac Exp $
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


#include "conf.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


#ifdef MEMTRACE
static int CONFIGVARIABLE_Counter=0;
static int CONFIGGROUP_Counter=0;
#endif


CONFIGVALUE *Config__Value_new(const char *value){
  CONFIGVALUE *v;

  v=(CONFIGVALUE*)malloc(sizeof(CONFIGVALUE));
  assert(v);
  memset(v, 0, sizeof(CONFIGVALUE));
  if (value) {
    v->value=(char*)malloc(strlen(value)+1);
    assert(v->value);
    memmove(v->value, value, strlen(value)+1);
  }
  return v;
}


void Config__Value_free(CONFIGVALUE *v){
  if (v) {
    if (v->value)
      free(v->value);
    free(v);
  }
}


CONFIGVARIABLE *Config__Variable_new(const char *name,
				     const char *value){
  CONFIGVARIABLE *v;

  v=(CONFIGVARIABLE*)malloc(sizeof(CONFIGVARIABLE));
  assert(v);
  memset(v, 0, sizeof(CONFIGVARIABLE));
  if (name) {
    v->name=(char*)malloc(strlen(name)+1);
    assert(v->name);
    memmove(v->name, name, strlen(name)+1);
  }
  if (value)
    v->values=Config__Value_new(value);
#ifdef MEMTRACE
  CONFIGVARIABLE_Counter++;
  DBG_INFO("Created variable, now %d",CONFIGVARIABLE_Counter);
#endif
  return v;
}


void Config__Variable_Clear(CONFIGVARIABLE *v) {
  assert(v);
  if (v->values) {
    CONFIGVALUE *val;
    CONFIGVALUE *next;

    val=v->values;
    while(val) {
      next=val->next;
      Config__Value_free(val);
      val=next;
    } /* while */
  } /* if values */
}


void Config__Variable_free(CONFIGVARIABLE *v){
  DBG_ENTER;
  if (v) {
    Config__Variable_Clear(v);
    free(v->name);
    free(v);
#ifdef MEMTRACE
    CONFIGVARIABLE_Counter--;
    DBG_INFO("Freed variable, now %d",CONFIGVARIABLE_Counter);
#endif
  }
  DBG_LEAVE;
}


CONFIGGROUP * Config__Group_new(const char *name){
  CONFIGGROUP *g;

  DBG_VERBOUS("Creating group \"%s\"",name);
  g=(CONFIGGROUP*)malloc(sizeof(CONFIGGROUP));
  assert(g);
  memset(g, 0, sizeof(CONFIGGROUP));
  if (name) {
    g->name=(char*)malloc(strlen(name)+1);
    assert(g->name);
    memmove(g->name, name, strlen(name)+1);
  }
#ifdef MEMTRACE
  CONFIGGROUP_Counter++;
  DBG_NOTICE("Created group, now %d",CONFIGGROUP_Counter);
#endif
  return g;
}


void Config__Group_free(CONFIGGROUP *g);

void Config__Group_Clear(CONFIGGROUP *g) {
  CONFIGGROUP *currg, *nextg;
  CONFIGVARIABLE *currv, *nextv;

  assert(g);

  /* free all variables */
  currv=g->variables;
  while(currv) {
    nextv=currv->next;
    Config__Variable_free(currv);
    currv=nextv;
  } /* while */
  g->variables=0;

  /* free all subgroups */
  currg=g->groups;
  while(currg) {
    nextg=currg->next;
    Config__Group_free(currg);
    currg=nextg;
  } /* while */
  g->groups=0;

}


void Config__Group_free(CONFIGGROUP *g){
  DBG_ENTER;
  if (g) {
    Config__Group_Clear(g);
    free(g->name);
    free(g);
#ifdef MEMTRACE
    CONFIGGROUP_Counter--;
    DBG_NOTICE("Freed group, now %d",CONFIGGROUP_Counter);
#endif
  }
  DBG_LEAVE;
}


void Config_AddGroup(CONFIGGROUP *p, CONFIGGROUP *w){
  CONFIGGROUP *curr;

  DBG_ENTER;
  assert(p);
  assert(w);

  w->parent=p;
  curr=p->groups;
  if (!curr) {
    p->groups=w;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=w;
  }
  DBG_LEAVE;
}


void Config_UnlinkGroup(CONFIGGROUP *p, CONFIGGROUP *w){
  CONFIGGROUP *curr;

  DBG_ENTER;
  assert(p);
  assert(w);

  curr=p->groups;
  if (curr) {
    if (curr==w) {
      p->groups=curr->next;
    }
    else {
      /* find predecessor */
      while(curr->next!=w) {
	curr=curr->next;
      } /* while */
      if (curr)
	curr->next=w->next;
    }
  }
  w->parent=0;
  w->next=0;
  DBG_LEAVE;
}


CONFIGGROUP *Config__FindGroup(CONFIGGROUP *p, const char *name){
  CONFIGGROUP *curr;

  DBG_ENTER;
  assert(p);
  assert(name);
  curr=p->groups;

  DBG_VERBOUS("Find group \"%s\"",name);
  while(curr) {
    if (curr->name) {
      if (Config_Compare(curr->name, name)==0) {
	DBG_DEBUG("Group \"%s\" found", name);
	DBG_LEAVE;
	return curr;
      }
    }
    curr=curr->next;
  } /* while */
  DBG_DEBUG("Group \"%s\" not found", name);
  DBG_LEAVE;
  return 0;
}


void Config__AddVariable(CONFIGGROUP *p, CONFIGVARIABLE *v){
  CONFIGVARIABLE *curr;

  DBG_ENTER;
  assert(p);
  assert(v);

  v->parent=p;
  curr=p->variables;
  if (!curr) {
    p->variables=v;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=v;
  }
  DBG_LEAVE;
}


void Config__UnlinkVariable(CONFIGGROUP *p, CONFIGVARIABLE *w){
  CONFIGVARIABLE *curr;

  DBG_ENTER;
  assert(p);
  assert(w);

  curr=p->variables;
  if (curr) {
    if (curr==w) {
      p->variables=curr->next;
    }
    else {
      /* find predecessor */
      while(curr->next!=w) {
	curr=curr->next;
      } /* while */
      if (curr)
	curr->next=w->next;
    }
  }
  w->parent=0;
  w->next=0;
  DBG_LEAVE;
}


CONFIGVARIABLE *Config__FindVariable(CONFIGGROUP *p, const char *name){
  CONFIGVARIABLE *curr;

  DBG_ENTER;
  assert(p);
  assert(name);
  curr=p->variables;

  while(curr) {
    if (curr->name)
      if (Config_Compare(curr->name, name)==0) {
	DBG_DEBUG("Variable \"%s\" found", name);
        DBG_LEAVE;
	return curr;
      }
    curr=curr->next;
  } /* while */
  DBG_DEBUG("Variable \"%s\" not found", name);
  DBG_LEAVE;
  return 0;
}


void Config__AddValue(CONFIGVARIABLE *p, CONFIGVALUE *v){
  CONFIGVALUE *curr;

  DBG_ENTER;
  assert(p);
  assert(v);

  curr=p->values;
  if (!curr) {
    p->values=v;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=v;
  }
  DBG_LEAVE;
}


void Config__UnlinkValue(CONFIGVARIABLE *p, CONFIGVALUE *w){
  CONFIGVALUE *curr;

  DBG_ENTER;
  assert(p);
  assert(w);

  curr=p->values;
  if (curr) {
    if (curr==w) {
      p->values=curr->next;
    }
    else {
      /* find predecessor */
      while(curr->next!=w) {
	curr=curr->next;
      } /* while */
      if (curr)
	curr->next=w->next;
    }
  }
  w->next=0;
  DBG_LEAVE;
}


void Config__SetValue(CONFIGVARIABLE *p, const char *value){
  assert(p);

  Config__Variable_Clear(p);
  if (value)
    p->values=Config__Value_new(value);
}


CONFIGVALUE *Config__FindValue(CONFIGVARIABLE *p, const char *value){
  CONFIGVALUE *curr;

  DBG_ENTER;
  assert(p);
  assert(value);
  curr=p->values;

  while(curr) {
    if (curr->value)
      if (Config_Compare(curr->value, value)==0) {
	DBG_DEBUG("Value \"%s\" found", value);
	DBG_LEAVE;
	return curr;
      }
    curr=curr->next;
  } /* while */
  DBG_DEBUG("Value \"%s\" not found", value);
  DBG_LEAVE;
  return 0;
}


void *Config__GetPath(CONFIGGROUP *root,
		      const char *path,
		      unsigned int mode) {
  CONFIGGROUP *curr;
  CONFIGVARIABLE *var;
  char currname[256];
  const char *psrc;
  char *pdest;
  int i;

  assert(root);
  assert(path);

  psrc=path;
  DBG_VERBOUS("Getting path \"%s\"",path);
  if (*psrc=='/')
    psrc++;

  while(*psrc) {
    /* get path element */
    pdest=currname;
    i=sizeof(currname)-1;
    currname[0]=0;
    while(*psrc && i--) {
      if (*psrc=='/')
	break;
      *pdest=*psrc;
      psrc++;
      pdest++;
    } /* while */
    if (!i) {
      DBG_ERROR("Path element too long (limit is %d chars)",
		sizeof(currname))
	return 0;
    }
    *pdest=0;
    if (*psrc)
      psrc++;

    if (*psrc==0) {
      /* handling name (last path element) */
      if (mode & CONFIGMODE_VARIABLE) {
	DBG_VERBOUS("Want variable");
	/* want a variable */
	if ((mode & CONFIGMODE_NAMECREATE_VARIABLE) ||
	    (mode & CONFIGMODE_PATHCREATE)) {
	  DBG_VERBOUS("Forced to create variable \"%s\" in \"%s\"",
		   currname, root->name);
	  var=Config__Variable_new(currname,0);
	  Config__AddVariable(root, var);
	}
	else {
	  var=Config__FindVariable(root, currname);
	  if (!var) {
	    if (mode & CONFIGMODE_NAMEMUSTEXIST) {
	      DBG_DEBUG("Path \"%s\" not found (%s)",path, currname);
	      return 0;
	    }
	    DBG_VERBOUS("Variable \"%s\" does not exists, creating it",
		      currname);
	    var=Config__Variable_new(currname,0);
	    Config__AddVariable(root, var);
	  }
	  else {
	    if (mode & CONFIGMODE_NAMEMUSTNOTEXIST) {
	      DBG_DEBUG("Path \"%s\" already exists (%s)",path, currname);
	      return 0;
	    }
	  }
	}
	DBG_DEBUG("Path \"%s\" available", path);
	DBG_LEAVE;
	return var;
      } /* if wantVariable */
      else {
	DBG_VERBOUS("Want group");
        /* want a group */
	if ((mode & CONFIGMODE_NAMECREATE_GROUP) ||
	    (mode & CONFIGMODE_PATHCREATE)) {
	  DBG_DEBUG("Forced to create group \"%s\" in \"%s\"",
		   currname, root->name);
	  curr=Config__Group_new(currname);
	  Config_AddGroup(root, curr);
	}
	else {
	  curr=Config__FindGroup(root, currname);
	  if (!curr) {
	    if (mode & CONFIGMODE_NAMEMUSTEXIST) {
	      DBG_DEBUG("Path \"%s\" not found (%s)",path, currname);
	      return 0;
	    }
	    DBG_DEBUG("Group \"%s\" does not exists in \"%s\", creating it",
		     currname, root->name);
	    curr=Config__Group_new(currname);
	    Config_AddGroup(root, curr);
	  }
	  else {
	    if (mode & CONFIGMODE_NAMEMUSTNOTEXIST) {
	      DBG_DEBUG("Path \"%s\" already exists (%s)",path, currname);
	      return 0;
	    }
	  }
	}
	DBG_VERBOUS("Path \"%s\" available", path);
	DBG_LEAVE;
	return curr;
      } /* "else" of "if wantVariable" */
    } /* if *path==0 */
    else {
      /* not at the end of the path */
      if (mode & CONFIGMODE_PATHCREATE) {
	DBG_VERBOUS("Forced to create group \"%s\" in \"%s\"",
		 currname, root->name);
	curr=Config__Group_new(currname);
	Config_AddGroup(root, curr);
      }
      else {
	curr=Config__FindGroup(root, currname);
	if (!curr) {
	  if (mode & CONFIGMODE_PATHMUSTEXIST) {
	    DBG_DEBUG("Path \"%s\" not found (%s)",path, currname);
	    return 0;
	  }
	  DBG_VERBOUS("Group \"%s\" does not exists, creating it",currname);
	  curr=Config__Group_new(currname);
	  Config_AddGroup(root, curr);
	}
	else {
	  if (mode & CONFIGMODE_PATHMUSTNOTEXIST) {
	    DBG_DEBUG("Path \"%s\" already exists (%s)",path, currname);
	    return 0;
	  }
	}
      }
      root=curr;
    }
  } /* while */
  DBG_ERROR("Uuups :-} We should never reach this point (\"%s\")", path);
  return 0;
}


CONFIGVALUE *Config__Value_duplicate(CONFIGVALUE *p){
  CONFIGVALUE *copy;

  assert(p);
  DBG_VERBOUS("Duplicating value \"%s\"",p->value);
  copy=Config__Value_new(p->value);
  return copy;
}


CONFIGVARIABLE *Config__Variable_duplicate(CONFIGVARIABLE *p){
  CONFIGVARIABLE *copy;
  CONFIGVALUE *val1, *val2;

  assert(p);
  DBG_VERBOUS("Duplicating variable \"%s\"",p->name);
  copy=Config__Variable_new(p->name,0);

  val1=p->values;
  while(val1) {
    val2=Config__Value_duplicate(val1);
    Config__AddValue(copy,val2);
    val1=val1->next;
  }
  return copy;
}


CONFIGGROUP *Config__Group_duplicate(CONFIGGROUP *p){
  CONFIGGROUP *copy;
  CONFIGVARIABLE *var1, *var2;
  CONFIGGROUP *g1, *g2;

  assert(p);
  DBG_VERBOUS("Duplicating group \"%s\"",p->name);
  copy=Config__Group_new(p->name);

  var1=p->variables;
  while(var1) {
    var2=Config__Variable_duplicate(var1);
    Config__AddVariable(copy,var2);
    var1=var1->next;
  }

  g1=p->groups;
  while(g1) {
    g2=Config__Group_duplicate(g1);
    Config_AddGroup(copy, g2);
    g1=g1->next;
  }
  return copy;
}


void Config_ImportTreeChildren(CONFIGGROUP *p, CONFIGGROUP *g) {
  CONFIGGROUP *currg, *newg;
  CONFIGVARIABLE *currv, *newv;

  assert(p);
  assert(g);

  /* add all variables */
  currv=g->variables;
  while(currv) {
    newv=Config__Variable_duplicate(currv);
    Config__AddVariable(p, newv);
    currv=currv->next;
  } /* while */

  /* free all subgroups */
  currg=g->groups;
  while(currg) {
    newg=Config__Group_duplicate(currg);
    Config_AddGroup(p, newg);
    currg=currg->next;
  } /* while */
}









const char *Config_GetValue(CONFIGGROUP *root,
			    const char *path,
			    const char *defaultValue,
			    int idx){
  CONFIGVARIABLE *var;
  CONFIGVALUE *val;

  assert(root);
  assert(path);
  DBG_VERBOUS("GetValue for \"%s\"",path);
  var=Config__GetPath(root,path,
		      CONFIGMODE_PATHMUSTEXIST |
		      CONFIGMODE_NAMEMUSTEXIST |
		      CONFIGMODE_VARIABLE);
  if (!var) {
    DBG_DEBUG("Returning default value for path \"%s\"",path);
    return defaultValue;
  }
  val=var->values;
  while (val && idx) {
    val=val->next;
    idx--;
  }
  if (!val) {
    DBG_DEBUG("Returning default value for path \"%s\"",path);
    return defaultValue;
  }

  return val->value;;
}


int Config_GetIntValue(CONFIGGROUP *root,
		       const char *path,
		       int defaultValue,
		       int idx){
  const char *p;
  int val;

  p=Config_GetValue(root, path, 0, idx);
  if (!p) {
    DBG_DEBUG("Returning default value for \"%s\"", path);
    return defaultValue;
  }
  if (sscanf(p,"%i",&val)!=1) {
    DBG_DEBUG("Bad value for \"%s\", will return default value instead",
	      path);
    return defaultValue;
  }
  return val;
}



time_t Config_GetTimeValue(CONFIGGROUP *root,
			   const char *path,
			   time_t defaultValue,
			   int idx){
  char buffer[128];
  struct tm tm;
  struct tm *tt;
  const char *p;
  time_t val;
  int year;

  p=Config_GetValue(root, path, 0, idx);
  if (!p) {
    DBG_INFO("Returning default value for \"%s\"", path);
    return defaultValue;
  }

  val=time(0);
  tt=localtime(&val);
  memmove(&tm, tt, sizeof(tm));

  buffer[0]=0;
  if (sscanf(p,"%d/%d/%d-%d:%d:%d",
	     &year,
	     &tm.tm_mon,
	     &tm.tm_mday,
	     &tm.tm_hour,
	     &tm.tm_min,
	     &tm.tm_sec)!=6) {
    DBG_INFO("Invalid time, returning default value for \"%s\"", path);
    return defaultValue;
  }
  tm.tm_year=year-1900;
  tm.tm_isdst=-1;
  val=mktime(&tm);
  return val;
}





int Config_SetValue(CONFIGGROUP *root,
		    unsigned int mode,
		    const char *path,
		    const char *value){
  CONFIGVARIABLE *var;

  assert(root);
  assert(path);

  DBG_VERBOUS("SetValue for \"%s\"",path);
  var=(CONFIGVARIABLE*)Config__GetPath(root,path,
				       mode | CONFIGMODE_VARIABLE);
  if (!var) {
    DBG_DEBUG("Path not available");
    return 1;
  }
  if (mode & CONFIGMODE_OVERWRITE_VARS) {
    Config__Variable_Clear(var);
    if (value)
      var->values=Config__Value_new(value);
  }
  else {
    CONFIGVALUE *val;

    val=Config__Value_new(value);
    Config__AddValue(var, val);
  }
  return 0;
}


int Config_SetIntValue(CONFIGGROUP *root,
		       unsigned int mode,
		       const char *path,
		       int value){
  char buffer[32];

  buffer[0]=0;
  sprintf(buffer,"%d",value);
  return Config_SetValue(root,
			 mode,
			 path,
			 buffer);
}


int Config_SetTimeValue(CONFIGGROUP *root,
			unsigned int mode,
			const char *path,
			time_t value){
  char buffer[128];
  struct tm *tm;

  tm=localtime(&value);
  assert(tm);

  buffer[0]=0;
  sprintf(buffer,"%4d/%02d/%02d-%02d:%02d:%02d",
	  tm->tm_year+1900,
	  tm->tm_mon,
	  tm->tm_mday,
	  tm->tm_hour,
	  tm->tm_min,
	  tm->tm_sec);
  return Config_SetValue(root,
			 mode,
			 path,
			 buffer);
}


int Config_AddValue(CONFIGGROUP *root,
		    unsigned int mode,
		    const char *path,
		    const char *value){
  CONFIGVARIABLE *var;
  CONFIGVALUE *val;

  assert(root);
  assert(path);
  DBG_VERBOUS("AddValue for \"%s\"",path);
  var=(CONFIGVARIABLE*)Config__GetPath(root,path,
				       mode | CONFIGMODE_VARIABLE);
  if (!var) {
    DBG_DEBUG("Path not available");
    return 1;
  }
  val=Config__Value_new(value);
  Config__AddValue(var, val);
  return 0;
}


int Config_ClearVariable(CONFIGGROUP *root,
			 unsigned int mode,
			 const char *path){
  CONFIGVARIABLE *var;

  assert(root);
  assert(path);
  DBG_VERBOUS("ClearValue for \"%s\"",path);
  var=(CONFIGVARIABLE*)Config__GetPath(root,path,
				       mode | CONFIGMODE_VARIABLE);
  if (!var) {
    DBG_DEBUG("Path not available");
    return 1;
  }
  Config__Variable_Clear(var);
  return 0;
}



CONFIGGROUP *Config_new(){
  return Config__Group_new(0);
}


void Config_free(CONFIGGROUP *g){
  Config__Group_free(g);
}


CONFIGGROUP *Config_ParseLine(CONFIGGROUP *root,
			      CONFIGGROUP *group,
			      const char *s,
			      int mode) {
  CONFIGVARIABLE *var;
  char name[256];
  char *np;
  char *p;
  const char *g;
  int i;
  int quotes;
  int esc;
  int firstval;

  assert(s);
  name[0]=0;

  /* check for group definition */
  g=s;
  while(*g && (unsigned char)(*g)<33)
    g++;
  if (*g=='[') {
    if (mode & CONFIGMODE_ALLOW_GROUPS) {
      /* ok, parse group name */
      CONFIGGROUP *grp;

      s=g;
      s++;
      while(*s && (unsigned char)(*s)<33)
	s++;
      p=name;
      i=sizeof(name)-1;
      while ((unsigned char)(*s)>31 && i && *s!=']' && *s!='#') {
	*p=*s;
	p++;
	s++;
      } /* while */
      if (!i) {
	DBG_ERROR("Groupname is too long (limit is %d chars)",sizeof(name)-1);
	return 0;
      }
      if (*s!=']') {
	DBG_ERROR("\"]\" expected");
	return 0;
      }
      *p=0;
      DBG_VERBOUS("Selecting group \"%s\"",name);
      grp=Config__GetPath(root, name, mode &~CONFIGMODE_VARIABLE);
      if (!grp) {
	DBG_DEBUG("Group \"%s\" is not available",name);
	return 0;
      }
      return grp;
    }
    else {
      DBG_ERROR("Group definition not allowed");
      return 0;
    }
  }

  /* get name */
  if (mode & CONFIGMODE_REMOVE_STARTING_BLANKS)
    while(*s && (unsigned char)(*s)<33)
      s++;
  i=sizeof(name)-1;
  p=name;
  while ((unsigned char)(*s)>31 && i-- && *s!='=' && *s!='#') {
    *p=*s;
    p++;
    s++;
  } /* while */
  if (!i) {
    DBG_ERROR("Name is too long (limit is %d chars)",sizeof(name)-1);
    return 0;
  }
  *p=0;
  np=name;

  /* post process name */
  if (mode & CONFIGMODE_REMOVE_TRAILING_BLANKS) {
    i=strlen(name)-1;
    while (i>=0) {
      if ((unsigned char)(name[i])<33)
	name[i]=0;
      else
        break;
      i--;
    }

    if (mode & CONFIGMODE_REMOVE_QUOTES) {
      i=strlen(name);
      if (i>1) {
	if (name[i-1]=='"' &&
	    name[0]=='"') {
	  name[i-1]=0;
	  np++;
	}
      }
    }
  }
  if ((unsigned char)(*s)<31 || *s=='#') {
    DBG_VERBOUS("Empty line");
    return group;
  }

  /* get equation mark */
  if (*s!='=') {
    DBG_ERROR("\"=\" expected");
    return 0;
  }
  s++;

  if (strlen(np)==0) {
    DBG_ERROR("Variable name must not be empty");
    return 0;
  }

  DBG_VERBOUS("Creating variable \"%s\"",np);
  if (mode & CONFIGMODE_ALLOW_PATH_IN_VARS)
    var=Config__GetPath(group,
			np,
			mode|CONFIGMODE_VARIABLE);
  else {
    if (!(mode & CONFIGMODE_NAMECREATE_VARIABLE)) {
      var=group->variables;
      while(var) {
	if (strcasecmp(var->name, np)==0)
	  break;
        var=var->next;
      } /* while */
    }
    else
      var=0;
    if (!var) {
      var=Config__Variable_new(np,0);
      Config__AddVariable(group, var);
    }
  }
  if (!var) {
    DBG_DEBUG("Could not create variable \"%s\"",np);
    return 0;
  }

  firstval=1;
  /* read komma separated values */
  while ((unsigned char)(*s)>31) {
    CONFIGVALUE *val;
    char value[1024];
    char *vp;

    value[0]=0;

    /* skip komma that may occur */
    if (mode & CONFIGMODE_REMOVE_STARTING_BLANKS)
      while(*s && (unsigned char)(*s)<33)
	s++;
    if (*s==0) {
      break;
    }
    if (*s==',') {
      if (firstval) {
	DBG_ERROR("Unexpected komma");
	return 0;
      }
      s++;
    }
    else {
      if (!firstval) {
	DBG_ERROR("Komma expected");
	return 0;
      }
    }

    /* get value */
    if (mode & CONFIGMODE_REMOVE_STARTING_BLANKS)
      while(*s && (unsigned char)(*s)<33)
	s++;
    i=sizeof(value)-1;
    p=value;
    /* copy value */
    quotes=0;
    esc=0;
    while ((unsigned char)(*s)>31 && i) {
      if (esc) {
	*p=*s;
	p++;
        i--;
	esc=0;
      }
      else {
	if (*s=='\\')
	  esc=1;
	else if (*s=='"') {
	  quotes++;
	  if (quotes==2) {
	    s++;
	    break;
	  }
	}
	else if (*s=='#' && !(quotes&1))
	  break;
	else if (*s==',' && !(quotes&1))
	  break;
	else {
	  *p=*s;
	  p++;
	  i--;
	}
      }
      s++;
    } /* while */
    if (!i) {
      DBG_ERROR("Value is too long (limit is %d chars)",sizeof(value)-1);
      return 0;
    }
    if (quotes&1) {
      DBG_ERROR("Unbalanced quotation marks");
      return 0;
    }
    if (esc)
      DBG_WARN("Backslash at the end of the line");
    *p=0;
    vp=value;
    /* post process value */
    if (mode & CONFIGMODE_REMOVE_TRAILING_BLANKS && quotes==0) {
      i=strlen(value)-1;
      while (i>=0) {
	if ((unsigned char)(value[i])<33)
	  value[i]=0;
	else
          break;
	i--;
      }

      if (mode & CONFIGMODE_REMOVE_QUOTES) {
	i=strlen(value);
	if (i>1) {
	  if (value[i-1]=='"' &&
	      value[0]=='"') {
	    value[i-1]=0;
	    vp++;
	  }
	}
      }
    }
    /* create value, append it */
    DBG_VERBOUS(" Creating value \"%s\"",vp);
    val=Config__Value_new(vp);
    Config__AddValue(var, val);

    if (*s=='#')
      break;
    firstval=0;
  } /* while (reading values) */

  return group;
}


int Config_ReadFile(CONFIGGROUP *root,
		    const char *fname,
		    int mode){
  FILE *f;
  char lbuffer[2048];
  CONFIGGROUP *curr;
  int ln;

  assert(root);
  assert(fname);


  curr=root;
  f=fopen(fname,"r");
  if (f==0) {
    DBG_ERROR("Error on fopen(%s): %s",fname,strerror(errno));
    return 1;
  }
  ln=1;
  while (!feof(f)) {
    lbuffer[0]=0;
    if (fgets(lbuffer, sizeof(lbuffer)-1, f)==0)
      break;
    curr=Config_ParseLine(root, curr, lbuffer, mode);
    if (!curr) {
      DBG_ERROR("Error in %s:%d",fname, ln);
      fclose(f);
      return 1;
    }
    ln++;
  } /* while */
  if (ferror(f)) {
    DBG_ERROR("Error on fgets(%s)",fname);
    fclose(f);
    return 0;
  }
  if (fclose(f)) {
    DBG_ERROR("Error on fclose(%s): %s",fname,strerror(errno));
  }
  return 0;
}


int Config__WriteGroup(FILE *f, CONFIGGROUP *root,
		       const char *path,
		       int mode) {
  CONFIGGROUP *grp;
  CONFIGVARIABLE *var;
  CONFIGVALUE *val;
  int i;

  assert(root);

  DBG_VERBOUS("Writing group under \"%s\"",path);
  /* write variables */
  var=root->variables;
  while (var) {
    fprintf(f,"%s=",var->name);
    val=var->values;
    i=0;
    while(val) {
      if (i++)
	fprintf(f,",");
      fprintf(f,"\"%s\"",val->value);
      val=val->next;
    } /* while */
    fprintf(f,"\n");
    var=var->next;
  } /* while */

  grp=root->groups;
  while(grp) {
    char pbuffer[256];
    int pblen;

    if (!grp->name) {
      DBG_ERROR("Unnamed group");
      return 1;
    }

    pblen=strlen(grp->name)+1;
    if (path)
      pblen+=strlen(path);
    if (pblen>sizeof(pbuffer)-1) {
      DBG_ERROR("Path too long (limit is %d bytes)",sizeof(pbuffer)-1);
      return 1;
    }
    /* print group name */
    pbuffer[0]=0;
    if (path) {
      if (strlen(path)) {
	strcpy(pbuffer,path);
	strcat(pbuffer,"/");
      }
    }
    strcat(pbuffer, grp->name);

    if (grp->variables ||
	(mode & CONFIGMODE_EMPTY_GROUPS)) {
      fprintf(f,"\n[%s]\n",pbuffer);
    }
    DBG_VERBOUS("About to write group \"%s\"",pbuffer);
    i=Config__WriteGroup(f, grp, pbuffer,mode);
    if (i) {
      DBG_ERROR("Error writing group \"%s\"",grp->name);
      return i;
    }
    grp=grp->next;
  } /* while */
  return 0;
}


int Config_WriteFile(CONFIGGROUP *root,
		     const char *fname,
		     int mode){
  FILE *f;
  int i;

  assert(root);
  assert(fname);

  f=fopen(fname,"w+");
  if (f==0) {
    DBG_ERROR("Error on fopen(%s): %s",fname,strerror(errno));
    return 1;
  }
  i=Config__WriteGroup(f, root,root->name, mode);
  if (i) {
    DBG_ERROR("Error writing group \"%s\"",root->name);
    fclose(f);
    return 1;
  }
  if (fclose(f)) {
    DBG_ERROR("Error on fclose(%s): %s",fname,strerror(errno));
  }
  return 0;

}


int Config_DumpGroup(FILE *f, CONFIGGROUP *root,
		     int ins) {
  CONFIGGROUP *grp;
  CONFIGVARIABLE *var;
  CONFIGVALUE *val;
  int i;

  assert(root);

  /* write variables */
  var=root->variables;
  while (var) {
    for (i=0; i<ins; i++)
      fprintf(f, "  ");
    fprintf(f,"%s=",var->name);
    val=var->values;
    i=0;
    while(val) {
      if (i++)
	fprintf(f,",");
      fprintf(f,"\"%s\"",val->value);
      val=val->next;
    } /* while */
    fprintf(f,"\n");
    var=var->next;
  } /* while */

  grp=root->groups;
  while(grp) {

    if (!grp->name) {
      DBG_ERROR("Unnamed group");
      return 1;
    }

    for (i=0; i<ins; i++)
      fprintf(f, "  ");
    fprintf(f,"[%s]\n",grp->name);
    i=Config_DumpGroup(f, grp, ins+1);
    if (i) {
      DBG_ERROR("Error writing group \"%s\"",grp->name);
      return i;
    }
    //fprintf(f,"\n");
    grp=grp->next;
  } /* while */
  return 0;
}



CONFIGGROUP *Config_GetGroup(CONFIGGROUP *root,
			     const char *path,
			     unsigned int mode){
  DBG_VERBOUS("GetGroup for \"%s\"",path);
  return (CONFIGGROUP*)Config__GetPath(root,
				       path,
				       mode & ~CONFIGMODE_VARIABLE);
}


CONFIGGROUP *Config_Duplicate(CONFIGGROUP *root) {
  return Config__Group_duplicate(root);
}


int Config_Compare(const char *s1, const char *s2){
  if (s1==s2)
    return 0;
  if (s1==0 || s2==0)
    return 1;

  while(*s1 && *s2) {
    if (toupper(*s1)!=toupper(*s2))
      return 1;
    s1++;
    s2++;
  } /* while */
  return (*s1!=*s2);
}


void Config_ClearGroup(CONFIGGROUP *g){
  assert(g);
  Config__Group_Clear(g);
}


void Config_AddVariable(CONFIGGROUP *p, CONFIGVARIABLE *v){
  Config__AddVariable(p, v);
}


void Config__ImportGroup(CONFIGGROUP *root, CONFIGGROUP *g) {
  CONFIGGROUP *tmp;
  CONFIGGROUP *gcopy;
  CONFIGVARIABLE *vcopy;
  CONFIGVARIABLE *currv;

  while(g) {
    DBG_DEBUG("Importing group %s below %s",g->name, root->name);
    tmp=Config_GetGroup(root, g->name, CONFIGMODE_NAMEMUSTEXIST);
    if (tmp==0) {
      /* does not exists, so simply add it and return */
      gcopy=Config_Duplicate(g);
      DBG_DEBUG("Group \"%s\" does not exist, creating it", g->name);
      Config_AddGroup(root, gcopy);
    }
    else {
      /* group does exist, so import it */
      if (g->groups!=0) {
	Config__ImportGroup(tmp, g->groups);
	DBG_DEBUG("Import of \"%s\" done.", g->name);
      }

      /* copy all variables */
      currv=g->variables;
      while (currv) {
	vcopy=Config__Variable_duplicate(currv);
	Config__AddVariable(tmp, vcopy);
	currv=currv->next;
      }
    }

    /* next group */
    g=g->next;
  } /* while */
}


void Config_ImportGroup(CONFIGGROUP *root, CONFIGGROUP *g) {
  Config__ImportGroup(root, g->groups);
}




