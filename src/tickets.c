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