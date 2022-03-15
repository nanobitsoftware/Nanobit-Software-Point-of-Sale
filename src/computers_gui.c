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

extern NWC_PARENT* CS_window;

NWC_PARENT* CM_window;

#define ID_CM_TYPE 1000
#define ID_CM_MAKE 1001
#define ID_CM_MODEL 1002
#define ID_CM_SPEED 1003
#define ID_CM_RAM   1004
#define ID_CM_OS    1005
#define ID_CM_NOTES 1006
#define ID_CM_BTN_ADD  1007
#define ID_CM_BTN_CANCEL 1008

extern int cmp_id;

void db_add_computer(void)
{
	//(id INTEGER PRIMARY KEY, csid INT, Make CHAR, Model CHAR, Ram CHAR, Speed CHAR, Windows CHAR, Notes CHAR, TicketToal INT, Type CHAR);
	char* type;
	char* make;
	char* model;
	char* speed;
	char* ram;
	char* os;
	char* notes;
	char* csid;
	char str[1024];
	char* err;
	char* update;

	csid = CTRL_gettext(CS_window, "csid");

	if (!csid)
	{
		if (!do_add_customer())
		{
			GiveError("Please Ensure Customer Data is already filled properly.", 0);
			return;
		}
	}

	if (atoi(csid) == 0)
	{
		if (!do_add_customer())
		{
			GiveError("Please Ensure Customer Data is already filled properly. Computer add window will now hide in the background.", 0);
			return;
		}
	}
	csid = CTRL_gettext(CS_window, "csid");

	type = CTRL_gettext(CM_window, "type");
	make = CTRL_gettext(CM_window, "make");
	model = CTRL_gettext(CM_window, "model");
	speed = CTRL_gettext(CM_window, "speed");
	ram = CTRL_gettext(CM_window, "ram");
	os = CTRL_gettext(CM_window, "os");
	notes = CTRL_gettext(CM_window, "notes");

	update = CTRL_gettext(CM_window, "Add Computer");

	if (!strcmp(update, "Update Computer"))
	{
		sprintf(str, "UPDATE Computer SET csid='%d', Make='%s', Model='%s', Ram='%s', cSpeed='%s', Windows='%s', Notes='%s', Type='%s' "
			"WHERE id='%d';", atoi(csid), make, model, ram, speed, os, notes, type, cmp_id);
	}
	else
	{
		sprintf(str, "INSERT INTO Computer (csid, Make, Model, Ram, cSpeed, Windows, Notes, TicketTotal, Type) "
			"VALUES('%d','%s','%s','%s','%s','%s','%s','%d','%s');", atoi(csid), make, model, ram, speed, os, notes, 0, type);
	}

	sqlite3_exec(db, str, NULL, NULL, &err);

	if (err)
	{
		GiveError(err, 0);
	}

	sprintf(str, "SELECT Type, Make, Model, TicketTotal FROM Computer WHERE CSID=%d;", atoi(csid));
	sqlite3_exec(db, str, do_computer_list, NULL, &err);

	free(type);
	free(make);
	free(model);
	free(speed);
	free(ram);
	free(os);
	free(notes);

	DestroyWindow(cmpwindow);
	cmpwindow = NULL;
	SetFocus(cswindow);
}

void create_add_computer(void)
{
	if (cmpwindow)
		return;

	CM_window = create_parent("Add Computer");
	set_parent_config(CM_window, (HWND)0, (LRESULT*)CMAWindow_proc, 0, 0, 470, 355, 0, FALSE, 0, 0);

	AddStatic_Parent(CM_window, "Computer Type:", 0, 0, 160, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(CM_window, "Computer Make:", 0, 20, 160, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(CM_window, "Computer Model:", 0, 40, 160, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(CM_window, "Computer Speed:", 0, 60, 160, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(CM_window, "Total Ram ammount:", 0, 80, 160, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(CM_window, "Operating System:", 0, 100, 160, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(CM_window, "Notes:", 0, 120, 160, 20, 0, 0, 0, TRUE);

	AddEdit_Parent(CM_window, "type", 160, 0, 280, 20, 0, ID_CM_TYPE, 0, TRUE);
	AddEdit_Parent(CM_window, "make", 160, 20, 280, 20, 0, ID_CM_MAKE, 0, TRUE);
	AddEdit_Parent(CM_window, "model", 160, 40, 280, 20, 0, ID_CM_MODEL, 0, TRUE);
	AddEdit_Parent(CM_window, "speed", 160, 60, 280, 20, 0, ID_CM_SPEED, 0, TRUE);
	AddEdit_Parent(CM_window, "ram", 160, 80, 280, 20, 0, ID_CM_RAM, 0, TRUE);
	AddEdit_Parent(CM_window, "os", 160, 100, 280, 20, 0, ID_CM_OS, 0, TRUE);
	AddEdit_Parent(CM_window, "notes", 5, 140, 435, 120, 0, ID_CM_NOTES, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, TRUE);
	SendMessage(get_control(CM_window, "notes")->handle, EM_LIMITTEXT, MAXNOTE, 0);

	SendMessage(get_control(CM_window, "type")->handle, EM_LIMITTEXT, 40, 0);
	SendMessage(get_control(CM_window, "make")->handle, EM_LIMITTEXT, 40, 0);
	SendMessage(get_control(CM_window, "model")->handle, EM_LIMITTEXT, 40, 0);
	SendMessage(get_control(CM_window, "speed")->handle, EM_LIMITTEXT, 40, 0);
	SendMessage(get_control(CM_window, "ram")->handle, EM_LIMITTEXT, 40, 0);
	SendMessage(get_control(CM_window, "os")->handle, EM_LIMITTEXT, 40, 0);

	AddButton_Parent(CM_window, "Add Computer", 5, 270, 120, 20, 0, ID_CM_BTN_ADD, 0, TRUE);
	AddButton_Parent(CM_window, "Cancel Add", 320, 270, 120, 20, 0, ID_CM_BTN_CANCEL, 0, TRUE);

	CTRL_SetText(CM_window, "type", "");
	CTRL_SetText(CM_window, "make", "");
	CTRL_SetText(CM_window, "model", "");
	CTRL_SetText(CM_window, "speed", "");
	CTRL_SetText(CM_window, "ram", "");
	CTRL_SetText(CM_window, "os", "");
	CTRL_SetText(CM_window, "notes", "");

	cmpwindow = CM_window->window_control;

	show_parent(CM_window);
}

LRESULT APIENTRY CMAWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_SETFOCUS:
	{
		SetFocus(get_control(CM_window, "type")->handle);
		break;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case ID_CM_BTN_ADD:
		{
			clist_clear_list(CS_window, "computer");
			db_add_computer();

			break;
		}

		case ID_CM_BTN_CANCEL:
		{
			DestroyWindow(cmpwindow);
			break;
		}
		}
		break;
	}

	case WM_DESTROY:
	{
		DestroyWindow(cmpwindow);
		cmpwindow = NULL;
		break;
	}

	default:
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}