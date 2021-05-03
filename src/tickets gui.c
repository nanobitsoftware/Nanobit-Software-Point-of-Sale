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


NWC_PARENT * TK_window;

#define TK_BTN_CHCS 1000
#define TK_BTN_LABOR 1001
#define TK_BTN_PARTS 1002
#define TK_BTN_PREMADE 1003
#define TK_BTN_VIEWCS 1004
#define TK_BTN_CHCMP 1005
#define TK_BTN_BILL 1006
#define TK_BTN_DISCOUNT 1007
#define DI_PERC 1008
#define DI_DOLL 1009
#define DI_ADD 1010
#define DI_CANCEL 1011
#define TK_BTN_PWO 1012
#define PAY_OK 1013
#define PAY_CANCEL 1014
#define PAY_VIEW 1015
#define TK_BTN_REFUND 1016
#define TK_BTN_VIEWPAY 1017


#define TK_LABOR 2000
#define TK_PARTS 2001
#define TK_NOTES 2002
#define REF_ADD 3000
#define REF_CANCEL 3001

CSDATA *cur_cs;
int cur_invoice;
int cur_ref_id;
int cur_cmp_id;
int ticket_mode;

double part_price;
double labor_price;
double total_tax;
double disc_labor;
double disc_parts;
double inv_total;

char invoice_created_date[1024];


double MU1,MU2,MU3,MU4,MU5;


BOOL   disc_type;

int idx_labor;
int idx_parts;

NWC_PARENT * DISC_window;
NWC_PARENT * Pay_window;
NWC_PARENT * payments;
NWC_PARENT * Refund;



extern NWC_PARENT * INV_ticket;
extern NWC_PARENT * Labor;



void create_refund_window (void)
{


	Refund = create_parent("Issue Refund");
	set_parent_config(Refund, (HWND)0,(LRESULT*)RefundWindow_proc, 0 ,0,230,110,0,  FALSE, 0 ,0	  );

	AddStatic_Parent(Refund, "Refund Amount:", 0,0,120,20,0,0,0,TRUE);
	AddEdit_Parent(Refund, "refund", 120,0,80,20,0,0,0,TRUE);
	AddButton_Parent(Refund, "Add", 0,40,80,20,0,REF_ADD,0,TRUE);
	AddButton_Parent(Refund, "Cancel", 120,40,80,20,0,REF_CANCEL,0,TRUE);
	refundwindow = Refund->window_control;
	ShowWindow(refundwindow,SW_SHOW);


}


void create_ticket_window (CSDATA *cs, int cid,int tid, int refid, int mode)
{
//	char frame_str[1024];
	char cs_name[1024];
	char sql[1024];
	char notes[MAXNOTE *2];
	sqlite3_stmt *stmt;
	int sr;

	
	if (!cs && tid <=0)
	{
		GiveError("Cannot start a ticket without a customer selection.",0);
		return;
	}
	
	if(cs_check_for_change(FALSE) == TRUE)
	{

		DestroyWindow(cswindow);
		cswindow = NULL;
	}
	else if (tid <=0)
	{
		return;
	}


	if (tickwindow)
	{
		SetFocus(tickwindow);
		return;
	}
	
	cur_cs = cs;
	cur_invoice = 0;
	cur_ref_id = 0;
	cur_cmp_id =0;
	disc_parts = 0;
	disc_labor = 0;
	disc_type = -1;
	inv_total = 0.00f;
	notes[0] = '\0';
	idx_labor =0;
	idx_parts =0;
	part_price = labor_price = total_tax = 0.00f;

	if (cid >0)
		cur_cmp_id = cid;
	
	sprintf(sql, "SELECT MU1, MU2, MU3, MU4, MU5 FROM POS WHERE id='1';");
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt,0))
				MU1 = (atof((char*)sqlite3_column_text(stmt,0)) / 100);
			if ((char*)sqlite3_column_text(stmt,1))
				MU2 = atof((char*)sqlite3_column_text(stmt,1)) / 100;
			if ((char*)sqlite3_column_text(stmt,2))
				MU3 = atof((char*)sqlite3_column_text(stmt,2)) / 100;
			if ((char*)sqlite3_column_text(stmt,3))
				MU4 = atof((char*)sqlite3_column_text(stmt,3)) / 100;
			if ((char*)sqlite3_column_text(stmt,4))
				MU5 = atof((char*)sqlite3_column_text(stmt,4)) / 100;
				

		}
	}

	ticket_mode = mode;

	sqlite3_finalize(stmt);

	invoice_created_date[0] = '\0';
	if (tid > 0)
	{
		cur_invoice = tid;
		sprintf(sql, "SELECT Computer, Notes, DiscountParts, DiscountLabor, DiscountType, TicketWritten FROM Tickets WHERE id='%d';", tid);
		sr = sqlite3_prepare(db,sql,-1,&stmt,0);

		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				if ((char*)sqlite3_column_text(stmt,0))
				{
					cur_cmp_id = atoi((char*)sqlite3_column_text(stmt,0));
					if ((char*)sqlite3_column_text(stmt,1))
						sprintf(notes, "%s", ((char*)sqlite3_column_text(stmt,1)));
					cid = cur_cmp_id;
					if ((char*)sqlite3_column_text(stmt,2))
					{
						disc_parts = atof((char*)sqlite3_column_text(stmt,2));
					}
					if ((char*)sqlite3_column_text(stmt,3))
					{
						disc_labor = atof((char*)sqlite3_column_text(stmt,3));
					}
					if ((char*)sqlite3_column_text(stmt,4))
					{
						disc_type = atoi((char*)sqlite3_column_text(stmt,4));
					}
					if ((char*)sqlite3_column_text(stmt,5))
					{
						
						sprintf(invoice_created_date, "%s", (char*)sqlite3_column_text(stmt,5));
					}
					else
						sprintf(invoice_created_date, "");
					


				}
				break;

			}
		}
		sqlite3_finalize(stmt);
		if (refid > 0)
			cur_ref_id = refid;


	}

	

	TK_window = create_parent("Ticket editor");
	set_parent_config(TK_window, (HWND)0,(LRESULT*)TKWindow_proc, 0 ,0,850,580,0,  FALSE, 0 ,0	  );
	
	AddButton_Parent(TK_window, "Change Customer", 0,0, 100,40, 0, TK_BTN_CHCS,BS_MULTILINE|BS_CENTER,TRUE);
	AddButton_Parent(TK_window, "Change Computer", 120, 0, 100,40,0,TK_BTN_CHCMP, BS_MULTILINE|BS_CENTER, TRUE);
	AddButton_Parent(TK_window, "Add Labor Ticket", 240,0,100,40,0,TK_BTN_LABOR, BS_MULTILINE|BS_CENTER, TRUE);
	AddButton_Parent(TK_window, "Add Parts Ticket", 360,0,100,40,0,TK_BTN_PARTS, BS_MULTILINE|BS_CENTER, TRUE);
	AddButton_Parent(TK_window, "Ticket Macros", 480,0,100,40,0,TK_BTN_PREMADE,BS_MULTILINE|BS_CENTER,TRUE);
	AddButton_Parent(TK_window, "View Cust. Info", 600,0,100,40,0,TK_BTN_VIEWCS, BS_MULTILINE|BS_CENTER,TRUE);
	AddButton_Parent(TK_window, "Print Work Order", 720,0,100,40,0,TK_BTN_PWO, BS_MULTILINE|BS_CENTER, TRUE);
	
	AddStatic_Parent(TK_window, "frame1", 0,50,820, 20,0,0,WS_BORDER,TRUE);
	AddStatic_Parent(TK_window, "frame2", 0,80,820, 20,0,0,WS_BORDER,TRUE);
	AddStatic_Parent(TK_window, "Labor:", 0,105, 120,20,0,0,0,TRUE);
	AddStatic_Parent(TK_window, "Parts:", 410,105,120,20,0,0,0,TRUE);

	AddCList_Parent(TK_window, "laborlist", 0, 125, 410, 200, 0, TK_LABOR, LVS_REPORT,TRUE);
	AddCList_Parent(TK_window, "partlist", 410,125, 410, 200, 0, TK_PARTS, LVS_REPORT,TRUE);
	clist_add_col(TK_window, "laborlist", 300, "Labor Description");
	clist_add_col(TK_window, "laborlist", 55, "Hours");
	clist_add_col(TK_window, "laborlist", 55, "Sale");
	clist_add_col(TK_window, "partlist", 245, "Parts Description");
	clist_add_col(TK_window, "partlist", 55, "Cost");
	clist_add_col(TK_window, "partlist", 55, "Sale");	
	clist_add_col(TK_window, "partlist", 55, "Inv Cnt");	
	

	AddStatic_Parent(TK_window, "Notes:", 0,330,120,20,0,0,0,TRUE);
	if (mode == MODE_READONLY)
	{
		AddEdit_Parent(TK_window, "notes", 0,350,820,100,0,TK_NOTES,ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|ES_READONLY,TRUE);
	}
	else
	{
		AddEdit_Parent(TK_window, "notes", 0,350,820,100,0,TK_NOTES,ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,TRUE);
	}


	AddStatic_Parent(TK_window, "totals", 0,465,820,20,0,0,WS_BORDER,TRUE);

	AddButton_Parent(TK_window, "Add Discount", 0, 490,100,40,0, TK_BTN_DISCOUNT, BS_MULTILINE|BS_CENTER, TRUE);
	AddButton_Parent(TK_window, "Pay/Closeout Ticket", 720, 490,100,40,0, TK_BTN_BILL, BS_MULTILINE|BS_CENTER, TRUE);
	AddButton_Parent(TK_window, "Issue Refund", 600,490,100,40,0,TK_BTN_REFUND,0,TRUE);
	if (mode == MODE_READONLY)
	{
		AddButton_Parent(TK_window, "View Payments", 480,490, 100,40,0,TK_BTN_VIEWPAY,0,TRUE);
	}
	
	sprintf(cs_name, "%s, %s", cs->last_name, cs->first_name);
	
	CTRL_SetText(TK_window, "frame1", "Customer: %-25s Phone: %13.13s", cs_name, cs->phone);
	update_ticket_totals();
	CTRL_SetText(TK_window, "notes", "");
	CTRL_SetText(TK_window, "frame2", "");
		
	CTRL_ChangeFont(TK_window, "frame1", "Courier New");
	CTRL_ChangeFont(TK_window, "frame2", "Courier New");
	CTRL_ChangeFont(TK_window, "notes", "Courier New");
	CTRL_ChangeFont(TK_window, "totals", "Courier New");
	SendMessage(get_control(TK_window, "notes")->handle, EM_LIMITTEXT, MAXNOTE,0);
	if (notes[0] != '\0')
		CTRL_SetText(TK_window, "notes", "%s", notes);
	sprintf(sql, "SELECT Make, Model, Type FROM Computer WHERE id='%d';", cid);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
				
				CTRL_SetText(TK_window, "frame2", "Computer Make: %-25s Computer Model: %-25s Computer Type: %-25s", 
					(char*)sqlite3_column_text(stmt,0),(char*)sqlite3_column_text(stmt,1),(char*)sqlite3_column_text(stmt,2));
				break;

		}
	}
	sqlite3_finalize(stmt);

	

	show_parent(TK_window);
	tickwindow = TK_window->window_control;


	switch (mode)
	{
	case MODE_READONLY:
		{
			parent_disable_all(TK_window);
			EnableWindow(get_control(TK_window, "notes")->handle,TRUE);
			EnableWindow(get_control(TK_window, "partlist")->handle,TRUE);
			EnableWindow(get_control(TK_window, "laborlist")->handle,TRUE);
			EnableWindow(get_control(TK_window, "View Payments")->handle,TRUE);
			
			break;
		}
	}




}

void prepare_ticket_print(void)
{
	char sql[1024];
	char str[MAXNOTE*4];

	BOOL pfound, lfound;
	sqlite3_stmt *stmt;
	int sr;
	double total;
	BOOL comma;

	comma = FALSE;


	pfound = lfound = FALSE;
	
	if (cur_invoice <= 0)
	{
		GiveError("Cannot print a blank invoice.",0);
		return;
	}
	str[0] = '\0';
	strcat(str, "\bCustomer Name: ");
	strcat(str, cur_cs->last_name);
	strcat(str, ", ");
	strcat(str, cur_cs->first_name);
	strcat(str, "\n\bCustomer Phone: ");
	strcat(str, cur_cs->phone);
	
	sprintf(sql, "SELECT Make, Model, Type, cSpeed, Windows FROM Computer WHERE id='%d';", cur_cmp_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
			{
				strcat(str,"\n\n\bComputer Make: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			}

			
			if (((char*)sqlite3_column_text(stmt,1)))
			{
				strcat(str, "\n\bComputer Model: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			}
			if (((char*)sqlite3_column_text(stmt,2)))
			{
				strcat(str, "\n\bComputer Type: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,2)));
			}
			if (((char*)sqlite3_column_text(stmt,3)))
			{
				strcat(str, "\n\bComputer Speed: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,3)));
			}
			if (((char*)sqlite3_column_text(stmt,4)))
			{
				strcat(str, "\n\bComputer Windows Version: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,4)));
			}




			strcat(str, "\n\n");
			


			
		}
	}
	sqlite3_finalize(stmt);

	
	strcat(str, "\n\n\bParts:\n");

	sprintf(sql, "SELECT PartDesc, Quantity, PartCost FROM InvoiceRef WHERE refid='%d' AND PartDesc NOT NULL;", cur_ref_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			strcat(str, "\033");
			if (((char*)sqlite3_column_text(stmt,1)))
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			strcat(str, "\032");
		if (((char*)sqlite3_column_text(stmt,2)))
			{
				sprintf(sql, "$%.2f", atof((char*) sqlite3_column_text(stmt,2)));
				strcat(str, sql);
			}
			strcat(str,"\n");

			pfound = TRUE;


			
		}
	}
	sqlite3_finalize(stmt);

	if (pfound == FALSE)
	{
		strcat(str, "No Parts Found for Invoice.");
	}
	strcat(str, "\n\n\bLabor:\n");

	sprintf(sql, "SELECT LaborDesc, LaborHours, LaborCost FROM InvoiceRef WHERE refid='%d' AND LaborDesc NOT NULL ;", cur_ref_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			strcat(str, "\033");
			if (((char*)sqlite3_column_text(stmt,1)))
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			strcat(str, "\032");
			if (((char*)sqlite3_column_text(stmt,2)))
			{
				sprintf(sql, "$%.2f", atof((char*) sqlite3_column_text(stmt,2)));
				strcat(str, sql);
			}

			strcat(str, "\n");
			lfound = TRUE;


			
		}
	}
	sqlite3_finalize(stmt);

	if (lfound == FALSE)
	{
		strcat(str, "No Labor Found for Invoice.");
	}

	strcat(str,"\n\n\bNotes:\n");
	sprintf(sql, "SELECT Notes FROM Tickets WHERE id='%d';", cur_invoice);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));


			
		}
	}
	total = 0.00f;
	sqlite3_finalize(stmt);
	sprintf(sql, "\n\n\bPart Subtotal: \032$%.2f\n\bLabor Subtotal: \032$%.2f", part_price, labor_price);
	strcat(str, sql);
	strcat(str,"\n\bDiscount Total");
	sprintf(sql, "\032$%.2f", disc_labor + disc_parts);
	strcat(str,sql);
	strcat(str, "\n\bTax");
	sprintf(sql, "\032$%.2f\n\bInvoice Total (Billed)", total_tax);
	strcat(str, sql);
	sprintf(sql, "\032$%.2f\n", inv_total);
	strcat(str, sql);
	
	sprintf(sql, "SELECT PaymentAmount FROM Payments WHERE Refkey='%d';", cur_ref_id);
	sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt,0))
			{
				total += atof((char*)sqlite3_column_text(stmt,0));
				
			}
		}
	}
	sqlite3_finalize(stmt);
	
	sprintf(sql, "\bAmount Owed \032$%.2f\n", inv_total - total);
	strcat(str,sql);

	if (total > 0.0)
	{
		strcat(str, "\nPayments: ");
		
		sprintf(sql, "SELECT PaymentAmount, PaymentType FROM Payments WHERE Refkey='%d';", cur_ref_id);
		sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				if ((char*)sqlite3_column_text(stmt,0))
				{
					if((char*)sqlite3_column_text(stmt,0))
					{
						if (comma == TRUE)
						{
							strcat(str,", ");
						}
						sprintf(sql,"$%.2f", atof( (char*)sqlite3_column_text(stmt,0)));
						strcat(str,sql);
					}
				}
				if ((char*)sqlite3_column_text(stmt,1))
				{
					sprintf(sql, " via %s", (char*)sqlite3_column_text(stmt,1));
					strcat(str,sql);
					comma = TRUE;
				}

			}
		}
		sqlite3_finalize(stmt);

	}

	strcat(str,"\n");
	


	//	print_work_order(cur_invoice, make_to_col(str));
	LOG(str);
	//create_print_preview(make_to_col(str));
	print_invoice(make_to_col(str),2);
	//draw_text_line(str);


	



}

void do_print_invoice(void)
{
	char sql[1024];
	char str[MAXNOTE*4];

	BOOL pfound, lfound;
	sqlite3_stmt *stmt;
	int sr;
	double total;
	BOOL comma;

	comma = FALSE;


	pfound = lfound = FALSE;
	
	if (cur_invoice <= 0)
	{
		GiveError("Cannot print a blank invoice.",0);
		return;
	}
	str[0] = '\0';
	strcat(str, "\bCustomer Name: ");
	strcat(str, cur_cs->last_name);
	strcat(str, ", ");
	strcat(str, cur_cs->first_name);
	strcat(str, "\n\bCustomer Phone: ");
	strcat(str, cur_cs->phone);
	
	sprintf(sql, "SELECT Make, Model, Type, cSpeed, Windows FROM Computer WHERE id='%d';", cur_cmp_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
			{
				strcat(str,"\n\n\bComputer Make: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			}

			
			if (((char*)sqlite3_column_text(stmt,1)))
			{
				strcat(str, "\n\bComputer Model: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			}
			if (((char*)sqlite3_column_text(stmt,2)))
			{
				strcat(str, "\n\bComputer Type: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,2)));
			}
			if (((char*)sqlite3_column_text(stmt,3)))
			{
				strcat(str, "\n\bComputer Speed: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,3)));
			}
			if (((char*)sqlite3_column_text(stmt,4)))
			{
				strcat(str, "\n\bComputer Windows Version: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,4)));
			}




			strcat(str, "\n\n");
			


			
		}
	}
	sqlite3_finalize(stmt);

	
	strcat(str, "\n\n\bParts:\n");

	sprintf(sql, "SELECT PartDesc, Quantity, PartCost FROM InvoiceRef WHERE refid='%d' AND PartDesc NOT NULL;", cur_ref_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			strcat(str, "\033");
			if (((char*)sqlite3_column_text(stmt,1)))
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			strcat(str, "\032");
		if (((char*)sqlite3_column_text(stmt,2)))
			{
				sprintf(sql, "$%.2f", atof((char*) sqlite3_column_text(stmt,2)));
				strcat(str, sql);
			}
			strcat(str,"\n");

			pfound = TRUE;


			
		}
	}
	sqlite3_finalize(stmt);

	if (pfound == FALSE)
	{
		strcat(str, "No Parts Found for Invoice.");
	}
	strcat(str, "\n\n\bLabor:\n");

	sprintf(sql, "SELECT LaborDesc, LaborHours, LaborCost FROM InvoiceRef WHERE refid='%d' AND LaborDesc NOT NULL ;", cur_ref_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			strcat(str, "\033");
			if (((char*)sqlite3_column_text(stmt,1)))
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			strcat(str, "\032");
			if (((char*)sqlite3_column_text(stmt,2)))
			{
				sprintf(sql, "$%.2f", atof((char*) sqlite3_column_text(stmt,2)));
				strcat(str, sql);
			}

			strcat(str, "\n");
			lfound = TRUE;


			
		}
	}
	sqlite3_finalize(stmt);

	if (lfound == FALSE)
	{
		strcat(str, "No Labor Found for Invoice.");
	}

	strcat(str,"\n\n\bNotes:\n");
	sprintf(sql, "SELECT Notes FROM Tickets WHERE id='%d';", cur_invoice);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));


			
		}
	}
	total = 0.00f;
	sqlite3_finalize(stmt);
	sprintf(sql, "\n\n\bPart Subtotal: \032$%.2f\n\bLabor Subtotal: \032$%.2f", part_price, labor_price);
	strcat(str, sql);
	strcat(str,"\n\bDiscount Total");
	sprintf(sql, "\032$%.2f", disc_labor + disc_parts);
	strcat(str,sql);
	strcat(str, "\n\bTax");
	sprintf(sql, "\032$%.2f\n\bInvoice Total (Billed)", total_tax);
	strcat(str, sql);
	sprintf(sql, "\032$%.2f\n", inv_total);
	strcat(str, sql);
	
	sprintf(sql, "SELECT PaymentAmount FROM Payments WHERE Refkey='%d';", cur_ref_id);
	sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt,0))
			{
				total += atof((char*)sqlite3_column_text(stmt,0));
				
			}
		}
	}
	sqlite3_finalize(stmt);
	
	sprintf(sql, "\bAmount Owed \032$%.2f\n", inv_total - total);
	strcat(str,sql);

	if (total > 0.0)
	{
		strcat(str, "\nPayments: ");
		
		sprintf(sql, "SELECT PaymentAmount, PaymentType FROM Payments WHERE Refkey='%d';", cur_ref_id);
		sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				if ((char*)sqlite3_column_text(stmt,0))
				{
					if((char*)sqlite3_column_text(stmt,0))
					{
						if (comma == TRUE)
						{
							strcat(str,", ");
						}
						sprintf(sql,"$%.2f", atof( (char*)sqlite3_column_text(stmt,0)));
						strcat(str,sql);
					}
				}
				if ((char*)sqlite3_column_text(stmt,1))
				{
					sprintf(sql, " via %s", (char*)sqlite3_column_text(stmt,1));
					strcat(str,sql);
					comma = TRUE;
				}

			}
		}
		sqlite3_finalize(stmt);

	}

	strcat(str,"\n");
	


	//	print_work_order(cur_invoice, make_to_col(str));
	//LOG(str);
	//create_print_preview(make_to_col(str));
	//create_print_preview(make_to_col(str));
	//draw_text_line(str);
	print_invoice(make_to_col(str),2);
}

void do_print_work_order(void)
{
	char sql[1024];
	char str[5000];
	BOOL pfound, lfound;
	sqlite3_stmt *stmt;
	int sr;

	pfound = lfound = FALSE;
	
	if (cur_invoice <= 0)
	{
		GiveError("Cannot print a blank invoice/Work Order.",0);
		return;
	}
	str[0] = '\0';
	strcat(str, "\bCustomer Name: ");
	strcat(str, cur_cs->last_name);
	strcat(str, ", ");
	strcat(str, cur_cs->first_name);
	strcat(str, "\n\bCustomer Phone: ");
	strcat(str, cur_cs->phone);
	
	sprintf(sql, "SELECT Make, Model, Type, cSpeed, Windows FROM Computer WHERE id='%d';", cur_cmp_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
			{
				strcat(str,"\n\n\bComputer Make: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			}

			
			if (((char*)sqlite3_column_text(stmt,1)))
			{
				strcat(str, "\n\bComputer Model: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			}
			if (((char*)sqlite3_column_text(stmt,2)))
			{
				strcat(str, "\n\bComputer Type: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,2)));
			}
			if (((char*)sqlite3_column_text(stmt,3)))
			{
				strcat(str, "\n\bComputer Speed: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,3)));
			}
			if (((char*)sqlite3_column_text(stmt,4)))
			{
				strcat(str, "\n\bComputer Windows Version: ");
				strcat(str, ((char*)sqlite3_column_text(stmt,4)));
			}




			strcat(str, "\n\n");
			


			
		}
	}
	sqlite3_finalize(stmt);

	
	strcat(str, "\n\n\bParts:\n");

	sprintf(sql, "SELECT PartDesc, Quantity, PartCost FROM InvoiceRef WHERE refid='%d' AND PartDesc NOT NULL;", cur_ref_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			strcat(str, "\n    Qty: ");
			if (((char*)sqlite3_column_text(stmt,1)))
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			strcat(str, "\n    Cost: ");
		if (((char*)sqlite3_column_text(stmt,2)))
			{
				sprintf(sql, "$%.2f", atof((char*) sqlite3_column_text(stmt,2)));
				strcat(str, sql);
			}
			strcat(str,"\n");

			pfound = TRUE;


			
		}
	}
	sqlite3_finalize(stmt);

	if (pfound == FALSE)
	{
		strcat(str, "No Parts Found for Invoice.");
	}
	strcat(str, "\n\n\bLabor:\n");

	sprintf(sql, "SELECT LaborDesc, LaborHours, LaborCost FROM InvoiceRef WHERE refid='%d' AND LaborDesc NOT NULL ;", cur_ref_id);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));
			strcat(str, "\n    Est. Hours: ");
			if (((char*)sqlite3_column_text(stmt,1)))
				strcat(str, ((char*)sqlite3_column_text(stmt,1)));
			strcat(str, "\n    Cost: ");
			if (((char*)sqlite3_column_text(stmt,2)))
			{
				sprintf(sql, "$%.2f", atof((char*) sqlite3_column_text(stmt,2)));
				strcat(str, sql);
			}

			strcat(str, "\n");
			lfound = TRUE;


			
		}
	}
	sqlite3_finalize(stmt);

	if (lfound == FALSE)
	{
		strcat(str, "No Labor Found for Invoice.");
	}

	strcat(str,"\n\n\bNotes:\n");
	sprintf(sql, "SELECT Notes FROM Tickets WHERE id='%d';", cur_invoice);
	sr = sqlite3_prepare(db,sql,-1,&stmt,0);

	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if (((char*)sqlite3_column_text(stmt,0)))
				strcat(str, ((char*)sqlite3_column_text(stmt,0)));


			
		}
	}
	
	sqlite3_finalize(stmt);
	sprintf(sql, "\nPart Subtotal: $%.2f\nLabor Subtotal: $%.2f", part_price, labor_price);
	strcat(str, sql);
	strcat(str,"\nDiscount Total: ");
	sprintf(sql, "$%.2f", disc_labor + disc_parts);
	strcat(str,sql);
	strcat(str, "\nTax: ");
	sprintf(sql, "$%.2f\n\bInvoice Total (Billed): ", total_tax);
	strcat(str, sql);
	sprintf(sql, "$%.2f\n", inv_total);
	strcat(str, sql);
	strcat(str, "\n\n");
	strcat(str, "By signing below, you agree to the stated work contained within this work order and that any revisions made will not be made without prior notice to customer.");
	strcat(str, "\nSignature:_________________________________________ Date: ___/___/_______\n");
	

	print_work_order(cur_invoice, make_to_col(str));
	create_print_preview(make_to_col(str));
	//draw_text_line(str);


}

int fill_payments(void *NotUsed, int argc, char **argv, char **azColName)
{

	clist_add_data(payments, "payments", argv, argc);


	return 0;
}
void show_payments(void)
{
	char sql[1024];
	char *err;

	if (paytotal)
		SetFocus(paytotal);

	payments = create_parent("Payments for Current Invoice");
	set_parent_config(payments, (HWND)0,(LRESULT*)PaymentsWindow_proc, CW_USEDEFAULT ,CW_USEDEFAULT,320,370,0,  FALSE, 0 ,0	  );
	AddCList_Parent(payments, "payments", 0,0,300,300,0,0,LVS_REPORT,TRUE);
	
	clist_add_col(payments, "payments", 90,"Payment Amount");
	clist_add_col(payments, "payments", 90,"Payment Type");
	clist_add_col(payments, "payments", 98,"Payment Date");
	
	AddButton_Parent(payments, "OK", 110,300,80,20,0,PAY_CANCEL,0,TRUE);
	paytotal = payments->window_control;
	ShowWindow(paytotal, SW_SHOW);

	clist_clear_list(payments, "payments");
	sprintf(sql, "SELECT PaymentAmount, PaymentType, PayDate FROM Payments WHERE Refkey='%d';", cur_ref_id);
	sqlite3_exec(db, sql, fill_payments, NULL,&err);
	if (err)
		GiveError(err,0);
	return;
	

}
void create_payment(void)
{
	char sql[1024];
	int sr;
	sqlite3_stmt *stmt;
	double total;


	total = fround(0.00f);

	if (paywindow)
		SetFocus(paywindow);

	if (cur_invoice <= 0)
	{
		GiveError("Payment cannot be made to non-existant invoices.",0);
		return;
	}


	Pay_window = create_parent("Invoice Payment");
	set_parent_config(Pay_window, (HWND)0,(LRESULT*)PayWindow_proc, CW_USEDEFAULT ,CW_USEDEFAULT,260,250,0,  FALSE, 0 ,0	  );
	
	AddStatic_Parent(Pay_window, "Payment Date : ", 0,0,100,20,0,0,0,TRUE);
	AddStatic_Parent(Pay_window, "pdate", 100,0,140,20,0,0,0,TRUE);
	
	AddStatic_Parent(Pay_window, "Amount Owed  : ", 0,20,100,20,0,0,0,TRUE);
	AddStatic_Parent(Pay_window, "owed", 100,20,140,20,0,0,0,TRUE);
	
	AddStatic_Parent(Pay_window, "Amount Paid  : ", 0,40,100,20,0,0,0,TRUE);
	AddStatic_Parent(Pay_window, "paid", 100,40,140,20,0,0,0,TRUE);
	
	AddStatic_Parent(Pay_window, "Last Payment : ", 0,60,100,20,0,0,0,TRUE);
	AddStatic_Parent(Pay_window, "ldate", 100,60,140,20,0,0,0,TRUE);
	
	AddStatic_Parent(Pay_window, "Amount to Pay:", 0,100,100,20,0,0,0,TRUE);
	AddEdit_Parent(Pay_window, "pay", 100,100,80,20,0,0,0,TRUE);
	
	AddStatic_Parent(Pay_window, "Payment Type : ", 0,120,100,20,0,0,0,TRUE);
	AddCombo_Parent(Pay_window, "paytype", 100,120,140,20,0,0,CBS_DROPDOWN, TRUE);

	AddStatic_Parent(Pay_window, "Auth/Check # :", 0,140,100,20,0,0,0,TRUE);
	AddEdit_Parent(Pay_window, "auth", 100,140,140,20,0,0,ES_NUMBER,TRUE);

	AddButton_Parent(Pay_window, "Pay", 0,180,80,20,0,PAY_OK,0,TRUE);
	AddButton_Parent(Pay_window, "Cancel", 160,180,80,20,0,PAY_CANCEL,0,TRUE);
	AddButton_Parent(Pay_window, "View", 80,180,80,20,0,PAY_VIEW,0,TRUE);

	sprintf(sql, "SELECT Pay1, Pay2, Pay3, Pay4, Pay5, Pay6, Pay7, Pay8, Pay9, Pay10 FROM POS;");
	sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			
			if ((char*)sqlite3_column_text(stmt,0))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,0));
			if ((char*)sqlite3_column_text(stmt,1))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,1));
			if ((char*)sqlite3_column_text(stmt,2))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,2));
			if ((char*)sqlite3_column_text(stmt,3))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,3));
			if ((char*)sqlite3_column_text(stmt,4))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,4));
			if ((char*)sqlite3_column_text(stmt,5))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,5));
			if ((char*)sqlite3_column_text(stmt,6))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,6));
			if ((char*)sqlite3_column_text(stmt,7))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,7));
			if ((char*)sqlite3_column_text(stmt,8))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,8));
			if ((char*)sqlite3_column_text(stmt,9))
				CTRL_combo_additem(Pay_window, "paytype",  (char*)sqlite3_column_text(stmt,9));
			break;

			
		}
	}
	CTRL_SetText(Pay_window, "ldate", "");
	sqlite3_finalize(stmt);	


	sprintf(sql, "SELECT PaymentAmount, PayDate FROM Payments WHERE Refkey='%d';", cur_ref_id);
	sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			if ((char*)sqlite3_column_text(stmt,0))
			{
				total += atof((char*)sqlite3_column_text(stmt,0));
				CTRL_SetText(Pay_window, "ldate", "%s", (char*)sqlite3_column_text(stmt,1));
			}
		}
	}
	sqlite3_finalize(stmt);
	CTRL_SetText(Pay_window, "paid", "$%.2f", total);
	CTRL_SetText(Pay_window, "pdate", "%s", get_date());
	if (total == 0.00)
	{
		CTRL_SetText(Pay_window, "owed", "$%.2f", inv_total);
	}
	else
	{
		CTRL_SetText(Pay_window, "owed", "$%.2f", inv_total - total);
	}



	paywindow = Pay_window->window_control;
	ShowWindow(paywindow,SW_SHOW);


}


void create_discount_window(void)
{
	if (discountwindow)
	{
		SetFocus(discountwindow);
		return;
	}

	DISC_window = create_parent("Add Discount");
	set_parent_config(DISC_window, (HWND)0,(LRESULT*)DiscWindow_proc, CW_USEDEFAULT ,CW_USEDEFAULT,310,130,0,  FALSE, 0 ,0	  );
	
	AddStatic_Parent(DISC_window, "Discount Type:", 0,0,130,20,0,0,0,TRUE);
	AddRadio_Parent(DISC_window, "Dollar", 130,0,80,20,0,DI_DOLL,0,TRUE);
	AddRadio_Parent(DISC_window, "Percent", 210,0,80,20,0,DI_PERC,0,TRUE);
	AddStatic_Parent(DISC_window, "Labor Discount:",0,40,120,20,0,0,0,TRUE);
	AddStatic_Parent(DISC_window, "Parts Discount:",0,60,120,20,0,0,0,TRUE);

	AddEdit_Parent(DISC_window, "disclabor", 120,40,80,20,0,0,0,TRUE);
	AddEdit_Parent(DISC_window, "discparts", 120,60,80,20,0,0,0,TRUE);

	AddButton_Parent(DISC_window, "Add", 210, 60, 80,20,0, DI_ADD, 0,TRUE);
	AddButton_Parent(DISC_window, "Cancel", 210,40,80,20,0,DI_CANCEL,0,TRUE);

	discountwindow = DISC_window->window_control;
	ShowWindow(discountwindow, SW_SHOW);
	
	
	if (disc_type == DISCOUNT_PERCENT)
	{
		CTRL_SetText(DISC_window, "disclabor", "%.0f", disc_labor);
		CTRL_SetText(DISC_window, "discparts", "%.0f", disc_parts);
		check_set(DISC_window, "Percent");
	}
	else if (disc_type == DISCOUNT_DOLLAR)
	{
		CTRL_SetText(DISC_window, "disclabor", "%.2f", disc_labor);
		CTRL_SetText(DISC_window, "discparts", "%.2f", disc_parts);
		check_set(DISC_window, "Dollar");
	}
	else
	{
		check_set(DISC_window, "Dollar");
		CTRL_SetText(DISC_window, "disclabor", "0");
		CTRL_SetText(DISC_window, "discparts", "0");
	}

}


void update_ticket_totals(void)
{
	char sql[1024];
	char *err;
	double total_discount;
	double sub_total;
	double t_parts;
	double t_labor;
	double discparts,disclabor;


	discparts = disclabor = 0.00f;
	


	
	if (disc_labor == 0 && disc_parts == 0)
	{
		total_discount = 0.00f;
		total_tax = 0.00f;
		if (tax_labor)
			total_tax += fround(labor_price * TAX);
		if (tax_parts)
			total_tax += fround(part_price * TAX);
		if (cur_cs->tax_free == TRUE)
			total_tax = 0;
		sub_total = (part_price + labor_price + total_tax);
	}
	else
	{
		if (disc_type == DISCOUNT_DOLLAR)
		{
			total_discount = (disc_parts + disc_labor);
			discparts = disc_parts;
			disclabor = disc_labor;
			t_parts = part_price - disc_parts;
			t_labor = labor_price - disc_labor;
			total_tax = 0.00f;
			if (tax_labor)
				total_tax += fround(labor_price * TAX);
			if (tax_parts)
				total_tax += fround(part_price * TAX);
			//total_tax = fround(t_parts * TAX);
			if (cur_cs->tax_free == TRUE)
				total_tax = 0;
			sub_total = (total_tax + t_parts) + (t_labor);


		
			
		}
		else if (disc_type == DISCOUNT_PERCENT)
		{
			total_discount = (labor_price * (disc_labor == 0 ? 0 : (disc_labor/100))) + (part_price * (disc_parts == 0 ? 0 : (disc_parts/100)));
			
			discparts = part_price * (disc_parts == 0 ? 0 : (disc_parts/100));
			disclabor = labor_price * (disc_labor == 0 ? 0 : (disc_labor/100));

			t_parts = part_price - (part_price * (disc_parts == 0 ? 0 : (disc_parts/100)));
			t_labor = labor_price - (labor_price * (disc_labor == 0 ? 0 : (disc_labor/100)));
			total_tax = 0.00f;
			if (tax_labor)
				total_tax += fround(labor_price * TAX);
			if (tax_parts)
				total_tax += fround(part_price * TAX);
			//total_tax = fround((t_parts * TAX)) ;
			if (cur_cs->tax_free == TRUE)
				total_tax = 0;

			sub_total = (total_tax + t_parts) + (t_labor);


		}
		else
		{
			total_discount = 0.00f;
			total_tax = 0.00f;
			if (tax_labor)
				total_tax += fround(labor_price * TAX);
			if (tax_parts)
				total_tax += fround(part_price * TAX);
			//total_tax = fround(part_price * TAX );
			if (cur_cs->tax_free == TRUE)
				total_tax = 0;
			sub_total = (part_price + labor_price + total_tax);

		}
	}

	inv_total = fround(sub_total);


	sub_total = fround(sub_total);


			


	CTRL_SetText(TK_window, "totals", "Invoice #: %6.6d Parts: $%-7.2f  Labor: $%-7.2f    Tax: $%-7.2f    Discounts: $%-7.2f    Total: $%-7.2f", cur_invoice, part_price,labor_price, total_tax, total_discount,
		(sub_total));
	if (cur_invoice > 0)
	{

		sprintf(sql, "UPDATE Tickets SET SubTotal = '%.2f', DiscountTotal='%.2f', DiscLabor='%.2f', DiscParts='%.2f' WHERE id='%d';", sub_total, total_discount,disclabor, discparts,cur_invoice);
		sqlite3_exec(db, sql, NULL,NULL, &err);
		if (err)
			GiveError(err,0);
	}
}




BOOL db_new_ticket (void)
{
	char sql[1024];
	sqlite3_stmt *stmt;
	int sr;
	char * err;
	char *notes;

	if (!cur_cs)
	{
		GiveError("A 'ticket' cannot be created without a valid Customer.",0);
		return FALSE;
	}

	notes = CTRL_gettext(TK_window, "notes");

	sqlite3_snprintf(1024,sql, "INSERT INTO Tickets (CSID, TicketWritten, SubTotal, DiscountLabor, DiscountParts, Invoiced, Computer, Notes) VALUES(%d, '%s', '0.00', '0.00', '0.00', '0', '%d','%q');", 
		cur_cs->cs_id, get_date(),cur_cmp_id, !notes ? "" : notes);
	sqlite3_exec(db, sql, NULL,NULL, &err);
	free (notes);
	sprintf(invoice_created_date, "%s", get_date());

	if (err)
	{
		GiveError(err,0);
		return FALSE;
	}
	
	sprintf(sql, "SELECT id FROM Tickets where id = (select max(id) from Tickets);");
	sr = sqlite3_prepare(db, sql,-1,&stmt,0);
	if (sr == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
			cur_invoice = atoi((char*)sqlite3_column_text(stmt,0));
	}
	else
		GiveError("sql error",0);
	


	sqlite3_finalize(stmt);

	cur_cs->total_tickets ++;

	sprintf(sql, "UPDATE Customers SET TotalTickets='%d', LastSeen='%s' WHERE id='%d';", cur_cs->total_tickets,get_date(), cur_cs->cs_id);
	sqlite3_exec(db,sql,NULL,NULL,&err);
	if (err)
		GiveError(err,0);

	if (cur_cmp_id > 0)
	{
		sprintf(sql, "UPDATE Computer SET TicketTotal = TicketTotal+1 WHERE id='%d';", cur_cmp_id);
		sqlite3_exec(db,sql, NULL,NULL,&err);
		if (err)
			GiveError(err,0);
		free (err);
	}


	update_wip();
	update_ticket_totals();



	return TRUE;
}

int fill_ticket_parts(void *NotUsed, int argc, char **argv, char **azColName)
{
	//2
	double part;
	double quant;
	char spart[1024];
	char squant[1024];


	sprintf(spart, "%s", argv[2]);
	sprintf(squant, "%s", argv[3]);

	part = atof(spart);
	quant = atof(squant);
	part_price += (part*quant);



	clist_add_data(TK_window, "partlist", argv, argc);
	return 0;
}
int fill_ticket_labor(void *NotUsed, int argc, char **argv, char **azColName)
{
	double labor;
	char slabor[1024];

	sprintf(slabor, "%s", argv[2]);

	labor = atof(slabor);

	labor_price += labor;



	clist_add_data(TK_window, "laborlist", argv, argc);
	return 0;
}


void update_ticket_labor(void)
{
	char sql[1024];
	char *err;

	if (cur_invoice == 0 || cur_ref_id == 0)
	{
		
		
		return;
	}

	labor_price = 0.00f;

	clist_clear_list(TK_window, "laborlist");

	sprintf(sql, "SELECT LaborDesc, LaborHours, LaborCost FROM InvoiceRef WHERE refid='%d' AND LaborDesc NOT NULL ;", cur_ref_id);
	
	sqlite3_exec(db, sql,fill_ticket_labor,NULL,&err);
	if (err)
	{
		GiveError("Update Ticket parts SQL error:",0);
		GiveError(err,0);
		free(err);
		return;
	}
	update_ticket_totals();

}

void update_ticket_parts(void)
{
	char sql[1024];
	char *err;

	if (cur_invoice == 0 || cur_ref_id == 0)
	{
		
		
		return;
	}

	part_price = total_tax = 0.00f;

	clist_clear_list(TK_window, "partlist");

	sprintf(sql, "SELECT PartDesc, PartRealCost, PartCost, Quantity FROM InvoiceRef WHERE refid='%d' AND PartDesc NOT NULL;", cur_ref_id);
	
	sqlite3_exec(db, sql,fill_ticket_parts,NULL,&err);
	if (err)
	{
		GiveError("Update Ticket parts SQL error:",0);
		GiveError(err,0);
		free(err);
		return;
	}
	total_tax = part_price * TAX;
	update_ticket_totals();



}

void add_labor_inventory(char *desc, char *hours, char *cost)
{
	char sql[2048];
	sqlite3_stmt *stmt;
	int sr;
	int inv_id;
	int cur_cs_id;
	int ref_id;
	char *err;
	char * notes;


	inv_id = -1;
	cur_cs_id = -1;

	sql[0] = '\0';




	if (cur_cs && cur_invoice == 0)
	{
		if (db_new_ticket() == FALSE)
		{
			GiveError("Error creating new ticket database. Exiting program.",1);
		}
		cur_cs_id = cur_cs->cs_id;
	}
	//CREATE TABLE Invoiceref(refkey INTEGER PRIMARY KEY, RefID INT, TicketID INT, LaborDesc CHAR, LaborCost DECIMAL(18,2), LaborHours INT, PartInvID INT, PartDesc CHAR,"
   //"PartCost DECIMAL(18,2), PartRealcost DECIMAL(18,2), csid INT, Pos INT, Quantity INT);");
	sprintf(sql, "INSERT INTO InvoiceRef (TicketID, LaborCost, LaborHours, LaborDesc, csid) VALUES('%d', '%.2f', '%s', '%s', '%d');",
		cur_invoice, atof(cost), hours, desc, cur_cs_id);
	
	sqlite3_exec(db, sql,NULL,NULL,&err);

	if (err)
	{
		GiveError("0",0);
		GiveError(err,0);
		free(err);
		return;
	}
	notes = CTRL_gettext(TK_window, "notes");
	
	if (cur_ref_id == 0)
	{

		sprintf(sql, "SELECT refkey FROM InvoiceRef WHERE refkey = (select max(refkey) from InvoiceRef);");
		sr = sqlite3_prepare(db,sql,-1,&stmt,0);

		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				ref_id = atoi((char*)sqlite3_column_text(stmt,0));
				cur_ref_id = ref_id;
		
			}
		}
		sqlite3_finalize(stmt);
		sprintf(sql, "UPDATE InvoiceRef SET RefID = '%d'  WHERE refkey='%d';", cur_ref_id,cur_ref_id);
		sqlite3_exec(db, sql, NULL,NULL,&err);
		if (err)
		{
			GiveError("1",0);
			GiveError(err,0);
			free(err);
			return;
		}

		
		sqlite3_snprintf(2048,sql, "UPDATE Tickets SET RefID = '%d', Invoice = '%d', Notes='%q' WHERE id='%d';", cur_ref_id, cur_invoice, notes == NULL ? " " : notes,cur_invoice);
		free(notes);
		sqlite3_exec(db,sql, NULL,NULL,&err);
		if (err)
		{
			GiveError("2",0);
			GiveError(err,0);
			free(err);
			return;
		}


	}
	else
	{
		sprintf(sql, "SELECT refkey FROM InvoiceRef WHERE refkey = (select max(refkey) from InvoiceRef);");
		sr = sqlite3_prepare(db,sql,-1,&stmt,0);

		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				ref_id = atoi((char*)sqlite3_column_text(stmt,0));
				//cur_ref_id = ref_id;
			}
		}
		sqlite3_finalize(stmt);
		sprintf(sql, "UPDATE InvoiceRef SET RefID = '%d'  WHERE refkey='%d';", cur_ref_id,ref_id);
		sqlite3_exec(db, sql, NULL,NULL,&err);
		if (err)
		{
			GiveError("3",0);
			GiveError(err,0);
			free(err);
			return;
		}


	}


	
	update_ticket_labor();
	update_ticket_totals();
}


void add_inv_ticket (char * invid, char * desc, char * quant, char * cost, char * retail)
{
	char sql[2048];
	sqlite3_stmt *stmt;
	int sr;
	int inv_id;
	int cur_cs_id;
	int ref_id;
	char *err;
	char *notes;


	inv_id = -1;
	cur_cs_id = -1;

	sql[0] = '\0';



	if (invid)
	{
		// Get the inventory info id thingie
		sprintf(sql, "SELECT id FROM Inventory WHERE InvID='%s';", invid);
		sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
			
				inv_id = atoi( (char*)sqlite3_column_text(stmt,0));
			}
		}
		sqlite3_finalize(stmt);
	}

	if (cur_cs && cur_invoice == 0)
	{
		if (db_new_ticket() == FALSE)
		{
			GiveError("Error creating new ticket database. Exiting program.",1);
		}
		cur_cs_id = cur_cs->cs_id;
	}
	sprintf(sql, "INSERT INTO InvoiceRef (TicketID, PartInvID, PartDesc, PartRealCost, PartCost, csid, Quantity) VALUES('%d', '%d', '%s', '%.2f', '%.2f', '%d', '%d');",
		cur_invoice, inv_id, desc, atof(cost), atof(retail), cur_cs_id, atoi(quant));
	
	sqlite3_exec(db, sql,NULL,NULL,&err);

	if (err)
	{
		GiveError("0",0);
		GiveError(err,0);
		free(err);
		return;
	}
	
	notes = CTRL_gettext(TK_window, "notes");
	

	if (cur_ref_id == 0)
	{

		sprintf(sql, "SELECT refkey FROM InvoiceRef WHERE refkey = (select max(refkey) from InvoiceRef);");
		sr = sqlite3_prepare(db,sql,-1,&stmt,0);

		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				ref_id = atoi((char*)sqlite3_column_text(stmt,0));
				cur_ref_id = ref_id;
		
			}
		}
		sqlite3_finalize(stmt);
		sprintf(sql, "UPDATE InvoiceRef SET RefID = '%d'  WHERE refkey='%d';", cur_ref_id,cur_ref_id);
		sqlite3_exec(db, sql, NULL,NULL,&err);
		if (err)
		{
			GiveError("1",0);
			GiveError(err,0);
			free(err);
			return;
		}

		sqlite3_snprintf(2048,sql, "UPDATE Tickets SET RefID = '%d', Invoice = '%d', Notes='%q' WHERE id='%d';", cur_ref_id, cur_invoice ,!notes ? " " : notes,cur_invoice);
		free (notes);
		sqlite3_exec(db,sql, NULL,NULL,&err);
		if (err)
		{
			GiveError("2",0);
			GiveError(err,0);
			free(err);
			return;
		}


	}
	else
	{
		sprintf(sql, "SELECT refkey FROM InvoiceRef WHERE refkey = (select max(refkey) from InvoiceRef);");
		sr = sqlite3_prepare(db,sql,-1,&stmt,0);

		if (sr == SQLITE_OK)
		{
			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				ref_id = atoi((char*)sqlite3_column_text(stmt,0));
				//cur_ref_id = ref_id;
			}
		}
		sqlite3_finalize(stmt);
		sprintf(sql, "UPDATE InvoiceRef SET RefID = '%d'  WHERE refkey='%d';", cur_ref_id,ref_id);
		sqlite3_exec(db, sql, NULL,NULL,&err);
		if (err)
		{
			GiveError("3",0);
			GiveError(err,0);
			free(err);
			return;
		}


	}

	update_ticket_totals();
	update_ticket_parts();









}


LRESULT APIENTRY DiscWindow_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char *labor;
	char *parts;
	
	char sql[1024];
	char *err;
	
	switch (msg)
	{
	case WM_DESTROY:
		{
			DestroyParent(DISC_window);
			discountwindow = NULL;

			return 0;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wparam))
			{
			case DI_PERC:
			case DI_DOLL:
				{
					break;
				}
			case DI_CANCEL:
				{
					DestroyWindow(discountwindow);
					discountwindow = NULL;
					break;
				}

			case DI_ADD:
				{
					labor = CTRL_gettext(DISC_window, "disclabor");
					parts = CTRL_gettext(DISC_window, "discparts");

					if (!labor && !parts)
					{
						GiveError("Both fields must be filled out before proceeding.",0);
						return 0;
					}

					if (cur_invoice <= 0)
					{
						GiveError("Please create the invoice before adding a discount; a proper invoice is created upon adding a labor or part item.",0);
						return 0 ;
					}

					if (check_get_status(DISC_window, "Dollar"))
					{
						if (labor)
						{
							if (!is_number(labor))
							{
								GiveError("Labor must have a valid number.",0);
								free (labor);
								free (parts);
								return 0;
							}
						}
						if (parts)
						{
							if (!is_number(parts))
							{
								GiveError("Parts must have a valid number.",0);
								free (labor);
								free (parts);
								return 0;
							}
						}
						disc_labor = labor == NULL ? 0 : fround(atof(labor));
						disc_parts = parts == NULL ? 0 : fround(atof(parts));
						disc_type = DISCOUNT_DOLLAR;

						sprintf(sql, "UPDATE Tickets SET DiscountLabor='%.2f', DiscountParts='%.2f', DiscountType='%d' WHERE id='%d';", labor == NULL ? 0.00f : fround(atof(labor)),
							parts == NULL ? 0.00f : fround(atof(parts)), DISCOUNT_DOLLAR, cur_invoice);
						sqlite3_exec(db, sql, NULL,NULL, &err);
						if (err)
							GiveError(err,0);
						update_ticket_totals();
						free (labor);
						free(parts);
						DestroyWindow(discountwindow);
						break;


					
					}
					if (check_get_status(DISC_window, "Percent"))
					{
						if (labor)
						{
							if (!is_number2(labor))
							{
								GiveError("Labor must have a valid whole number.",0);
								free (labor);
								free (parts);
								return 0;
							}
							if (atoi(labor) > 100)
							{
								GiveError("You cannot assign a discount over 100%.",0);
								free(labor);
								free(parts);
								return 0;
							}
						}
						if (parts)
						{
							if (!is_number2(parts))
							{
								GiveError("Parts must have a valid whole number.",0);
								free (labor);
								free (parts);
								return 0;
							}
							if (atoi(parts) > 100)
							{
								GiveError("You cannot assign a discount over 100%.",0);
								free(labor);
								free(parts);
								return 0;
							}
						}
						disc_labor = labor == NULL ? 0 : atoi(labor);
						disc_parts = parts == NULL ? 0 : atoi(parts);
						disc_type = DISCOUNT_PERCENT;
						sprintf(sql, "UPDATE Tickets SET DiscountLabor='%d', DiscountParts='%d', DiscountType='%d' WHERE id='%d';", labor == NULL ? 0 : atoi(labor),
							parts == NULL ? 0 : atoi(parts), DISCOUNT_PERCENT, cur_invoice);
						sqlite3_exec(db, sql, NULL,NULL, &err);
						if (err)
							GiveError(err,0);
						update_ticket_totals();
						free (labor);
						free(parts);
						DestroyWindow(discountwindow);
						break;


					
					}




						
					break;
				}


				break;

			}
			

		}




	
		
	default:
		{
		

			return DefWindowProc (hwnd, msg, wparam, lparam);
			break;
		}
	}
	return DefWindowProc (hwnd, msg, wparam, lparam);

}

LRESULT APIENTRY PayWindow_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char sql[2048];
	char *amount;
	double total;
	char *auth;
	int idx;
	char *pay_type;
	int sr;
	sqlite3_stmt *stmt;
	double paid;
	char *err;

	switch (msg)
	{
	case WM_DESTROY:
		{

			DestroyParent(Pay_window);
			
			paywindow = NULL;
			update_wip();
			return 0;
		}
	case WM_SETFOCUS:
		{
			SetFocus(get_control(Pay_window, "pay")->handle);
			break;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
			case PAY_CANCEL:
				{
					DestroyWindow(paywindow);
					break;
				}
				break;
			case PAY_VIEW:
				{
					if (cur_invoice <=0)
					{
						GiveError("Cannot add payment to a non-existant invoice.",0);
						break;
					}
					show_payments();
					break;
				}
			case PAY_OK:
				{
					if (cur_invoice <=0)
					{
						GiveError("Cannot add payment to a non-existant invoice.",0);
						break;
					}
					

					idx = CTRL_combo_get_sel_idx(Pay_window, "paytype");
					if (idx < 0)
					{
						GiveError("Please select a payment type first.",0);
						break;
					}
					
					
					amount = CTRL_gettext(Pay_window, "pay");
					if (!amount)
					{
						GiveError("You must provide a valid payment amount.",0);
						break;
					}
					if (!is_number(amount))
					{
						GiveError("You must provide a valid payment amount.",0);
						free(amount);
						break;
					}
					if ((fround(atof(amount)) <= .00))
					{
						GiveError("You must provide a valid payment amount.",0);
						free (amount);
						break;
					}
					auth = CTRL_gettext(Pay_window, "auth");

					total = fround(atof(amount));

					if (total > inv_total)
					{
						GiveError("Over charge of invoice is not permitted1.",0);
						
						free (auth);
						free (amount);
						break;
					}

					paid = 0.00f;

					sprintf(sql, "SELECT PaymentAmount FROM Payments WHERE Refkey='%d';", cur_ref_id);
					sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
					if (sr == SQLITE_OK)
					{
						while (sqlite3_step(stmt) == SQLITE_ROW)
						{
							if ((char*)sqlite3_column_text(stmt,0))
							{
								paid += (atof((char*)sqlite3_column_text(stmt,0)));
							}
						}
					}
					sqlite3_finalize(stmt);

					if ((fround(paid) + fround(total)) > fround(inv_total))
					{
						GiveError("Total combined payments are greater than current invoice subtotal. Please re-check amount due.",0);
						
						free (auth);
						free (amount);
						break;
					}

					pay_type = combo_get_text (Pay_window, "paytype",idx);
// "CREATE TABLE Payments (id INTEGER PRIMARY KEY, Refkey INT, PaymentType CHAR, PaymentAmount DECIMAL(18,2), Authkey CHAR, CheckNumber CHAR, PayDate CHAR);");

					sprintf(sql, "INSERT INTO Payments (RefKey, PaymentType, PaymentAmount, Authkey, PayDate, csid, Invoice, TaxFree) VALUES('%d', '%s', '%.2f', '%s', '%s', '%d','%d','%d');",
						cur_ref_id, pay_type, total, auth == NULL ? "" : auth, get_date(), cur_cs->cs_id,cur_invoice, cur_cs->tax_free == TRUE ? 1 : 0);
					sqlite3_exec(db, sql, NULL,NULL,&err);
					if (err)
						GiveError(err,0);
					if ((fround(paid) + fround(total))  < fround(inv_total))
					{
						
						if (MessageBoxEx(tickwindow, "Payment posted, but a balance still exists; Post another payment?", "Bill Invoice", MB_YESNO, 0) == IDNO)
						{
							free (auth);
							free (amount);
							DestroyWindow(paywindow);
							break;
						}
						else
						{
							//update the window
							total = 0.00f;
							sprintf(sql, "SELECT PaymentAmount, PayDate FROM Payments WHERE Refkey='%d';", cur_ref_id);
							sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
							if (sr == SQLITE_OK)
							{
								while (sqlite3_step(stmt) == SQLITE_ROW)
								{
									if ((char*)sqlite3_column_text(stmt,0))
									{
										total += atof((char*)sqlite3_column_text(stmt,0));
										CTRL_SetText(Pay_window, "ldate", "%s", (char*)sqlite3_column_text(stmt,1));
									}
								}
							}
							sqlite3_finalize(stmt);
							CTRL_SetText(Pay_window, "paid", "$%.2f", total);
							CTRL_SetText(Pay_window, "pdate", "%s", get_date());
							if (total == 0)
							{
								CTRL_SetText(Pay_window, "owed", "$%.2f", inv_total);
							}
							else
							{
								CTRL_SetText(Pay_window, "owed", "$%.2f", inv_total - total);
							}

							SetFocus(get_control(Pay_window, "pay")->handle);
							break;
							free (auth);
							free(amount);
						}

					}
					else
					{
						if (MessageBoxEx(tickwindow, "Invoice Paid in Full; Close-out ticket now?", "Close out Ticket", MB_YESNO,0) == IDNO)
						{
							free (auth);
							free (amount);
							DestroyWindow(paywindow);
							break;
						}
						else
						{
							do_print_invoice();
							DestroyWindow(paywindow);
							sprintf(sql, "UPDATE Tickets SET Invoiced='1' WHERE id='%d';", cur_invoice);
							sqlite3_exec(db, sql, NULL,NULL, &err);
							if (err)
								GiveError(err,1);
							
							cur_cs->total_spent += inv_total;
							sprintf(sql, "UPDATE Customers SET TotalSpent='%.2f' WHERE id='%d';", cur_cs->total_spent,cur_cs->cs_id);
							sqlite3_exec(db,sql,NULL,NULL,&err);
							if(err)
								GiveError(err,0);

							DestroyWindow(tickwindow);
							free (auth);
							free(amount);
							break;
						}
					}




					
				}





			}
			break;
		}


		
	default:
		{
		

			return DefWindowProc (hwnd, msg, wparam, lparam);
			break;
		}
	}
	return DefWindowProc (hwnd, msg, wparam, lparam);

}




LRESULT APIENTRY TKWindow_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char sql[MAXNOTE * 2];
	char *err;
	char *notes;
	NMHDR *nm;
	NMLVKEYDOWN *nk;

	sqlite3_stmt *stmt;
	int sr;
	int i;
	int idx;
	double total_paid;

	total_paid = 0.00f;


	switch (msg)
	{
	
	
	case WM_NOTIFY:
		{
			
			
						

			nm = (NMHDR*)lparam;
			if (!nm)
				return 0;
			if (nm->code == LVN_KEYDOWN)
			{
				nk = (LPNMLVKEYDOWN) lparam; 
				if (nk->wVKey == VK_DELETE)
				{
					switch (nm->idFrom)
					{
					case TK_LABOR:
						{
							i=0;
							idx = -1;
							idx_labor = SendMessage(get_control(TK_window, "laborlist")->handle,LVM_GETNEXTITEM,(WPARAM)-1,(LPARAM)LVNI_SELECTED);
							if (idx_labor == -1)
								break;
							

							sprintf(sql, "SELECT  refkey FROM InvoiceRef WHERE refid='%d' AND LaborDesc NOT NULL;", cur_ref_id);

							sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
							if (sr == SQLITE_OK)
							{
								while (sqlite3_step(stmt) == SQLITE_ROW)
								{
									if (i == idx_labor)
									{
							
										if ((char*)sqlite3_column_text(stmt,0))
											idx = atoi((char*)sqlite3_column_text(stmt,0));

										break;
									}
									i++;
									
								}
							}
							sqlite3_finalize(stmt);

							if (idx == -1)
							{
								GiveError("Error deleting ticket item.",0);
								return 0;
							}

							
							sprintf(sql, "DELETE FROM InvoiceRef WHERE refkey='%d';", idx);
							sqlite3_exec(db, sql, NULL,NULL,&err);
							if(err)
								GiveError(err,0);
							free(err);
							update_ticket_labor();
							update_ticket_totals();



							break;
						}
						
					case TK_PARTS:
						{
							i=0;
							idx = -1;
							idx_labor = SendMessage(get_control(TK_window, "partlist")->handle,LVM_GETNEXTITEM,(WPARAM)-1,(LPARAM)LVNI_SELECTED);
							if (idx_labor == -1)
								break;
							

							sprintf(sql, "SELECT  refkey FROM InvoiceRef WHERE refid='%d' AND PartDesc NOT NULL;", cur_ref_id);

							sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
							if (sr == SQLITE_OK)
							{
								while (sqlite3_step(stmt) == SQLITE_ROW)
								{
									if (i == idx_labor)
									{
							
										if ((char*)sqlite3_column_text(stmt,0))
											idx = atoi((char*)sqlite3_column_text(stmt,0));

										break;
									}
									i++;
									
								}
							}
							sqlite3_finalize(stmt);

							if (idx == -1)
							{
								GiveError("Error deleting ticket item.",0);
								return 0;
							}

							
							sprintf(sql, "DELETE FROM InvoiceRef WHERE refkey='%d';", idx);
							sqlite3_exec(db, sql, NULL,NULL,&err);
							if(err)
								GiveError(err,0);
							free(err);
							update_ticket_parts();
							update_ticket_totals();



							break;
						}
					}
				}


				break;
			}
			
			if (nm->code == NM_DBLCLK)
			{
				switch (nm->idFrom)
				{


				case TK_LABOR:
					{
						i=0;
						idx_labor = SendMessage(get_control(TK_window, "laborlist")->handle,LVM_GETNEXTITEM,(WPARAM)-1,(LPARAM)LVNI_SELECTED);
						if (idx_labor == -1)
							break;


						create_labor_ticket();
						CTRL_SetText(Labor, "Add", "Update");
						


						sprintf(sql, "SELECT  LaborDesc, LaborHours, LaborCost, refkey FROM InvoiceRef WHERE refid='%d' AND LaborDesc NOT NULL;", cur_ref_id);
						
						sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
						if (sr == SQLITE_OK)
						{
							while (sqlite3_step(stmt) == SQLITE_ROW)
							{
								if (i == idx_labor)
								{
									if ((char*)sqlite3_column_text(stmt,0))
										CTRL_SetText(Labor, "desc", "%s", (char*)sqlite3_column_text(stmt,0));
									if ((char*)sqlite3_column_text(stmt,1))
										CTRL_SetText(Labor, "hours", "%s", (char*)sqlite3_column_text(stmt,1));
									if ((char*)sqlite3_column_text(stmt,2))
										CTRL_SetText(Labor, "cost", "%s", (char*)sqlite3_column_text(stmt,2));
									if ((char*)sqlite3_column_text(stmt,3))
										idx_labor = atoi((char*)sqlite3_column_text(stmt,3));

									break;
								}
								i++;


							}
						}
						sqlite3_finalize(stmt);

						break;
					}
				case TK_PARTS:
					{
						i=0;
						idx_parts = SendMessage(get_control(TK_window, "partlist")->handle,LVM_GETNEXTITEM,(WPARAM)-1,(LPARAM)LVNI_SELECTED);
						if (idx_parts == -1)
							break;


						create_inv_ticket();
						CTRL_SetText(INV_ticket, "Add", "Update");
						EnableWindow(get_control(INV_ticket, "Search Inventory")->handle, FALSE);


						sprintf(sql, "SELECT PartInvID, PartDesc, PartRealCost, PartCost, Quantity, refkey FROM InvoiceRef WHERE refid='%d' AND PartDesc NOT NULL;", cur_ref_id);
						
						sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
						if (sr == SQLITE_OK)
						{
							while (sqlite3_step(stmt) == SQLITE_ROW)
							{
								if (i == idx_parts)
								{
									if ((char*)sqlite3_column_text(stmt,0))
										CTRL_SetText(INV_ticket, "invid", "%s", (char*)sqlite3_column_text(stmt,0));
									if ((char*)sqlite3_column_text(stmt,1))
										CTRL_SetText(INV_ticket, "desc", "%s", (char*)sqlite3_column_text(stmt,1));
									if ((char*)sqlite3_column_text(stmt,2))
										CTRL_SetText(INV_ticket, "cost", "%s", (char*)sqlite3_column_text(stmt,2));
									if ((char*)sqlite3_column_text(stmt,3))
										CTRL_SetText(INV_ticket, "retail", "%s", (char*)sqlite3_column_text(stmt,3));
									if ((char*)sqlite3_column_text(stmt,4))
										CTRL_SetText(INV_ticket, "quant", "%s", (char*)sqlite3_column_text(stmt,4));
									if ((char*)sqlite3_column_text(stmt,5))
										idx_parts = atoi((char*)sqlite3_column_text(stmt,5));

									break;
								}
								i++;


							}
						}
						sqlite3_finalize(stmt);

						break;
					}

				}
			
				//idx = SendMessage(get_control(INV_search, "invlist")->handle,LVM_GETNEXTITEM,(WPARAM)-1,(LPARAM)LVNI_SELECTED);


	

				return 0;
			}
			break;
		}

	case WM_DESTROY:
		{
			notes = CTRL_gettext(TK_window, "notes");

			if (cur_invoice > 0)
			{
				sqlite3_snprintf(MAXNOTE * 2, sql,  "UPDATE Tickets SET Notes = '%q' WHERE id='%d';", !notes ? "" : notes, cur_invoice);
				
				sqlite3_exec(db, sql, NULL,NULL,&err);
				if (err)
					GiveError(err,0);

			}
			free(notes);

			if (laborwindow)
			{
				DestroyWindow(laborwindow);
				laborwindow = NULL;
			}
			if (invwindowticket)
			{
				DestroyWindow(invwindowticket);
				invwindowticket = NULL;
			}
			if (discountwindow)
			{
				DestroyWindow(discountwindow);
				discountwindow = NULL;
			}
			

			DestroyParent(TK_window);
			cur_cs = NULL;
			tickwindow = NULL;
			update_wip();
			return 0;
		}


	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
			case TK_BTN_REFUND:
				{
					if (cur_invoice == 0)
					{
						GiveError("Cannot add a refund to a non-existant invoice.",0);
						break;
					}

					create_refund_window();
					break;
				}
			case TK_BTN_LABOR:
				{
					create_labor_ticket();
					break;
				}
				break;
			case TK_BTN_PARTS:
				{
					create_inv_ticket ();
					break;
				}
			case TK_BTN_DISCOUNT:
				{
					create_discount_window();
					break;
				}
			case TK_BTN_VIEWCS:
				{
					fill_cs(cur_cs);
					break;
				}
			case TK_BTN_PWO:
				{
					//create_print_preview(0);
					
					do_print_work_order();
					//prepare_ticket_print();
					break;
				}
			case TK_BTN_VIEWPAY:
				{
					show_payments();
					break;
				}
			case TK_BTN_BILL:
				{
					if (cur_invoice <= 0)
					{
						GiveError("Cannot bill out a non-exsiting invoice.",0);
						break;
					}
					
					total_paid = 0.00f;
					sprintf (sql, "SELECT PaymentAmount FROM Payments where RefKey='%d';", cur_ref_id);
					sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
					if (sr == SQLITE_OK)
					{
						while (sqlite3_step(stmt) == SQLITE_ROW)
						{
							if ((char*)sqlite3_column_text(stmt,0))
								total_paid += atof((char*)sqlite3_column_text(stmt,0));
						}


					}
					sqlite3_finalize(stmt);

					if (fround(total_paid) == fround(inv_total))
					{
						if (MessageBoxEx(tickwindow, "Ticket paid in full; close out ticket now?", "Close out Ticket", MB_YESNO,0) == IDNO)
						{
							create_payment();
							break;
						}
						
						do_print_invoice();
						
						sprintf(sql, "UPDATE Tickets SET Invoiced='1' WHERE id='%d';", cur_invoice);
						sqlite3_exec(db, sql, NULL,NULL, &err);
						if (err)
							GiveError(err,1);
						cur_cs->total_spent += total_paid;
						sprintf(sql, "UPDATE Customers SET TotalSpent='%.2f' WHERE id = '%d';", cur_cs->total_spent,cur_cs->cs_id);
						sqlite3_exec(db,sql,NULL,NULL,&err);
						if(err)
							GiveError(err,0);
						DestroyWindow(tickwindow);

						break;
					}




					if (MessageBoxEx(tickwindow, "Do you really wish to bill-out this invoice?", "Bill Invoice", MB_YESNO, 0) == IDNO)
					{
						break;
					}
					create_payment();



					break;
				}

			}
			break;
		}
	case WM_KILLFOCUS:
		{
			notes = CTRL_gettext(TK_window, "notes"); 
			
			if (cur_invoice > 0)
			{
				sqlite3_snprintf(MAXNOTE * 2, sql,  "UPDATE Tickets SET Notes = '%q' WHERE id='%d';", !notes ? "" : notes, cur_invoice);
				sqlite3_exec(db, sql, NULL,NULL,&err);
				if (err)
					GiveError(err,0);

			}
			free(notes);


			
			break;
		}
		
	default:
		{
		

			return DefWindowProc (hwnd, msg, wparam, lparam);
			break;
		}
	}
	return DefWindowProc (hwnd, msg, wparam, lparam);

}




LRESULT APIENTRY PaymentsWindow_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	switch (msg)
	{
	case WM_DESTROY:
		{
			DestroyParent(payments);
			
			paytotal = NULL;
			
			return 0;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
			case PAY_CANCEL:
				{
					DestroyWindow(paytotal);
					break;
				}
				break;
			}
			break;
		}


		
	default:
		{
		

			return DefWindowProc (hwnd, msg, wparam, lparam);
			break;
		}
	}
	return DefWindowProc (hwnd, msg, wparam, lparam);

}

LRESULT APIENTRY RefundWindow_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	char *total;
	double total_paid;
	char sql[1024];
	sqlite3_stmt *stmt;
	int sr;
	char *err;

	switch (msg)
	{

	case WM_DESTROY:
		{
			DestroyParent(Refund);

			refundwindow = NULL;

			return 0;
		}
	case WM_SETFOCUS:
		{
			SetFocus(get_control(Refund, "refund")->handle);
			break;
		}

	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
			case REF_CANCEL:
				{
					DestroyWindow(refundwindow);
					return 0;
				}
				break;
			case REF_ADD:
				{
					
					total_paid = 0.00f;
					total = CTRL_gettext(Refund, "refund");
					if (!total)
					{
						GiveError("You must supply an ammount to refund.",0);
						free(total);
						break;
					}
					if (!is_number(total))
					{
						GiveError("You must supply a valid number for the refund.",0);
						free (total);
						break;
					}
					sprintf (total, "%.2f", fround(atof(total)));

					sprintf (sql, "SELECT PaymentAmount FROM Payments where RefKey='%d';", cur_ref_id);
					sr = sqlite3_prepare(db, sql, -1, &stmt, 0);
					if (sr == SQLITE_OK)
					{
						while (sqlite3_step(stmt) == SQLITE_ROW)
						{
							if ((char*)sqlite3_column_text(stmt,0))
								total_paid += atof((char*)sqlite3_column_text(stmt,0));
						}


					}
					sqlite3_finalize(stmt);
					
					if (fround(atof(total)) > fround(total_paid))
					{
						GiveError("Refund exceeds amount paid on invoice. Cannot over-refund an amount.",0);
						free(total);
						break;
					}


					sprintf (sql, "INSERT INTO Payments (Refkey, PaymentType, PaymentAmount, AuthKey, CheckNumber, PayDate, csid, Invoice) VALUES('%d', 'REFUND', '%.2f', 'REFUND', 'REFUND','%s', '%d', '%d');",
						cur_ref_id, -atof(total), get_date(), cur_cs->cs_id, cur_invoice);
					sqlite3_exec(db, sql, NULL,NULL, &err);
					if (err)
						GiveError(err,0);
					free(err);
					free (total);
					DestroyWindow(refundwindow);
				}
				break;
			}
			break;
		}



	default:
		{


			return DefWindowProc (hwnd, msg, wparam, lparam);
			break;
		}
		
	

	
	}
		return DefWindowProc (hwnd, msg, wparam, lparam);
}

