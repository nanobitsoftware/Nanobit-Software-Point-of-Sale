#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <Commdlg.h>
#include <winsock.h>
#include <richedit.h>
#include <stdarg.h>
#include <Mmsystem.h>
#include <errno.h>
#include "Nano PoS.h"




void write_buffer(const char *str)
{
	FILE *fp;

	if (str == NULL)
		return;

	if ((fp = fopen(DEBUG_FILE, "a")) == NULL)
	{
		GiveError("Unable to open debug file. Please restart with debugging support off.", FALSE);
		return;
	}

	fprintf(fp, "%s\n", str);
	fclose (fp); 
}
void LOG (char *fmt, ...)
{

	char buf[200000];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	write_buffer(buf);


}



int read_string (char buf[], FILE *fp)
{

	int c;
	int len=0;
	int f;
	if (fp == NULL)
		return -1;
	len = 0;
	f=0;
	while (!feof(fp))
	{
		f++;




		c = getc(fp);


		
		if (c == '\0')
			return len;
		if (c == '\n')
			return len;
		
		if (c == '\r')
			return len;
		buf[len++] = c;
		//              
		//		if (f >= 100)
		//return len;        



	}
	return EOF;
}




char * load_file (void)
{
	OPENFILENAME filename;
	static char str[1024];



	memset(&filename, 0, sizeof(filename));


	filename.lStructSize = sizeof(filename);
	filename.hwndOwner = NULL;
	filename.lpstrFilter = "Text file (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
	filename.lpstrFile = str;
	filename.lpstrFile[0] = '\0';
	filename.nMaxFile = sizeof(str);
	filename.Flags =  OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	filename.lpstrDefExt = "txt";
	filename.lpstrInitialDir = NULL;
	filename.nFilterIndex =1;



	if ((GetOpenFileName(&filename)) == 0)
	{
		return NULL;
	}

	return str;					  

}
char * save_file (char *filter)
{
	OPENFILENAME filename;
	static char str[1024];



	memset(&filename, 0, sizeof(filename));


	filename.lStructSize = sizeof(filename);
	filename.hwndOwner = NULL;
	filename.lpstrFilter = filter;
	filename.lpstrFile = str;
	filename.lpstrFile[0] = '\0';
	filename.nMaxFile = sizeof(str);
	filename.Flags =  OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	filename.lpstrDefExt = "txt";
	filename.lpstrInitialDir = NULL;
	filename.nFilterIndex =1;

	

	if ((GetSaveFileName(&filename)) == 0)
	{
		return NULL;
	}

	return str;

}



void GiveError(char * wrong, BOOL KillProcess)
{
	if (wrong == NULL)
	{
		MessageBox(NULL, "An unknown error has occured, please restart Nano PoS and try again.", "Nano PoS Error", MB_TASKMODAL|MB_ICONSTOP | MB_OK);
		if (KillProcess)
			exit(0);
		else
			return;
	}

	MessageBox(NULL, wrong, "Nano PoS Error",MB_TASKMODAL| MB_ICONSTOP | MB_OK);

	if (KillProcess)
		exit(0);
	else
		return;
}

