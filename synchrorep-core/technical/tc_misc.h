/*
 * tc_misc.h
 *
 *  Created on: 8 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  Miscellaneous utilities
 *
 */

#ifndef TC_MISC_H_
#define TC_MISC_H_

typedef struct
{
	char			*context;
	char			*reason;
} error_infos;

char*				cut_uri(char *uri, unsigned int maxlen);
char*				cut_line(char *uri, unsigned int line_length);
char*				cut_line_word(char *uri, unsigned int line_length);
char*				underline(char *str);
bool				test_alphanumeric(char *entry);
long				convert_string_to_int(char *c_string);

#endif /* TC_MISC_H_ */
