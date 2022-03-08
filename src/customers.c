#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <time.h>
#include "Nano PoS.h"

#include "NWC.h"
#include "sqlite3.h"

extern int total_customers;

CSDATA* new_customer(void)
{
	CSDATA* cs;
	static CSDATA c_null;

	cs = (CSDATA*)malloc(sizeof(*cs));

	if (!cs)
	{
		GiveError("Memory failed to allocate for customer creation. Closing program; please restart.", 1);
		return NULL;
	}
	*cs = c_null;

	cs->cs_id = 0;
	cs->zip_code = 0;
	cs->unpaid_due = 0.0;
	cs->times_visited = 0;
	cs->corporate_id = 0;
	cs->tax_id = 0;
	cs->open_tickets = 0;
	cs->total_tickets = 0;
	cs->total_spent = 0.0;
	cs->last_seen = (char*)malloc(30 * sizeof(char*));
	cs->cs_since = (char*)malloc(30 * sizeof(char*));
	cs->first_name = (char*)malloc(MSL * sizeof(char*));

	cs->last_name = (char*)malloc(MSL * sizeof(char*));

	cs->corporate_accountant = (char*)malloc(MSL * sizeof(char*));
	cs->st_address = (char*)malloc(MSL * sizeof(char*));
	cs->city = (char*)malloc(MSL * sizeof(char*));
	cs->phone = (char*)malloc(MSL * sizeof(char*));
	cs->phone_cell = (char*)malloc(MSL * sizeof(char*));
	cs->phone_home = (char*)malloc(MSL * sizeof(char*));
	cs->phone_office = (char*)malloc(MSL * sizeof(char*));
	cs->phone_spouce = (char*)malloc(MSL * sizeof(char*));
	cs->phone_fax = (char*)malloc(MSL * sizeof(char*));
	cs->notes = (char*)malloc(MAXNOTE * sizeof(char*));
	cs->sirname = (char*)malloc(25 * sizeof(char*));
	cs->state = (char*)malloc(25 * sizeof(char*));
	cs->has_credit = FALSE;
	cs->deny_work = FALSE;
	cs->tax_free = FALSE;
	cs->corporate_account = FALSE;
	cs->cash_only = FALSE;
	cs->accept_check = TRUE;
	cs->accept_creditcard = TRUE;
	cs->next = NULL;

	lastcs = get_last_cs();
	if (lastcs != NULL)
	{
		lastcs->next = cs;
	}

	lastcs = cs;

	total_customers += 1;
	//LOG("Total CS: %d", total_customers);
	return cs;
}

CSDATA* get_last_cs(void)
{
	CSDATA* cs;
	CSDATA* cf;

	cs = cf = NULL;

	for (cs = cslist; cs; cs = cs->next)
		cf = cs;
	return cf;
}

int add_cs_db(void* NotUsed, int argc, char** argv, char** azColName)
{
	int i;
	CSDATA* cc;
	BOOL Alloced = FALSE;
	NotUsed = 0;

	cc = new_customer();

	for (i = 0; i < argc; i++)
	{
		if (!strcmp(azColName[i], "id"))
			cc->cs_id = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "LastName"))
			sprintf(cc->last_name, "%s", argv[i]);
		if (!strcmp(azColName[i], "FirstName"))
			sprintf(cc->first_name, "%s", argv[i]);
		if (!strcmp(azColName[i], "CorporateAccountant"))
			sprintf(cc->corporate_accountant, "%s", argv[i]);
		if (!strcmp(azColName[i], "Street"))
			sprintf(cc->st_address, "%s", argv[i]);
		if (!strcmp(azColName[i], "City"))
			sprintf(cc->city, "%s", argv[i]);
		if (!strcmp(azColName[i], "State"))
			sprintf(cc->state, "%s", argv[i]);
		if (!strcmp(azColName[i], "Phone"))
			sprintf(cc->phone, "%s", argv[i]);
		if (!strcmp(azColName[i], "PhoneCell"))
			sprintf(cc->phone_cell, "%s", argv[i]);
		if (!strcmp(azColName[i], "PhoneHome"))
			sprintf(cc->phone_home, "%s", argv[i]);
		if (!strcmp(azColName[i], "PhoneOffice"))
			sprintf(cc->phone_office, "%s", argv[i]);
		if (!strcmp(azColName[i], "PhoneSpouce"))
			sprintf(cc->phone_spouce, "%s", argv[i]);
		if (!strcmp(azColName[i], "PhoneFax"))
			sprintf(cc->phone_fax, "%s", argv[i]);
		if (!strcmp(azColName[i], "Notes"))
			sprintf(cc->notes, "%s", argv[i]);
		if (!strcmp(azColName[i], "Sirname"))
			sprintf(cc->sirname, "%s", argv[i]);
		if (!strcmp(azColName[i], "LastSeen"))
			sprintf(cc->last_seen, "%s", argv[i]);
		if (!strcmp(azColName[i], "CSSince"))
			sprintf(cc->cs_since, "%s", argv[i]);

		if (!strcmp(azColName[i], "Zip"))
			cc->zip_code = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "TimesVisited"))
			cc->times_visited = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "CorporateID"))
			cc->corporate_id = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "TaxID"))
			cc->tax_id = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "OpenTickets"))
			cc->open_tickets = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "TotalTickets"))
			cc->total_tickets = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "UnpaidDue"))
			cc->unpaid_due = argv[i] == NULL ? 0.0f : (float)atof(argv[i]); 
		if (!strcmp(azColName[i], "TotalSpent"))
			cc->total_spent = argv[i] == NULL ? 0.0f : (float)atof(argv[i]);
		if (!strcmp(azColName[i], "HasCredit"))
			cc->has_credit = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "DenyWork"))
			cc->deny_work = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "TaxFree"))
			cc->tax_free = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "CorporateAccount"))
			cc->corporate_account = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "CashOnly"))
			cc->cash_only = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "AcceptCredit"))
			cc->accept_creditcard = argv[i] == NULL ? 0 : atoi(argv[i]);
		if (!strcmp(azColName[i], "AcceptCheck"))
			cc->accept_check = argv[i] == NULL ? 0 : atoi(argv[i]);
	}

	if (cslist == NULL)
		cslist = cc;

	cc->next = NULL;

	return 0;
}

BOOL is_customer(CSDATA* c)
{
	CSDATA* cs;

	if (!c)
		return FALSE;

	for (cs = cslist; cs; cs = cs->next)
	{
		if ((!strcmp(cs->last_name, c->last_name)) && (!strcmp(cs->first_name, c->first_name)) && (!strcmp(cs->phone, c->phone)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

CSDATA* get_cs_by_id(int i)
{
	CSDATA* cs;

	for (cs = cslist; cs; cs = cs->next)
	{
		if (cs->cs_id == i)
			return cs;
	}
	return NULL;
}