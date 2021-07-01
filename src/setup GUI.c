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

NWC_PARENT* Setup;

#define SETUP_BTN_OK 1000
#define SETUP_BTN_CANCEL 1001

// The following check boxes need UNIQUE ids
// Since we're using them as blank names. And want to
// Look them up using get_control_by_id. Don't want clashes.
#define SETUP_CHECK_LABOR 29102
#define SETUP_CHECK_PARTS 12210
extern double MU1, MU2, MU3, MU4, MU5;

void create_setup_window(void)
{
	char sql[2048];

	sqlite3_stmt* stmt;
	int sr;

	if (setupwindow)
	{
		SetFocus(setupwindow);
		return;
	}

	Setup = create_parent("Nanobit Point of Sale Setup");
	set_parent_config(Setup, (HWND)0, (LRESULT*)SetupWindow_proc, 0, 0, 460, 630, 0, FALSE, 0, 0);
	AddStatic_Parent(Setup, "Company Name:", 0, 0, 130, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "name", 130, 0, 300, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Company Motto:", 0, 20, 130, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "motto", 130, 20, 300, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Sreet Address:", 0, 40, 130, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "street", 130, 40, 300, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "City:", 0, 60, 50, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "city", 50, 60, 140, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "State:", 200, 60, 60, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "state", 260, 60, 40, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Zip:", 305, 60, 40, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "zip", 350, 60, 60, 20, 0, 0, ES_NUMBER, TRUE);

	AddStatic_Parent(Setup, "Phone Number:", 0, 100, 120, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "phone", 120, 100, 100, 20, 0, 0, 0, TRUE);

	AddStatic_Parent(Setup, "Sales Tax %:", 300, 100, 100, 20, 0, 0, 0, TRUE);
	AddCheck_Parent(Setup, "Tax Labor  :", 300, 120, 112, 20, 0, SETUP_CHECK_LABOR, BS_LEFTTEXT, TRUE);
	AddCheck_Parent(Setup, "Tax Parts  :", 300, 140, 112, 20, 0, SETUP_CHECK_PARTS, BS_LEFTTEXT, TRUE);

	AddEdit_Parent(Setup, "tax", 400, 100, 30, 20, 0, 0, ES_NUMBER, TRUE);
	//AddCheck_Parent(Setup, " ", 400, 120, 50, 20, 0, SETUP_CHECK_LABOR,0, TRUE);
	//AddCheck_Parent(Setup, " ", 400, 140, 50, 20, 0, SETUP_CHECK_PARTS, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 140, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 160, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 180, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 200, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 220, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 240, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 260, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 280, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 300, 100, 20, 0, 0, 0, TRUE);
	AddStatic_Parent(Setup, "Payment Type:", 0, 320, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay1", 100, 140, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay2", 100, 160, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay3", 100, 180, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay4", 100, 200, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay5", 100, 220, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay6", 100, 240, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay7", 100, 260, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay8", 100, 280, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay9", 100, 300, 100, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "pay10", 100, 320, 100, 20, 0, 0, 0, TRUE);

	AddStatic_Parent(Setup, "Markup $.01 - $1.99: %", 200, 240, 180, 20, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(Setup, "Markup $2 - $9.99: %", 200, 260, 180, 20, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(Setup, "Markup $10 - $99.99: %", 200, 280, 180, 20, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(Setup, "Markup $100 - $299.99: %", 200, 300, 180, 20, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(Setup, "Markup $300+ : %", 200, 320, 180, 20, 0, 0, ES_RIGHT, TRUE);
	AddEdit_Parent(Setup, "mu1", 380, 240, 50, 20, 0, 0, ES_NUMBER, TRUE);
	AddEdit_Parent(Setup, "mu2", 380, 260, 50, 20, 0, 0, ES_NUMBER, TRUE);
	AddEdit_Parent(Setup, "mu3", 380, 280, 50, 20, 0, 0, ES_NUMBER, TRUE);
	AddEdit_Parent(Setup, "mu4", 380, 300, 50, 20, 0, 0, ES_NUMBER, TRUE);
	AddEdit_Parent(Setup, "mu5", 380, 320, 50, 20, 0, 0, ES_NUMBER, TRUE);

	AddStatic_Parent(Setup, "Warranty Information", 0, 360, 200, 20, 0, 0, 0, TRUE);
	AddEdit_Parent(Setup, "warranty", 0, 380, 430, 150, 0, 0, ES_MULTILINE | WS_THICKFRAME | ES_AUTOVSCROLL | WS_VSCROLL, TRUE);

	AddButton_Parent(Setup, "Save", 0, 550, 80, 20, 0, SETUP_BTN_OK, 0, TRUE);
	AddButton_Parent(Setup, "Cancel", 350, 550, 80, 20, 0, SETUP_BTN_CANCEL, 0, TRUE);

	SendMessage(get_control(Setup, "state")->handle, EM_LIMITTEXT, 2, 0);
	SendMessage(get_control(Setup, "zip")->handle, EM_LIMITTEXT, 5, 0);
	SendMessage(get_control(Setup, "mu1")->handle, EM_LIMITTEXT, 3, 0);
	SendMessage(get_control(Setup, "mu2")->handle, EM_LIMITTEXT, 2, 0);
	SendMessage(get_control(Setup, "mu3")->handle, EM_LIMITTEXT, 2, 0);
	SendMessage(get_control(Setup, "mu4")->handle, EM_LIMITTEXT, 2, 0);
	SendMessage(get_control(Setup, "mu5")->handle, EM_LIMITTEXT, 2, 0);
	SendMessage(get_control(Setup, "warranty")->handle, EM_LIMITTEXT, MAXNOTE, 0);
	SendMessage(get_control(Setup, "phone")->handle, EM_LIMITTEXT, 12, 0);
	SendMessage(get_control(Setup, "tax")->handle, EM_LIMITTEXT, 2, 0);

	setupwindow = Setup->window_control;

	ShowWindow(setupwindow, SW_SHOW);

	sprintf(sql, "SELECT CompanyName, CompanyMotto, CompanyAddress, CompanyZip, CompanyState, WarrantyInfo, CompanyPhone, MU1, MU2, MU3, MU4, MU5,"
		"Tax, Pay1, Pay2, Pay3, Pay4, Pay5, Pay6, Pay7, Pay8, Pay9, Pay10, CompanyCity, Tax_Labor, Tax_Parts FROM POS Where id='1';");

	sr = sqlite3_prepare(db, sql, -1, &stmt, 0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt, 0))
				CTRL_SetText(Setup, "name", "%s", (char*)sqlite3_column_text(stmt, 0));
			if ((char*)sqlite3_column_text(stmt, 1))
				CTRL_SetText(Setup, "motto", "%s", (char*)sqlite3_column_text(stmt, 1));
			if ((char*)sqlite3_column_text(stmt, 2))
				CTRL_SetText(Setup, "street", "%s", (char*)sqlite3_column_text(stmt, 2));
			if ((char*)sqlite3_column_text(stmt, 3))
				CTRL_SetText(Setup, "zip", "%s", (char*)sqlite3_column_text(stmt, 3));
			if ((char*)sqlite3_column_text(stmt, 4))
				CTRL_SetText(Setup, "state", "%s", (char*)sqlite3_column_text(stmt, 4));
			if ((char*)sqlite3_column_text(stmt, 5))
				CTRL_SetText(Setup, "warranty", "%s", (char*)sqlite3_column_text(stmt, 5));
			if ((char*)sqlite3_column_text(stmt, 6))
				CTRL_SetText(Setup, "phone", "%s", (char*)sqlite3_column_text(stmt, 6));
			if ((char*)sqlite3_column_text(stmt, 7))
				CTRL_SetText(Setup, "mu1", "%s", (char*)sqlite3_column_text(stmt, 7));
			if ((char*)sqlite3_column_text(stmt, 8))
				CTRL_SetText(Setup, "mu2", "%s", (char*)sqlite3_column_text(stmt, 8));
			if ((char*)sqlite3_column_text(stmt, 9))
				CTRL_SetText(Setup, "mu3", "%s", (char*)sqlite3_column_text(stmt, 9));
			if ((char*)sqlite3_column_text(stmt, 10))
				CTRL_SetText(Setup, "mu4", "%s", (char*)sqlite3_column_text(stmt, 10));
			if ((char*)sqlite3_column_text(stmt, 11))
				CTRL_SetText(Setup, "mu5", "%s", (char*)sqlite3_column_text(stmt, 11));
			if ((char*)sqlite3_column_text(stmt, 12))
				CTRL_SetText(Setup, "tax", "%s", (char*)sqlite3_column_text(stmt, 12));
			if ((char*)sqlite3_column_text(stmt, 13))
				CTRL_SetText(Setup, "pay1", "%s", (char*)sqlite3_column_text(stmt, 13));
			if ((char*)sqlite3_column_text(stmt, 14))
				CTRL_SetText(Setup, "pay2", "%s", (char*)sqlite3_column_text(stmt, 14));
			if ((char*)sqlite3_column_text(stmt, 15))
				CTRL_SetText(Setup, "pay3", "%s", (char*)sqlite3_column_text(stmt, 15));
			if ((char*)sqlite3_column_text(stmt, 16))
				CTRL_SetText(Setup, "pay4", "%s", (char*)sqlite3_column_text(stmt, 16));
			if ((char*)sqlite3_column_text(stmt, 17))
				CTRL_SetText(Setup, "pay5", "%s", (char*)sqlite3_column_text(stmt, 17));
			if ((char*)sqlite3_column_text(stmt, 18))
				CTRL_SetText(Setup, "pay6", "%s", (char*)sqlite3_column_text(stmt, 18));
			if ((char*)sqlite3_column_text(stmt, 19))
				CTRL_SetText(Setup, "pay7", "%s", (char*)sqlite3_column_text(stmt, 19));
			if ((char*)sqlite3_column_text(stmt, 20))
				CTRL_SetText(Setup, "pay8", "%s", (char*)sqlite3_column_text(stmt, 20));
			if ((char*)sqlite3_column_text(stmt, 21))
				CTRL_SetText(Setup, "pay9", "%s", (char*)sqlite3_column_text(stmt, 21));
			if ((char*)sqlite3_column_text(stmt, 22))
				CTRL_SetText(Setup, "pay10", "%s", (char*)sqlite3_column_text(stmt, 22));
			if ((char*)sqlite3_column_text(stmt, 23))
				CTRL_SetText(Setup, "city", "%s", (char*)sqlite3_column_text(stmt, 23));
			if (sqlite3_column_int(stmt, 24))
			{
				if (sqlite3_column_int(stmt, 24) == 0)
					tax_labor = FALSE;
				else
					tax_labor = TRUE;
			}
			if ((int)sqlite3_column_int(stmt, 25))
			{
				if (sqlite3_column_int(stmt, 25) == 0)
					tax_parts = FALSE;
				else
					tax_parts = TRUE;
			}
		}
	}
	sqlite3_finalize(stmt);
	if (tax_labor)
		check_set(Setup, "Tax Labor  :");
	if (tax_parts)
		check_set(Setup, "Tax Parts  :");
}

LRESULT APIENTRY SetupWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char* name, * motto, * street, * city, * state, * zip, * pay1, * pay2, * pay3;
	char* pay4, * pay5, * pay6, * pay7, * pay8, * pay9, * pay10, * mu1, * mu2, * mu3, * mu4;
	char* mu5, * warranty, * phone, * tax;

	char sql[25000];
	char* err;

	switch (msg)
	{
	case WM_DESTROY:
	{
		DestroyParent(Setup);
		setupwindow = NULL;
		return 0;
	}
	case WM_SETFOCUS:
	{
		if (get_control(Setup, "name"))
			SetFocus(get_control(Setup, "name")->handle);
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case SETUP_BTN_CANCEL:
		{
			DestroyWindow(setupwindow);
			break;
		}
		case SETUP_BTN_OK:
		{
			name = CTRL_gettext(Setup, "name");
			motto = CTRL_gettext(Setup, "motto");
			street = CTRL_gettext(Setup, "street");
			city = CTRL_gettext(Setup, "city");
			state = CTRL_gettext(Setup, "state");
			zip = CTRL_gettext(Setup, "zip");
			pay1 = CTRL_gettext(Setup, "pay1");
			pay2 = CTRL_gettext(Setup, "pay2");
			pay3 = CTRL_gettext(Setup, "pay3");
			pay4 = CTRL_gettext(Setup, "pay4");
			pay5 = CTRL_gettext(Setup, "pay5");
			pay6 = CTRL_gettext(Setup, "pay6");
			pay7 = CTRL_gettext(Setup, "pay7");
			pay8 = CTRL_gettext(Setup, "pay8");
			pay9 = CTRL_gettext(Setup, "pay9");
			pay10 = CTRL_gettext(Setup, "pay10");
			mu1 = CTRL_gettext(Setup, "mu1");
			mu2 = CTRL_gettext(Setup, "mu2");
			mu3 = CTRL_gettext(Setup, "mu3");
			mu4 = CTRL_gettext(Setup, "mu4");
			mu5 = CTRL_gettext(Setup, "mu5");
			warranty = CTRL_gettext(Setup, "warranty");
			phone = CTRL_gettext(Setup, "phone");
			tax = CTRL_gettext(Setup, "tax");
			if (check_get_status(Setup, "Tax Labor  :"))
			{
				tax_labor = TRUE;
			}
			else
			{
				tax_labor = FALSE;
			}
			if (check_get_status(Setup, "Tax Parts  :"))
				tax_parts = TRUE;
			else
				tax_parts = FALSE;

			sqlite3_snprintf(24999, sql, "UPDATE POS SET Pay1='%s', Pay2='%s', Pay3='%s', Pay4='%s', Pay5='%s', Pay6='%s', Pay7='%s', Pay8='%s', Pay9='%s',"
				"Pay10='%s', CompanyName='%s', CompanyMotto='%q', CompanyAddress='%s', CompanyZip='%s', CompanyState='%s', WarrantyInfo='%s', CompanyPhone='%s',"
				"MU1='%s', MU2='%s', MU3='%s', MU4='%s', MU5='%s', Tax='%s', CompanyCity='%s', Tax_Labor=%d, Tax_Parts=%d WHERE id='1';",
				pay1, pay2, pay3, pay4, pay5, pay6, pay7, pay8, pay9, pay10, name, motto, street, zip, state, make_to_page(warranty), phone, mu1, mu2, mu3, mu4, mu5, tax, city, tax_labor ? 1 : 0, tax_parts ? 1 : 0);

			sqlite3_exec(db, sql, NULL, NULL, &err);
			if (err)
				GiveError(err, 0);

			if (mu1)
				MU1 = atof(mu1) / 100;
			if (mu2)
				MU2 = atof(mu2) / 100;
			if (mu3)
				MU3 = atof(mu3) / 100;
			if (mu4)
				MU4 = atof(mu4) / 100;
			if (mu5)
				MU5 = atof(mu5) / 100;
			if (tax)
				TAX = atof(tax) / 100;

			GiveError("Settings Saved.", 0);
			DestroyWindow(setupwindow);
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