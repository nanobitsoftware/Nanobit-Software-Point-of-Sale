#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <time.h>
#include <commctrl.h>
#include "Nano PoS.h"
#include "NWC.h"

void create_db(void)
{
}

void close_db(void)
{
}

BOOL is_db_locked(FILE* db)
{
	return FALSE; // Not locked
}

BOOL lock_db(FILE* fp)
{
	return FALSE;
}

BOOL unlock_db(FILE* fp)
{
	return FALSE;
}

int write_db(FILE* fp, char* stmt)
{
	return 0;
}