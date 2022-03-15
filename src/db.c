/*
dbc
Nanobit Point of Sale - Enhanced Edition
-------------------------------------------
Written by; Bi0teq
Between 2010 and 2022
-------------------------------------------
Owned and operated by Nanobit Softare (R) (C), 2023
Version 0.0.0.1


Most code is in an alpha stage. These tags will be updated as the code matures.
These tags will also be added with information regarding the
specific file that theyre in a the time.

 Code description:
 Even though this file wants to be big a bad...right now it's just a piss poor
 attempt at being a db front end.


 */



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