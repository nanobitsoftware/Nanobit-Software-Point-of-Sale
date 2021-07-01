#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <commctrl.h>
#include "Nano PoS.h"

#include "NWC.h"

#define INV_BTN_ADD_ADD    1000
#define INV_BTN_ADD_CANCEL 1001
#define INV_BTN_SEARCH_FIND 1002
#define INV_BTN_SEARCH_CANCEL 1003
#define INV_BTN_SEARCH_EDIT   1004
#define INV_BTN_SEARCH_DEL    1005
#define INV_LS_SEARCH         1006
#define INV_BTN_SEARCH_ADD    1007
#define INV_BTN_TIK_SEARCH   1008
#define INV_BTN_TIK_ADD        1009
#define INV_BTN_TIK_CANCEL    1010
#define LABOR_BTN_ADD      1011
#define LABOR_BTN_CANCEL   1012
#define INV_RETAIL         2000
#define INV_COST           2001
char last_search[1024];

NWC_PARENT* INV_add;
NWC_PARENT* INV_search;
NWC_PARENT* INV_ticket;
NWC_PARENT* Labor;

int cur_inv_id;

extern int idx_labor;
extern int idx_parts;
extern int cur_ref_id;
extern double MU1, MU2, MU3, MU4, MU5;
extern int ticket_mode;

void create_inv_add(BOOL updating)
{
	if (invwindowadd)
	{
		SetFocus(invwindowadd);
		return;
	}

	INV_add = create_parent("Add Inventory");
	set_parent_config(INV_add, (HWND)0, (LRESULT*)InvAddWindow_proc, CW_USEDEFAULT, CW_USEDEFAULT, 510, 170, 0, FALSE, 0, WS_TABSTOP | DS_3DLOOK | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_SIZEBOX);
	AddStatic_Parent(INV_add, "Inv. ID:", 0, 0, 90, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invid", 90, 0, 120, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(INV_add, "Item Make:", 215, 0, 90, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invmake", 305, 0, 180, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(INV_add, "Item Model:", 0, 20, 90, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invmodel", 90, 20, 395, 20, 0, 0, 0, TRUE);

	AddStatic_Parent(INV_add, "Supplier:", 0, 40, 90, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invsup", 90, 40, 395, 20, 0, 0, 0, TRUE);

	AddStatic_Parent(INV_add, "Count:", 0, 60, 60, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invcount", 90, 60, 100, 20, 0, 0, ES_NUMBER, TRUE);
	if (updating == TRUE)
	{
		AddStatic_Parent(INV_add, "invsale", 200, 60, 120, 20, 0, 0, 0, TRUE);
	}
	AddStatic_Parent(INV_add, "Cost:", 0, 80, 60, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invcost", 90, 80, 100, 20, 0, INV_COST, 0, TRUE);
	AddStatic_Parent(INV_add, "Retail:", 0, 100, 60, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_add, "invretail", 90, 100, 100, 20, 0, 0, 0, TRUE);

	AddButton_Parent(INV_add, "Add", 200, 80, 80, 20, 0, INV_BTN_ADD_ADD, 0, TRUE);
	AddButton_Parent(INV_add, "Cancel", 405, 80, 80, 20, 0, INV_BTN_ADD_CANCEL, 0, TRUE);

	invwindowadd = INV_add->window_control;
	ShowWindow(invwindowadd, SW_SHOW);
	SendMessage(get_control(INV_add, "invid")->handle, EM_LIMITTEXT, 50, 0);
	SendMessage(get_control(INV_add, "invmake")->handle, EM_LIMITTEXT, 50, 0);
	SendMessage(get_control(INV_add, "invmodel")->handle, EM_LIMITTEXT, 120, 0);
	SendMessage(get_control(INV_add, "invsup")->handle, EM_LIMITTEXT, 100, 0);
	SendMessage(get_control(INV_add, "invcount")->handle, EM_LIMITTEXT, 5, 0);
	SendMessage(get_control(INV_add, "invcost")->handle, EM_LIMITTEXT, 12, 0);
	SendMessage(get_control(INV_add, "invretail")->handle, EM_LIMITTEXT, 12, 0);
	if (updating)
	{
		CTRL_SetText(INV_add, "Add", "Update");
		SendMessage(invwindowadd, WM_SETTEXT, strlen("Update Inventory"), (LPARAM)(LPCSTR)"Update Inventory");
	}
}

void inv_fill_from_ticket(int idx)
{
	char SQL[1024];
	int i;
	char tempstr[1024];
	sqlite3_stmt* stmt;
	int sr;

	i = 0;

	if (!tickwindow)
	{
		GiveError("Cannot add part to ticket; ticket window does not exist.", 0);
		return;
	}

	DestroyWindow(invwindowsearch);

	if (!strcmp(last_search, "*"))
	{
		sprintf(SQL, "SELECT InvID, Make, Model, Cost, Retail, id FROM Inventory;");
	}
	else
	{
		sprintf(SQL, "SELECT InvID, Make, Model, Cost, Retail, id FROM Inventory WHERE Make LIKE '%%%s%%';", last_search);
	}

	sr = sqlite3_prepare(db, SQL, -1, &stmt, 0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (idx == i)
			{
				CTRL_SetText(INV_ticket, "invid", "%s", (char*)sqlite3_column_text(stmt, 0));
				//CTRL_SetText(INV_add, "invmake", "%s", (char*)sqlite3_column_text(stmt,1));
				//CTRL_SetText(INV_add, "invmodel", "%s", (char*)sqlite3_column_text(stmt,2));
				sprintf(tempstr, "%s, %s", (char*)sqlite3_column_text(stmt, 1), (char*)sqlite3_column_text(stmt, 2));
				CTRL_SetText(INV_ticket, "desc", "%s", tempstr);

				CTRL_SetText(INV_ticket, "cost", "%s", (char*)sqlite3_column_text(stmt, 3));
				CTRL_SetText(INV_ticket, "retail", "%s", (char*)sqlite3_column_text(stmt, 4));
				CTRL_SetText(INV_ticket, "quant", "0");
				sqlite3_finalize(stmt);
				return;

				//				if (!strcmp((char*)sqlite3_column_text(stmt,0),invid))
			}
			i++;
		}
	}
}

void inv_fill_from_find(int idx)
{
	char SQL[1024];
	int i;
	sqlite3_stmt* stmt;
	int sr;

	i = 0;

	if (!strcmp(last_search, "*"))
	{
		sprintf(SQL, "SELECT InvID, Make, Model, Supplier, Count, Cost, Retail, TotalSold, id FROM Inventory;");
	}
	else
	{
		sprintf(SQL, "SELECT InvID, Make, Model, Supplier, Count, Cost, Retail, TotalSold, id FROM Inventory WHERE Make LIKE '%%%s%%';", last_search);
	}
	sr = sqlite3_prepare(db, SQL, -1, &stmt, 0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (idx == i)
			{
				if (invwindowadd)
					DestroyWindow(invwindowadd);

				create_inv_add(TRUE);
				CTRL_SetText(INV_add, "invid", "%s", (char*)sqlite3_column_text(stmt, 0));
				CTRL_SetText(INV_add, "invmake", "%s", (char*)sqlite3_column_text(stmt, 1));
				CTRL_SetText(INV_add, "invmodel", "%s", (char*)sqlite3_column_text(stmt, 2));
				CTRL_SetText(INV_add, "invsup", "%s", (char*)sqlite3_column_text(stmt, 3));
				CTRL_SetText(INV_add, "invcount", "%s", (char*)sqlite3_column_text(stmt, 4));
				CTRL_SetText(INV_add, "invcost", "%s", (char*)sqlite3_column_text(stmt, 5));
				CTRL_SetText(INV_add, "invretail", "%s", (char*)sqlite3_column_text(stmt, 6));
				CTRL_SetText(INV_add, "invsale", "Total Sold: %d", (char*)sqlite3_column_text(stmt, 7));
				cur_inv_id = atoi(sqlite3_column_text(stmt, 8));
				sqlite3_finalize(stmt);
				return;

				//				if (!strcmp((char*)sqlite3_column_text(stmt,0),invid))
			}
			i++;
		}
	}
}

void delete_inventory(int idx)
{
}

int populate_invfind(void* NotUsed, int argc, char** argv, char** azColName)
{
	clist_add_data(INV_search, "invlist", argv, argc);
	return 0;
}
void do_invfind(void)
{
	char* find;
	char SQL[1024];
	char* err;
	if (!invwindowsearch)
	{
		create_inv_search();
	}

	find = CTRL_gettext(INV_search, "invsearch");

	if (!find)
	{
		GiveError("Search field must have a value.\n\nSearches are based on the MAKE of the inventory; to show all, search for '*'.", 0);
		return;
	}
	sprintf(last_search, "%s", find);
	if (!strcmp(find, "*"))
	{
		sprintf(SQL, "SELECT InvID, Make, Model, Cost, Count FROM Inventory;");
	}
	else
	{
		sprintf(SQL, "SELECT InvID, Make, Model, Cost, Count FROM Inventory WHERE Make LIKE '%%%s%%';", find);
		//	GiveError(SQL,0);
	}
	clist_clear_list(INV_search, "invlist");
	sqlite3_exec(db, SQL, populate_invfind, NULL, &err);

	if (err)
	{
		GiveError(err, 1);
	}
}

void create_inv_search(void)
{
	if (invwindowsearch)
	{
		SetFocus(invwindowsearch);
		return;
	}

	last_search[0] = '\0';
	INV_search = create_parent("Search Inventory");
	set_parent_config(INV_search, (HWND)0, (LRESULT*)InvSearchWindow_proc, CW_USEDEFAULT, CW_USEDEFAULT, 510, 420, 0, FALSE, 0, WS_TABSTOP | DS_3DLOOK | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_SIZEBOX);
	AddStatic_Parent(INV_search, "Searches by MAKE. Search for '*' for a full inventory list:", 0, 0, 485, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_search, "invsearch", 0, 20, 485, 20, 0, 0, 0, TRUE);
	AddCList_Parent(INV_search, "invlist", 0, 40, 485, 300, 0, INV_LS_SEARCH, LBS_HASSTRINGS | LVS_REPORT | LBS_NOTIFY | LVS_SINGLESEL, TRUE);
	clist_add_col(INV_search, "invlist", 80, "Inv ID");
	clist_add_col(INV_search, "invlist", 100, "Make");
	clist_add_col(INV_search, "invlist", 180, "Model");
	clist_add_col(INV_search, "invlist", 60, "Cost");
	clist_add_col(INV_search, "invlist", 60, "On hand");
	AddButton_Parent(INV_search, "Find", 0, 345, 90, 20, 0, INV_BTN_SEARCH_FIND, 0, TRUE);
	AddButton_Parent(INV_search, "Add New", 100, 345, 90, 20, 0, INV_BTN_SEARCH_ADD, WS_DISABLED, TRUE);
	AddButton_Parent(INV_search, "Edit", 200, 345, 90, 20, 0, INV_BTN_SEARCH_EDIT, WS_DISABLED, TRUE);
	AddButton_Parent(INV_search, "Delete", 300, 345, 90, 20, 0, INV_BTN_SEARCH_DEL, WS_DISABLED, TRUE);
	AddButton_Parent(INV_search, "Cancel", 400, 345, 85, 20, 0, INV_BTN_SEARCH_CANCEL, 0, TRUE);

	invwindowsearch = INV_search->window_control;
	ShowWindow(invwindowsearch, SW_SHOW);
	SendMessage(get_control(INV_search, "invsearch")->handle, EM_LIMITTEXT, 100, 0);

	if (!tickwindow)
	{
		EnableWindow(get_control(INV_search, "Edit")->handle, TRUE);
		EnableWindow(get_control(INV_search, "Delete")->handle, TRUE);
		EnableWindow(get_control(INV_search, "Add New")->handle, TRUE);
	}
}

void create_inv_ticket(void)
{
	if (invwindowticket)
	{
		SetFocus(invwindowticket);
		return;
	}

	last_search[0] = '\0';

	INV_ticket = create_parent("Add Inventory to Ticket");
	set_parent_config(INV_ticket, (HWND)0, (LRESULT*)InvTicketWindow_proc, CW_USEDEFAULT, CW_USEDEFAULT, 435, 230, 0, FALSE, 0, 0);
	AddStatic_Parent(INV_ticket, "Inv ID:", 0, 0, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_ticket, "invid", 80, 0, 100, 20, 0, 0, WS_DISABLED, TRUE);
	AddButton_Parent(INV_ticket, "Search Inventory", 190, 0, 140, 20, 0, INV_BTN_TIK_SEARCH, 0, TRUE);
	AddStatic_Parent(INV_ticket, "Description: ", 0, 20, 120, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_ticket, "desc", 0, 40, 410, 60, 0, 0, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, TRUE);
	AddStatic_Parent(INV_ticket, "Quantity: ", 0, 100, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_ticket, "quant", 80, 100, 60, 20, 0, 0, ES_NUMBER, TRUE);
	AddStatic_Parent(INV_ticket, "Cost: ", 0, 120, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_ticket, "cost", 80, 120, 80, 20, 0, INV_COST, 0, TRUE);
	AddStatic_Parent(INV_ticket, "Retail: ", 0, 140, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_ticket, "retail", 80, 140, 80, 20, 0, INV_RETAIL, 0, TRUE);
	AddStatic_Parent(INV_ticket, "Total: ", 0, 160, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(INV_ticket, "total", 80, 160, 80, 20, 0, 0, WS_DISABLED, TRUE);
	AddButton_Parent(INV_ticket, "Add", 330, 160, 80, 20, 0, INV_BTN_TIK_ADD, 0, TRUE);
	AddButton_Parent(INV_ticket, "Cancel", 330, 140, 80, 20, 0, INV_BTN_TIK_CANCEL, 0, TRUE);

	invwindowticket = INV_ticket->window_control;
	ShowWindow(invwindowticket, SW_SHOW);
}

void create_labor_ticket(void)
{
	if (laborwindow)
	{
		SetFocus(laborwindow);
		return;
	}

	Labor = create_parent("Add Labor to Ticket");
	set_parent_config(Labor, (HWND)0, (LRESULT*)LaborWindow_proc, CW_USEDEFAULT, CW_USEDEFAULT, 425, 230, 0, FALSE, 0, 0);
	AddStatic_Parent(Labor, "Labor Description:", 0, 0, 220, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Labor, "desc", 0, 20, 400, 100, 0, 0, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, TRUE);
	AddStatic_Parent(Labor, "Hours:", 0, 120, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Labor, "hours", 80, 120, 80, 20, 0, 0, ES_NUMBER, TRUE);
	AddStatic_Parent(Labor, "Cost:", 0, 140, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Labor, "cost", 80, 140, 80, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Labor, "Total Cost:", 0, 160, 80, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Labor, "total", 80, 160, 80, 20, 0, 0, ES_NUMBER | WS_DISABLED, TRUE);
	AddButton_Parent(Labor, "Add", 320, 160, 80, 20, 0, LABOR_BTN_ADD, 0, TRUE);
	AddButton_Parent(Labor, "Cancel", 320, 140, 80, 20, 0, LABOR_BTN_CANCEL, 0, TRUE);
	laborwindow = Labor->window_control;
	ShowWindow(laborwindow, SW_SHOW);
}

LRESULT APIENTRY LaborWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char* desc;
	char* hours;
	char* cost;
	char sql[1024];
	char* err;

	switch (msg)
	{
	case WM_DESTROY:
	{
		DestroyParent(Labor);
		laborwindow = NULL;
		return 0;
	}
	case WM_SETFOCUS:
	{
		SetFocus(get_control(Labor, "Hours")->handle);
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case LABOR_BTN_CANCEL:
		{
			DestroyWindow(laborwindow);
			break;
		}
		case LABOR_BTN_ADD:
		{
			if (ticket_mode == MODE_READONLY)
			{
				GiveError("Ticket is in read-only mode and cannot be modified.", 0);
				break;
			}
			if (!tickwindow)
			{
				GiveError("Ticket window does not exist; cannot add labor at this time.", 0);
				break;
			}
			desc = CTRL_gettext(Labor, "desc");
			hours = CTRL_gettext(Labor, "hours");
			cost = CTRL_gettext(Labor, "cost");

			if (!desc)
			{
				GiveError("A proper labor description must be supplied.", 0);
				free(cost);
				free(desc);
				free(hours);
				break;
			}

			if (!hours || !cost)
			{
				GiveError("Both Hours and Cost must be filled out before continuing.", 0);
				free(cost);
				free(desc);
				free(hours);
				break;
			}

			if (!is_number(cost) || !is_number(hours))
			{
				GiveError("Proper numbers may only be supplied for the hours and cost location.", 0);
				free(cost);
				free(desc);
				free(hours);
				break;
			}

			sprintf(cost, "%.2f", fround(atof(cost)));

			err = CTRL_gettext(Labor, "Add");

			if (!strcmp(err, "Update"))
			{
				free(err);
				sprintf(sql, "UPDATE InvoiceRef SET LaborDesc='%s', LaborHours='%d', LaborCost='%.2f' WHERE refkey='%d';", desc, atoi(hours), atof(cost), idx_labor);
				sqlite3_exec(db, sql, NULL, NULL, &err);
				if (err)
					GiveError(err, 0);
				update_ticket_labor();
				update_ticket_totals();

				free(cost);
				free(err);
				free(desc);
				free(hours);
				DestroyWindow(laborwindow);
				break;
			}

			add_labor_inventory(desc, hours, cost);
			free(cost);
			free(desc);
			free(hours);
			DestroyWindow(laborwindow);
			break;
		}
		}
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

LRESULT APIENTRY InvTicketWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char* invid;
	char* desc;
	char* quant;
	char* cost;
	char* retail;
	char* err;
	char* invretail;
	char* invcount;
	char* invcost;
	char sql[1024];

	switch (msg)
	{
	case WM_DESTROY:
	{
		DestroyParent(INV_ticket);
		invwindowticket = NULL;
		return 0;
	}
	case WM_SETFOCUS:
	{
		SetFocus(get_control(INV_ticket, "quant")->handle);
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case INV_BTN_TIK_CANCEL:
		{
			DestroyWindow(invwindowticket);
			invwindowticket = NULL;
			break;
		}
		case INV_BTN_TIK_SEARCH:
		{
			if (invwindowsearch)
				SetFocus(invwindowsearch);
			else
			{
				create_inv_search();
			}
			break;
		}

		case INV_BTN_TIK_ADD:
		{
			if (ticket_mode == MODE_READONLY)
			{
				GiveError("Ticket is in read-only mode and cannot be modified.", 0);
				break;
			}
			invid = CTRL_gettext(INV_ticket, "invid");
			desc = CTRL_gettext(INV_ticket, "desc");
			quant = CTRL_gettext(INV_ticket, "quant");
			cost = CTRL_gettext(INV_ticket, "cost");
			retail = CTRL_gettext(INV_ticket, "retail");

			if (!desc || !quant || !cost || !retail)
			{
				GiveError("All fields must be filled out before an inventory item can be added to the ticket.", 0);
				free(invid);
				free(desc);
				free(quant);
				free(cost);
				free(retail);
				return 0;
			}

			if (!is_number(quant) || !is_number(cost) || !is_number(retail))
			{
				GiveError("Quantity, Cost and retail must be valid numbers.", 0);
				free(invid);
				free(desc);
				free(quant);
				free(cost);
				free(retail);
				return 0;
			}

			sprintf(retail, "%.2f", fround(atof(retail)));
			sprintf(cost, "%.2f", fround(atof(cost)));

			if (!tickwindow)
			{
				GiveError("Cannot add to a non-existant ticket.", 0);

				free(invid);
				free(desc);
				free(quant);
				free(cost);
				free(retail);
				DestroyWindow(invwindowticket);
				break;
			}
			err = CTRL_gettext(INV_ticket, "Add");

			if (!strcmp(err, "Update"))
			{
				free(err);

				sprintf(sql, "UPDATE InvoiceRef SET PartDesc='%s', PartRealCost='%.2f', PartCost='%.2f', Quantity='%d' WHERE refkey='%d';",
					desc, atof(cost), atof(retail), atoi(quant), idx_parts);
				sqlite3_exec(db, sql, NULL, NULL, &err);
				if (err)
					GiveError(err, 0);
				update_ticket_parts();
				update_ticket_totals();

				free(invid);
				free(desc);
				free(quant);
				free(cost);
				free(retail);
				free(err);
				DestroyWindow(invwindowticket);
				break;
			}

			add_inv_ticket(invid, desc, quant, cost, retail);

			free(invid);
			free(desc);
			free(quant);
			free(cost);
			free(retail);
			free(err);
			DestroyWindow(invwindowticket);

			break;
		}
		}
	case EN_KILLFOCUS:
	{
		if ((int)LOWORD(wparam) == INV_RETAIL)
		{
			invretail = CTRL_gettext(INV_ticket, "retail");
			invcount = CTRL_gettext(INV_ticket, "quant");

			if (!invretail || !invcount)
				break;

			if (!is_number(invretail) || !is_number(invcount))
			{
				free(invretail);
				free(invcount);
				break;
			}

			CTRL_SetText(INV_ticket, "total", "$%.2f", fround(atof(invretail)) * atoi(invcount));
			free(invretail);
			free(invcount);
		}

		if ((int)LOWORD(wparam) == INV_COST)
		{
			invcost = CTRL_gettext(INV_ticket, "cost");

			if (!invcost)
			{
				free(invcost);
				break;
			}
			if (!is_number(invcost))
			{
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 0.01f && fround(atof(invcost)) <= 1.99f)
			{
				CTRL_SetText(INV_ticket, "retail", "%.2f", fround(atof(invcost)) * MU1 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 2.00f && fround(atof(invcost)) <= 9.99f)
			{
				CTRL_SetText(INV_ticket, "retail", "%.2f", fround(atof(invcost)) * MU2 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 10.00f && fround(atof(invcost)) <= 99.99)
			{
				CTRL_SetText(INV_ticket, "retail", "%.2f", fround(atof(invcost)) * MU3 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 100.00f && fround(atof(invcost)) <= 299.99f)
			{
				CTRL_SetText(INV_ticket, "retail", "%.2f", fround(atof(invcost)) * MU4 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 300.00f)
			{
				CTRL_SetText(INV_ticket, "retail", "%.2f", fround(atof(invcost)) * MU5 + fround(atof(invcost)));
				free(invcost);
				break;
			}
		}

		break;
	}
	}

	default:
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT APIENTRY InvAddWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char* invmake;
	char* invid;
	char* invmodel;
	char* invsup;
	char* invcount;
	char* invcost;
	char* invretail;
	char* invbut;
	char SQL[1024];
	char* err;
	int sr;
	sqlite3_stmt* stmt = 0;

	switch (msg)
	{
	case WM_SETFOCUS:
	{
		SetFocus(get_control(INV_add, "invid")->handle);
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case INV_BTN_ADD_CANCEL:
		{
			DestroyWindow(invwindowadd);
			invwindowadd = NULL;
			break;
		}
		case INV_BTN_ADD_ADD:
		{
			invmake = CTRL_gettext(INV_add, "invmake");
			invid = CTRL_gettext(INV_add, "invid");
			invmodel = CTRL_gettext(INV_add, "invmodel");
			invsup = CTRL_gettext(INV_add, "invsup");
			invcount = CTRL_gettext(INV_add, "invcount");
			invcost = CTRL_gettext(INV_add, "invcost");
			invretail = CTRL_gettext(INV_add, "invretail");
			invbut = CTRL_gettext(INV_add, "Add");

			if (!invmake || !invid || !invmodel || !invsup || !invcount)
			{
				GiveError("You must fill out every input before adding an inventory item.", 0);
				break;
			}

			if (!is_number(invcost) || !is_number(invretail))
			{
				GiveError("Cost and retail must be in numerical format (e.g. 200.01)", 0);
				break;
			}

			sprintf(invretail, "%.2f", fround(atof(invretail)));
			sprintf(invcost, "%.2f", fround(atof(invcost)));

			sprintf(SQL, "SELECT InvID FROM Inventory;");

			if (strcmp(invbut, "Update"))
			{
				sr = sqlite3_prepare(db, SQL, -1, &stmt, 0);
				if (sr == SQLITE_OK)
				{
					while (sqlite3_step(stmt) == SQLITE_ROW)
					{
						if (!strcmp((char*)sqlite3_column_text(stmt, 0), invid))
						{
							GiveError("Inventory ID already exists.", 0);
							sqlite3_finalize(stmt);
							free(invmake);
							free(invid);
							free(invmodel);
							free(invsup);
							free(invcount);
							free(invcost);
							free(invretail);
							return 0;
						}
					}
				}
				sqlite3_finalize(stmt);
			}

			if (strcmp(invbut, "Update"))
			{
				sprintf(SQL, "INSERT INTO Inventory (InvID, Make, Model, Supplier, Count, Cost, Retail) VALUES("
					"'%s', '%s', '%s', '%s', '%s', '%.2f', '%.2f');", invid, invmake, invmodel, invsup, invcount, atof(invcost), atof(invretail));
			}
			else
			{
				sprintf(SQL, "UPDATE Inventory SET InvID='%s', Make='%s', Model='%s', Supplier='%s', Count='%s', Cost='%.2f', Retail='%.2f' WHERE id=%d;",
					invid, invmake, invmodel, invsup, invcount, atof(invcost), atof(invretail), cur_inv_id);
				cur_inv_id = -1;
			}

			sqlite3_exec(db, SQL, NULL, NULL, &err);
			if (err)
			{
				GiveError(err, 1);
			}

			sprintf(SQL, "%s added to database successfully.", invid);
			GiveError(SQL, 0);
			free(invmake);
			free(invid);
			free(invmodel);
			free(invsup);
			free(invcount);
			free(invcost);
			free(invretail);

			DestroyWindow(invwindowadd);
			invwindowadd = NULL;
			return 0;

			break;
		}
		}
	case EN_KILLFOCUS:
	{
		if ((int)LOWORD(wparam) == INV_COST)
		{
			invcost = CTRL_gettext(INV_add, "cost");

			if (!invcost)
			{
				free(invcost);
				break;
			}
			if (!is_number(invcost))
			{
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 0.01f && fround(atof(invcost)) <= 1.99f)
			{
				CTRL_SetText(INV_add, "retail", "%.2f", fround(atof(invcost)) * MU1 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 2.00f && fround(atof(invcost)) <= 9.99f)
			{
				CTRL_SetText(INV_add, "retail", "%.2f", fround(atof(invcost)) * MU2 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 10.00f && fround(atof(invcost)) <= 99.99)
			{
				CTRL_SetText(INV_add, "retail", "%.2f", fround(atof(invcost)) * MU3 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 100.00f && fround(atof(invcost)) <= 299.99f)
			{
				CTRL_SetText(INV_add, "retail", "%.2f", fround(atof(invcost)) * MU4 + fround(atof(invcost)));
				free(invcost);
				break;
			}

			if (fround(atof(invcost)) >= 300.00f)
			{
				CTRL_SetText(INV_add, "retail", "%.2f", fround(atof(invcost)) * MU5 + fround(atof(invcost)));
				free(invcost);
				break;
			}
		}

		break;
	}
	break;
	}

	case WM_DESTROY:
		DestroyParent(INV_add);
		invwindowadd = NULL;
		return 0;

	default:
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT APIENTRY InvSearchWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//LOG("Hwnd: %d, Msg: %d, wparam: %d, lparam: %d", hwnd,msg, wparam, lparam);
	NMHDR* nm;
	int idx;

	switch (msg)
	{
	case WM_DESTROY:
	{
		DestroyParent(INV_search);
		invwindowsearch = NULL;
		return 0;
	}
	case WM_SETFOCUS:
	{
		SetFocus(get_control(INV_search, "invsearch")->handle);
		break;
	}
	case WM_NOTIFY:
	{
		nm = (NMHDR*)lparam;
		if (nm->code == NM_DBLCLK)
		{
			idx = SendMessage(get_control(INV_search, "invlist")->handle, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);

			if (idx == -1)
				return 0;

			if (!tickwindow)
				inv_fill_from_find(idx);
			else
				inv_fill_from_ticket(idx);

			return 0;
		}
		break;
	}

	case WM_COMMAND:
	{
		if (wparam == IDOK)
		{
			do_invfind();

			break;
		}
		switch (LOWORD(wparam))
		{
		case INV_BTN_SEARCH_FIND:
		{
			do_invfind();
			break;
		}
		case INV_BTN_SEARCH_CANCEL:
		{
			DestroyWindow(invwindowsearch);
			invwindowsearch = NULL;
			break;
		}
		case INV_BTN_SEARCH_EDIT:
		{
			idx = SendMessage(get_control(INV_search, "invlist")->handle, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);

			if (idx == -1)
			{
				GiveError("Please select an inventory item to edit.", 0);
				return 0;
			}

			inv_fill_from_find(idx);
			break;
		}
		case INV_BTN_SEARCH_ADD:
		{
			if (invwindowadd)
				DestroyWindow(invwindowadd);
			create_inv_add(FALSE);
			break;
		}
		}
	}

	default:
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}