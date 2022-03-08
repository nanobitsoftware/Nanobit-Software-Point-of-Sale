#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <malloc.h>
#include "Nano PoS.h"
#include "NWC.h"

#undef malloc // These stay in THIS scope.
#undef free   //
#undef realloc

unsigned long int full_mem = 0;
unsigned long int malloc_calls = 0;
unsigned long int free_calls = 0;
unsigned long int total_alloc;
const unsigned char uninit = 0x0; // Unitialized string to make sure memory is cleared before
// It's passed off.
typedef struct mem_heap HEAP;
#define ALIGN 4

unsigned long int alloced = 0;
unsigned long int unalloced = 0;

#define MALLOC_MAGIC 0x00040000UL

/* Don't ask. This is just a way to make some form of OOP coding in C. I only want 'self' usage. So I made a
 * little subsystem that pushes and pops a type, and the function calling. For some checks.
 */

struct my_own_self
{
	void* self; // We'll cast here.
	void* func; // What function we're sending to, so we can check to make sure they're correct (threading checks basically)
	time_t tick; // Time keeping
};

typedef struct my_own_self SELF;

SELF self_list;

struct mem_heap
{
	//char m_add[20];
	unsigned long int m_add;
	size_t size;
	char  file[100];
	int    line;
	HEAP* next;
	HEAP* prev;
	void* chunk;
};

HEAP* firstheap;
HEAP* heaplist;
HEAP* freeheap;
HEAP* lastheap;

BOOL ERRORS;

HEAP* new_heap(void)
{
	static HEAP  h;

	HEAP* ph;
	if (!IS_IN_DEBUGGING_MODE)
		return NULL;

	if (freeheap == NULL)
		ph = malloc(sizeof(*ph));
	else
	{
		ph = freeheap;
		freeheap = freeheap->next;
	}

	*ph = h;
	total_alloc += (sizeof(*ph));

	return ph;
}

void add_heap(HEAP* hp)
{ 
	char temp[5000] = "";
	int b_type = 0;
	float t_total = 0.0;

	temp[0] = '\0';
	b_type = 0;

	if (!IS_IN_DEBUGGING_MODE)
		return;

	if (REPORT_ALLOCATION)
	{
		t_total = (float)total_alloc;

		if (t_total > 1024)
		{
			b_type = 1; // KB
			t_total = t_total / 1024;
		}
		if (t_total > 1024)
		{
			b_type = 2; // MB
			t_total = t_total / 1024;
		}
		if (t_total > 1024)
		{
			b_type = 3; // GB
			t_total = t_total / 1024;
		}

		LOG("--------------------------------------------------------------------------------------------------");
		LOG("Allocation:");
		LOG("Calling file: %s", hp->file);
		LOG("Calling line: %d", hp->line);
		LOG("Calling size: %lu", hp->size);
		LOG("Total allocation: %5.5f %s", t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes");
		LOG("Calling address (returned): 0X%X", hp->m_add);
		LOG("--------------------------------------------------------------------------------------------------\r\n");
	}
	hp->next = NULL;

	if (lastheap != NULL)
		lastheap->next = hp;

	hp->prev = lastheap;

	lastheap = hp;

	if (firstheap == NULL)
		firstheap = hp;
	alloced += 1;				      

	return;
}

void show_heap(void)
{
	unsigned long int i = 0;

	long int size;
	long int s_size;
	char s[100];

	HEAP* hp;
	HEAP* last;

	char dheap[1000] = "";
	char str[200] = "";

	return;
	if (!IS_IN_DEBUGGING_MODE)
		return;

	str[0] = '\0';

	dheap[0] = '\0';
	last = lastheap;

	for (i = 0, hp = firstheap; hp != last; hp = hp->next, i++)
	{
		if (hp->chunk == NULL)
		{
			size = -1;
		}
		else
		{
			//size = *((char*)hp->chunk - sizeof(double));
			(char*)hp->chunk -= sizeof(double);
			memset(s, '\0', 100);
			memcpy(s, (char*)hp->chunk, sizeof(double));
			size = atoi(s);

			s_size = size;
			//size = (*((int*)(hp->chunk - sizeof(double))) + sizeof(double));
		}

		if (size > 1024)
		{
			size = size / 1024;
			sprintf(str, " KB");
		}
		if (size > 1024)
		{
			size = size / 1024;
			sprintf(str, " MB");
		}
		if (size > 1024)
		{
			size = size / 1024;
			sprintf(str, "GB");
		}

		if (i % 1000 == 0)
		{
			//			nasty = FALSE;
			//			update_term();

			//			nasty = TRUE;
		}

		sprintf(dheap, "%lu) Address: 0x%lx, File: %s, Line: %d Size(bit-stored): %ld%s", i, hp->m_add == 0 ? 0 : hp->m_add, hp->file == NULL ? "Undefined" : hp->file, hp->line, size, str[0] == '\0' ? " B" : str);

		//		realize_lines(dheap);
		dheap[0] = '\0';
		str[0] = '\0';
	}
	//nasty = FALSE;
	//update_term();
}

void walk_heap(void)
{
	unsigned long int i = 0;
	unsigned long int count = 0;
	HEAP* hp;

	if (!IS_IN_DEBUGGING_MODE)
		return;
	for (i = 0, hp = firstheap; hp; hp = hp->next, i++)
	{
		LOG("Walkheap: %d) m_add: 0x%x, file: %s, line: %d", i, hp->m_add == 0 ? 0 : hp->m_add, hp->file == NULL ? "Undefined" : hp->file, hp->line);
		count += (unsigned long int)hp->size;
	}
	LOG("Walkheap: Total size unfreed: %d bytes", count);
	LOG("Walkheap: Allocations called: %d. Deallocations called: %d, total: %d. (This number SHOULD be zero. \n\tIf not, then we got some problems.\n", alloced, unalloced, alloced - unalloced);
}

void dump_heap(void)
{
	unsigned long int i = 0;
	unsigned long int count = 0;
	unsigned long int total = 0;
	char buf[5000] = "";

	HEAP* hp;

	if (!IS_IN_DEBUGGING_MODE)
		return;
	buf[0] = '\0';

	for (i = 0, hp = firstheap; hp; hp = hp->next, i++)
	{
		count += (unsigned long int)hp->size;
		total++;
	}

	LOG("Dumpheap: %d total allocations managed. %d bytes size total.\r\n", total, count);
	//give_term_debug("Dumpheap: %d total allocations managed. %s size total.\r\n", total,commaize(count, buf));
	return;
}

int count_heap(void)
{
	unsigned long int i = 0;
	HEAP* hp;

	if (!IS_IN_DEBUGGING_MODE)
		return 0;

	for (i = 0, hp = firstheap; hp; hp = hp->next)
		i++;
	return i;
}
void del_heap(unsigned long int m_add, int line, char* file)
{
	HEAP* h;
	char temp[5000] = "";
	int b_type = 0;
	float t_total = 0.0;

	BOOL found = FALSE;

	temp[0] = '\0';
	b_type = 0;

	if (!IS_IN_DEBUGGING_MODE)
		return;
	if (REPORT_DEALLOCATION)
	{
		t_total = (float)total_alloc;

		if (t_total > 1024)
		{
			b_type = 1; // KB
			t_total = t_total / 1024;
		}
		if (t_total > 1024)
		{
			b_type = 2; // MB
			t_total = t_total / 1024;
		}
		if (t_total > 1024)
		{
			b_type = 3; // GB
			t_total = t_total / 1024;
		}

		LOG("--------------------------------------------------------------------------------------------------");
		LOG("Deallocation:");
		LOG("Calling file: %s", file);
		LOG("Calling line: %d", line);
	}

	for (h = firstheap; h; h = h->next)
	{
		if (h->m_add == m_add)

		{
			found = TRUE;
			if (REPORT_DEALLOCATION)
			{
				LOG("Size freeing: %lu", h->size);
			}

			if (h->prev != NULL)
				h->prev->next = h->next;

			if (h == firstheap)
				firstheap = h->next;

			if (h->next != NULL)
				h->next->prev = h->prev;

			if (h == lastheap)
				lastheap = h->prev;
			//free (h->m_add);
			//free (h->file);
			h->chunk = NULL;
			free(h);
			unalloced += 1;
			break;
		}
	}
	if (REPORT_DEALLOCATION)
	{
		LOG("Total allocation: %5.5f %s", t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes");
		LOG("Calling address (returned): 0X%X", m_add);
		LOG("--------------------------------------------------------------------------------------------------\r\n");
	}

	if (found == FALSE)
		LOG("Del_heap: m_add does not match a heap we manage. m_add; 0x%x\n\tCalling file: %s, calling line: %d\n", m_add, file, line);

	return;
}

BOOL own_heap(unsigned long int m_add)
{
	HEAP* h;

	// Do we own the address we're trying to free?
	if (IS_IN_DEBUGGING_MODE == FALSE)
		return TRUE; // Always return true if we're not debugging.

	// Loop through heap and see if we own this address.
	// Return TRUE if we do own it. False if not.

	for (h = firstheap; h; h = h->next)
	{
		if (h->m_add == m_add)
			return TRUE;
	}
	return FALSE;
}

void* nano_malloc(size_t chunk, const char* file, int line)
{
	int upper_mult;
	static  void* mem;

	char* tail;

	char madd[1000];

	unsigned long int m_add = 0;

	extern char ERROR_STRING[5000];

	ERROR_STRING[0] = '\0';
	if (ERRORS)
		return NULL;

	mem = NULL;
	if (chunk < 0)
		chunk = 1;

	upper_mult = (int)chunk;

	while ((chunk) % ALIGN != 0)
	{
		chunk++;
	}

	if (!(mem = malloc(chunk)))
	{
		sprintf(ERROR_STRING, "Memory failed to allocate! File: %s, line: %d,size: %zd", file, line, chunk);
		LOG(ERROR_STRING);
		ERRORS = TRUE;
		GiveError(ERROR_STRING, TRUE);
		exit(1);
		return NULL;
	}
	//memset(mem, 0, chunk);

	total_alloc +=(unsigned long int ) chunk;

	//chunk |= MALLOC_MAGIC;

	//sprintf((char*)mem,  "%d", chunk);

	if (IS_IN_DEBUGGING_MODE)
	{
		HEAP* h;
		sprintf(madd, "%p", mem);

		m_add = strtoul(madd, &tail, 16);

		h = new_heap();
		h->m_add = m_add;
		h->size = chunk;

		h->chunk = ((char*)mem);
		memcpy(h->file, file, strlen(file));
		h->file[strlen(file)] = '\0';
		h->line = line;
		add_heap(h);
		madd[0] = '\0';
	}

	return mem;
}

void nano_free(void* seg, const char* file, int line)
{
	char madd[100];
	unsigned long int m_add;
	char* tail;

	if (!seg)
		return;

	fflush(NULL);
	if (IS_IN_DEBUGGING_MODE == 1)
	{
		sprintf(madd, "%p", ((char*)seg));
		m_add = strtoul(madd, &tail, 16);
		if (!own_heap(m_add))
		{
			char str[1024];
			sprintf(str, "Attempted to free address %ld, but we did not own it. Not freeing.",
				m_add);
			LOG(str);
			return;
		}

		del_heap(m_add, line, (char*)file);
		madd[0] = '\0';
	}

	if (seg)
	{
		if (!seg)
			GiveError("Malloc Failure", 1);

		free(seg);

		seg = NULL;
	}
	else
	{
		//LOG("Memory error: %p. (%s/%d)", seg, file,line);
		GiveError("Memory Error", 1);
	}

	return;
}

void* nano_realloc(void* seg, size_t sz, const char* file, int line)
{
	void* to_ret;
	return NULL;
	while ((sz + sizeof(double)) % ALIGN != 0)
	{
		sz++;
	}

	total_alloc -= (*((int*)seg - sizeof(double)));
	(*(int*)seg) -= sizeof(double);
	to_ret = realloc(seg, sz);
	*((DWORD*)seg) = (DWORD)sz;

	total_alloc += (unsigned long int )sz;

	return (void*)((*((int*)to_ret)) + sizeof(double));
}

void return_usage(void)
{
	return;
}

unsigned long int get_memory_usage()
{
	return total_alloc;
}

void push_self(void* func, void* self)
{
	if (!self || !func)
		return;

	self_list.func = func;
	self_list.self = self;
	self_list.tick = GetTickCount();
}

void* check_self(void* func)
{
	void* old_self;
	void* old_func;
	time_t old_tick;
	if (!func)
		return NULL;

	if (self_list.func == func)
	{
		old_self = self_list.self;
		old_func = self_list.func;
		old_tick = self_list.tick;

		return (void*)old_self;
	}

	if (self_list.tick - GetTickCount() > 1000)
	{
		// are we greate than 1 second? Surely something went wrong. Let's clear the self list.
		self_list.self = self_list.func = NULL;
		self_list.tick = 0;
	}
	return NULL;
}

void pop_self(void* func, void* self)
{
	if (!self || !func)
		return;

	if (self_list.self == self && self_list.func == func)
	{
		self_list.self = self_list.func = NULL;
		self_list.tick = 0;
	}
	return;
}