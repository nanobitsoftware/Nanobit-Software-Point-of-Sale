#define MAX_STRINGS 4096;

typedef struct string_struct STRING;
typedef struct string_list STRLIST;

STRLIST** strpool;

struct string_struct
{
	char* string;
	unsigned int len;
};

struct string_list
{
	STRING string;
	char* hash;
};
