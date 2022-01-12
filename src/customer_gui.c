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

// Inputs
#define ID_CS_FIRST 10000
#define ID_CS_LAST 10001
#define ID_CS_ADDRESS 10002
#define ID_CS_CITY    10003
#define ID_CS_ZIP     10004
#define ID_CS_PHONE1  10005
#define ID_CS_PHONE2  10006
#define ID_CS_PHONE3  10007
#define ID_CS_OFFICE  10008
#define ID_CS_STATE   10009
#define ID_CS_NOTES   10012
#define ID_CS_CRACT   10010
#define ID_CS_TAXFREE 10011
#define ID_CS_DENY    10013
#define ID_CS_CASH    10014
#define ID_CS_NOCREDIT 10015
#define ID_CS_NOCHECK 10016
#define ID_CS_TAXID   10017
#define ID_FD_LIST    10018
#define ID_FD_FDNUM   10019
#define ID_FD_LAST    10020
#define ID_CS_TSPENT  10021
#define ID_CS_TINV    10022
#define ID_CS_TCRED   10023
#define ID_CS_SEEN    10024
#define ID_CS_CSSINCE 10025
#define ID_CS_CSID    10026
#define ID_CS_COMPUTERS 10027

// Buttons

#define ID_CS_BTN_ADD 2000
#define ID_CS_BTN_CANCEL 2001
#define ID_CS_BTN_TICKET 2002
#define ID_CS_BTN_FIND   2003
#define ID_FD_BTN_OK     2004
#define ID_FD_BTN_CANCEL 2005
#define ID_FD_BTN_FIND   2006
#define ID_CS_BTN_COMPUTER 2007

NWC_PARENT* CS_window;
NWC_PARENT* FD_window;
HWND zip_ctrl;

int cmp_id;

extern NWC_PARENT* CM_window;

void test(void)
{
	return;
}

BOOL cs_check_for_change(BOOL closing)
{
	char* last_name;
	char* first_name;
	char* address;
	char* city;
	char* state;
	char* zip;
	char* phone1;
	char* phone2;
	char* phone3;
	char realphone[100];
	char* notes;
	char* taxid;

	char* csid;
	BOOL coract, acceptchk, acceptcre, cashonly, denywork, tfree;
	HWND cact, acheck, acredit, acash, deny, taxfree = 0;
	CSDATA* cs;
	BOOL changed;
	char sql[2048];
	char* err;

	changed = FALSE;

	csid = CTRL_gettext(CS_window, "csid");

	if (!csid)
		return 0;

	if (!cswindow)
		return 0;
	coract = acceptchk = acceptcre = cashonly = denywork = tfree = 0;

	last_name = CTRL_gettext(CS_window, "last");
	first_name = CTRL_gettext(CS_window, "first");
	address = CTRL_gettext(CS_window, "street");
	city = CTRL_gettext(CS_window, "city");
	state = CTRL_gettext(CS_window, "state");
	zip = CTRL_gettext(CS_window, "zip");
	phone1 = CTRL_gettext(CS_window, "ph1");
	phone2 = CTRL_gettext(CS_window, "ph2");
	phone3 = CTRL_gettext(CS_window, "ph3");
	notes = CTRL_gettext(CS_window, "notes");
	taxid = CTRL_gettext(CS_window, "taxid");

	cact = get_control(CS_window, "Corporate Account")->handle;
	acheck = get_control(CS_window, "Accept Only Cash.")->handle;
	acredit = get_control(CS_window, "Do Not Accept Credit.")->handle;
	acash = get_control(CS_window, "Do Not Accept Checks.")->handle;
	deny = get_control(CS_window, "Deny Work to Customer.")->handle;
	taxfree = get_control(CS_window, "Tax Free Customer")->handle;

	if (last_name == NULL && first_name == NULL && phone1 == NULL && phone2 == NULL && phone3 == NULL)
		return TRUE;
	if (last_name == NULL || first_name == NULL || phone1 == NULL || phone2 == NULL || phone3 == NULL)
	{
		if (closing == FALSE)
			GiveError("You must supply a First and Last name as well as a valid Phone Number for customer.\r\nIf you wish to cancel and not save ANY data, press the cancel button (or press escape).", 0);
		if (last_name == NULL)
			SetFocus(get_control(CS_window, "last")->handle);
		else if (first_name == NULL)
			SetFocus(get_control(CS_window, "first")->handle);
		else if (phone1 == NULL)
			SetFocus(get_control(CS_window, "ph1")->handle);
		else if (phone2 == NULL)
			SetFocus(get_control(CS_window, "ph2")->handle);
		else
			SetFocus(get_control(CS_window, "ph3")->handle);
		return FALSE;
	}

	sprintf(realphone, "%s-%s-%s", phone1, phone2, phone3);

	for (cs = cslist; cs; cs = cs->next)
	{
		if (atoi(csid) == cs->cs_id)
		{
			// customer found. Check data.
			if (strcmp(last_name, cs->last_name))
			{
				changed = TRUE;
				sprintf(cs->last_name, "%s", last_name);
			}
			if (strcmp(first_name, cs->first_name))
			{
				changed = TRUE;
				sprintf(cs->first_name, "%s", first_name);
			}
			if (strcmp(address, cs->st_address))
			{
				changed = TRUE;
				sprintf(cs->st_address, "%s", address);
			}

			if (strcmp(city, cs->city))
			{
				changed = TRUE;
				sprintf(cs->city, "%s", city);
			}
			if (strcmp(state, cs->state))
			{
				changed = TRUE;
				sprintf(cs->state, "%s", state);
			}
			if (atoi(zip) != cs->zip_code)
			{
				changed = TRUE;
				cs->zip_code = atoi(zip);
			}

			if (strcmp(realphone, cs->phone))
			{
				changed = TRUE;
				sprintf(cs->phone, "%s", realphone);
			}
			if (strcmp(notes, cs->notes))
			{
				changed = TRUE;
				sprintf(cs->notes, "%s", notes);
			}
			if (taxid)
			{
				if (atoi(taxid) != cs->tax_id)
					cs->tax_id = atoi(taxid);
			}

			break;
		}
	}
	if (changed == TRUE)
	{
		/*CREATE TABLE Customers(id , LastName , FirstName , CorporateAccountant, Street , City , State , Phone ,"
						   " Notes ,  Zip INT, TimesVisited INT, CorporateID INT,"
						   "TaxID INT, OpenTickets INT, TotalTickets INT, LastSeen CHAR,  DenyWork INT, TaxFree INT,"
						   "CorporateAccount INT, CashOnly INT, AcceptCredit INT, AcceptCheck INT, Cssince CHAR);"*/
		sprintf(sql, "UPDATE Customers SET "

			"LastName = '%s', FirstName = '%s', Street = '%s', City = '%s', State = '%s', Zip = '%s', Phone = '%s', Notes = '%s',"
			"CorporateID = '%d', TaxID = '%d', LastSeen = '%s', DenyWork = '%d', TaxFree = '%d', CorporateAccount = '%d', CashOnly = '%d', AcceptCredit = '%d',"
			" AcceptCheck = '%d' WHERE id=%d;",
			last_name, first_name, address, city, state, zip, realphone, notes, cs->corporate_id, cs->tax_id, cs->last_seen, cs->deny_work, cs->tax_free,
			cs->corporate_account, cs->cash_only, cs->accept_creditcard, cs->accept_check, atoi(csid));

		sqlite3_exec(db, sql, NULL, NULL, &err);

		if (err)
			GiveError(err, 0);
		free(last_name);
		free(first_name);
		free(address);
		free(city);
		free(state);
		free(zip);
		free(phone1);
		free(phone2);
		free(phone3);
		free(notes);
		free(csid);
		free(taxid);
	}
	return TRUE;
}

void update_cs_check()
{
	char* csid;
	BOOL coract, acceptchk, acceptcre, cashonly, denywork, tfree;
	char sql[1024];
	char* err;
	CSDATA* cs;

	csid = CTRL_gettext(CS_window, "csid");

	if (!csid)
		return;

	if (!cswindow)
		return;

	coract = check_get_status(CS_window, "Corporate Account");
	cashonly = check_get_status(CS_window, "Accept Only Cash.");
	acceptcre = check_get_status(CS_window, "Do Not Accept Credit.");
	acceptchk = check_get_status(CS_window, "Do Not Accept Checks.");
	denywork = check_get_status(CS_window, "Deny Work to Customer.");
	tfree = check_get_status(CS_window, "Tax Free Customer");

	for (cs = cslist; cs; cs = cs->next)
	{
		if (cs->cs_id == atoi(csid))
		{
			cs->corporate_account = coract;
			cs->cash_only = cashonly;
			cs->accept_creditcard = acceptcre;
			cs->accept_check = acceptchk;
			cs->deny_work = denywork;
			cs->tax_free = tfree;
			break;
		}
	}

	sprintf(sql, "UPDATE Customers SET DenyWork = '%d', TaxFree = '%d', CorporateAccount = '%d', CashOnly = '%d', AcceptCredit = '%d',"
		" AcceptCheck = '%d' WHERE id=%d;", denywork, tfree, coract, cashonly, acceptcre, acceptchk, atoi(csid));
	sqlite3_exec(db, sql, NULL, NULL, &err);

	if (err)
		GiveError(err, 0);
	free(csid);
}

void create_cs_find_window(void)
{
	if (csfind != NULL)
		return;

	FD_window = create_parent("Find Customer");
	set_parent_config(FD_window, (HWND)0, (LRESULT*)FDWindow_proc, 0, 0, 360, 450, 0, FALSE, 0, WS_TABSTOP | DS_3DLOOK | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_SIZEBOX);

	AddStatic_Parent(FD_window, "Find by Last Name:", 0, 0, 160, 18, 0, 0, WS_BORDER, TRUE);
	AddStatic_Parent(FD_window, "Or Find by Phone#:", 0, 20, 160, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(FD_window, "last", 160, 0, 180, 20, 0, ID_FD_LAST, 0, TRUE);
	AddEdit_Parent(FD_window, "num", 160, 20, 180, 20, 0, ID_FD_LAST, ES_NUMBER, TRUE);
	AddList_Parent(FD_window, "cslist", 0, 42, 340, 340, 0, ID_FD_LIST, LBS_NOTIFY, TRUE);
	AddButton_Parent(FD_window, "Find", 0, 386, 80, 20, 0, ID_FD_BTN_FIND, 0, TRUE);
	AddButton_Parent(FD_window, "Ok", 120, 386, 80, 20, 0, ID_FD_BTN_OK, 0, TRUE);
	AddButton_Parent(FD_window, "Cancel", 260, 386, 80, 20, 0, ID_FD_BTN_CANCEL, 0, TRUE);
	CTRL_SetText(FD_window, "last", "");
	CTRL_SetText(FD_window, "num", "");

	SendMessage(get_control(FD_window, "num")->handle, EM_LIMITTEXT, 10, 0);
	SendMessage(get_control(FD_window, "last")->handle, EM_LIMITTEXT, 25, 0);

	show_parent(FD_window);
	csfind = FD_window->window_control;
	return;
}

void populate_find_list(void)
{
	CSDATA* cs;
	char* phone;
	char* last;
	char str[1024];

	phone = last = NULL;

	last = CTRL_gettext(FD_window, "last");
	phone = CTRL_gettext(FD_window, "num");

	if ((!last && !phone))
	{
		GiveError("You must provide either a phone number or last name to search for a customer.", 0);
		return;
	}

	CTRL_List_clearlist(FD_window, "cslist");

	if (!strcmp(last, "*"))
	{
		for (cs = cslist; cs; cs = cs->next)
		{
			sprintf(str, "%s, %s, %s", cs->last_name, cs->first_name, cs->phone);
			CTRL_List_additem(FD_window, "cslist", str);
		}
	}
	else
	{
		for (cs = cslist; cs; cs = cs->next)
		{
			if ((last && !strprefix(cs->last_name, last)) || (phone == NULL ? 0 : phone[0] != '\0' && strstr(cs->phone, (phone == NULL || phone[0] == '\0') ? "" : phone)))

			{
				sprintf(str, "%s, %s, %s", cs->last_name, cs->first_name, cs->phone);
				CTRL_List_additem(FD_window, "cslist", str);
			}
		}
	}
};

void fill_cs(CSDATA* cs)
{
	char* token;
	char delim[] = "-";
	char phone[100];
	char sql[1024];
	char* err;

	if (cswindow == NULL)
		create_cs_window();

	if (!cs)
		return;
	SetFocus(cswindow);

	CTRL_SetText(CS_window, "last", cs->last_name);
	CTRL_SetText(CS_window, "first", cs->first_name);
	CTRL_SetText(CS_window, "street", cs->st_address);
	CTRL_SetText(CS_window, "zip", "%d", cs->zip_code);
	CTRL_SetText(CS_window, "city", cs->city);
	CTRL_SetText(CS_window, "state", cs->state);
	CTRL_SetText(CS_window, "notes", cs->notes);
	CTRL_SetText(CS_window, "tspent", "$%8.2f", cs->total_spent);
	CTRL_SetText(CS_window, "tinv", "%d", cs->total_tickets);
	CTRL_SetText(CS_window, "tcred", "$%8.2f", cs->unpaid_due);
	CTRL_SetText(CS_window, "seen", cs->last_seen);
	CTRL_SetText(CS_window, "cssince", cs->cs_since);
	CTRL_SetText(CS_window, "csid", "%6.6d", cs->cs_id);
	CTRL_SetText(CS_window, "taxid", "%d", cs->tax_id);

	if (cs->accept_check > 0)
		check_set(CS_window, "Do Not Accept Checks.");
	if (cs->deny_work > 0)
		check_set(CS_window, "Deny Work to Customer.");
	if (cs->tax_free > 0)
	{
		check_set(CS_window, "Tax Free Customer");
		EnableWindow(get_control(CS_window, "taxid")->handle, TRUE);
	}
	if (cs->corporate_account > 0)
		check_set(CS_window, "Corporate Account");
	if (cs->cash_only > 0)
		check_set(CS_window, "Accept Only Cash.");
	if (cs->accept_creditcard > 0)
		check_set(CS_window, "Do Not Accept Credit.");

	sprintf(phone, "%s", cs->phone);

	token = strtok(phone, delim);
	CTRL_SetText(CS_window, "ph1", token);
	token = strtok(NULL, delim);
	CTRL_SetText(CS_window, "ph2", token);
	token = strtok(NULL, delim);
	CTRL_SetText(CS_window, "ph3", token);

	clist_clear_list(CS_window, "computer");
	sprintf(sql, "SELECT Type, Make, Model, TicketTotal FROM Computer WHERE CSID=%d;", cs->cs_id);

	sqlite3_exec(db, sql, do_computer_list, NULL, &err);
	if (err)
		GiveError(err, 0);

	return;
}
int do_computer_list(void* NotUsed, int argc, char** argv, char** azColName)
{
	clist_add_data(CS_window, "computer", argv, argc);
	return 0;
}
void create_cs_window(void)
{
	HWND temp_ctrl;

	if (cswindow != NULL)
		return;

	if (tickwindow)
	{
		DestroyWindow(tickwindow);
		tickwindow = NULL;
	}

	cmp_id = 0;

	CS_window = create_parent("Customer Database");
	set_parent_config(CS_window, (HWND)0, (LRESULT*)CSWindow_proc, 0, 0, 600, 640, 0, FALSE, 0, WS_TABSTOP | DS_3DLOOK | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_SIZEBOX);

	AddStatic_Parent(CS_window, "Last Name:", 0, 115, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "last", 80, 115, 200, 20, 0, ID_CS_LAST, 0, TRUE);

	AddStatic_Parent(CS_window, "First Name:", 300, 115, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "first", 380, 115, 200, 20, 0, ID_CS_FIRST, 0, TRUE);

	AddStatic_Parent(CS_window, "Street Address:", 0, 140, 580, 45, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "street", 5, 160, 570, 20, 0, ID_CS_ADDRESS, 0, TRUE);

	AddStatic_Parent(CS_window, "Zip Code:", 450, 190, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "zip", 530, 190, 50, 20, 0, ID_CS_ZIP, ES_NUMBER, TRUE);

	AddStatic_Parent(CS_window, "City:", 0, 190, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "city", 80, 190, 200, 20, 0, ID_CS_CITY, WS_TABSTOP, TRUE);

	AddStatic_Parent(CS_window, "State:", 300, 190, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "state", 380, 190, 35, 20, 0, ID_CS_STATE, WS_TABSTOP, TRUE);

	AddStatic_Parent(CS_window, "Phone #:", 0, 220, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "ph1", 80, 220, 35, 20, 0, ID_CS_PHONE1, ES_NUMBER, TRUE);
	AddEdit_Parent(CS_window, "ph2", 117, 220, 35, 20, 0, ID_CS_PHONE2, ES_NUMBER, TRUE);
	AddEdit_Parent(CS_window, "ph3", 154, 220, 45, 20, 0, ID_CS_PHONE3, ES_NUMBER, TRUE);

	AddStatic_Parent(CS_window, "Notes:", 0, 250, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "notes", 5, 268, 570, 80, 0, ID_CS_NOTES, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, TRUE);

	AddStatic_Parent(CS_window, "Computers:", 0, 350, 80, 18, 0, 0, WS_BORDER, TRUE);

	AddCList_Parent(CS_window, "computer", 5, 370, 570, 100, 0, ID_CS_COMPUTERS, LBS_HASSTRINGS | LVS_REPORT, TRUE);

	clist_add_col(CS_window, "computer", 141, "Computer Type");
	clist_add_col(CS_window, "computer", 141, "Computer Make");
	clist_add_col(CS_window, "computer", 141, "Computer Model");
	clist_add_col(CS_window, "computer", 141, "Total Invoices");

	//clist_add_data(CS_window, "computer", tt, 2);
	//sqlite3_exec(db, "SELECT LastName, FirstName, Street, Phone FROM Customers;", do_list,NULL, NULL);

	AddCheck_Parent(CS_window, "Deny Work to Customer.", 0, 470, 200, 20, 0, ID_CS_DENY, WS_BORDER, TRUE);

	AddCheck_Parent(CS_window, "Tax Free Customer", 0, 490, 200, 20, 0, ID_CS_TAXFREE, WS_BORDER, TRUE);
	AddStatic_Parent(CS_window, "Tax ID:", 350, 490, 80, 18, 0, 0, WS_BORDER, TRUE);
	AddEdit_Parent(CS_window, "taxid", 430, 490, 140, 20, 0, ID_CS_TAXID, ES_NUMBER | WS_DISABLED, TRUE);

	AddCheck_Parent(CS_window, "Corporate Account", 0, 510, 200, 20, 0, ID_CS_CRACT, WS_BORDER, TRUE);
	AddCheck_Parent(CS_window, "Accept Only Cash.", 0, 530, 200, 20, 0, ID_CS_CASH, WS_BORDER, TRUE);
	AddCheck_Parent(CS_window, "Do Not Accept Credit.", 0, 550, 200, 20, 0, ID_CS_NOCREDIT, WS_BORDER, TRUE);
	AddCheck_Parent(CS_window, "Do Not Accept Checks.", 0, 570, 200, 20, 0, ID_CS_NOCHECK, WS_BORDER, TRUE);

	AddButton_Parent(CS_window, "find", 0, 0, 110, 80, 0, ID_CS_BTN_FIND, BS_MULTILINE, TRUE);
	AddStatic_Parent(CS_window, "frame", 120, 0, 440, 110, 0, 0, WS_BORDER, TRUE);

	AddButton_Parent(CS_window, "Add Customer", 430, 510, 140, 20, 0, ID_CS_BTN_ADD, 0, TRUE);
	AddButton_Parent(CS_window, "New Ticket", 430, 530, 140, 20, 0, ID_CS_BTN_TICKET, 0, TRUE);
	AddButton_Parent(CS_window, "Add Computer", 430, 550, 140, 20, 0, ID_CS_BTN_COMPUTER, 0, TRUE);
	AddButton_Parent(CS_window, "Cancel", 430, 570, 140, 20, 0, ID_CS_BTN_CANCEL, 0, TRUE);

	AddStatic_Parent(CS_window, "Total Spent:", 122, 2, 120, 18, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(CS_window, "Total Invoices:", 122, 20, 120, 18, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(CS_window, "Total Credit:", 122, 38, 120, 18, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(CS_window, "Last Seen:", 122, 56, 120, 18, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(CS_window, "Customer Since:", 122, 74, 120, 18, 0, 0, ES_RIGHT, TRUE);
	AddStatic_Parent(CS_window, "Customer ID:", 122, 92, 120, 16, 0, 0, ES_RIGHT, TRUE);

	AddStatic_Parent(CS_window, "tspent", 260, 2, 120, 18, 0, ID_CS_TSPENT, 0, TRUE);
	AddStatic_Parent(CS_window, "tinv", 260, 20, 120, 18, 0, ID_CS_TINV, 0, TRUE);
	AddStatic_Parent(CS_window, "tcred", 260, 38, 120, 18, 0, ID_CS_TCRED, 0, TRUE);
	AddStatic_Parent(CS_window, "seen", 260, 56, 120, 18, 0, ID_CS_SEEN, 0, TRUE);
	AddStatic_Parent(CS_window, "cssince", 260, 74, 120, 18, 0, ID_CS_CSSINCE, 0, TRUE);
	AddStatic_Parent(CS_window, "csid", 260, 92, 120, 16, 0, ID_CS_CSID, 0, TRUE);

	CTRL_SetText(CS_window, "last", "");
	CTRL_SetText(CS_window, "first", "");
	CTRL_SetText(CS_window, "street", "");
	CTRL_SetText(CS_window, "city", "");
	CTRL_SetText(CS_window, "zip", "");
	CTRL_SetText(CS_window, "state", "");
	CTRL_SetText(CS_window, "ph1", "");
	CTRL_SetText(CS_window, "ph2", "");
	CTRL_SetText(CS_window, "ph3", "");
	CTRL_SetText(CS_window, "notes", "");
	CTRL_SetText(CS_window, "taxid", "");
	CTRL_SetText(CS_window, "find", "Find Customer (F3)");
	CTRL_SetText(CS_window, "frame", "");
	CTRL_SetText(CS_window, "tspent", "$0.00");
	CTRL_SetText(CS_window, "tinv", "0");
	CTRL_SetText(CS_window, "tcred", "$0.00");
	CTRL_SetText(CS_window, "seen", "");
	CTRL_SetText(CS_window, "cssince", "");
	CTRL_SetText(CS_window, "csid", "0");

	temp_ctrl = get_control(CS_window, "notes")->handle;

	SendMessage(temp_ctrl, EM_LIMITTEXT, MAXNOTE, 0); // Limit amount of input.
	SendMessage(get_control(CS_window, "state")->handle, EM_LIMITTEXT, 2, 0);
	show_parent(CS_window);

	SetFocus(get_control(CS_window, "last")->handle);

	zip_ctrl = get_control(CS_window, "zip")->handle;

	cswindow = CS_window->window_control;
}

BOOL do_add_customer(void)
{
	/* FUCK sql */

	char* last_name;
	char* first_name;
	char* address;
	char* city;
	char* state;
	char* zip;
	char* phone1;
	char* phone2;
	char* phone3;
	char realphone[100];
	char* notes;
	char* taxid;
	char SQLStr[MAXNOTE * 5];
	BOOL coract, acceptchk, acceptcre, cashonly, denywork, tfree;

	CSDATA* cs;

	if (!cswindow)
		return 0;
	coract = acceptchk = acceptcre = cashonly = denywork = tfree = 0;

	last_name = CTRL_gettext(CS_window, "last");
	first_name = CTRL_gettext(CS_window, "first");
	address = CTRL_gettext(CS_window, "street");
	city = CTRL_gettext(CS_window, "city");
	state = CTRL_gettext(CS_window, "state");
	zip = CTRL_gettext(CS_window, "zip");
	phone1 = CTRL_gettext(CS_window, "ph1");
	phone2 = CTRL_gettext(CS_window, "ph2");
	phone3 = CTRL_gettext(CS_window, "ph3");
	notes = CTRL_gettext(CS_window, "notes");
	taxid = CTRL_gettext(CS_window, "taxid");

	coract = check_get_status(CS_window, "Corporate Account");
	cashonly = check_get_status(CS_window, "Accept Only Cash.");
	acceptcre = check_get_status(CS_window, "Do Not Accept Credit.");
	acceptchk = check_get_status(CS_window, "Do Not Accept Checks.");
	denywork = check_get_status(CS_window, "Deny Work to Customer.");
	tfree = check_get_status(CS_window, "Tax Free Customer");

	if (last_name == NULL || first_name == NULL || phone1 == NULL || phone2 == NULL || phone3 == NULL)
	{
		GiveError("You must supply a First and Last name as well as a valid Phone Number before adding a customer.", 0);
		if (last_name == NULL)
			SetFocus(get_control(CS_window, "last")->handle);
		else if (first_name == NULL)
			SetFocus(get_control(CS_window, "first")->handle);
		else if (phone1 == NULL)
			SetFocus(get_control(CS_window, "ph1")->handle);
		else if (phone2 == NULL)
			SetFocus(get_control(CS_window, "ph2")->handle);
		else
			SetFocus(get_control(CS_window, "ph3")->handle);
		return 0;
	}

	sprintf(realphone, "%s-%s-%s", phone1, phone2, phone3);

	for (cs = cslist; cs; cs = cs->next)
	{
		if ((!strcmp(cs->last_name, last_name)) && (!strcmp(cs->first_name, first_name)) && (!strcmp(cs->phone, realphone)))
		{
			GiveError("Customer already exists in the database.", 0);
			return 0;
		}
	}

	sprintf(SQLStr, "INSERT INTO Customers (LastName, FirstName, Street, City, State, Zip, Phone, Notes, TaxID, CorporateAccount, AcceptCheck, AcceptCredit, CashOnly, "
		"DenyWork, TaxFree, LastSeen, Cssince) VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s','%s');",
		last_name, first_name, address, city, state, zip, realphone, notes, taxid, coract, acceptchk, acceptcre, cashonly, denywork, tfree, get_date(), get_date());
	sqlite3_exec(db, SQLStr, NULL, NULL, NULL);
	sprintf(SQLStr, "select * from customers order by id desc limit 1;");
	sqlite3_exec(db, SQLStr, add_cs_db, NULL, NULL);

	fill_cs(get_last_cs());
	return TRUE;
}

LRESULT APIENTRY FDWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CSDATA* cs = 0;
	int idx;
	int i;
	char* phone;
	char* last;

	switch (msg)
	{
	case WM_SETFOCUS:
	{
		SetFocus(get_control(FD_window, "last")->handle);
		break;
	}
	case WM_COMMAND:
	{
		if (wparam == IDOK)
		{
			populate_find_list();

			break;
		}

		switch (LOWORD(wparam))
		{
		case ID_FD_LIST:
		{
			if (HIWORD(wparam) == LBN_DBLCLK)
			{
				if ((idx = CTRL_list_get_sel_idx(FD_window, "cslist")) == -1)
				{
					GiveError("Please select a customer from the list.", 0);
					break;
				}
				i = 0;

				last = CTRL_gettext(FD_window, "last");
				phone = CTRL_gettext(FD_window, "num");

				if (!strcmp(last, "*"))
				{
					for (cs = cslist; cs; cs = cs->next)
					{
						if (i == idx)
						{
							fill_cs(cs);
							DestroyWindow(csfind);
							break;
						}
						i++;
					}
				}
				else
				{
					for (cs = cslist; cs; cs = cs->next)
					{
						if ((last[0] != '\0' && !strprefix(cs->last_name, last)) || (phone == NULL ? 0 : phone[0] != '\0' && strstr(cs->phone, phone == NULL ? "" : phone)))
						{
							if (i == idx)
							{
								fill_cs(cs);
								DestroyWindow(csfind);
								//return the csdata
								break;
							}
							i++;
						}
					}
				}
				break;
			}
			break;
		}

		case ID_FD_BTN_FIND:
		{
			populate_find_list();
			break;
		}
		case ID_FD_BTN_OK:
		{
			if ((idx = CTRL_list_get_sel_idx(FD_window, "cslist")) == -1)
			{
				GiveError("Please select a customer from the list.", 0);
				break;
			}
			i = 0;

			last = CTRL_gettext(FD_window, "last");
			phone = CTRL_gettext(FD_window, "num");

			for (cs = cslist; cs; cs = cs->next)
			{
				if ((last[0] != '\0' && !strprefix(cs->last_name, last)) || (phone == NULL ? 0 : phone[0] != '\0' && strstr(cs->phone, phone == NULL ? "" : phone)))
				{
					if (i == idx)
					{
						DestroyWindow(csfind);
						if (!cswindow)
							create_cs_window();
						fill_cs(cs);
						//return the csdata
						break;
					}
					i++;
				}
			}
			break;
		}
		case ID_FD_BTN_CANCEL:
		{
			DestroyWindow(csfind);
			csfind = NULL;
			break;
		}
		}
		break;
	}

	case WM_DESTROY:
	{
		DestroyParent(FD_window);
		DestroyWindow(csfind);

		csfind = NULL;

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

LRESULT APIENTRY CSWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char state[1000];
	char city[1000];
	char sql[1024];
	char* csid;
	char* zip;
	BOOL zip_focus = FALSE;
	int ctrl_int;
	NMHDR* nh;
	int idx;
	sqlite3_stmt* stmt;
	int sr;
	int i;
	int cmpid;

	//LOG("Hwnd: %d, Msg: %d, wparam: %d, lparam: %d", hwnd,msg, wparam, lparam);

	switch (msg)
	{
	case WM_SETFOCUS:
	{
		SetFocus((get_control(CS_window, "last") != NULL) ?
			get_control(CS_window, "last")->handle :
			NULL);
		break;
	}
	case WM_NOTIFY:
	{
		ctrl_int = (int)wparam;
		nh = (LPNMHDR)lparam;

		switch (nh->code)
		{
		case NM_DBLCLK:
		{
			if (nh->idFrom == ID_CS_COMPUTERS)
			{
				csid = CTRL_gettext(CS_window, "csid");

				if (!csid)
				{
					free(csid);
					break;
				}
				idx = SendMessage(get_control(CS_window, "computer")->handle, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);

				if (idx == -1)
				{
					break;
				}

				sprintf(sql, "SELECT Make, Model, Ram, cSpeed, Windows, Notes, Type, id  FROM Computer WHERE CSID=%d;", get_cs_by_id(atoi(csid))->cs_id);
				sr = sqlite3_prepare(db, sql, -1, &stmt, 0);

				i = 0;

				if (sr == SQLITE_OK)
				{
					while (sqlite3_step(stmt) == SQLITE_ROW)
					{
						if (i == idx)
						{
							//cmpid = atoi((char*)sqlite3_column_text(stmt,0));

							create_add_computer();
							CTRL_SetText(CM_window, "Add Computer", "Update Computer");
							CTRL_SetText(CM_window, "make", (char*)sqlite3_column_text(stmt, 0));
							CTRL_SetText(CM_window, "model", (char*)sqlite3_column_text(stmt, 1));
							CTRL_SetText(CM_window, "ram", (char*)sqlite3_column_text(stmt, 2));
							CTRL_SetText(CM_window, "speed", (char*)sqlite3_column_text(stmt, 3));
							CTRL_SetText(CM_window, "os", (char*)sqlite3_column_text(stmt, 4));
							CTRL_SetText(CM_window, "notes", (char*)sqlite3_column_text(stmt, 5));
							CTRL_SetText(CM_window, "type", (char*)sqlite3_column_text(stmt, 6));
							cmp_id = atoi((char*)sqlite3_column_text(stmt, 7));

							break;
						}
						i++;
					}
				}
				break;
			}
			break;
		}
		}

		break;
	}

	case WM_CLOSE:
	{
		if (cs_check_for_change(FALSE) == TRUE)
		{
			DestroyWindow(cswindow);
			cswindow = NULL;
		}

		ShowWindow(cswindow, SW_SHOW);

		break;
	}

	case WM_DESTROY:
	{
		DestroyParent(CS_window);
		return 0;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case ID_CS_BTN_FIND:
		{
			clist_clear_list(CS_window, "computer");
			if (csfind != NULL)
			{
				SetFocus(csfind);
			}
			else
			{
				create_cs_find_window();
			}

			break;
		}

		case ID_CS_CRACT:
		case ID_CS_TAXFREE:
		case ID_CS_DENY:
		case ID_CS_CASH:
		case ID_CS_NOCREDIT:
		case ID_CS_NOCHECK:
		{
			if (LOWORD(wparam) == ID_CS_TAXFREE)
			{
				if (check_get_status(CS_window, "Tax Free Customer") == TRUE)
				{
					EnableWindow(get_control(CS_window, "taxid")->handle, TRUE);
				}
				else
				{
					EnableWindow(get_control(CS_window, "taxid")->handle, FALSE);
				}
			}

			update_cs_check();
			break;
		}

		case ID_CS_BTN_ADD:
		{
			do_add_customer();
			break;
		}
		case ID_CS_BTN_COMPUTER:
		{
			if (cmpwindow)
				SetFocus(cmpwindow);
			else
				create_add_computer();
			break;
		}
		case ID_CS_BTN_TICKET:
		{
			i = 0;
			csid = CTRL_gettext(CS_window, "csid");
			if (!strcmp(csid, "0"))
			{
				GiveError("You must select a customer before beginning a ticket.", 0);
				free(csid);
				break;
			}

			idx = SendMessage(get_control(CS_window, "computer")->handle, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);

			if (idx == -1)
			{
				if (MessageBoxEx(cswindow, "No computer selected; Add ticket without computer?", "No computer selected", MB_YESNO, 0) == IDNO)
				{
					free(csid);

					break;
				}
			}

			sprintf(sql, "SELECT id FROM Computer WHERE CSID=%d;", get_cs_by_id(atoi(csid))->cs_id);
			sr = sqlite3_prepare(db, sql, -1, &stmt, 0);

			if (sr == SQLITE_OK)
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					if (i == idx)
					{
						cmpid = atoi((char*)sqlite3_column_text(stmt, 0));
						break;
					}
					i++;
				}
			}

			create_ticket_window(get_cs_by_id(atoi(csid)), cmpid, 0, 0, MODE_NORMAL);

			free(csid);
			break;
		}
		case ID_CS_BTN_CANCEL:
		{
			DestroyWindow(cswindow);
			cswindow = NULL;
			break;
		}
		}

	case EN_KILLFOCUS:
	{
		if ((int)LOWORD(wparam) != ID_CS_ZIP)
		{
			break;
		}

		state[0] = '\0';
		city[0] = '\0';
		zip = CTRL_gettext(CS_window, "zip");

		if (check_zip(zip, state, city) == TRUE)
		{
			CTRL_SetText(CS_window, "city", city);
			CTRL_SetText(CS_window, "state", state);
			free(zip);
			break;
		}
		else
			free(zip);

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