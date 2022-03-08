#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <time.h>
#include <uxtheme.h>
#include "Nano PoS.h"
#include "NWC.h"
#include <commctrl.h>
#include "sqlite3.h"

//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' ""version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define ID_POS_BTN_ESTIMATE  1000
#define ID_POS_BTN_CUSTOMER  1001
#define ID_POS_BTN_START     1002
#define ID_POS_BTN_WIP       1003
#define ID_POS_BTN_INVENTORY 1004
#define ID_POS_BTN_BROWSER   1005
#define ID_POS_BTN_SETUP     1006
#define ID_POS_BTN_EMPLOYEE  1007
#define ID_FILE_EXIT         4000
#define ID_REPORT_DAILY      4001
#define ID_FIND_INVOICE	     4002
#define WIP_LIST 1008

extern NWC_PARENT* CS_window; 
extern NWC_PARENT* FD_window;
extern NWC_PARENT* INV_add;
extern NWC_PARENT* INV_search;
extern NWC_PARENT* INV_ticket;
extern NWC_PARENT* Labor;
extern NWC_PARENT* TK_window;
extern NWC_PARENT* Setup;
extern NWC_PARENT* Pay_window;
extern NWC_PARENT* payments;
extern NWC_PARENT* Refund;
extern NWC_PARENT* Daily;
extern NWC_PARENT* CALC_window;
extern double MU1, MU2, MU3, MU4, MU5;

NWC_PARENT* POS_parent; // Main window for NanoPOS

char ERROR_STRING[5000];

NWC_PARENT* tp;

HDC hDC, MemDCExercising;

static int cscallback(void* NotUsed, int argc, char** argv, char** azColName)
{
	int i;
	NotUsed = 0;

	for (i = 0; i < argc; i++)
	{
		LOG("%s:%s", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil)
{
	MSG messages; // Message buffer for windows

	FILE* dbf;
	char SQLString[50000];
	int dbret;
	HWND last_focus;
	sqlite3_stmt* stmt;
	int sr;

	char* err;

	srand((UINT)time(NULL));

	LPINITCOMMONCONTROLSEX comctrl;

	comctrl = malloc(sizeof(*comctrl));
	comctrl->dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_COOL_CLASSES | ICC_STANDARD_CLASSES;
	comctrl->dwSize = sizeof(*comctrl);

	if (!InitCommonControlsEx(comctrl))
		GiveError("Common Controls failed to Initialize.", 1);

	db = NULL;
	csfree = NULL;
	cslist = NULL;
	mainwindow = NULL;
	cswindow = NULL;
	csfind = NULL;
	cmpwindow = NULL;
	invwindowadd = NULL;
	invwindowticket = NULL;
	invwindowsearch = NULL;
	tickwindow = NULL;
	printpreview = NULL;
	printpreview_panel = NULL;
	tax_labor = tax_parts = FALSE;
	g_hInst = hThisInstance;
	MU1 = MU2 = MU3 = MU4 = MU5 = 1.00f;

	if (!(dbf = fopen(DB_FILE, "r")))
	{
		GiveError("Database not Found. Creating INDEX -- This may take a moment.", FALSE);
		dbret = sqlite3_open(DB_FILE, &db);

		if (dbret)
		{
			GiveError("Unable to open DB.", TRUE);
		}

		sprintf(SQLString,
			"CREATE TABLE Customers(id INTEGER PRIMARY KEY AUTOINCREMENT, LastName CHAR, FirstName CHAR, "
			"CorporateAccountant CHAR, Street CHAR, City CHAR, State CHAR, Phone CHAR, PhoneCell CHAR, "
			"PhoneHome CHAR, PhoneOffice CHAR, PhoneSpouce CHAR, PhoneFax CHAR, Notes CHAR, Sirname CHAR, "
			"Zip INT, TimesVisited INT, CorporateID INT, TaxID INT, OpenTickets INT, TotalTickets INT, "
			"LastSeen CHAR, UnpaidDue DECIMAL(18,2), TotalSpent DECIMAL(18,2), HasCredit INT, DenyWork INT, "
			"TaxFree INT, CorporateAccount INT, CashOnly INT, AcceptCredit INT, AcceptCheck INT, Cssince CHAR);");

		sqlite3_exec(db, SQLString, NULL, NULL, &err);
		if (err)
			GiveError(err, 0);

		sprintf(SQLString,
			"CREATE TABLE Tickets(id INTEGER PRIMARY KEY,Locked INT, Invoice INT, RefID INTEGER, "
			"NotesTotal INT, Subtotal DECIMAL(18,2), DiscountLabor DECIMAL(18,2), DiscountParts DECIMAL(18,2),"
			"DiscountType INT, TicketWritten CHAR, TicketCompleted CHAR, TickedBilled CHAR, Charged INT, "
			"Open INT, ServiceWriter CHAR, CSID INT, Computer INT, DeviceID INT, Invoiced INT, Notes CHAR,"
			"DiscountTotal DECIMAL(18,2), DiscLabor DECIMAL(18,2), DiscParts DECIMAL(18,2));"

			"CREATE TABLE Users (id INTEGER PRIMARY KEY, Name STRING, Password TEXT, Hash TEXT, Admin INT, "
			"Created INT, Enabled INT, LastUsed INT, TotalSales DECIMAL(18,2), TotalRefunds DECIMAL(18,2), "
			"TotalVoids DECIMAL(18,2), CustomersCreated INT, InvoicesCreated INT, InvoicesPaidOut INT, "
			"CanRefund INT, CanCreate INT, CanDeleteTicket INT, UserLevel INT);"

			"CREATE TABLE Telemetry(id INTEGER PRIMARY KEY, Startup INT, Shutdown INT, User STRING, "
			"FreeMem INT, Duplicate INT); "

			"CREATE TABLE Inventory(id INTEGER PRIMARY KEY, Type INT, Count INT, TotalSold CHAR, "
			"LastSold INT, Cost DECIMAL(18,2), Retail DECIMAL(18,2), Discount DECIMAL(18,2), "
			"OnDiscount INT, Make CHAR, Model CHAR, Desc CHAR, InvID CHAR UNIQUE, Supplier CHAR); "

			"CREATE TABLE DeviceType (id INTEGER PRIMARY KEY, Name CHAR, RefID INT, Phone INT, "
			"Computer INT, Laptop INT, AIO INT, TV INT, Console INT, Tablet INT, Other INT);");

		sqlite3_exec(db, SQLString, NULL, NULL, &err);
		if (err)
			GiveError(err, 0);

		sprintf(SQLString,
			"CREATE TABLE Employees(id INTEGER PRIMARY KEY, Zipcode INT, Sales DECIMAL(18,2), "
			"Name CHAR, Phone CHAR, PhoneCell CHAR, PhoneHome CHAR, PhoneSpouce CHAR,"
			"Address CHAR, City CHAR, CurrentlyEmployeed INT);"

			"CREATE TABLE Invoiceref(refkey INTEGER PRIMARY KEY, RefID INT, TicketID INT, "
			"LaborDesc CHAR, LaborCost DECIMAL(18,2), LaborHours INT, PartInvID INT, PartDesc CHAR,"
			"PartCost DECIMAL(18,2), PartRealcost DECIMAL(18,2), csid INT, Pos INT, Quantity INT);");

		sqlite3_exec(db, SQLString, NULL, NULL, &err);
		if (err)
			GiveError(err, 0);

		sprintf(SQLString,
			"CREATE TABLE Computer(id INTEGER PRIMARY KEY, csid INT, DeviceID INT, Make CHAR, "
			"Model CHAR, Ram CHAR, cSpeed CHAR, Windows CHAR, Notes CHAR, TicketTotal INT, "
			"Type CHAR, Total_Spent DECIMAL (18,2), Last_Invoice INT);"

			"CREATE TABLE Device   (id INTEGER PRIMARY KEY, csid INT, DeviceID INT, Make CHAR, "
			"Model CHAR, Ram CHAR, Screen_Size CHAR, Bit INT, cSpeed CHAR, Submodel CHAR, "
			"Color CHAR, Notes CHAR, TicketTotal INT, Type CHAR, Total_Spent DECIMAL (18,2), "
			"Last_Invoice INT);"

			"CREATE TABLE WIP (id INTEGER PRIMARY KEY, csid INT, InvoiceID INT, Started CHAR);"

			"CREATE TABLE POS (id INTEGER PRIMARY KEY, Pay1 CHAR, Pay2 CHAR, Pay3 CHAR, Pay4 CHAR, "
			"Pay5 CHAR, Pay6 CHAR, Pay7 CHAR, Pay8 CHAR, Pay9 CHAR, Pay10 CHAR, StoreNumber INT, "
			"CompanyName CHAR, CompanyMotto CHAR, CompanyAddress CHAR, CompanyZiP CHAR, "
			"CompanyState CHAR, WarrantyInfo CHAR, CompanyPhone CHAR, MU1 INT, MU2 INT, MU3 INT, "
			"MU4 INT, MU5 INT, Tax INT, CompanyCity CHAR, Tax_Labor INT, Tax_Parts INT, Store_ID INT, "
			"Enable_Refunds INT, Password_Refunds INT, Enable_Discounts INT, Max_Discount_Dollar INT, "
			"Max_Discount_Percent INT, DB_Version TEXT, Allow_Markup_Override INT, Admin_Password TEXT, "
			"Refund_Password TEXT, Discount_Password TEXT, Check_For_Updates INT, Debug_Enabled INT, "
			"Text_Logging INT, Window_pos_x INT, Window_pos_y, INT, Window_width INT, Window_Height INT, "
			"Auto_DB_Backup INT,  Text1 TEXT, Text2 TEXT, Text3 TEXT, Text4 TEXT, Text5 TEXT, Text6 TEXT, "
			"Text7 TEXT, Text8 TEXT, Text9 TEXT, Text10 TEXT, Number1 INT, Number2 INT, Number3 INT, "
			"Number4 INT, Number5 INT, Number6 INT, Number7 INT Number8 INT, Number9 INT, Number10 INT, "
			"Logo BLOB);"

			"CREATE TABLE Payments (id INTEGER PRIMARY KEY, Refkey INT, PaymentType CHAR, "
			"PaymentAmount DECIMAL(18,2), PaymentTax DECIMAL(18,2), PaymentTotal DECIMAL(18,2), "
			"PaymentPreTax Decimal(28,2), CCCCV TEXT, CCEXPDAT TEXT, CCZIPCODE TEXT, "
			"Authkey CHAR, CheckNumber CHAR, PayDate CHAR, csid INT, Invoice INT, TaxFree INT);");

		sqlite3_exec(db, SQLString, NULL, NULL, &err);

		if (err)
			GiveError(err, 0);

		sprintf(SQLString, "CREATE TABLE Macros (id INTEGER PRIMARY KEY, Name CHAR, Refkey INT);");

		sqlite3_exec(db, SQLString, NULL, NULL, &err);

		if (err)
			GiveError(err, 0);

		GiveError("Database is created; You will now be shown the setup options. Please fill these out before you continue.", 0);

		sprintf(SQLString, "INSERT INTO POS (CompanyName) VALUES('Company Name');");
		sqlite3_exec(db, SQLString, NULL, NULL, NULL);
		create_setup_window();

		sqlite3_close(db);

		// create the db stuff here.
	}
	if (dbf)
		fclose(dbf);

	dbret = sqlite3_open(DB_FILE, &db);

	if (dbret)
	{
		GiveError("Unable to open DB.", TRUE);
	}

	/*
	sprintf(SQLString, "ALTER TABLE Tickets ADD DiscLabor DECIMAL(18,2);");
	sqlite3_exec(db,SQLString, NULL,NULL,&err);
	if (err)
		GiveError(err,1);
	else
		GiveError("Sql updated.",1);
	*/

	/*	sprintf(SQLString, "SELECT csid,id FROM Payments;");

		sr = sqlite3_prepare(db,SQLString,-1,&stmt,0);

		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				if ((char*)sqlite3_column_text(stmt,0))
				{
					if (get_cs_by_id(atoi((char*)sqlite3_column_text(stmt,0))))
					{
						sprintf(SQLString, "UPDATE Payments SET TaxFree='%d' WHERE id='%s';", get_cs_by_id(atoi((char*)sqlite3_column_text(stmt,0)))->tax_free == TRUE ? "1" : "0",
							(char*)sqlite3_column_text(stmt,1));
						sqlite3_exec(db, SQLString, NULL, NULL, &err);
					}
				}
			}
		}

		sqlite3_finalize(stmt);
		GiveError("Tables updated?",1);
		*/

	sprintf(SQLString, "SELECT * FROM Customers");
	sqlite3_exec(db, SQLString, add_cs_db, NULL, NULL);

	sprintf(SQLString, "SELECT CompanyName, CompanyAddress, CompanyZip, CompanyState, CompanyCity, CompanyPhone, CompanyMotto, WarrantyInfo FROM POS where id='1';");
	sr = sqlite3_prepare(db, SQLString, -1, &stmt, 0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt, 0))
				sprintf(Company_Name, "%s", (char*)sqlite3_column_text(stmt, 0));
			if ((char*)sqlite3_column_text(stmt, 1))
				sprintf(Company_Address, "%s", (char*)sqlite3_column_text(stmt, 1));
			if ((char*)sqlite3_column_text(stmt, 2))
				sprintf(Company_Zip, "%s", (char*)sqlite3_column_text(stmt, 2));
			if ((char*)sqlite3_column_text(stmt, 3))
				sprintf(Company_State, "%s", (char*)sqlite3_column_text(stmt, 3));
			if ((char*)sqlite3_column_text(stmt, 4))
				sprintf(Company_City, "%s", (char*)sqlite3_column_text(stmt, 4));
			if ((char*)sqlite3_column_text(stmt, 5))
				sprintf(Company_Phone, "%s", (char*)sqlite3_column_text(stmt, 5));
			if ((char*)sqlite3_column_text(stmt, 6))
				sprintf(Company_Motto, "%s", (char*)sqlite3_column_text(stmt, 6));
			if ((char*)sqlite3_column_text(stmt, 7))
				sprintf(Company_Warranty, "%s", (char*)sqlite3_column_text(stmt, 7));
		}
	}
	sqlite3_finalize(stmt);

	lastcs = NULL;
	lastinv = NULL;
	lastemp = NULL;

	POS_parent = create_parent("Nanobit Point of Sale system 2011(c)");

	set_parent_config(POS_parent, (HWND)0, (LRESULT*)POS_MainWindow, 0, 0, 740, 768, 0, FALSE, 0, 0);

	AddButton_Parent(POS_parent, "Work In Progress", 0, 0, 90, 90, 0, ID_POS_BTN_WIP, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Estimate", 90, 0, 90, 90, 0, ID_POS_BTN_ESTIMATE, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Customers (F4)", 180, 0, 90, 90, 0, ID_POS_BTN_CUSTOMER, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Quick Sale", 270, 0, 90, 90, 0, ID_POS_BTN_START, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Inventory Managment", 360, 0, 90, 90, 0, ID_POS_BTN_INVENTORY, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Launch Browser", 450, 0, 90, 90, 0, ID_POS_BTN_BROWSER, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Employee Clock in / out", 540, 0, 90, 90, 0, ID_POS_BTN_EMPLOYEE, BS_MULTILINE, TRUE);
	AddButton_Parent(POS_parent, "Setup && Configure", 630, 0, 90, 90, 0, ID_POS_BTN_SETUP, BS_MULTILINE, TRUE);
	AddCList_Parent(POS_parent, "wip", 0, 95, 720, 620, 0, WIP_LIST, LBS_HASSTRINGS | LVS_REPORT, TRUE);
	clist_add_col(POS_parent, "wip", 60, "Invoice #");
	clist_add_col(POS_parent, "wip", 220, "Customer Name");
	clist_add_col(POS_parent, "wip", 160, "Ticket Started");
	clist_add_col(POS_parent, "wip", 160, "Computer Type");
	clist_add_col(POS_parent, "wip", 115, "Current Cost");

	show_parent(POS_parent);

	mainwindow = POS_parent->window_control;

	update_wip();

	sprintf(SQLString, "SELECT MU1, MU2, MU3, MU4, MU5, Tax FROM POS WHERE id='1';");

	sr = sqlite3_prepare(db, SQLString, -1, &stmt, 0);

	TAX = 0.00f;

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt, 0))
				MU1 = (atof((char*)sqlite3_column_text(stmt, 0)) / 100);
			if ((char*)sqlite3_column_text(stmt, 1))
				MU2 = atof((char*)sqlite3_column_text(stmt, 1)) / 100;
			if ((char*)sqlite3_column_text(stmt, 2))
				MU3 = atof((char*)sqlite3_column_text(stmt, 2)) / 100;
			if ((char*)sqlite3_column_text(stmt, 3))
				MU4 = atof((char*)sqlite3_column_text(stmt, 3)) / 100;
			if ((char*)sqlite3_column_text(stmt, 4))
				MU5 = atof((char*)sqlite3_column_text(stmt, 4)) / 100;
			if ((char*)sqlite3_column_text(stmt, 5))
			{
				LOG("Tax: %s", (char*)sqlite3_column_text(stmt, 5));
				TAX = atof((char*)sqlite3_column_text(stmt, 5)) / 100;
				LOG("TAX AFTER: %.2f", TAX);
			}
		}
	}

	sqlite3_finalize(stmt);

	if (setupwindow)
		SetFocus(setupwindow);

	//	CRYPTHASH hash;
	//	hash.hash[0] = '\0';
	//	crypt_password("testpassword", make_salt(),&hash);
	//	GiveError(hash.password, 0);
	//	GiveError(hash.hash, 0);
	//	GiveError(hash.pw_salt, 0);
	//	GiveError(hash.salt, 0);
		//return 0;
	while (1)

	{
		GetMessage(&messages, NULL, 0, 0);
		last_focus = GetFocus();

		if (program_has_focus())
		{
			if ((GetAsyncKeyState(VK_F3) & 0x80000000) && (!(GetAsyncKeyState(VK_MENU) & 0x80000000)))
			{
				if (parent_has_focus(FD_window))
				{
					TranslateMessage(&messages);
					DispatchMessage(&messages);
					continue;
				}

				//	if (cswindow)
					//	clist_clear_list(CS_window, "computer");

				if (csfind != NULL)
				{
					SetFocus(csfind);
				}
				else
				{
					create_cs_find_window();
				}
			}
			if ((GetAsyncKeyState(VK_F1) & 0x80000000) && (!(GetAsyncKeyState(VK_MENU) & 0x80000000)))
			{
				if (parent_has_focus(CALC_window))
				{
					TranslateMessage(&messages);
					DispatchMessage(&messages);
					continue;
				}

				//	if (cswindow)
					//	clist_clear_list(CS_window, "computer");

				if (calcwindow != NULL)
				{
					SetFocus(calcwindow);
				}
				else
				{
					create_calc_window();
				}
			}

			if (GetAsyncKeyState(VK_F4) & 0x80000000 && (!(GetAsyncKeyState(VK_MENU) & 0x80000000)))
			{
				if (parent_has_focus(CS_window))
				{
					TranslateMessage(&messages);
					DispatchMessage(&messages);
					continue;
				}
				if (cswindow)
					SetFocus(cswindow);
				else
					create_cs_window();
				continue;
			}

			if (GetAsyncKeyState(VK_ESCAPE) & MSB)
			{
				if (dailywindow && parent_has_focus(Daily))
				{
					DestroyWindow(dailywindow);
					dailywindow = NULL;
					continue;
				}
				if (cswindow && parent_has_focus(CS_window))
				{
					DestroyWindow(cswindow);
					cswindow = NULL;
					continue;
				}
				if (csfind && parent_has_focus(FD_window))
				{
					DestroyWindow(csfind);
					csfind = NULL;
					continue;
				}
				if (invwindowadd && parent_has_focus(INV_add))
				{
					DestroyWindow(invwindowadd);
					invwindowadd = NULL;
					continue;
				}
				if (invwindowsearch && parent_has_focus(INV_search))
				{
					DestroyWindow(invwindowsearch);
					invwindowsearch = NULL;
					continue;
				}

				if (invwindowticket && parent_has_focus(INV_ticket))
				{
					DestroyWindow(invwindowticket);
					invwindowticket = NULL;
					continue;
				}
				if (laborwindow && parent_has_focus(Labor))
				{
					DestroyWindow(laborwindow);
					laborwindow = NULL;
					continue;
				}
				if (tickwindow && parent_has_focus(TK_window))
				{
					DestroyWindow(tickwindow);
					tickwindow = NULL;
					continue;
				}
				if (setupwindow && parent_has_focus(Setup))
				{
					DestroyWindow(setupwindow);
					setupwindow = NULL;
					continue;
				}

				if (paytotal && parent_has_focus(payments))
				{
					DestroyWindow(paytotal);
					paytotal = NULL;
					continue;
				}
				if (paywindow && parent_has_focus(Pay_window))
				{
					DestroyWindow(paywindow);
					paywindow = NULL;
					continue;
				}

				if (refundwindow && parent_has_focus(Refund))
				{
					DestroyWindow(refundwindow);
					refundwindow = NULL;
					continue;
				}
			}
		}

		if (IsWindow(cmpwindow) && !IsWindow(csfind))
		{
			if (!IsDialogMessage(cmpwindow, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}

		if (IsWindow(cswindow) && !IsWindow(csfind))
		{
			if (!IsDialogMessage(cswindow, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}
		if (IsWindow(csfind))
		{
			if (!IsDialogMessage(csfind, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}
		if (IsWindow(invwindowadd))
		{
			if (!IsDialogMessage(invwindowadd, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}
		if (IsWindow(invwindowsearch))
		{
			if (!IsDialogMessage(invwindowsearch, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}

		if (IsWindow(invwindowticket))
		{
			if (!IsDialogMessage(invwindowticket, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}

		if (IsWindow(laborwindow))
		{
			if (!IsDialogMessage(laborwindow, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}
		if (IsWindow(setupwindow))
		{
			if (!IsDialogMessage(setupwindow, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}

		if (IsWindow(paywindow))
		{
			if (!IsDialogMessage(paywindow, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}
		if (IsWindow(dailywindow))
		{
			if (!IsDialogMessage(dailywindow, &messages))
			{
				TranslateMessage(&messages);
				DispatchMessage(&messages);
				continue;
			}
			continue;
		}

		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	return (int)messages.wParam;
}

void update_wip(void)
{
	char sql[1024];
	char sql2[1024];
	char sql3[1024];
	sqlite3_stmt* stmt;
	sqlite3_stmt* stmt2;
	sqlite3_stmt* stmt3;
	int sr;
	int csid;
	int invid;
	int invref;
	int cmpid;
	char subtotal[2000];
	char date_written[200];
	char cs_name[1024];
	char computer[1024];
	char invoice[1024];
	NWC_CTRL* ctrl;
	LVITEM listItem;
	LVITEM subItem;

	sr = csid = invid = invref = cmpid = 0;

	computer[0] = '\0';

	sprintf(sql, "SELECT CSID, RefID, TicketWritten, SubTotal, id, Computer FROM Tickets WHERE Invoiced=0;");

	sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
	clist_clear_list(POS_parent, "wip");
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			csid = atoi((char*)sqlite3_column_text(stmt, 0));
			//invref = atoi((char*)sqlite3_column_text(stmt,1));
			sprintf(date_written, "%s", (char*)sqlite3_column_text(stmt, 2));
			sprintf(subtotal, "%.2f", atof((char*)sqlite3_column_text(stmt, 3)));
			invid = atoi((char*)sqlite3_column_text(stmt, 4));

			if (sqlite3_column_text(stmt, 5))
			{
				cmpid = atoi((char*)sqlite3_column_text(stmt, 5));
			}

			sprintf(sql2, "SELECT LastName, FirstName FROM Customers WHERE id='%d';", csid);

			sr = sqlite3_prepare(db, sql2, -1, &stmt2, 0);

			if (sr == SQLITE_OK)
			{
				while (sqlite3_step(stmt2) == SQLITE_ROW)
				{
					sprintf(cs_name, "%s, %s", (char*)sqlite3_column_text(stmt2, 0), (char*)sqlite3_column_text(stmt2, 1));
				}
			}
			sqlite3_finalize(stmt2);

			sprintf(sql3, "SELECT Type FROM Computer WHERE id='%d';", cmpid);

			sr = sqlite3_prepare(db, sql3, -1, &stmt3, 0);

			computer[0] = '\0';
			if (sr == SQLITE_OK)
			{
				while (sqlite3_step(stmt3) == SQLITE_ROW)
				{
					sprintf(computer, "%s", (char*)sqlite3_column_text(stmt3, 0));
				}
			}
			sqlite3_finalize(stmt3);

			sprintf(invoice, "%6.6d", invid);

			ctrl = get_control(POS_parent, "wip");

			listItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_DI_SETITEM;
			listItem.state = 0;
			listItem.stateMask = 0;
			listItem.pszText = invoice;
			listItem.iItem = ctrl->clist_index;

			listItem.iSubItem = 0;

			listItem.lParam = (LPARAM)NULL;

			ListView_InsertItem(ctrl->handle, &listItem);

			subItem.mask = LVIF_TEXT;

			subItem.iItem = ctrl->clist_index;
			subItem.iSubItem = 0;
			subItem.pszText = invoice;
			ListView_SetItem(ctrl->handle, &subItem);

			subItem.iItem = ctrl->clist_index;
			subItem.iSubItem = 1;
			subItem.pszText = cs_name;
			ListView_SetItem(ctrl->handle, &subItem);

			subItem.iItem = ctrl->clist_index;
			subItem.iSubItem = 2;
			subItem.pszText = date_written;
			ListView_SetItem(ctrl->handle, &subItem);

			subItem.iItem = ctrl->clist_index;
			subItem.iSubItem = 3;
			subItem.pszText = computer;
			ListView_SetItem(ctrl->handle, &subItem);

			subItem.iItem = ctrl->clist_index;
			subItem.iSubItem = 4;
			subItem.pszText = subtotal;
			ListView_SetItem(ctrl->handle, &subItem);

			ctrl->clist_index++;
		}
	}
	sqlite3_finalize(stmt);
}

LRESULT APIENTRY POS_MainWindow(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//LOG("Hwnd: %d, Msg: %d, wparam: %d, lparam: %d", hwnd,msg, wparam, lparam);
	NMHDR* nm;
	int idx;
	char sql[1024];
	sqlite3_stmt* stmt;
	int i;
	int tid;
	int csid;
	int sr;
	int refid;
	HMENU hmenu, submenu;

	switch (msg)
	{
	case WM_DESTROY:
	{
		//GiveError("Thank you for using Nanobit POS",0);
		walk_heap();
		exit(1);
		break;
	}

	case WM_CREATE:

	{
		hmenu = CreateMenu();
		submenu = CreatePopupMenu();

		AppendMenu(submenu, MF_STRING, ID_FILE_EXIT, "&Exit");
		AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, "&File");
		SetMenu(hwnd, hmenu);

		submenu = CreatePopupMenu();
		AppendMenu(submenu, MF_STRING, ID_REPORT_DAILY, "Print Daily Report");
		AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, "&Reports");
		SetMenu(hwnd, hmenu);

		submenu = CreatePopupMenu();
		AppendMenu(submenu, MF_STRING, ID_FIND_INVOICE, "&Find Invoice");
		AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, "&Invoices");
		SetMenu(hwnd, hmenu);

		break;
	}

	case WM_NOTIFY:
	{
		i = idx = 0;
		nm = (NMHDR*)lparam;
		if (nm->code == NM_DBLCLK)
		{
			idx =(int) SendMessage(get_control(POS_parent, "wip")->handle, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);

			if (idx == -1)
				return 0;

			sprintf(sql, "SELECT id, CSID, RefID FROM Tickets WHERE Invoiced=0;");

			sr = sqlite3_prepare(db, sql, -1, &stmt, 0);

			if (sr == SQLITE_OK)
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					if (i == idx)
					{
						if ((char*)sqlite3_column_text(stmt, 0))
							tid = atoi((char*)sqlite3_column_text(stmt, 0));
						if ((char*)sqlite3_column_text(stmt, 1))
							csid = atoi((char*)sqlite3_column_text(stmt, 1));
						if ((char*)sqlite3_column_text(stmt, 2))
							refid = atoi((char*)sqlite3_column_text(stmt, 2));
						if (!get_cs_by_id(csid))
						{
							GiveError("Invalid CS", 0);
							sqlite3_finalize(stmt);
							break;
						}
						if (tickwindow)
						{
							DestroyWindow(tickwindow);
							tickwindow = NULL;
						}

						create_ticket_window(get_cs_by_id(csid), 0, tid, refid, MODE_NORMAL);
						update_ticket_labor();
						update_ticket_parts();
						break;
					}
					i++;
				}
			}
			sqlite3_finalize(stmt);

			return 0;
		}
		break;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case ID_REPORT_DAILY:
		{
			create_daily_reports();

			break;
		}
		case ID_POS_BTN_INVENTORY:
		{
			if (tickwindow)
			{
				GiveError("Please exit any open tickets before editing the inventory.", 0);
				break;
			}

			if (invwindowsearch)
			{
				SetFocus(invwindowsearch);
			}
			else
			{
				create_inv_search();
			}
			break;
		}

		case ID_POS_BTN_WIP: // Work in Progress button
		{
			//test();
			//create_print_preview (NULL);
//					create_inv_add();
					//create_inv_search();
					//create_inv_ticket ();
					//create_labor_ticket();
			;

			break;
		}
		case ID_POS_BTN_CUSTOMER:
		{
			//	ShowWindow(mainwindow, SW_HIDE);
			if (cswindow)
			{
				SetFocus(cswindow);
			}
			else
			{
				create_cs_window();
			}
			break;
		}
		case ID_POS_BTN_BROWSER:
		{
			//ShellExecute(NULL, "open", "http://www.nanobit.net", NULL, NULL, SW_SHOWNORMAL);
	//		NWC_BoundBox(POS_parent, 5,0);
		//	UpdateWindow(POS_parent->window_pointer);

			break;
		}
		case ID_POS_BTN_SETUP:
		{
			create_setup_window();

			break;
		}
		case ID_FILE_EXIT:
		{
			DestroyWindow(mainwindow);
			break;
		}
		case ID_FIND_INVOICE:
		{
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