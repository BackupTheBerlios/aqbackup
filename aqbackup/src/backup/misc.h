/***************************************************************************
 $RCSfile: misc.h,v $
                             -------------------
    cvs         : $Id: misc.h,v 1.1 2003/06/07 21:07:47 aquamaniac Exp $
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


#ifndef AQBACKUP_MISC_H
#define AQBACKUP_MISC_H


#define AQLIST_ADD(typ, sr, head) \
  typ *curr;                \
                            \
  DBG_ENTER;                \
  assert(sr);               \
  assert(head);             \
                            \
  curr=*head;               \
  if (!curr) {              \
    *head=sr;               \
  }                         \
  else {                    \
    while(curr->next) {     \
      curr=curr->next;      \
    }                       \
    curr->next=sr;          \
  }


#define AQLIST_DEL(typ, sr, head) \
  typ *curr;                   \
                               \
  assert(sr);                  \
  assert(head);                \
  curr=*head;                  \
  if (curr) {                  \
    if (curr==sr) {            \
      *head=curr->next;        \
    }                          \
    else {                     \
      while(curr->next!=sr) {  \
	curr=curr->next;       \
      }                        \
      if (curr)                \
	curr->next=sr->next;   \
    }                          \
  }                            \
  sr->next=0;



#define TEXT_FLAGS_DEL_LEADING_BLANKS  0x00000001
#define TEXT_FLAGS_DEL_TRAILING_BLANKS 0x00000002
#define TEXT_FLAGS_DEL_MULTIPLE_BLANKS 0x00000004
#define TEXT_FLAGS_NEED_DELIMITER      0x00000008
#define TEXT_FLAGS_NULL_IS_DELIMITER   0x00000010


/**
 * This function cuts out a word from a given string.
 * @return address of the new word, 0 on error
 * @param src pointer to the beginning of the source string
 * @param delims pointer to a string containing all delimiters
 * @param buffer pointer to the destination buffer
 * @param maxsize length of the buffer. Actually up to this number of
 * characters are copied to the buffer. If after this number of chars no
 * delimiter follows the string will be terminated. You will have to check
 * whether there is a delimiter directly after the copied string
 * @param flags defines how the source string is to be processed
 * @param next pointer to a pointer to receive the address up to which the
 * source string has been handled. You can use this to continue with the
 * source string behind the word we've just cut out. This variable is only
 * modified upon successfull return
 */
char *Text_GetWord(const char *src,
		   const char *delims,
		   char *buffer,
		   unsigned int maxsize,
		   unsigned int flags,
		   const char **next);

/**
 * This function does escaping like it is used for HTTP URL encoding.
 * All characters which are not alphanumeric are escaped by %XX where
 * XX ist the hexadecimal code of the character.
 */
char *Text_Escape(const char *src,
		  char *buffer,
		  unsigned int maxsize);

char *Text_Unescape(const char *src,
		    char *buffer,
		    unsigned int maxsize);

char *Text_ToHex(const char *src, int l, char *buffer, unsigned maxsize);

/**
 * Converts a string to Hex. After "groupsize" bytes the "delimiter" is
 * inserted.
 */
char *Text_ToHexGrouped(const char *src,
			int l,
			char *buffer,
			unsigned maxsize,
			unsigned int groupsize,
			char delimiter,
			int skipLeadingZeros);

int Text_FromHex(const char *src, char *buffer, unsigned maxsize);

/**
 * Compares two strings. If either of them is given but empty, that string
 * will be treaten as not given. This way a string NOT given equals a string
 * which is given but empty.
 * @param ign set to !=0 to ignore cases
 */
int Text_Compare(const char *s1, const char *s2, int ign);


/**
 * This function compares two string and returns the number of matches or
 * -1 on error.
 * @param w string to compare
 * @param p pattern to compare against
 * @param sensecase if 0 then cases are ignored
 */
int Text_ComparePattern(const char *w, const char *p, int sensecase);


int Path_Create(const char *prefix, const char *path);


#endif



