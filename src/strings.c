#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <math.h>
//#include <crypt.h>
#include "Nano PoS.h"

char str_empty[1];
char* string_space;
char* top_string;
#undef strdup

#define MAX_BUF 512

double fround(double f)
{
	f *= 100;
	f += 0.5;
	f = (float)((int)f);
	f /= 100;

	return f;
}

BOOL is_number(char* str)
{
	char* point;

	if (!str)
		return FALSE;

	for (point = str; *point; point++)
	{
		switch (*point)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
			//case '-':
		{
			continue;
			break;
		}

		default:
			return FALSE;
		}
	}
	return TRUE;
}

BOOL is_number2(char* str)
{
	char* point;

	if (!str)
		return FALSE;

	for (point = str; *point; point++)
	{
		switch (*point)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			continue;
			break;
		}

		default:
			return FALSE;
		}
	}
	return TRUE;
}

// Going to call nano_malloc directly here for not-so-obvious reasons.
// This function is rolled, locally, for our memory manager.

char* str_dup1(const char* str, char* file, int line)
{
	char* result;

	if (!str)
		return NULL;

	result = (char*)nano_malloc((strlen(str) + 10) * sizeof(char*), file, line);
	//	result  = (char*)malloc((strlen(str) + 10) * sizeof(char*));
	if (result == (char*)0)
	{
		free(result);

		return (char*)0;
	}
	strcpy(result, str);
	return result;
}

/* Personal string search. Only searches for one char, not a whole string. */
BOOL str_search(const char* str)
{
	const char* point = str;

	BOOL efound = FALSE;
	BOOL mfound = FALSE;
	if (str == NULL)
		return FALSE;
	for (;; point++)
	{
		if (*point == '\033')
			efound = TRUE;
		if (*point == 'm')
			mfound = TRUE;
		if (!*point)
			break;
	}
	if (efound && !mfound)
		return FALSE;
	else
		return TRUE;
}
int str_ret(const char* str)
{
	const char* point = str;
	int i;
	for (i = 0; *point; point++, i++)
	{
		if (*point == '\033')
			return i;
	}
	return 0;
}
void strip_junk(char* str)
{
	const char* point;
	char fleh[8000] = "";

	char* buf;

	buf = fleh;

	if (str == NULL)
		return;
	point = str;
	for (point = str; *point; point++)
	{
		if (*point != '\033')
		{
			*buf = *point;
			*++buf = '\0';
		}
	}
	*++buf = '\0';
	str = buf;
	return;
}

BOOL strprefix(const char* astr, const char* bstr)
{
	if (astr == NULL)
		return TRUE;
	if (bstr == NULL)
		return TRUE;

	for (; (*astr || *bstr) && *bstr != '\0'; astr++, bstr++)
	{
		if (LOWER(*bstr) != LOWER(*astr))
			return TRUE;
	}

	return FALSE;
}

char* one_argument(char* argument, char* arg_first)
{
	char cEnd;

	while (isspace(*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*arg_first = LOWER(*argument);
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}
char* script_strip(char* argument, char* arg_first)
{
	char cEnd;

	while (isspace(*argument))
		argument++;

	cEnd = '{';
	if (*argument == '{' || *argument == '}')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd || *argument == '}')
		{
			argument++;
			break;
		}

		*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (isspace(*argument))
		argument++;
	while (*argument == '{' || *argument == '}')
		argument++;

	return argument;
}

BOOL simple_str_match(char* input, char* pattern)
{
	char* inputPtr = input;
	char* whereInput = input;
	char* wherePattern = pattern;

	for (; inputPtr != '\0'; inputPtr++)
	{
		whereInput = inputPtr;
		wherePattern = pattern;

		if (*whereInput == *wherePattern)
		{
			for (; wherePattern != '\0'; wherePattern++)
			{
				whereInput++;

				if (*whereInput != *wherePattern)
				{
					break;
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

char* commaize(unsigned long long int x, char buf[]) // Turn a ULONG_INT into a comma string.
	// IE: 300000 = 300,000. Probably not the
	// most efficent way to do it, but ah well
{
	char* to_ret = NULL;
	char temp[1024] = "";
	char* point = temp;
	int i, c, f = 0;
	buf[0] = '\0';

	temp[0] = '\0';
	sprintf(temp, "%llu", x); // Yes yes, it's a hack.

	to_ret = temp;

	if (x <= 999)
	{
		buf[0] = '\0';
		sprintf(&buf[0], "%lu", x);

		return &buf[0];
	}
	else if (x > 999)
	{
		point += strlen(temp);

		if (!*--point)
			return to_ret;
		i = strlen(temp) - 1;
		c = i / 3; // How many commas. :)
		i += c;
		temp[i + c + 1] = '\0';

		for (; *point; point--, i--)
		{
			if (i < 0)
			{
				buf[0] = '\0';
				strcat(buf, temp);

				return to_ret;
			}
			if (f == 3)
			{
				f = 0;
				temp[i] = ',';
				*++point;

				continue;
			}

			temp[i] = *point;
			f++;
		}

		to_ret = temp;
		buf[0] = '\0';
		strcat(buf, temp);

		return &buf[0];
	}

	return &buf[0];
}

BOOL string_compare(const char* ostr, const char* tstr)
{
	if (ostr == NULL || tstr == NULL)
	{
		return TRUE;
	}
	for (; *ostr || *tstr; ostr++, tstr++)
	{
		if (LOWER(*ostr) != LOWER(*tstr))

			return TRUE;
	}
	return FALSE;
}

char* get_date(void)
{
	time_t t;
	struct tm* t_m;

	static char d[1024];
	t = time(NULL);

	t_m = localtime(&t);

	sprintf(d, "%2.2d/%2.2d/%d %2.2d:%2.2d%s", t_m->tm_mon + 1, t_m->tm_mday, t_m->tm_year + 1900, t_m->tm_hour == 0 ? 12 : (t_m->tm_hour) > 12 ? (t_m->tm_hour) - 12 : (t_m->tm_hour), t_m->tm_min,
		(t_m->tm_hour) > 12 ? "pm" : "am");

	return d;
}

char* get_date_notime(void)
{
	time_t t;
	struct tm* t_m;

	static char d[1024];
	t = time(NULL);

	t_m = localtime(&t);

	sprintf(d, "%2.2d/%2.2d/%d", t_m->tm_mon + 1, t_m->tm_mday, t_m->tm_year + 1900);

	return d;
}
/*
 * Truncate line to MAX_CHAR_COL, wrapping at
 * last space found. */

char* make_to_col(char* str)
{
	char* tmp_str;
	unsigned long int i;
	int i_space;
	int x;
	char* point;
	char* last_space;

	if (!str)
		return NULL;

	if (strlen(str) <= MAX_CHAR_COL)
		return str;

	i = strlen(str);

	i = (i + (i / 2));

	if (i < 1)
		i = 2;
	if (i > strlen(str))
		i = (strlen(str) * 2);
	tmp_str = malloc(sizeof(char*) * i);

	last_space = NULL;
	i = 0;
	x = 0;

	for (point = str; *point; point++)
	{
		if (*point == ' ')
		{
			last_space = point;
			i_space = i;
		}

		if (*point == '\n')
			x = 0;

		if (x >= MAX_CHAR_COL)
		{
			if (last_space == NULL)
			{
				// break at the line anyways.
				tmp_str[i] = '\n';
				tmp_str[i + 1] = '\0';
				i = i + 1;
				x = 0;
				point--;

				last_space = NULL;
			}
			else
			{
				tmp_str[i_space] = '\n';
				tmp_str[i_space + 1] = '\0';
				i = i_space + 1;
				x = 0;

				point = last_space;
				last_space = NULL;
			}
			continue;
		}
		tmp_str[i] = *point;
		tmp_str[i + 1] = '\0';
		i++;
		x++;
		if (*point == '\b' || *point == '\033' || *point == '\032')
		{
			x--;

			continue;
		}
	}

	return tmp_str;
}

char* make_to_page(char* str)
{
	char* tmp_str;
	unsigned long int i;
	int i_space;
	int x;
	char* point;
	char* last_space;

	if (!str)
		return NULL;

	if (strlen(str) <= 90)
		return str;

	i = strlen(str);
	i = (i + (i / 2));

	if (i < 1)
		i = 2;
	if (i > strlen(str))
		i = (strlen(str) * 2);
	tmp_str = malloc(sizeof(char*) * i);

	last_space = NULL;
	i = 0;
	x = 0;

	for (point = str; *point; point++)
	{
		if (*point == ' ')
		{
			last_space = point;
			i_space = i;
		}

		if (*point == '\n')
			x = 0;

		if (x >= 100)
		{
			if (last_space == NULL)
			{
				// break at the line anyways.
				tmp_str[i] = '\n';
				tmp_str[i + 1] = '\0';
				i = i + 1;
				x = 0;
				point--;

				last_space = NULL;
			}
			else
			{
				tmp_str[i_space] = '\n';
				tmp_str[i_space + 1] = '\0';
				i = i_space + 1;
				x = 0;

				point = last_space;
				last_space = NULL;
			}
			continue;
		}
		tmp_str[i] = *point;
		tmp_str[i + 1] = '\0';
		i++;
		x++;
		if (*point == '\b' || *point == '\033' || *point == '\032')
		{
			x--;

			continue;
		}
	}

	return tmp_str;
}

int count_lines(char* str)
{
	char* point;
	int len;

	if (!str)
		return 0;

	len = 0;

	for (point = str; *point; point++)
	{
		if (*point == '\n')
			len++;
	}
	return len == 0 ? 1 : (len + 1);
}

BOOL is_date(char* range)
{
	char str[1024];
	char delim[] = "/";
	char* token;
	BOOL correct_date;

	correct_date = TRUE;

	if (!range)
		return FALSE;

	sprintf(str, "%s", range);
	// Lots of sanity checking. Mine or the code? Who knows.

	token = strtok(str, delim);

	if ((token) && !is_number2(token))
		correct_date = FALSE;

	if (!token)
		correct_date = FALSE;

	if ((token) && strlen(token) > 2 || strlen(token) < 2)
		correct_date = FALSE;

	if (atoi(token) > 12)
		correct_date = FALSE;

	token = strtok(NULL, delim);
	if (!token)
		correct_date = FALSE;
	if (!(token) && is_number2(token))
		correct_date = FALSE;

	if ((token) && strlen(token) > 2 || strlen(token) < 2)
		correct_date = FALSE;

	if (atoi(token) > 31)
		correct_date = FALSE;

	token = strtok(NULL, delim);
	if (!token)
		correct_date = FALSE;
	if ((token) && !is_number2(token))
		correct_date = FALSE;

	if ((token) && strlen(token) > 4 || strlen(token) < 4)
		correct_date = FALSE;
	if (atoi(token) > 2500 || atoi(token) < 1950)
		correct_date = FALSE;

	return correct_date;
}