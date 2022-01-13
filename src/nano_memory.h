




#define MAX_STRINGS 4096;
#define a 2
#define b 4
#define c 8
#define d 16
#define e 32
#define f 64
#define g 128
#define h 512
#define i 1024
#define j 2048
#define k 4096
#define l 8192
#define m 16384
#define n 32768



typedef struct string_struct STRING;
typedef struct string_list STRLIST;

STRLIST** strpool;



struct string_struct
{
	char*	     string;
	unsigned int len;

};

struct string_list
{
	STRING string;
	char* hash;

};

