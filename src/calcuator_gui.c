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
#include "calculator.h"


NWC_PARENT* CALC_window;



struct calc_layout layout[] =
{
	{"C",1,1	,CALC_W, CALC_H,ID_CALC_C},
	{"Tax",1,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_T},
	{"CE",1,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_CE},
	{"/",1,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_D},
	{"7",CALC_BTN_VSPACE * 1,1,CALC_W, CALC_H,ID_CALC_7},
	{"8",CALC_BTN_VSPACE * 1,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_8},
	{"9",CALC_BTN_VSPACE * 1,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_9},
	{"*",CALC_BTN_VSPACE * 1,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_M},
	{"4",CALC_BTN_VSPACE * 2,1,CALC_W, CALC_H,ID_CALC_4},
	{"5",CALC_BTN_VSPACE * 2,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_5},
	{"6",CALC_BTN_VSPACE * 2,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_6},
	{"-",CALC_BTN_VSPACE * 2,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_N},
	{"1",CALC_BTN_VSPACE * 3,1,CALC_W, CALC_H,ID_CALC_1},
	{"2",CALC_BTN_VSPACE * 3,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_2},
	{"3",CALC_BTN_VSPACE * 3,CALC_BTN_SPACE * 2,CALC_W, CALC_H,ID_CALC_3},
	{"+",CALC_BTN_VSPACE * 3,CALC_BTN_SPACE * 3,CALC_W, CALC_H,ID_CALC_A},
	{"-/+",CALC_BTN_VSPACE * 4,1,CALC_W, CALC_H,ID_CALC_N},
	{"0",CALC_BTN_VSPACE * 4,CALC_BTN_SPACE * 1,CALC_W, CALC_H,ID_CALC_0},
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
	if (msg == WM_KEYUP)
		LOG("wParam: LOWORD: 0x%x", wparam);
	

	switch (msg)
	{

	case WM_KEYUP:
	{
		break;
	}

	case WM_KEYDOWN:
	{
		switch (wparam)
		{
			//return  CallWindowProc(OldProc, hwnd, msg, wParam, lParam);
		case 0x31: case 0x61: calc_append_number('1');break;
		case 0x32: case 0x62: calc_append_number('2');break;
		case 0x33: case 0x63: calc_append_number('3');break;
		case 0x34: case 0x64: calc_append_number('4');break;
		case 0x35: case 0x65: calc_append_number('5');break;
		case 0x36: case 0x66: calc_append_number('6');break;
		case 0x37: case 0x67: calc_append_number('7');break;
		case 0x38: case 0x68: calc_append_number('8');break;
		case 0x39: case 0x69: calc_append_number('9');break;
		case 0x30: case 0x60: calc_append_number('0');break;
		case 0x6E: case 0xBE: calc_append_number('.'); break;
		case 0x6B: calc_append_number('+');break;
		case 0x6F: calc_append_number('-');break;
		case 0x6D: calc_append_number('/');break;
		case 0x6A: calc_append_number('*');break;
		case 0xBB: calc_append_number('=');break;
		}
		
	}
	case WM_COMMAND:
	{


		switch ( LOWORD(wparam) )
		{
			case ID_CALC_1:
			
				calc_append_number('1');
				break;
			case ID_CALC_2:
				calc_append_number('2');
				break;
			case ID_CALC_3:
				calc_append_number('3');
				break;
			case ID_CALC_4:
				calc_append_number('4');
				break;
			case ID_CALC_5:
				calc_append_number('5');
				break;
			case ID_CALC_6:
				calc_append_number('6');
				break;
			case ID_CALC_7:
				calc_append_number('7');
				break;
			case ID_CALC_8:
				calc_append_number('8');
				break;
			case ID_CALC_9:
				calc_append_number('9');
				break;
			case ID_CALC_0:
				calc_append_number('0');
				break;
			case ID_CALC_DE:
				calc_append_number('.');
				break;
				
		}

		break;
	}

	case WM_CLOSE:
	{
		
			DestroyWindow(calcwindow);
			calcwindow = NULL;
	
		break;
	}

	

	default: 
		//SetFocus(get_control(CALC_window, "=")->handle);
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	
	}

	






