/*
tickets.c
Nanobit Point of Sale - Enhanced Edition
-------------------------------------------
Written by; Bi0teq
Between 2010 and 2022
-------------------------------------------
Owned and operated by Nanobit Softare (R) (C), 2023
Version 0.0.0.1


Most code is in an alpha stage. These tags will be updated as the code matures.
These tags will also be added with information regarding the
specific file that theyre in a the time.

 Code description:
 Takes all the DB info from the tivkets.gui and does all the data processing on it
 as well as sets up and controls most of the behidn the scenes GUi stuff. NOthing special, really,
 except it does all the work and gets none of the credit. I appreicate you, tickets.c; I apprecite you and love you. 


 */

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

TICKETDATA* new_ticket(void)
{
	TICKETDATA* tick;
	int i; 

	tick = (TICKETDATA*)malloc(sizeof(*tick));

	if (!tick)
	{
		GiveError("Memory failed to allocate for a new ticket; program will now close. Try restarting.", 1);
		return NULL;
	}

	tick->ticket_id = 0;
	tick->invoice_number = 0;
	tick->notes_total = 0;
	tick->sub_total = 0.0;
	tick->discount_labor = 0.0;
	tick->discount_parts = 0.0;
	tick->ticket_written = 0;
	tick->ticket_completed = 0;
	tick->ticket_billed = 0;
	tick->charged = FALSE;
	tick->open = FALSE;
	tick->service_writer = (char*)malloc(MSL * sizeof(char*));
	tick->technician = (char*)malloc(MSL * sizeof(char*));
	tick->customer = NULL;
	tick->notes = (NOTEDATA**)malloc(sizeof(*tick->notes) * MAX_ENTRY);
	tick->parts = (PARTSDATA**)malloc(sizeof(*tick->parts) * MAX_ENTRY);
	tick->labor = (LABORDATA**)malloc(sizeof(*tick->labor) * MAX_ENTRY);

	for (i = 0; i < MAX_ENTRY; i++)
	{
		tick->notes[i] = NULL;
		tick->parts[i] = NULL;
		tick->labor[i] = NULL;
	}

	return tick;
}