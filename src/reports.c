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


#define MONTHCAL 1001

HWND usrh;
HWND cal;

#define DAILY_CAL 1000
#define DAILY_PRINT 1001

NWC_PARENT *callback_parent;
char *	callback_child;

NWC_PARENT *Daily;

void get_user_date(NWC_PARENT *p, char *c, int x, int y)
{
	
	
	WNDCLASS wc;
	RECT rc={0,0,0,0};
	
	if (!p || !c)
		return;

	callback_parent = p;
	callback_child = c;


	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground =(HBRUSH) (COLOR_WINDOW );
	wc.lpfnWndProc = (WNDPROC)UsrDate_proc;
	wc.lpszClassName = "Date Picker";
	wc.lpszMenuName = NULL;
	wc.hInstance =g_hInst;
	wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.style = CS_DBLCLKS;
	RegisterClass(&wc);



	
	//set_parent_config(usrdate, (HWND)0,(LRESULT*)UsrDate_proc, CW_USEDEFAULT ,CW_USEDEFAULT,800,800,0,  FALSE, 0 ,0	  );
	usrh = CreateWindowEx(0, "Date Picker", "Date Picker",WS_POPUP | WS_BORDER,x,y,400,400, mainwindow, 0, g_hInst, 0);
	
	ShowWindow(usrh, SW_SHOW);
	cal = CreateWindowEx(0, MONTHCAL_CLASS,0, WS_BORDER | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE,rc.left,rc.top,rc.right,rc.bottom,usrh, (HMENU)MONTHCAL, g_hInst,0);

	MonthCal_GetMinReqRect(cal, &rc);
	SetWindowPos(cal, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	SetWindowPos(usrh, NULL, x,y,rc.right,rc.bottom, SWP_NOZORDER);
	ShowWindow(cal, SW_SHOW);


	

	return;
}

void create_daily_reports(void)
{
	if (dailywindow)
	{
		SetFocus(dailywindow);
		return;
	}

	Daily = create_parent("Choose date for Daily report.");
	set_parent_config(Daily, (HWND)0,(LRESULT*)DailyWindow_proc, 0 ,0,400,90,0,  FALSE, 0 ,0	  );
	AddStatic_Parent(Daily, "Choose date:", 0,0, 120,20,0,0,0,TRUE);
	AddEdit_Parent(Daily, "range", 120,0,120,20,0,0,0,TRUE);
	AddButton_Parent(Daily, "Calendar",250,0,80,20,0,DAILY_CAL, 0,TRUE); 
	AddButton_Parent(Daily, "Print Report", 120,25,120,20,0,DAILY_PRINT,0,TRUE);
	dailywindow = Daily->window_control;
	CenterWindow(dailywindow,mainwindow);
	ShowWindow(dailywindow, SW_SHOW);

	SendMessage(get_control(Daily, "range")->handle, EM_LIMITTEXT, 10,0);
	CTRL_SetText(Daily, "range", get_date_notime());


}

LRESULT APIENTRY DailyWindow_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{


	RECT r;
	NWC_CTRL *button;
	char str[1024],sql[1024],cs_name[1024],pay_type[1024],pay_date[1024],pay_tax[1024],labor_desc[1024],part_desc[1024];
	char delim[] = "/";

	char *report,*laborpart,*part,*discount,*range;
	
	sqlite3_stmt *stmt1, *stmt2;
	
	int sr1,sr2,refkey,csid,last_refkey,inv_num,labor_hours,part_count,cs_id;

	double pay_total;

	double part_cost,grand_total,total_tax,total_refund,discount_labor;
	double discount_parts,total_discount,grand_discount, grand_discparts, grand_disclabor,total_parts;

	BOOL tax_free,found,part_ran,labor_ran,correct_date;

	
	
	switch (msg)
	{
	case WM_DESTROY:
		{
			DestroyParent(Daily);
			dailywindow = NULL;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wparam))
			{
			case DAILY_CAL:
				{
					
					ShowWindow(dailywindow, SW_HIDE);
					GetWindowRect(dailywindow, &r);
					button = get_control(Daily, "range");
					r.left += button->x;
					r.top += button->y;

					get_user_date(Daily, "range", r.left, r.top);
					break;
				}
			case DAILY_PRINT:
				{

					refkey = csid = 0;
					pay_total = 0.00f;
					cs_name[0] = '\0';
					pay_type[0] = '\0';
					pay_tax[0] = '\0';
					correct_date = TRUE;
					pay_date[0] = '\0';
					cs_id = part_count = 0;
					inv_num = 0;
					total_discount = grand_total = total_tax = total_refund = part_cost = discount_parts = discount_labor = fround(0.00f);
					grand_discount = grand_discparts = grand_disclabor=0.00f;
					total_parts = 0.00f;
					range = CTRL_gettext(Daily, "range");
					found = FALSE;
					if (!range)
					{
						GiveError("Invalid date.", 0);
						free (range);
						break;
					}

					correct_date = is_date(range);



					if (correct_date == FALSE)
					{
						GiveError("You must enter a correct date in the mm/dd/yyyy format. (02/26/1984)", 0);
						free (range);
						break;
					}


					report = malloc(sizeof(char*) * MAXNOTE * 10);
					laborpart = malloc(sizeof(char*) * MAXNOTE * 10);
					part = malloc(sizeof(char*) * MAXNOTE * 10);
					discount = malloc(sizeof(char*) * MAXNOTE * 10);
					if (!report)
						GiveError("Error allocating memory for report. Aborting program; please try again.",1);

					report[0] = '\0';
					laborpart[0] = '\0';
					part[0] = '\0';
					discount[0] = '\0';

					sprintf(report, "\nDaily report for %s - Printed: %s\n", range, get_date_notime());
					sprintf(str, "%-25.25s %-7.7s %-15.15s %-10.10s %-20.20s\n---------------------------------------------------------------------------------------\n\n", 
						"Customer Name", "Invoice", "Payment Type", "Total", "Payment Date");
					strcat(report,str);
					
					sprintf(sql, "SELECT RefKey, PayDate, PaymentType, csid, PaymentAmount, Invoice, TaxFree"
						" FROM Payments WHERE PayDate LIKE '%s%%';", range);

					sr1 = sqlite3_prepare(db, sql, -1, &stmt1, 0);

					last_refkey = 0;
					if (sr1 == SQLITE_OK)
					{
						while (sqlite3_step(stmt1) == SQLITE_ROW)
						{
							found = TRUE;
							tax_free = FALSE;
							part_ran = labor_ran = FALSE;
							
							if ((char*)sqlite3_column_text(stmt1,0))
								refkey = atoi((char*)sqlite3_column_text(stmt1,0));
							if ((char*)sqlite3_column_text(stmt1,1))
								sprintf(pay_date, "%s", (char*)sqlite3_column_text(stmt1,1));
							if ((char*)sqlite3_column_text(stmt1,2))
								sprintf(pay_type, "%s", (char*)sqlite3_column_text(stmt1,2));

							if ((char*)sqlite3_column_text(stmt1,3))
								cs_id = atoi( (char*)sqlite3_column_text(stmt1,3));
							if ((char*)sqlite3_column_text(stmt1,4))
							{
								pay_total = atof( (char*)sqlite3_column_text(stmt1,4));
								
								if (!strcmp(pay_type, "REFUND"))
								{
									
									total_refund += -((pay_total));
									
									grand_total += ((pay_total));
									
								}
								else
									grand_total += ((pay_total));


							}

							if ((char*)sqlite3_column_text(stmt1,5))
							{
								
								inv_num = atoi((char*)sqlite3_column_text(stmt1, 5));
							}

							if ((char*)sqlite3_column_text(stmt1,6))
							{
								tax_free = atoi((char*)sqlite3_column_text(stmt1,6)) == 1 ? TRUE : FALSE;
							}

							sprintf(cs_name, "%s, %s", get_cs_by_id(cs_id)->last_name, get_cs_by_id(cs_id)->first_name);

							sprintf(str, "%-25.25s %-7.6d %-15.15s %-10.2f %-20.20s\n", cs_name, inv_num, pay_type, pay_total, pay_date);
							strcat(report, str);

							if (last_refkey != refkey)
							{
								if (laborpart[0] == '\0')
									sprintf(laborpart, "\nLabor (Non Taxable) Sales:\n\n"
									"%-25.25s %-7.7s %-9.9s %-15.15s %-10.10s\n---------------------------------------------------------------------------------------\n", 
									"Customer Name", "Invoice", "Est. Time", "Labor Desc", "Sale");

								sprintf(sql, "SELECT LaborDesc, LaborCost, LaborHours FROM InvoiceRef WHERE Refid='%d' AND LaborDesc NOT NULL;", refkey);

								sr2 = sqlite3_prepare(db, sql, -1, &stmt2, 0);
								if (sr2 == SQLITE_OK)
								{
									while (sqlite3_step(stmt2) == SQLITE_ROW)
									{

										if ((char*)sqlite3_column_text(stmt2,0))
											sprintf(labor_desc, "%s", (char*)sqlite3_column_text(stmt2,0));
										if ((char*)sqlite3_column_text(stmt2,1))
											pay_total = fround(atof((char*)sqlite3_column_text(stmt2,1)));
										if ((char*)sqlite3_column_text(stmt2,2))
											labor_hours = (atoi((char*)sqlite3_column_text(stmt2,2)));
										if (strlen(labor_desc) > 15)
										{
											labor_desc[15] = '\0';
											labor_desc[14] = '.';
											labor_desc[13] = '.';
										}

										sprintf(str, "%-25.25s %-7.6d %-9d %-15.15s $%-10.2f\n", cs_name, inv_num, labor_hours, labor_desc, pay_total);
										strcat(laborpart, str);
									}
								}

								sqlite3_finalize(stmt2);




								if (part[0] == '\0')
								{
									sprintf(part, "\nPart (Taxable) Sales:\n\n"
										"%-25.25s %-7.7s %-9.9s %-15.15s %-8.8s %-8.8s %8.8s\n---------------------------------------------------------------------------------------\n", 
										"Customer Name", "Invoice", "Quantity", "Part Desc", "Sale", "Cost", "Tax Free");
								}

								sprintf(sql, "SELECT PartDesc, PartCost, PartRealCost, Quantity FROM InvoiceRef WHERE Refid='%d' AND PartDesc NOT NULL;", refkey);

								sr2 = sqlite3_prepare(db, sql, -1, &stmt2, 0);
								if (sr2 == SQLITE_OK)
								{
									while (sqlite3_step(stmt2) == SQLITE_ROW)
									{
										

										if ((char*)sqlite3_column_text(stmt2,0))
											sprintf(part_desc, "%s", (char*)sqlite3_column_text(stmt2,0));
										if ((char*)sqlite3_column_text(stmt2,3))
											part_count = (atoi((char*)sqlite3_column_text(stmt2,3)));

										if ((char*)sqlite3_column_text(stmt2,1))
										{
											pay_total = fround(atof((char*)sqlite3_column_text(stmt2,1)));
											total_parts += (pay_total*part_count);
											if (tax_free == FALSE)
											{
												
												total_tax += fround(((pay_total*part_count) * TAX));
												total_tax = fround(total_tax);
												
											}

										}
										if ((char*)sqlite3_column_text(stmt2,2))
											part_cost = fround(atof((char*)sqlite3_column_text(stmt2,2)));


										if (strlen(part_desc) > 15)
										{
											part_desc[15] = '\0';
											part_desc[14] = '.';
											part_desc[13] = '.';
										}

										sprintf(str, "%-25.25s %-7.6d %-9d %-15.15s $%-8.2f $%-8.2f %-8.8s\n", cs_name, inv_num, part_count, part_desc, pay_total, part_cost, 
											tax_free == TRUE ? "Yes" : "No");
										strcat(part, str);
									}


								}
								if (discount[0] == '\0')
								{
									sprintf(discount, "\nDiscounts:\n\n"
										"%-25.25s %-7.7s %-11.11s %-11.11s %-11.11s\n---------------------------------------------------------------------------------------\n", 
										"Customer Name", "Invoice", "Parts", "Labor", "Total");
								}

								sprintf(sql, "SELECT  DiscParts, DiscLabor, DiscountTotal FROM Tickets WHERE id='%d' AND DiscountTotal != 0;", inv_num);

								sr2 = sqlite3_prepare(db, sql, -1, &stmt2, 0);
								if (sr2 == SQLITE_OK)
								{
									while (sqlite3_step(stmt2) == SQLITE_ROW)
									{

										if ((char*)sqlite3_column_text(stmt2,0))
											discount_parts = fround(atof((char*)sqlite3_column_text(stmt2,0)));
										if ((char*)sqlite3_column_text(stmt2,1))
											discount_labor = fround(atof((char*)sqlite3_column_text(stmt2,1)));
										if ((char*)sqlite3_column_text(stmt2,2))
											total_discount = fround(atof((char*)sqlite3_column_text(stmt2,2)));
										grand_discount += total_discount;
										grand_discparts += discount_parts;
										grand_disclabor += discount_labor;

										sprintf(str, "%-25.25s %-7.6d %-11.2f %-11.2f %-11.2f\n", cs_name, inv_num, discount_parts, discount_labor, total_discount);
										strcat(discount, str);
									}

								}
								sqlite3_finalize(stmt2);
							}

							if (last_refkey != refkey)
								last_refkey = refkey;

							




						}


					}
					if (found == FALSE)
					{
						GiveError("No reports found for the specified date.",0);
						free (report);
						free (laborpart);
						free (part);
						break;
					}


					sqlite3_finalize(stmt1);

				
					total_tax = (fround((total_parts-grand_discparts) * TAX));
				
					sprintf(str,"\n\nTotal Sales: $%.2f\nTotal Sales Tax: $%.2f\nRefund Total: $%.2f\nDiscount Total: %.2f\nTotal Income: $%.2f\n", grand_total, total_tax, total_refund, grand_discount,
						(grand_total - total_tax) );
					strcat(report, laborpart);
					strcat(report, part);
					strcat(report, discount);
					strcat(report,str);
					print_report(report);
					
					free (report);
					free (laborpart);
					free (part);
					free (discount);

					DestroyWindow(dailywindow);

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


LRESULT APIENTRY UsrDate_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	NMHDR *nm;
	SYSTEMTIME st;
	char str[1024];
	

	switch (msg)
	{
	case WM_NOTIFY:
		{
			nm = (NMHDR*)lparam;
			if (!nm)
				return 0;
			if (nm->code == MCN_SELECT)
			{
				MonthCal_GetCurSel(cal, &st);
				sprintf(str, "%2.2d/%2.2d/%2.2d", st.wMonth, st.wDay, st.wYear);
				if (callback_parent)
					if (callback_child)
					{
						CTRL_SetText(callback_parent, callback_child, str);

					}
					DestroyWindow(usrh);
					DestroyWindow(cal);
					ShowWindow(dailywindow, SW_SHOW);
					return 0;
				

				
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


