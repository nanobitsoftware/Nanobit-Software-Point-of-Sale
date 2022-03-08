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


#define ID_CALC_INPUT 20000 // Input and answer bar
#define ID_CALC_1  10000	// BUtton:1 
#define ID_CALC_2  10001	// Button:2
#define ID_CALC_3  10002	// Button:3
#define ID_CALC_4  10003	// Button:4
#define ID_CALC_5  10004	// Button:5
#define ID_CALC_6  10005	// Button:6
#define ID_CALC_7  10006	// Button:7
#define ID_CALC_8  10007	// Button:8
#define ID_CALC_9  10008	// Button:9
#define ID_CALC_0  10009	// Button:0
#define ID_CALC_A  10010	// Button:+
#define	ID_CALC_S  10011	// Button:-
#define ID_CALC_D  10012	// Button:/
#define ID_CALC_M  10013	// Button:*
#define ID_CALC_E  10014	// Button:=
#define ID_CALC_T  10015	// Button:Tax%
#define ID_CALC_C  10016	// Button:Clear
#define ID_CALC_N  10017	// Button:NEG/POS
#define ID_CALC_DE 10018	// Button:.
#define ID_CALC_B  10019	//Button:<--
#define ID_CALC_CE 10020	// Button:CE
#define CALC_H 40
#define CALC_W  50
#define CALC_X 206//225  // X Size
#define CALC_Y 234//276	//   Y Size
#define CALC_ONTOP TRUE
#define CALC_INPUT_WIDTH 500
#define CALC_INPUT_HEIGHT 40
#define CALC_BTN_SPACE 50
#define CALC_BTN_VSPACE 40
NWC_PARENT* CALC_window;


struct calc_layout
{
	char text[128];
	int  y;
	int  x;
	int  width;
	int  heiht;
	int  id;

};


struct calc_layout layout[] =
{
	{"C",1,1	,CALC_W, CALC_H,ID_CALC_C},                 
	{"Tax",1,CALC_BTN_SPACE*1,CALC_W, CALC_H,ID_CALC_T},  
	{"CE",1,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_CE}, 
	{"/",1,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_D},
	{"7",CALC_BTN_VSPACE*1,1,CALC_W, CALC_H,ID_CALC_7},	    
	{"8",CALC_BTN_VSPACE*1,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_8},  
	{"9",CALC_BTN_VSPACE * 1,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_9},	 
	{"*",CALC_BTN_VSPACE * 1,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_M},
	{"4",CALC_BTN_VSPACE * 2,1,CALC_W, CALC_H,ID_CALC_4},	
	{"5",CALC_BTN_VSPACE*2,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_5}, 
	{"5",CALC_BTN_VSPACE * 2,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_5},	
	{"-",CALC_BTN_VSPACE * 2,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_N},
	{"1",CALC_BTN_VSPACE * 3,1,CALC_W, CALC_H,ID_CALC_1},	
	{"2",CALC_BTN_VSPACE*3,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_2}, 
	{"3",CALC_BTN_VSPACE * 3,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_3},	 
	{"+",CALC_BTN_VSPACE * 3,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_A},
	{"-/+",CALC_BTN_VSPACE * 4,1,CALC_W, CALC_H,ID_CALC_N}, 
	{"0",CALC_BTN_VSPACE*4,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_0}, 
	{".",CALC_BTN_VSPACE * 4,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_DE}, 
	{"=",CALC_BTN_VSPACE * 4,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_A},

	{"",0,0,0,0,-1}
};




void create_calc_window(void)
{
	HWND temp;
	int i;
	int x;
	int b;

	i = b = x = 0;


	temp = 0;

	if (calcwindow != NULL) 
		return; // We already have the window. Do not recreate.


	CALC_window = create_parent("NanoCalculator");
	set_parent_config(CALC_window, ( HWND ) 0, ( LRESULT* ) CALCwindow_proc, 0, 0, CALC_X, CALC_Y,
		0, CALC_ONTOP, WS_EX_TOPMOST, 0);
//	LONG lStyle = GetWindowLong(CALC_window->window_pointer, GWL_STYLE);
//	lStyle |= WS_THICKFRAME; // 6px white stripe cause of this.
	//lStyle = lStyle & ~WS_CAPTION;
	//SetWindowLongA(CALC_window->window_pointer, lStyle, lStyle);
	b = 0;
	AddEdit_Parent(CALC_window, "CALCINPUT", 3, 3, 200, 25, 0, ID_CALC_INPUT, 0, TRUE);
	for (i=0;i<4;i++)
		for ( x = 0; x < 5; x++ )
		{
			if ( layout[b].text == "" )
				continue;

			AddButton_Parent(CALC_window, layout[b].text, layout[b].x, layout[b].y+30, layout[b].width, layout[b].heiht, 0, layout[b].id, 0, TRUE);
			b++;

		}



	calcwindow = CALC_window->window_control;
	ShowWindow(calcwindow, SW_SHOW);
	CenterWindow(calcwindow, mainwindow);
	
	SetWindowLongPtr(calcwindow, GWL_STYLE, WS_SYSMENU);
	//SetWindowPos(calcwindow, HWND_TOPMOST, 100, 100, 500, 500, SWP_SHOWWINDOW);
	ShowWindow(calcwindow, TRUE);

	SetFocus(calcwindow);

}

LRESULT APIENTRY CALCwindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
	{
		
			DestroyWindow(calcwindow);
			calcwindow = NULL;
	
		break;
	}

	

	default: {
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
