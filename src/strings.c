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
char *string_space;
char *top_string;
#undef strdup


#define MAX_BUF 512

double fround(double f)
{
	f *= 100;
	f += 0.5;
	f = (float)((int) f);
	f /= 100;

	return f;

}


BOOL is_number (char *str)
{
	char *point;

	if (!str)
		return FALSE;

	for (point = str;*point;point++)
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

BOOL is_number2 (char *str)
{
	char *point;

	if (!str)
		return FALSE;

	for (point = str;*point;point++)
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

char *str_dup1 (const char *str, char * file, int line)
{



	char *result;

	

	if (!str)
		return NULL;

	result  = (char*)nano_malloc((strlen(str) + 10) * sizeof(char*), file,line);
//	result  = (char*)malloc((strlen(str) + 10) * sizeof(char*));
	if (result == (char*)0)
	{
		free (result);

		return (char*)0;
	}
	strcpy(result, str);
	return result;



}


/* Personal string search. Only searches for one char, not a whole string. */
BOOL str_search (const char *str)
{

	const char *point = str;


	BOOL efound=FALSE;
	BOOL mfound=FALSE;
	if (str == NULL)
		return FALSE;
	for (;;point++)
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
int str_ret(const char *str)
{
	const char *point=str;
	int i;
	for (i=0;*point;point++,i++)
	{
		if (*point == '\033')
			return i;
	}
	return 0;
}
void strip_junk(char * str)
{

	const char *point;
	char fleh[8000];
	char *buf;
	buf = fleh;

	if (str == NULL)
		return;
	point = str;
	for (point=str;*point;point++)
	{
		if (*point != '\033')
		{
			*buf = *point;
			*++buf ='\0';
		}

	}
	*++buf = '\0';
	str = buf;
	return;
}



BOOL strprefix (const char *astr, const char *bstr)
{

	if (astr == NULL)
		return TRUE;
	if (bstr == NULL)
		return TRUE;

	
	for (; (*astr || *bstr) && *bstr != '\0'; astr++, bstr++)
	{
		if (LOWER (*bstr) != LOWER (*astr))
			return TRUE;
	}

	return FALSE;
}

char *one_argument (char *argument, char *arg_first)
{
	char cEnd;

	while (isspace (*argument))
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
		*arg_first = LOWER (*argument);
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (isspace (*argument))
		argument++;


	return argument;
}
char *script_strip (char *argument, char *arg_first)
{
	char cEnd;

	while (isspace (*argument))
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

	while (isspace (*argument))
		argument++;
	while (*argument == '{' || *argument == '}')
		argument++;


	return argument;
}

BOOL simple_str_match(char *input, char *pattern)
{
	char *inputPtr = input;
	char *whereInput = input;
	char *wherePattern = pattern;

	for ( ; inputPtr != '\0' ; inputPtr++)
	{
		whereInput = inputPtr;
		wherePattern = pattern;

		if (*whereInput == *wherePattern)
		{
			for ( ; wherePattern != '\0' ; wherePattern++)
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

char * commaize (unsigned long long int x, char buf[]) // Turn a ULONG_INT into a comma string.
	// IE: 300000 = 300,000. Probably not the
	// most efficent way to do it, but ah well
{

	char * to_ret = NULL;
	char temp[1024]="";
	char *point = temp;
	int i,c,f=0;
	buf[0] = '\0';



	temp[0] = '\0';
	sprintf(temp,"%lu",x); // Yes yes, it's a hack.

	to_ret=temp;


	if (x <= 999)
	{
		buf[0] = '\0';
		sprintf(&buf[0], "%lu",x);



		return &buf[0];


	}
	else if (x > 999)
	{
		point += strlen(temp);

		if (!*--point)
			return to_ret;
		i = strlen(temp)-1;
		c = i / 3; // How many commas. :)
		i +=c;
		temp[i+c+1] = '\0';


		for (;*point;point--,i--)
		{
			if (i < 0)
			{
				buf[0] = '\0';
				strcat(buf, temp);

				return to_ret;
			}
			if (f==3)
			{
				f=0;
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


BOOL string_compare(const char *ostr, const char *tstr)
{
	if (ostr == NULL || tstr == NULL)
	{
		return TRUE;
	}
	for (;*ostr||*tstr;ostr++,tstr++)
	{
		if (LOWER(*ostr) != LOWER(*tstr))

			return TRUE;

	}
	return FALSE;
}


char * get_date(void)
{
	time_t t;
	struct tm * t_m;
	
	static char d[1024];
	t = time(NULL);
	
	t_m = localtime(&t);
	
	sprintf(d, "%2.2d/%2.2d/%d %2.2d:%2.2d%s", t_m->tm_mon+1, t_m->tm_mday, t_m->tm_year + 1900, t_m->tm_hour == 0 ? 12 :  (t_m->tm_hour) > 12 ? (t_m->tm_hour) - 12 : (t_m->tm_hour), t_m->tm_min, 
		(t_m->tm_hour) > 12 ? "pm" : "am");
	
	

	
	return d;
}

char * get_date_notime(void)
{
	time_t t;
	struct tm * t_m;
	
	static char d[1024];
	t = time(NULL);
	
	t_m = localtime(&t);
	
	sprintf(d, "%2.2d/%2.2d/%d", t_m->tm_mon+1, t_m->tm_mday, t_m->tm_year + 1900 );
	
	

	
	return d;
}
/* 
 * Truncate line to MAX_CHAR_COL, wrapping at
 * last space found. */

char * make_to_col(char *str)
{
	
	char *tmp_str;
	int i;
	int i_space;
	int x;
	char *point;
	char *last_space;

	if (!str)
		return NULL;

	if (strlen(str) <= MAX_CHAR_COL)
		return str;

	i = strlen(str);

	tmp_str = malloc(sizeof(char*) * (i + (i/2)));
	
	last_space = NULL;
	i=0;
	x=0;

	for (point = str;*point;point++)
	{
		if (*point == ' ')
		{
			last_space = point;
			i_space = i;
		}

		if (*point == '\n')
			x =0;

		

		if (x >= MAX_CHAR_COL)
		{
			if (last_space == NULL)
			{
				// break at the line anyways.
				tmp_str[i] = '\n';
				tmp_str[i+1] = '\0';
				i=i+1;
				x=0;
				point--;
				
				last_space = NULL;
				
			}
			else
			{

				tmp_str[i_space] = '\n';
				tmp_str[i_space+1] = '\0';
				i=i_space+1;
				x=0;
				
				point = last_space;
				last_space = NULL;
			}
			continue;
		}
		tmp_str[i] = *point;
		tmp_str[i+1] = '\0';
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



char * make_to_page(char *str)
{
	
	char *tmp_str;
	int i;
	int i_space;
	int x;
	char *point;
	char *last_space;

	if (!str)
		return NULL;
	
	if (strlen(str) <= 90)
		return str;

	i = strlen(str);

	tmp_str = malloc(sizeof(char*) * (i + (i/2)));
	
	last_space = NULL;
	i=0;
	x=0;

	for (point = str;*point;point++)
	{
		if (*point == ' ')
		{
			last_space = point;
			i_space = i;
		}

		if (*point == '\n')
			x =0;							  

		

		if (x >= 100)
		{
			if (last_space == NULL)
			{
				// break at the line anyways.
				tmp_str[i] = '\n';
				tmp_str[i+1] = '\0';
				i=i+1;
				x=0;
				point--;
				
				last_space = NULL;
				
			}
			else
			{

				tmp_str[i_space] = '\n';
				tmp_str[i_space+1] = '\0';
				i=i_space+1;
				x=0;
				
				point = last_space;
				last_space = NULL;
			}
			continue;
		}
		tmp_str[i] = *point;
		tmp_str[i+1] = '\0';
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


int count_lines(char *str)
{
	char *point;
	int len;

	if (!str)
		return 0;

	

	len = 0;

	for (point = str;*point;point++)
	{
		if (*point == '\n')
			len++;
	}
	return len == 0 ? 1 : (len+1);

}


BOOL is_date(char *range)
{
	char str[1024];
	char delim[] = "/";
	char *token;
	BOOL correct_date;


	correct_date = TRUE;

	if (!range)
		return FALSE;

	sprintf(str, "%s", range);
	// Lots of sanity checking. Mine or the code? Who knows.

	token = strtok (str, delim);

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

/* Go through a complex mathematical need to create a 512
 * byte salt so that we an use later for our passwords.
 * OVer kill? You're damnright it is. And I am okay with that.
 * Returns the 512 byte salt 
 */
char *make_salt(void)
{
	char table[88] = 
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"1234567890!@#$%^&*()`-=~_+"
		"\\|,./<>?\'\"";
	
	char q_table[250]; // Fifth random table

	char r_table[250];  // For random salt creation
	char s_table[250]; // Fourth random table

	char t_table[250];  // For picking randoms from random

	char u_table[250]; // Third Random table
	char v_table [250] ; // Sixth. Should be 512+ now.

	static char ssalt[2048]; // We'll use 512 character salts.
							
	int i = 0;
	int r = 0;
	int c = strlen(table);

	int seed_count = 0; // So we know when to switch boxes.
	char s1;  // Seed 1 item

	char s2;  // Seed 2 item, xor.
	int mode = 0; // The mode we're using in seed creation
	ssalt[0] = '\0';

	for (i = 0; i < 2048; i++)
		ssalt[i] = '\0'; // Zero it out so we dont resuse it everytime.



	// Take the plain text first, check if its valid.
	//if (!plaintext)
	//	return NULL; // Not valid.
//	if (strlen(plaintext) < 4) // We need more than 4 chars. Yes, could be better.
	//	return NULL;

	// Let's salt the password.
	// Generate the salt:

	for (i = 0; i < c; i++)
	{
		// Let's do some random iterations, fill a new table with random data,
		// We'll go ahead and fill all 6 random tables here. in one go.

		if (!table[i])
			break;
		if (table[i] == '\0')
			break;

		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		q_table[i] = table[r];
		q_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		r_table[i] = table[r];
		r_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		s_table[i] = table[r];
		s_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		t_table[i] = table[r];
		t_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		u_table[i] = table[r];
		u_table[i + 1] = '\0';
		r = rand() % ((c + 1) - 0) + 0;
		if (r < 0)
			r = 0;
		if (r > c)
			r = c;
		v_table[i] = table[r];
		v_table[i + 1] = '\0';
	}
	// Now, let's do 512 iterations, pulling from each table, making 512 bit 
	// salt.
	mode = 0;
	seed_count = 0;
	c = strlen(table) - 1;
	for (i = 0; i < 512; i++)
	{
		
		r = rand() % ((c + 1) - 0) + 0; // Random number.
		seed_count++;
		
		if (seed_count < c)
		{
			mode = 0;
		}
		else if (seed_count < (c*2))// && seed_count < (c*3)))
		{
			mode = 1;
		}


		else if (seed_count < (c * 3))// && seed_count < (c*4)) )
		{
			mode = 2;
		}
		else if (seed_count < (c * 4))// && seed_count < (c*5))))
		{
			mode = 3;
		}
		else if (seed_count < (c * 5))// && seed_count < (c*6)))
		{
			mode = 4;
		}
		else if (seed_count < (c * 6)) //&& seed_count < (c*7)))
		{
			mode = 5;
		}
		else if (seed_count < (c * 7))
		{
			mode = 6;
		}
		else if (seed_count < (c * 8))
		{
			mode = 7;
		}

		if (mode ==0)
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 2 ? q_table[r] : r_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}
		if (mode == 1)

		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.
		
			ssalt[i] = i % 5 ? s_table[r] : t_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}

		else if (mode == 2)
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.
		
			ssalt[i] = i % 3 ? u_table[r] : v_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}
		else if (mode == 3)
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? q_table[r] : s_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}
		else if (mode == 4)
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? r_table[r] : t_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}
		else if (mode == 5)
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? q_table[r] : v_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}

		else if (mode == 6)
		{
			r = rand() % ((c + 1) - 0) + 0; // Random number.

			ssalt[i] = i % 3 ? u_table[r] : s_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}

		else if (mode == 5)
		{
			r = rand() % ((c + 1) - 0) + 0; 
			ssalt[i] = i % 3 ? v_table[r] : t_table[r];
			if (ssalt[i] == '\0')
				ssalt[i] = '@';
			ssalt[i + 1] = '\0';
			continue;
		}


	}
	LOG("Salt: %s", ssalt);
	return ssalt;

}


// We WILL modify the salt during this operation.
// Do not send a copy of it. send the actual pointer
// to the real buffer.
char* crypt_password(char* plaintext, char* salt)
{
	int pwlen = 0;
	int saltlen = 0;
	char* pwsalt = NULL; // we'll malloc this.

	if (!plaintext || !salt)
	{
		LOG("No password or salt was given to crypt_password() -- Check this out.");
		return NULL;
	}

	// Let's take length of password and remove that length from the salt.
	// That way we have 512 bits at all times. If the password is larger
	// than 512...Well, We'll do something bad and just put the salt
	// at the end and make it 1024.

	pwlen = strlen(plaintext);

	if (pwlen <= 0)
		return NULL; // We need a password.
	if (pwlen > 512)
		saltlen = 512;
	else
		saltlen = (pwlen - 512);

	if (saltlen < 0)
		saltlen = 0;
	// Truncate the salt.
	salt[saltlen] = '\0';

	// Create the new buffer for the pw + salt.
	pwsalt = malloc(sizeof(char*) * ((pwlen + saltlen)+10)); // Add ten just as a buffer.

	if (!pwsalt)
		GiveError("Unable to allocate memory for password encryption.", TRUE);

	sprintf(pwsalt, "%s%s", plaintext, salt);
	pwsalt[pwlen + saltlen] = '\0';

	// We now have a password + salt that we can turn in to a hash.

	


	//return crypt(pwsalt);
}