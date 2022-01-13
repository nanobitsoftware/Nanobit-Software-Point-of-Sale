




#define MAX_STRINGS 4096

		  /*
#define a 1
#define b a*2
#define c b*2
#define d c*2
#define e d*2
#define f e*2
#define g f*2
#define h g*2
#define i h*2
#define j i*2
#define k j*2
#define l k*2
#define m l*2
#define n m*2
#define o n*2
#define p o*2
#define q p*2
#define r q*2
#define s r*2
#define t s*2
#define u t*2
#define v u*2
#define w v*2
#define x w*2
#define y x*2
#define z y*2

			*/

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
	unsigned long int pool_size;

};

