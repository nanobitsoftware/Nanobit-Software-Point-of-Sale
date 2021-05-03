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

HDC prndc;
HFONT hf;
#define OUTWIDTH 8
int x_offset, y_offset;
HDC pdc;
RECT r;
int cur_pages;
int cur_page;
int cur_line;
extern int cur_invoice;
extern char invoice_created_date[1024];


int get_max_char (int colsize)
{
	HFONT hf;
	TEXTMETRIC tm;
	HDC dc;

	hf = CreateFont(13,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
	dc = GetDC(mainwindow);
	SelectObject(dc, hf);

	GetTextMetrics(dc, &tm);
	ReleaseDC(mainwindow, dc);
	 

	return (colsize / tm.tmAveCharWidth)-3;
}

int get_max_lines(int rowsize)
{
		HFONT hf;
	TEXTMETRIC tm;
	HDC dc;

	hf = CreateFont(13,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
	dc = GetDC(mainwindow);
	SelectObject(dc, hf);

	GetTextMetrics(dc, &tm);
	ReleaseDC(mainwindow, dc);
	 
	LOG("Heigth: %d, Rec: %d", tm.tmHeight, rowsize);

	return (rowsize / tm.tmHeight)-3;
}



int pages_col(char * str)
{
	int i;
	int stop;
	char *point;
	int pages;
	int lines;

	i = stop = pages = 0;
	lines = 1;

	stop = get_max_char(550);

	if (!str)
		return 0;

	for (point = str;*point;point++)
	{


		if (*point == '\n')
		{
			i=0;
			lines++;
			continue;
		}

		if (i == stop)
		{
			lines++;
			i=0;
			continue;
		}
		i++;
	}

	if (lines > get_max_lines(640))
	{
		return  lines / get_max_lines(640);
	}
	else
		return 1;

	

}



HDC get_printer_dc(void)
{
	DWORD n,r;
	PRINTER_INFO_4 *pinfo;
	HDC hdc;
	PRINTDLG pd;
	HWND hwnd;
	
	if (prndc != NULL)
		return GetDC(mainwindow);

	/*ZeroMemory(&pd, sizeof(pd));
	pd.lStructSize = sizeof(pd);
	pd.hwndOwner = mainwindow;
	pd.hDevMode = NULL;     // Don't forget to free or store hDevMode.
	pd.hDevNames = NULL;     // Don't forget to free or store hDevNames.
	pd.Flags = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC;
	pd.nCopies = 1;
	pd.nFromPage = 0xFFFF;
	pd.nToPage = 0xFFFF;
	pd.nMinPage = 1;
	pd.nMaxPage = 0xFFFF;*/
//							 
	EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4,NULL,0, &n,&r);
	pinfo = malloc(n);
	EnumPrinters (PRINTER_ENUM_LOCAL, NULL, 4, (PBYTE) pinfo, n,&n,&r);
	GiveError(pinfo->pPrinterName, 0);
	hdc = CreateDC("WINSPOOL", pinfo->pPrinterName, NULL,NULL);
// hdc =- CreateDCA (NULL, "WINSPOOL", NULL, 
/*if (PrintDlg(&pd) == TRUE)
	{
		prndc = pd.hDC;
		return pd.hDC;


	}
	else
	{
		prndc = NULL;
		GiveError("No printer selected.", FALSE);
	}*/
	prndc = hdc;
	free (pinfo);
	return hdc;
}

void draw_text_line(char *str)
{
	char *point;
	char buf[1024];
	int i;

	i=0;
	if (StartDoc(pdc,0)>0)
		if(StartPage(pdc)>0);
	for (point = str;*point;point++)
	{
		if (*point == '\n')
		{
			TextOut(pdc,(r.left+20) * x_offset, ((cur_line*(13*y_offset))+r.top+200) * y_offset, buf, strlen(buf));
			
			buf[0] = '\0';
			cur_line++;
			i=0;
			continue;
		}

		buf[i] = *point;
		buf[i+1] = '\0';
		if (strlen(buf) >= 76)
		{
			TextOut(pdc,(r.left+20) * x_offset, ((cur_line*(13*y_offset))+r.top+200) * y_offset, buf, strlen(buf));
			
			buf[0] = '\0';
			cur_line++;
			i=0;
			continue;
		}

		i++;
		
	}
	if (buf[0] != '\0')
	{
			TextOut(pdc,(r.left+20) * x_offset, ((cur_line*(13*y_offset))+r.top+200) * y_offset, buf, strlen(buf));
			
			buf[0] = '\0';
			cur_line++;
	}
	//if (EndPage(pdc)>0)
		//if (EndDoc(pdc)>0)
				//exit(1);
}

void print_work_order(int inv,char * order)
{
	RECT r, pr;
	
	HPEN oldpen;
	
	

	
	
	

	int i;


	char *point;
	int line;

	char buf[1024];
	 static DOCINFO    di;


	




	char sstr[1024];
	line=0;
	 
	GetClientRect(NULL,&r);
	
	//di.lpszOutput = "wo2.xps";


	if (!prndc)
		prndc = get_printer_dc();
	

	if (prndc == NULL)
		GiveError("Printer is BAD.",1);
	
	pr.left  = (GetDeviceCaps (prndc, PHYSICALWIDTH) -
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH) / 2 
		- GetDeviceCaps (prndc, PHYSICALOFFSETX) ;

	pr.right = pr.left + 
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH ;


	pr.top    = GetDeviceCaps (prndc, LOGPIXELSY) -
		GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	pr.bottom = GetDeviceCaps (prndc, PHYSICALHEIGHT); //- 
	//	GetDeviceCaps (prndc, LOGPIXELSY) ;
		//-GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	GetClientRect(printpreview, &r);

	r.left = pr.left / 7;
	r.top = 1;//pr.top / 7;
	r.right = (pr.right / 7);
	r.bottom = (pr.bottom / 7);

	cur_pages = (count_lines(order) / 60)+1;
	if (cur_page == 0)
		cur_page = 1;
	


	


	x_offset = 7;
	y_offset = 7;
	if (!prndc)
		prndc = get_printer_dc();
	if (StartDoc(prndc, &di) > 0)
		if (StartPage(prndc) > 0)
		{
			
			
			oldpen = SelectObject(prndc, CreatePen(PS_SOLID, 8, RGB(0,0,0)));
			DeleteObject(hf);
			hf = CreateFont(40*y_offset,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
			SelectObject(prndc, hf);
			
			sprintf(sstr, "-Work Order-", inv);
			TextOut(prndc,(r.right / 2 - (strlen(sstr) * 8)) * x_offset, (r.top+(2)) * y_offset, sstr, strlen(sstr));

			DeleteObject(hf);
			hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
			
			SelectObject(prndc, hf);

			Rectangle(prndc,(r.right-(120)) * x_offset, r.top * y_offset,r.right * x_offset,(r.top+(25))*y_offset);
			


			sprintf(sstr, "Invoice: %6.6d", inv);
			TextOut(prndc,(r.right-(118)) * x_offset, (r.top+(2)) * y_offset, sstr, strlen(sstr));

			sprintf(sstr, "Page: %d of %d", cur_page, cur_pages);
			TextOut(prndc,(r.left) * x_offset, (r.top) * y_offset, sstr, strlen(sstr));


			i=0;

			for (point = order;*point;point++)
			{
				if (line > 60)
				{
					EndPage(prndc);
					StartPage(prndc);
					cur_page++;

					DeleteObject(hf);
					hf = CreateFont(40*y_offset,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
					SelectObject(prndc, hf);

					sprintf(sstr, "-Work Order-", inv);
					TextOut(prndc,(r.right / 2 - (strlen(sstr) * 8)) * x_offset, (r.top+(2)) * y_offset, sstr, strlen(sstr));

					DeleteObject(hf);
					hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");

					SelectObject(prndc, hf);

					Rectangle(prndc,(r.right-(120)) * x_offset, r.top * y_offset,r.right * x_offset,(r.top+(25))*y_offset);



					sprintf(sstr, "Invoice: %6.6d", inv);
					TextOut(prndc,(r.right-(118)) * x_offset, (r.top+(2)) * y_offset, sstr, strlen(sstr));

					sprintf(sstr, "Page: %d of %d", cur_page, cur_pages);
					TextOut(prndc,(r.left) * x_offset, (r.top) * y_offset, sstr, strlen(sstr));
					line = 0;
				}


				if (*point == '\n')
				{
					TextOut(prndc,(r.left + 20)*x_offset, ((line*(13))+r.top+40) * y_offset, buf, strlen(buf));

					buf[0] = '\0';
					i=0;
					line++;
					continue;
				}
				if (*point == '\b' || *point == '\033' || *point == '\032')
				{
					continue;
				}

				buf[i] = *point;
				buf[i+1]  = '\0';
				i++;
			}
			if (buf[0] != '\0')
			{
				TextOut(prndc,(r.left + 20)*x_offset, ((line*(13))+r.top+40) * y_offset, buf, strlen(buf));
				buf[0] = '\0';
				i=0;
				line++;
			}	



			//oldpen = SelectObject(prndc, CreatePen(PS_SOLID, 8, RGB(0,0,0)));


		}
		if (EndPage(prndc)>0)
			if (EndDoc(prndc)>0)
				//exit(1);
				i=0;

		//ShellExecute(NULL,"open", "wo2.xps", NULL,NULL, SW_SHOWNORMAL);
		//DeleteDC(prndc);
		//prndc = NULL;
	

}




void print_report(char *report)
{
	RECT r, pr;
	
	HPEN oldpen;
	
	
	

	int i;


	char *point;
	int line;

	char buf[1024];
	 static DOCINFO    di = { sizeof (DOCINFO), TEXT ("Nano POS Report") } ;

	

	char sstr[1024];
	line=0;
	 
	GetClientRect(NULL,&r);


	if (!prndc)
		prndc = get_printer_dc();
	

	if (prndc == NULL)
		GiveError("Printer is BAD.",1);
	
	pr.left  = (GetDeviceCaps (prndc, PHYSICALWIDTH) -
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH) / 2 
		- GetDeviceCaps (prndc, PHYSICALOFFSETX) ;

	pr.right = pr.left + 
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH ;


	pr.top    = GetDeviceCaps (prndc, LOGPIXELSY) -
		GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	pr.bottom = GetDeviceCaps (prndc, PHYSICALHEIGHT);

	GetClientRect(printpreview, &r);

	r.left = pr.left / 7;
	r.top = 1;
	r.right = (pr.right / 7);
	r.bottom = (pr.bottom / 7);

	cur_pages = (count_lines(report) / 60)+1;
	cur_page =1;

	


	


	x_offset = 7;
	y_offset = 7;

	buf[0] = '\0';
	sstr[0] = '\0';
	if (!prndc)
	prndc = get_printer_dc();
	if (StartDoc(prndc, 0) > 0)
		if (StartPage(prndc) > 0)
		{
			
			
			oldpen = SelectObject(prndc, CreatePen(PS_SOLID, 8, RGB(0,0,0)));

			DeleteObject(hf);
			hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
			
			SelectObject(prndc, hf);


			sprintf(sstr, "Page: %d of %d", cur_page, cur_pages);
			TextOut(prndc,(r.left) * x_offset, (r.top) * y_offset, sstr, strlen(sstr));


			i=0;

			for (point = report;*point;point++)
			{
				if (line > 60)
				{
					EndPage(prndc);
					StartPage(prndc);
					cur_page++;

					sprintf(sstr, "Page: %d of %d", cur_page, cur_pages);

					TextOut(prndc,(r.left) * x_offset, (r.top) * y_offset, sstr, strlen(sstr));
					line = 0;
				}


				if (*point == '\n')
				{
					TextOut(prndc,(r.left + 20)*x_offset, ((line*(13))+r.top+10) * y_offset, buf, strlen(buf));

					buf[0] = '\0';
					i=0;
					line++;
					continue;
				}
				if (*point == '\b' || *point == '\033' || *point == '\032')
				{
					continue;
				}

				buf[i] = *point;
				buf[i+1]  = '\0';
				i++;
			}
			if (buf[0] != '\0')
			{
				TextOut(prndc,(r.left + 20)*x_offset, ((line*(13))+r.top+10) * y_offset, buf, strlen(buf));
				buf[0] = '\0';
				i=0;
				line++;
			}	




		}
		if (EndPage(prndc)>0)
			if (EndDoc(prndc)>0)
				i=0;

	

}





void draw_inv_footer (HDC dc, RECT r, int x_offset, int y_offset)
{
	char buf[1024];
	int line;
	char *point;
	int i;

	i=0;


	sprintf(buf, "Signature:_____________________________ Date:____/____/____");
	TextOut(dc,(r.left+20)*x_offset, ((48*(13))+r.top+220) * y_offset, buf, strlen(buf));

	line = 49;
	LOG("Warranty \"%s\"", Company_Warranty);
	for (point = Company_Warranty;*point;point++)
	{


		if (*point == '\n')
		{


			TextOut(dc,(r.left+20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));

			buf[0] = '\0';
			i=0;
			line++;
			continue;
		}


		buf[i] = *point;
		buf[i+1]  = '\0';
		i++;
	}
	if (buf[0] != '\0')
	{
		TextOut(dc,(r.left+20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
		buf[0] = '\0';
		i=0;
		line++;
	}

}

void draw_inv_header (HDC dc, RECT r, int x_offset, int y_offset, int invoice)
{
	char sstr[1024];
	char csz[1024];


	sprintf(sstr, "INVOICE");
	TextOut(dc,(r.right / 2 - (strlen(sstr) * 8)) * x_offset, (r.top+(2)) * y_offset, sstr, strlen(sstr));


	DeleteObject(hf);
	hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
	SelectObject(dc, hf);	


	TextOut(dc,(r.right / 2 - (strlen(Company_Name) * 4)) * x_offset, ((3*(13))+r.top+20) * y_offset, Company_Name, strlen(Company_Name));
	TextOut(dc,(r.right / 2 - (strlen(Company_Motto) * 4)) * x_offset, ((4*(13))+r.top+20) * y_offset, Company_Motto, strlen(Company_Motto));
	TextOut(dc,(r.right / 2 - (strlen(Company_Address) * 4)) * x_offset, ((5*(13))+r.top+20) * y_offset, Company_Address, strlen(Company_Address));
	sprintf(csz, "%s, %s %s", Company_City, Company_State, Company_Zip);
	TextOut(dc,(r.right / 2 - (strlen(csz) * 4)) * x_offset, ((6*(13))+r.top+20) * y_offset, csz, strlen(csz));
	sprintf(csz, "Tel: %s", Company_Phone);
	TextOut(dc,(r.right / 2 - (strlen(csz) * 4)) * x_offset, ((7*(13))+r.top+20) * y_offset, csz, strlen(csz));


	sprintf(sstr, "Invoice: %6.6d",invoice);


	Rectangle(dc,(r.right-(120)) * x_offset, r.top * y_offset,r.right * x_offset,(r.top+(25))*y_offset);
	TextOut(dc,(r.right-(118)) * x_offset, (r.top+(2)) * y_offset, sstr, strlen(sstr));
	
	sprintf(sstr, "Page %d of %d", cur_page, cur_pages);
	TextOut(dc,(r.left+0)*x_offset, ((1*(13))+r.top) * y_offset, sstr, strlen(sstr));


	Rectangle (dc, (r.left+10) * x_offset, (r.top + 200) * y_offset, (r.right - 10) * x_offset, (r.bottom - 100) * y_offset);
	Rectangle (dc, (r.left+ 550) * x_offset, (r.top+200)*y_offset, (r.right-90)*x_offset, (r.bottom-100)*y_offset);

	sprintf(sstr, "Created %s, Printed %s", invoice_created_date, get_date());
	TextOut(dc,(r.left+0)*x_offset, ((r.top+180)) * y_offset, sstr, strlen(sstr));


	sprintf(sstr, "  Qty.    Price");
	TextOut(dc,(r.left+550)*x_offset, ((r.top+180)) * y_offset, sstr, strlen(sstr));


}


	

void print_invoice (char *str, int copies)
{
	RECT r, pr;
	
	HPEN oldpen;

	
	

	int i;


	char *point;
	HDC dc;
	int line;
	char buf[1024];
	BOOL movecol1;
	BOOL movecol2;
	BOOL bold;

	extern int cur_invoice;
	int c;

	


	





	buf[0] = '\0';
	line=0;

	bold = movecol1 = movecol2 = FALSE; 
	GetClientRect(NULL,&r);


	
	if (!prndc)
		prndc = get_printer_dc();
	
	pr.left  = (GetDeviceCaps (prndc, PHYSICALWIDTH) -
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH) / 2 
		- GetDeviceCaps (prndc, PHYSICALOFFSETX) ;

	pr.right = pr.left + 
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH ;


	pr.top    = GetDeviceCaps (prndc, LOGPIXELSY) -
		GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	pr.bottom = GetDeviceCaps (prndc, PHYSICALHEIGHT); //- 
	//	GetDeviceCaps (prndc, LOGPIXELSY) ;
		//-GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	GetClientRect(printpreview, &r);

	r.left = pr.left / 7;
	r.top = 1;
	r.right = (pr.right / 7);
	r.bottom = (pr.bottom / 7);



	cur_pages = (count_lines(str) / 45)+1;
	if (cur_page == 0)
		cur_page = 1;

	x_offset = 7;
	y_offset = 7;
	
	cur_pages = (count_lines(str) / 45)+1;
	cur_page = 1;
		
	if (StartDoc(prndc, 0) > 0)
		for (c=0;c<copies;c++)
		{
			cur_page = 1;
		if (StartPage(prndc) > 0)
		{
			dc = prndc;
			oldpen = SelectObject(dc, CreatePen(PS_SOLID, 8, RGB(0,0,0)));

			hf = CreateFont(30*y_offset,0,0,0,1000,0,0,0,0,0,0,0,0,"Courier New");
			SelectObject(dc, hf);
			
			draw_inv_header(dc, r, x_offset, y_offset, cur_invoice);
	
			i=0;
			line  = 0;

			for (point = str;*point;point++)
			{
				if (line >45)
				{
					EndPage(dc);
					StartPage(dc);
					line = 0;
					hf = CreateFont(30*y_offset,0,0,0,1000,0,0,0,0,0,0,0,0,"Courier New");
					SelectObject(dc, hf);
					cur_page++;
					draw_inv_header(dc,r, x_offset,y_offset,cur_invoice);



				}
				if (bold == TRUE)
				{
					hf = CreateFont(12*y_offset,0,0,0,1000,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
					SelectObject(dc, hf);

				}

				if (*point == '\n')
				{
					if (bold == TRUE)
					{
						hf = CreateFont(12*y_offset,0,0,0,1000,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
						SelectObject(dc, hf);

					}

					if (movecol1 == TRUE)
					{
						TextOut(dc,(r.left+555)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						movecol1 = FALSE;
						buf[0] = '\0';
						i=0;
						line++;

						continue;
					}
					if (movecol2 == TRUE)
					{
						TextOut(dc,(r.right-85)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						movecol2 = FALSE;
						buf[0] = '\0';
						i=0;
						line++;
						continue;
					}


					TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
					if (bold == TRUE)
					{

						hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
						SelectObject(dc, hf);
						bold = FALSE;
					}


					buf[0] = '\0';
					i=0;
					line++;
					continue;
				}
				if (*point == '\033')
				{
					if (buf[0] != '\0')
					{

						TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						if (bold == TRUE)
						{

							hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
							SelectObject(dc, hf);
							bold = FALSE;
						}


						buf[0] = '\0';
						i=0;
					}

					movecol1 = TRUE;
					continue;
				}
				if (*point == '\032')
				{
					if (buf[0] != '\0')
					{
						if (movecol1 == TRUE)
						{
							TextOut(dc,(r.left+555)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
							movecol1 = FALSE;
							buf[0] = '\0';
							i=0;
							movecol2 = TRUE;
							continue;
						}

						TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						if (bold == TRUE)
						{

							hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
							SelectObject(dc, hf);
							bold = FALSE;
						}

						buf[0] = '\0';
						i=0;
					}
					movecol1 = FALSE;
					movecol2 = TRUE;
					continue;
				}
				if (*point == '\b')
				{
					bold = TRUE;
					if (buf[0] != '\0')
					{
						if (movecol1 == TRUE)
						{
							TextOut(dc,(r.left+555)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
							movecol1 = FALSE;
							buf[0] = '\0';
							bold = TRUE;
							i=0;
							movecol2 = TRUE;
							continue;
						}

						TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));

						buf[0] = '\0';
						i=0;
						bold = TRUE;
					}
					bold = TRUE;
					continue;
				}








				buf[i] = *point;
				buf[i+1]  = '\0';
				i++;
			}
			if (buf[0] != '\0')
			{
				TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
				buf[0] = '\0';
				i=0;
				line++;
			}	

			i=0;
			buf[0] = '\0';
			
			draw_inv_footer(dc, r, x_offset,y_offset);

			
		}





		if (EndPage(prndc)>0)
			continue;
		}
			if (EndDoc(prndc)>0)
				return;




	
}

	
	 

	

	

void DrawInvoiceBox (HDC dc, char *str)
{
	RECT r, pr;
	
	HPEN oldpen;

	
	

	int i;


	char *point;
	int line;
	char buf[1024];
	BOOL movecol1;
	BOOL movecol2;
	BOOL bold;

	extern int cur_invoice;

	


	





	buf[0] = '\0';
	line=0;

	bold = movecol1 = movecol2 = FALSE; 
	GetClientRect(NULL,&r);


	if (!dc)
		GiveError("Invalid DC for drawing.",1);

	if (!prndc)
		prndc = get_printer_dc();
	
	pr.left  = (GetDeviceCaps (prndc, PHYSICALWIDTH) -
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH) / 2 
		- GetDeviceCaps (prndc, PHYSICALOFFSETX) ;

	pr.right = pr.left + 
		GetDeviceCaps (prndc, LOGPIXELSX) * OUTWIDTH ;


	pr.top    = GetDeviceCaps (prndc, LOGPIXELSY) -
		GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	pr.bottom = GetDeviceCaps (prndc, PHYSICALHEIGHT); //- 
	//	GetDeviceCaps (prndc, LOGPIXELSY) ;
		//-GetDeviceCaps (prndc, PHYSICALOFFSETY) ;

	GetClientRect(printpreview, &r);

	r.left = pr.left / 7;
	r.top = r.top / 7;
	r.right = (pr.right / 7);
	r.bottom = (pr.bottom / 7);


	x_offset = 1;
	y_offset = 1;

	cur_pages = (count_lines(str) / 45)+1;
	if (cur_page == 0)
		cur_page = 1;
	
	
	//DeleteObject(oldpen);
	//oldpen = SelectObject(dc, CreatePen(PS_SOLID, 1, RGB(0,0,0)));
	pdc = prndc;
	DeleteObject(hf);	
	hf = CreateFont(30*y_offset,0,0,0,1000,0,0,0,0,0,0,0,0,"Courier New");
	SelectObject(dc, hf);
			
	draw_inv_header(dc, r, x_offset, y_offset, cur_invoice);


	
	
	i=0;


	for (point = str;*point;point++)
	{
		if (line >45)
		{
			//EndPage(dc);
		//	StartPage(dc);
			line = 0;
			DeleteObject(hf);
			hf = CreateFont(30*y_offset,0,0,0,1000,0,0,0,0,0,0,0,0,"Courier New");
			SelectObject(dc, hf);
			cur_page++;
			draw_inv_header(dc,r, x_offset,y_offset,cur_invoice);
			



		}	
		if (bold == TRUE)
		{
			DeleteObject(hf);
			hf = CreateFont(14*y_offset,0,0,0,1000,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
			SelectObject(dc, hf);

		}

		if (*point == '\n')
		{
			if (bold == TRUE)
			{
				DeleteObject(hf);
				hf = CreateFont(14*y_offset,0,0,0,1000,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
				SelectObject(dc, hf);

			}

			if (movecol1 == TRUE)
			{
				TextOut(dc,(r.left+555)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
				movecol1 = FALSE;
				buf[0] = '\0';
				i=0;
				line++;
				
				continue;
			}
			if (movecol2 == TRUE)
			{
				TextOut(dc,(r.right-85)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
				movecol2 = FALSE;
				buf[0] = '\0';
				i=0;
				line++;
				continue;
			}

			
			TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
			if (bold == TRUE)
			{

				DeleteObject(hf);
				hf = CreateFont(14*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
				SelectObject(dc, hf);
				bold = FALSE;
			}


			buf[0] = '\0';
			i=0;
			line++;
			continue;
		}
		if (*point == '\033')
		{
			if (buf[0] != '\0')
			{
				
				TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
				if (bold == TRUE)
				{

					DeleteObject(hf);
					hf = CreateFont(14*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
					SelectObject(dc, hf);
					bold = FALSE;
				}


				buf[0] = '\0';
				i=0;
			}

			movecol1 = TRUE;
			continue;
		}
		if (*point == '\032')
		{
			if (buf[0] != '\0')
			{
				if (movecol1 == TRUE)
				{
					TextOut(dc,(r.left+555)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
					movecol1 = FALSE;
					buf[0] = '\0';
					i=0;
					movecol2 = TRUE;
					continue;
				}

				TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
				if (bold == TRUE)
				{

					DeleteObject(hf);
					hf = CreateFont(14*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
					SelectObject(dc, hf);
					bold = FALSE;
				}

				buf[0] = '\0';
				i=0;
			}
			movecol1 = FALSE;
			movecol2 = TRUE;
			continue;
		}
		if (*point == '\b')
		{
			bold = TRUE;
			if (buf[0] != '\0')
			{
				if (movecol1 == TRUE)
				{
					TextOut(dc,(r.left+555)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
					movecol1 = FALSE;
					buf[0] = '\0';
					bold = TRUE;
					i=0;
					movecol2 = TRUE;
					continue;
				}

				TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));

				buf[0] = '\0';
				i=0;
				bold = TRUE;
			}
			bold = TRUE;
			continue;
		}








		buf[i] = *point;
		buf[i+1]  = '\0';
		i++;
	}
	if (buf[0] != '\0')
	{
		TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
		buf[0] = '\0';
		i=0;
		line++;
	}

	i=0;
	buf[0] = '\0';

	draw_inv_footer(dc, r, x_offset,y_offset);


	//StretchBlt(oldc, 0,0, r.right, /*700*/r.bottom, dc, 0,0,r.right,r.bottom, SRCCOPY);
	
	//return;

/*	return;

	
	

	if (StartDoc(prndc,0) > 0)
		if (StartPage(prndc)>0)
		{
			StretchBlt(prndc, r.left * 7,r.top*7, r.right*7, r.bottom*7, tdc, 0,0,r.right,r.bottom, SRCCOPY);
		}
		if (EndPage(prndc)>0)
			if (EndDoc(prndc)>0)
				exit(1);

	

	*/
	
	
	//return;
		//	return;
	x_offset = 7;
	y_offset = 7;
	
	cur_pages = (count_lines(str) / 45)+1;
	cur_page = 1;
		
	if (StartDoc(prndc, 0) > 0)
		if (StartPage(prndc) > 0)
		{
			dc = prndc;
			oldpen = SelectObject(dc, CreatePen(PS_SOLID, 8, RGB(0,0,0)));

			hf = CreateFont(30*y_offset,0,0,0,1000,0,0,0,0,0,0,0,0,"Courier New");
			SelectObject(dc, hf);
			
			draw_inv_header(dc, r, x_offset, y_offset, cur_invoice);
	
			i=0;
			line  = 0;

			for (point = str;*point;point++)
			{
				if (line >45)
				{
					EndPage(dc);
					StartPage(dc);
					line = 0;
					hf = CreateFont(30*y_offset,0,0,0,1000,0,0,0,0,0,0,0,0,"Courier New");
					SelectObject(dc, hf);
					cur_page++;
					draw_inv_header(dc,r, x_offset,y_offset,cur_invoice);



				}
				if (bold == TRUE)
				{
					hf = CreateFont(12*y_offset,0,0,0,1000,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
					SelectObject(dc, hf);

				}

				if (*point == '\n')
				{
					if (bold == TRUE)
					{
						hf = CreateFont(12*y_offset,0,0,0,1000,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
						SelectObject(dc, hf);

					}

					if (movecol1 == TRUE)
					{
						TextOut(dc,(r.left+555)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						movecol1 = FALSE;
						buf[0] = '\0';
						i=0;
						line++;

						continue;
					}
					if (movecol2 == TRUE)
					{
						TextOut(dc,(r.right-85)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						movecol2 = FALSE;
						buf[0] = '\0';
						i=0;
						line++;
						continue;
					}


					TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
					if (bold == TRUE)
					{

						hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
						SelectObject(dc, hf);
						bold = FALSE;
					}


					buf[0] = '\0';
					i=0;
					line++;
					continue;
				}
				if (*point == '\033')
				{
					if (buf[0] != '\0')
					{

						TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						if (bold == TRUE)
						{

							hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
							SelectObject(dc, hf);
							bold = FALSE;
						}


						buf[0] = '\0';
						i=0;
					}

					movecol1 = TRUE;
					continue;
				}
				if (*point == '\032')
				{
					if (buf[0] != '\0')
					{
						if (movecol1 == TRUE)
						{
							TextOut(dc,(r.left+555)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
							movecol1 = FALSE;
							buf[0] = '\0';
							i=0;
							movecol2 = TRUE;
							continue;
						}

						TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
						if (bold == TRUE)
						{

							hf = CreateFont(12*y_offset,0,0,0,0,0,0,0,0,0,0,PROOF_QUALITY,0,"Courier New");
							SelectObject(dc, hf);
							bold = FALSE;
						}

						buf[0] = '\0';
						i=0;
					}
					movecol1 = FALSE;
					movecol2 = TRUE;
					continue;
				}
				if (*point == '\b')
				{
					bold = TRUE;
					if (buf[0] != '\0')
					{
						if (movecol1 == TRUE)
						{
							TextOut(dc,(r.left+555)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));
							movecol1 = FALSE;
							buf[0] = '\0';
							bold = TRUE;
							i=0;
							movecol2 = TRUE;
							continue;
						}

						TextOut(dc,(r.left + 20)*x_offset, ((line*(13))+r.top+220) * y_offset, buf, strlen(buf));

						buf[0] = '\0';
						i=0;
						bold = TRUE;
					}
					bold = TRUE;
					continue;
				}








				buf[i] = *point;
				buf[i+1]  = '\0';
				i++;
			}
			if (buf[0] != '\0')
			{
				TextOut(dc,(r.left + 20)*x_offset, ((line*(13*y_offset))+r.top+220) * y_offset, buf, strlen(buf));
				buf[0] = '\0';
				i=0;
				line++;
			}	

			i=0;
			buf[0] = '\0';
			
			draw_inv_footer(dc, r, x_offset,y_offset);

			
		}





		if (EndPage(prndc)>0)
			if (EndDoc(prndc)>0)
			{
				DestroyWindow(printpreview);
				printpreview = NULL;
				//exit(1);
			}

		return;


	
}





BOOL create_print_preview (char *str)
{

	WNDCLASS wnd;
	
	HDC dc;

	if (!printpreview)
	{
	wnd.style         = CS_HREDRAW | CS_VREDRAW ;
	wnd.lpfnWndProc   = printpreview_proc ;
	wnd.cbClsExtra    = 0 ;
	wnd.cbWndExtra    = 0 ;
	wnd.hInstance     = g_hInst ;
	wnd.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wnd.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wnd.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wnd.lpszMenuName  = TEXT("Print Preview") ;
	wnd.lpszClassName = TEXT("Print Preview");

	RegisterClass(&wnd);
	printpreview = CreateWindow ("Print Preview", TEXT ("Print Preview"),
						  WS_OVERLAPPEDWINDOW,
						  CW_USEDEFAULT, CW_USEDEFAULT,
						  1024, 768,
						  NULL, NULL, g_hInst, NULL) ;

	

	cur_page = 0;
	cur_pages = 0;
	}
	dc = GetDC(printpreview);
	DeleteObject(hf);
	hf = CreateFont(14,0,0,0,0,0,0,0,0,0,0,0,0,"Courier New");
	
	SetTextColor(dc, RGB(0,0,0));

	ShowWindow(printpreview,SW_SHOW);
	SelectObject(dc, hf);
	
	DrawInvoiceBox(dc, str);
	cur_line=0;
	DeleteDC(dc);
	cur_pages = 0;
	cur_page = 0;
	return TRUE;
}

LRESULT APIENTRY printpreview_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	
	switch (msg)
	{


	case WM_PAINT:
		BeginPaint(printpreview,NULL);
		//DrawInvoiceBox(GetDC(printpreview), 0);
	//	cur_line =0;
		prepare_ticket_print();
		EndPaint(printpreview, NULL);
		break;
		
	default:
		{
		

			return DefWindowProc (hwnd, msg, wparam, lparam);
			break;
		}
	}
	return DefWindowProc (hwnd, msg, wparam, lparam);
}