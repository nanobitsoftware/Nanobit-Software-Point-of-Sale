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
#define CALC_H 80
#define CALC_W  110
#define CALC_X 500  // X Size
#define CALC_Y 500 	//   Y Size
#define CALC_ONTOP TRUE
#define CALC_INPUT_WIDTH 500
#define CALC_INPUT_HEIGHT 40

NWC_PARENT* CALC_window;


struct calc_layout
{
	char text[128];
	int  x;
	int  y;
	int  width;
	int  heiht;
	int  id;

};


struct calc_layout layout[] =
{
	{"C",1,1,CALC_W, CALC_H,ID_CALC_C},       {"Tax",1,1,CALC_W, CALC_H,ID_CALC_T}, {"CE",1,1,CALC_W, CALC_H,ID_CALC_CE},{"/",1,1,CALC_W, CALC_H,ID_CALC_D},
	{"7",1,1,CALC_W, CALC_H,ID_CALC_7},	      {"8",1,1,CALC_W, CALC_H,ID_CALC_8},	{"9",1,1,CALC_W, CALC_H,ID_CALC_9},	 {"*",1,1,CALC_W, CALC_H,ID_CALC_M},
	{"4",1,1,CALC_W, CALC_H,ID_CALC_4},	      {"5",1,1,CALC_W, CALC_H,ID_CALC_5},	{"5",1,1,CALC_W, CALC_H,ID_CALC_5},	 {"-",1,1,CALC_W, CALC_H,ID_CALC_N},
	{"1",1,1,CALC_W, CALC_H,ID_CALC_1},		  {"2",1,1,CALC_W, CALC_H,ID_CALC_2},   {"3",1,1,CALC_W, CALC_H,ID_CALC_3},	 {"+",1,1,CALC_W, CALC_H,ID_CALC_A},
	{"Neg/Pos",1,1,CALC_W, CALC_H,ID_CALC_N}, {"0",1,1,CALC_W, CALC_H,ID_CALC_0},   {".",1,1,CALC_W, CALC_H,ID_CALC_DE}, {"=",1,1,CALC_W, CALC_H,ID_CALC_A},

	{NULL,0,0,0,0,-1}
};




void create_calc_window(void)
{
	HWND temp;

	temp = 0;

	if (calcwindow != NULL)
		return; // We already have the window. Do not recreate.


	CALC_window = create_parent("NanoCalculator");
	set_parent_config(CALC_window, (HWND)0, (LRESULT*)CALCwindow_proc, 0, 0, CALC_X, CALC_Y,
		0, CALC_ONTOP, 0, WS_TABSTOP | DS_3DLOOK |   WS_CLIPCHILDREN );

	calcwindow = CALC_window->window_control;

	ShowWindow(calcwindow, SW_SHOW);


}

LRESULT APIENTRY CALCwindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{

	default: {
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
