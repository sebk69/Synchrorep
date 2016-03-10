/*
 * tc_misc.cpp
 *
 *  Created on: 8 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  Miscellaneous utilities
 *
 */

#include "tc_misc.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

using namespace std;

char*
cut_uri(char *uri, unsigned int max_len)
{
	if(strlen(uri) < max_len)
		return uri;

	int				start_len = max_len / 2 - 2,
					end_len = max_len / 2 - 1;
	char			*result = new char[max_len],
					*start = new char[start_len + 1],
					*end = new char[end_len + 1];
	start = strdup(uri);
	end = strdup(strlen(uri) - end_len + uri);
	start[start_len + 1] = '\0';
	strcpy(result, start);
	strcat(result, "...");
	strcat(result, end);

	free(start);
	free(end);

	return result;
}

bool
test_alphanumeric(char *entry)
{
	int				i;
	for(i=0; entry[i] != '\0'; i++)
		if(!((entry[i] >= 'a' && entry[i] <= 'z') || (entry[i] >= 'A' && entry[i] <= 'Z')
				|| (entry[i] >= '0' && entry[i] <= '9')))
			return false;
	return true;
}

char*
cut_line(char *str, unsigned int line_length)
{
	unsigned int	i = 0;
	unsigned int	j = 0;
	unsigned int	cur_line_length = 0;
	char			*result = new char[strlen(str) + 100];
	char			*result2;

	while(i < strlen(str))
	{
		result[j] = str[i];
		cur_line_length++;
		if(cur_line_length >= line_length)
		{
			j++;
			result[j] = '\n';
			cur_line_length = 0;
		}
		i++;
		j++;
	}
	result[j] = '\0';

	result2 = strdup(result);
	delete result;
	return result2;
}

char*
cut_line_word(char *str, unsigned int line_length)
{
	unsigned int	i = 0;
	unsigned int	j = 0;
	unsigned int	w_beg = 0;
	unsigned int	cur_line_length = 0;
	char			*result = new char[strlen(str) + 100];
	char			*result2;

	while(i < strlen(str))
	{
		if(cur_line_length == line_length)
		{
			result[j] = '\n';
			j++;
			cur_line_length = i - w_beg;
		}
		if(str[i] == ' '
				|| str[i] == '\n'
				|| str[i] == '-'
				|| str[i] == '/')
		{
			while(w_beg <= i)
			{
				result[j] = str[w_beg];
				j++;
				w_beg++;
			}
		}
		if(str[i] == '\n')
			cur_line_length = 0;
		i++;
		cur_line_length++;
	}
	while(w_beg < i)
	{
		result[j] = str[w_beg];
		j++;
		w_beg++;
	}
	result[j] = '\0';

	result2 = strdup(result);
	delete result;
	return result2;
}

char*
underline(char *str)
{
	unsigned int			i;
	unsigned int			j = 0;
	char					*result = new char[strlen(str) * 2 + 1];
	char					*result2;

	for(i=0; i<strlen(str);i++)
	{
		result[j] = '_';
		j++;
		result[j] = str[i];
		j++;
	}
	result[j] = '\0';
	result2 = strdup(result);
	delete result;
	return result2;
}

long
convert_string_to_int(char *c_string)
{
	string				str(c_string);
	istringstream		str2(str);
	long				result;

	str2>>result;

	return result;
}
