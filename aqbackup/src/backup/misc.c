/***************************************************************************
 $RCSfile: misc.c,v $
                             -------------------
    cvs         : $Id: misc.c,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
    begin       : Sun May 25 2003
    copyright   : (C) 2003 by Martin Preuss
    email       : openhbci@aquamaniac.de

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



#include "misc.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <chameleon/chameleon.h>
#include <chameleon/debug.h>


char *Text_GetWord(const char *src,
		   const char *delims,
		   char *buffer,
		   unsigned int maxsize,
		   unsigned int flags,
		   const char **next){
  unsigned int size;
  int lastWasBlank;
  int lastBlankPos;

  assert(maxsize);

  /* skip leading blanks, if wanted */
  if (flags & TEXT_FLAGS_DEL_LEADING_BLANKS) {
    while(*src && *src<33)
      src++;
  }

  /* get word */
  size=0;
  lastWasBlank=0;
  lastBlankPos=-1;
  while(*src &&
	strchr(delims, *src)==0 &&
	size<(maxsize-1)) {
    if (!lastWasBlank ||
	(lastWasBlank && !(flags & TEXT_FLAGS_DEL_MULTIPLE_BLANKS))) {
      /* only copy if last char was NOT blank or
       * last was blank but the caller does not want to have multiple
       * blanks removed */
      buffer[size]=*src;
      size++;
    }
    /* remember next loop whether this char was a blank */
    if (isspace(*src)) {
      lastWasBlank=1;
      lastBlankPos=size;
    }
    else {
      lastWasBlank=0;
      lastBlankPos=-1;
    }
    /* advance source pointer */
    src++;
  } /* while */

  /* add trailing null to correctly terminate the buffer */
  buffer[size]=0;

  /* check whether the source string was correctly terminated */
  if (flags & TEXT_FLAGS_NEED_DELIMITER) {
    if (*src) {
      if (strchr(delims, *src)==0) {
	DBG_ERROR("No delimiter found within specified length");
	return 0;
      }
    }
    else {
      if (!(flags & TEXT_FLAGS_NULL_IS_DELIMITER)) {
	DBG_ERROR("String ends without delimiter");
	return 0;
      }
    }
  }

  /* remove trailing blanks, if wanted */
  if (flags & TEXT_FLAGS_DEL_TRAILING_BLANKS) {
    if (lastBlankPos!=-1)
      buffer[lastBlankPos]=0;
  }

  *next=src;
  return buffer;
}


char *Text_Escape(const char *src,
		  char *buffer,
		  unsigned int maxsize) {
  unsigned int size;

  size=0;
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!(
	  (x>='A' && x<='Z') ||
	  (x>='a' && x<='z') ||
	  (x>='0' && x<='9'))) {
      unsigned char c;

      if ((maxsize-1)<size+3) {
	DBG_ERROR("Buffer too small");
	return 0;
      }
      buffer[size++]='%';
      c=(((unsigned char)(*src))>>4)&0xf;
      if (c>9)
	c+=7;
      c+='0';
      buffer[size++]=c;
      c=((unsigned char)(*src))&0xf;
      if (c>9)
	c+=7;
      c+='0';
      buffer[size++]=c;
    }
    else {
      if (size<(maxsize-1))
	buffer[size++]=*src;
      else {
	DBG_ERROR("Buffer too small");
	return 0;
      }
    }

    src++;
  } /* while */

  buffer[size]=0;
  return buffer;
}


char *Text_Unescape(const char *src,
		    char *buffer,
		    unsigned int maxsize){
  unsigned int size;

  size=0;

  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (
	(x>='A' && x<='Z') ||
	(x>='a' && x<='z') ||
	(x>='0' && x<='9')) {
      if (size<(maxsize-1))
	buffer[size++]=*src;
      else {
	DBG_ERROR("Buffer too small");
	return 0;
      }
    }
    else {
      if (*src=='%') {
	unsigned char d1, d2;
	unsigned char c;

	/* skip '%' */
	src++;
	if (!(*src) || !isxdigit(*src)) {
	  DBG_ERROR("Incomplete escape sequence (no digits)");
	  return 0;
	}
	/* read first digit */
	d1=(unsigned char)(toupper(*src));

	/* get second digit */
	src++;
	if (!(*src) || !isxdigit(*src)) {
	  DBG_ERROR("Incomplete escape sequence (only 1 digit)");
	  return 0;
	}
	d2=(unsigned char)(toupper(*src));
	/* compute character */
	d1-='0';
	if (d1>9)
	  d1-=7;
	c=(d1<<4)&0xf0;
	d2-='0';
	if (d2>9)
	  d2-=7;
	c+=(d2&0xf);
	/* store character */
	if (size<(maxsize-1))
	  buffer[size++]=(char)c;
	else {
	  DBG_ERROR("Buffer too small");
	  return 0;
	}
      }
      else {
	DBG_ERROR("Found non-alphanum "
		  "characters in escaped string (\"%s\")",
		  src);
        return 0;
      }
    }
    src++;
  } /* while */

  buffer[size]=0;
  return buffer;
}


char *Text_ToHex(const char *src, int l, char *buffer, unsigned maxsize) {
  unsigned int pos;
  unsigned int size;

  if ((l*2)+1 > maxsize) {
    DBG_ERROR("Buffer too small");
    return 0;
  }

  pos=0;
  size=0;
  while(pos<l) {
    unsigned char c;

    c=(((unsigned char)(src[pos]))>>4)&0xf;
    if (c>9)
      c+=7;
    c+='0';
    buffer[size++]=c;
    c=((unsigned char)(src[pos]))&0xf;
    if (c>9)
      c+=7;
    c+='0';
    buffer[size++]=c;
    pos++;
  }
  buffer[size]=0;
  return buffer;
}


char *Text_ToHexGrouped(const char *src,
			int l,
			char *buffer,
			unsigned maxsize,
			unsigned int groupsize,
			char delimiter,
			int skipLeadingZeroes) {
  unsigned int pos;
  unsigned int size;
  unsigned int j;

  j=0;

  pos=0;
  size=0;
  j=0;
  while(pos<l) {
    unsigned char c;
    int skipThis;

    skipThis=0;
    c=(((unsigned char)(src[pos]))>>4)&0xf;
    if (skipLeadingZeroes) {
      if (c==0)
	skipThis=1;
      else
	skipLeadingZeroes=0;
    }
    if (c>9)
      c+=7;
    c+='0';
    if (!skipThis) {
      if (size+1>=maxsize) {
	DBG_ERROR("Buffer too small");
        return 0;
      }
      buffer[size++]=c;
      j++;
      if (j==groupsize) {
	if (size+1>=maxsize) {
	  DBG_ERROR("Buffer too small");
	  return 0;
	}
	buffer[size++]=delimiter;
	j=0;
      }
    }

    skipThis=0;
    c=((unsigned char)(src[pos]))&0xf;
    if (skipLeadingZeroes) {
      if (c==0 && pos+1<l)
	skipThis=1;
      else
	skipLeadingZeroes=0;
    }
    if (c>9)
      c+=7;
    c+='0';
    if (size+1>=maxsize) {
      DBG_ERROR("Buffer too small");
      return 0;
    }
    if (!skipThis) {
      buffer[size++]=c;
      j++;
      if (j==groupsize) {
	if (pos+1<l) {
	  if (size+1>=maxsize) {
	    DBG_ERROR("Buffer too small");
	    return 0;
	  }
	  buffer[size++]=delimiter;
	}
	j=0;
      }
    }
    pos++;
  }
  buffer[size]=0;
  return buffer;
}


int Text_FromHex(const char *src, char *buffer, unsigned maxsize){
  unsigned int pos;
  unsigned int size;

  pos=0;
  size=0;
  while(*src) {
    unsigned char d1, d2;
    unsigned char c;

    /* read first digit */
    if (!isxdigit(*src)) {
      DBG_ERROR("Bad char in hex string");
      return -1;
    }
    d1=(unsigned char)(toupper(*src));

    /* get second digit */
    src++;
    if (!(*src) || !isxdigit(*src)) {
      DBG_ERROR("Incomplete hex byte (only 1 digit)");
      return -1;
    }
    d2=(unsigned char)(toupper(*src));
    src++;

    /* compute character */
    d1-='0';
    if (d1>9)
      d1-=7;
    c=(d1<<4)&0xf0;
    d2-='0';
    if (d2>9)
      d2-=7;
    c+=(d2&0xf);
    /* store character */
    if (size<(maxsize))
      buffer[size++]=(char)c;
    else {
      DBG_ERROR("Buffer too small");
      return -1;
    }
  } /* while */

  return size;
}


int Text_Compare(const char *s1, const char *s2, int ign) {
  if (s1)
    if (strlen(s1)==0)
      s1=0;
  if (s2)
    if (strlen(s2)==0)
      s2=0;
  if (!s1 && !s2)
    return 0;
  if (!s1 && s2)
    return 1;
  if (s1 && !s2)
    return -1;
  if (ign)
    return strcasecmp(s1, s2);
  else
    return strcmp(s1, s2);
}




int Text__cmpSegment(const char *w, unsigned int *wpos,
		     const char *p, unsigned int *ppos,
		     int sensecase,
		     unsigned int *matches) {
  char a;
  char b;
  unsigned wlength;
  unsigned plength;

  a=0;
  b=0;
  wlength=strlen(w);
  plength=strlen(p);


  while (*wpos<wlength && *ppos<plength) {
    a=w[*wpos];
    b=p[*ppos];
    if (b=='*')
      return 1;
    if (!sensecase) {
      a=toupper(a);
      b=toupper(b);
    }
    // count matches
    if (a==b)
      (*matches)++;
    if (a!=b && b!='?')
      return 0;
    (*wpos)++;
    (*ppos)++;
  }
  // both at end, would be ok
  if (*wpos==wlength && *ppos==plength)
    return 1;
  // word ends, pattern doesnt, would be ok if pattern is '*' here
  if (*wpos>=wlength && *ppos<plength)
    if (p[*ppos]=='*')
      return 1;
  // otherwise no match ;-/
  return 0;
}


int Text__findSegment(const char *w, unsigned int *wpos,
		      const char *p, unsigned int *ppos,
		      int sensecase,
		      unsigned int *matches) {
  unsigned int lwpos, lppos, lmatches;
  unsigned wlength;

  wlength=strlen(w);
  lwpos=*wpos;
  lppos=*ppos;
  lmatches=*matches;
  while(lwpos<wlength) {
    *ppos=lppos;
    *wpos=lwpos;
    *matches=lmatches;
    if (Text__cmpSegment(w,wpos,p,ppos,sensecase,matches))
      return 1;
    lwpos++;
  }
  return 0;
}


int Text_ComparePattern(const char *w, const char *p, int sensecase) {
  unsigned int ppos;
  unsigned int wpos;
  unsigned int matches;
  unsigned int plength;

  ppos=wpos=matches=0;
  plength=strlen(p);

  // compare until first occurrence of '*'
  if (!Text__cmpSegment(w,&wpos,p,&ppos,sensecase,&matches))
    return -1;

  while(1) {
    // if pattern ends we have done it
    if (ppos>=plength)
      return matches;
    // skip '*' in pattern
    ppos++;
    // if pattern ends behind '*' the word matches
    if (ppos>=plength)
      return matches;
    // find next matching segment
    if (!Text__findSegment(w,&wpos,p,&ppos,sensecase,&matches))
      return -1;
  } // while
  // I know, we'll never get to here ;-)
  return -1;
}


int Path_Create(const char *prefix,
		const char *path) {
  char buffer[256];
  char elbuffer[128];
  char *p;

  if ((strlen(prefix)+
       strlen(path)+10)>sizeof(buffer)) {
    DBG_ERROR("Buffer too small");
    return 1;
  }
  strcpy(buffer, prefix);
  if (buffer[0]) {
    if (buffer[strlen(buffer)-1]=='/')
      buffer[strlen(buffer)-1]=0;
  }

  while(*path) {
    const char *next;

    /* skip all leading '/' */
    if (*path=='/') {
      path++;
      continue;
    }

    /* get next element */
    p=Text_GetWord(path,
		   "/\\",
		   elbuffer,
		   sizeof(elbuffer),
		   TEXT_FLAGS_NEED_DELIMITER |
		   TEXT_FLAGS_NULL_IS_DELIMITER,
		   &next);
    if (!p) {
      DBG_ERROR("Error in path \"%s\"",path);
    }

    /* add element to buffer */
    if ((strlen(buffer)+
	 strlen(elbuffer)+10)>sizeof(buffer)) {
      DBG_ERROR("Buffer too small");
      return 1;
    }
    strcat(buffer,"/");
    strcat(buffer, elbuffer);

    /* create directory if needed */
    DBG_INFO("Creating directory \"%s\"", buffer);
    if (mkdir(buffer, S_IRWXU)) {
      if (errno!=EEXIST) {
	DBG_ERROR("mkdir(%s): %s", buffer, strerror(errno));
	return 1;
      }
    }

    path=next;
  } /* while */

  return 0;
}



