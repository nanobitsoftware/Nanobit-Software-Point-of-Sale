#include "sqlite3.h"
#include "nano_memory.h"

#define IS_IN_DEBUGGING_MODE 0
#define REPORT_DEALLOCATION 0
#define REPORT_ALLOCATION 0
#define DEBUG_FILE "c://nanopos//debug.txt"
//#define malloc(x) nano_malloc(x, __FILE__, __LINE__)
//#define free(x)   nano_free  (x, __FILE__, __LINE__)
#define realloc(x, y) nano_realloc (x,y, __FILE__, __LINE__)
#define str_dup(x)  str_dup1(x, __FILE__, __LINE__)
//#define str_dup(x) _strdup(x)

#define WIN32_LEAN_AND_MEAN
#define NOCOMM

#define DISCOUNT_DOLLAR 0
#define DISCOUNT_PERCENT 1

#define BOOL short int
#define bool BOOL
#define MSL 80
#define MAXNOTE 4096
#define MAX_ENTRY 512
#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define FALSE 0
#define TRUE 1

HINSTANCE g_hInst;
#define strcmp string_compare

#define DB_FILE "c://nanopos//nanoposdb.sql"

#define MSB 1

#define MAX_CHAR_COL 74

#define MODE_NORMAL 1
#define MODE_READONLY 2
#define MODE_ESTIMATE 3

/* typedefs for structs */

typedef struct cs_data CSDATA;
typedef struct ticket_data   TICKETDATA;
typedef struct inv_data      INVDATA;
typedef struct note_data     NOTEDATA;
typedef struct employee_data EMPLOYEEDATA;
typedef struct labor_data    LABORDATA;
typedef struct parts_data	 PARTSDATA;
typedef struct printer_data  printdata;
typedef struct crypt_hash    CRYPTHASH;
double TAX;

sqlite3* db;

// Globals
CSDATA* lastcs;
CSDATA* cslist;
CSDATA* csfree;
INVDATA* lastinv;
INVDATA* invlist;
EMPLOYEEDATA* lastemp;
EMPLOYEEDATA* emplist;
HWND mainwindow;
HWND cswindow;
HWND csfind;
HWND cmpwindow;
HWND invwindowadd;
HWND invwindowticket;
HWND invwindowsearch;
HWND tickwindow;
HWND printpreview;
HWND printpreview_panel;
HWND laborwindow;
HWND discountwindow;
HWND setupwindow;
HWND paywindow;
HWND paytotal;
HWND refundwindow;
HWND dailywindow;
HWND calcwindow;



char Company_Name[1024];
char Company_Motto[1024];
char Company_Address[1024];
char Company_City[1024];
char Company_Zip[1024];
char Company_State[1024];
char Company_Phone[1024];

char Company_Warranty[1024];

int total_customers;

BOOL tax_labor;
BOOL tax_parts;

/* Structs*/

struct cs_data
{
	int		 	  cs_id;
	int		 	  zip_code;
	int		 	  times_visited;
	int		 	  corporate_id;
	int		 	  tax_id;
	int		 	  open_tickets;
	int			  total_tickets;
	double	 	  unpaid_due;
	double	 	  total_spent;
	char* last_seen;
	char* first_name;
	char* last_name;
	char* corporate_accountant;
	char* st_address;
	char* city;
	char* state;
	char* phone;
	char* phone_cell;
	char* phone_home;
	char* phone_office;
	char* phone_spouce;
	char* phone_fax;
	char* notes;
	char* sirname;
	char* cs_since;
	BOOL	 	  has_credit;
	BOOL	 	  deny_work;
	BOOL	 	  tax_free;
	BOOL	 	  corporate_account;
	BOOL	 	  cash_only;
	BOOL	 	  accept_creditcard;
	BOOL		  accept_check;
	BOOL		  changed;
	CSDATA* next; // Linked list.
	TICKETDATA** ticket_list; // List of tickets.
};

struct ticket_data
{
	int		 	 ticket_id;
	int		 	 invoice_number;
	int			 notes_total;
	float	 	 sub_total;
	float	 	 discount_labor;
	float	 	 discount_parts;
	char* ticket_written;
	char* ticket_completed;
	char* ticket_billed;
	BOOL		 charged;
	BOOL		 open;
	BOOL		 changed;
	char* service_writer;
	char* technician;
	CSDATA* customer;
	NOTEDATA** notes; // Array of notes
	PARTSDATA** parts;
	LABORDATA** labor;
};

struct inv_data
{
	int		  id;
	int		  inv_type;
	int		  inv_count;
	int		  total_sold;
	char* last_sold;
	float	  cost;
	float	  retail;
	float	  discount;
	BOOL	  on_discount;
	BOOL      changed;
	char* item_name;
	char* item_description;
	INVDATA* next; // Linked list.
};

struct labor_data
{
	char* desc;
	float	cost;
	int		slide_rate_add;
	int		slide_rate_hours;
	BOOL    hourly;
	BOOL    slide_rate;
};

struct parts_data
{
	char* desc;
	float	  cost;
	INVDATA* inventory;
};

struct note_data
{
	char* note;
	BOOL	highlight;
	BOOL	urgent;
};

struct settings
{
	int		tax_rate;
	int		total_tickets_written;
	int		total_sales;
	int		total_cs;
	int		total_inventory_count;
	int		total_inventory_cost;
	int		hourly_rate;
	int		slide_rate_hours;
	int		slide_rate_add;
	float	markup_1;
	float	markup_2;
	float	markup_3;
	float	markup_4;
	float	markup_5;
	float	markup_6;
	char* company_address;
	char* company_phone;
	char* company_fax;
	char* company_name;
	char* company_motto;
	char* warranty;
};

struct employee_data
{
	int		       zip_code;
	int		       employee_id;
	float	       contributed_sales;
	char* name;
	char* phone;
	char* phone_cell;
	char* phone_home;
	char* phone_spouce;
	char* address;
	char* city;
	BOOL	       currently_employeed;
	BOOL           changed;
	EMPLOYEEDATA* next; //Linked list.
};

struct zip_data
{
	char* zipcode;
	char* city;
	char* state;
};

struct print_info
{
	int		pages;
	char* header;
	char* footer;
	char* body;

	int		invoice;
};

struct crypt_hash
{
	char		hash[64];
	char        salt[1024];
	char        password[1024];
	char        pw_salt[1024];
};





//static int add_cs_db (void *NotUsed, int argc, char **argv, char **azColName);
void write_buffer(const char* str);
void LOG(char* fmt, ...);
int read_string(char buf[], FILE* fp);
char* load_file(void);
char* save_file(char* filter);
void GiveError(char* wrong, BOOL KillProcess);
LRESULT APIENTRY POS_MainWindow(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void* nano_realloc(void* seg, size_t sz, const char* file, int line);
void nano_free(void* seg, const char* file, int line);
void* nano_malloc(size_t chunk, const char* file, int line);
char* str_dup1(const char* str, char* file, int line);
TICKETDATA* new_ticket(void);
CSDATA* new_customer(void);
void create_cs_window(void);
int read_string(char buf[], FILE* fp);
void test(void);
BOOL check_zip(char* zip, char state[], char city[]);
LRESULT APIENTRY CSWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT APIENTRY FDWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void create_cs_find_window(void);
void populate_find_list(void);
int add_cs_db(void* NotUsed, int argc, char** argv, char** azColName);
BOOL do_add_customer(void);
void fill_cs(CSDATA* cs);
BOOL string_compare(const char* ostr, const char* tstr);
CSDATA* get_last_cs(void);
char* get_date(void);
BOOL is_customer(CSDATA* c);
void create_add_computer(void);
LRESULT APIENTRY CMAWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void db_add_computer(void);
int do_computer_list(void* NotUsed, int argc, char** argv, char** azColName);
BOOL cs_check_for_change(BOOL closing);
void update_cs_check();
LRESULT APIENTRY TKWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void create_ticket_window(CSDATA* cs, int cid, int tid, int refid, int mode);
CSDATA* get_cs_by_id(int i);
void test_print(void);
LRESULT APIENTRY print(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void paint_preiew(void);
LRESULT APIENTRY printpreview_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
BOOL create_print_preview(char* str);
void DrawInvoiceBox(HDC dc, char* str);
void create_inv_add(BOOL updating);
void create_inv_search(void);
LRESULT APIENTRY InvAddWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
BOOL is_number(char* str);
double fround(double f);
void walk_heap(void);
LRESULT APIENTRY InvSearchWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void do_invfind(void);
void inv_fill_from_find(int idx);
void delete_inventory(int idx);
LRESULT APIENTRY InvTicketWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void create_inv_ticket(void);
void add_inv_ticket(char* invid, char* desc, char* quant, char* cost, char* retail);
BOOL db_new_ticket(void);
void update_ticket_totals(void);
void update_ticket_parts(void);
void inv_fill_from_ticket(int idx);
void update_wip(void);
void create_labor_ticket(void);
LRESULT APIENTRY LaborWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void add_labor_inventory(char* desc, char* hours, char* cost);
void update_ticket_labor(void);
int fill_ticket_labor(void* NotUsed, int argc, char** argv, char** azColName);
BOOL strprefix(const char* astr, const char* bstr);
void create_discount_window(void);
LRESULT APIENTRY DiscWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
BOOL is_number2(char* str);
void create_setup_window(void);
LRESULT APIENTRY SetupWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void prepare_ticket_print(void);
int get_max_char(int colsize);
int pages_col(char* str);
void draw_text_line(char* str);
void print_work_order(int inv, char* order);
char* make_to_col(char* str);
int count_lines(char* str);
void create_payment(void);
LRESULT APIENTRY PayWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void show_payments(void);
int fill_payments(void* NotUsed, int argc, char** argv, char** azColName);
LRESULT APIENTRY PaymentsWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
char* make_to_page(char* str);
void create_refund_window(void);

LRESULT APIENTRY RefundWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void draw_inv_header(HDC dc, RECT r, int x_offset, int y_offset, int invoice);
void draw_inv_footer(HDC dc, RECT r, int x_offset, int y_offset);
void do_print_work_order(void);
void do_print_invoice(void);
void print_invoice(char* str, int copies);
LRESULT APIENTRY UsrDate_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
char* make_salt(void);
char* crypt_password(char* plaintext, char* salt, CRYPTHASH* chash);

char* get_date_notime(void);
LRESULT APIENTRY DailyWindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void create_daily_reports(void);
double get_add_percent(double total, float percent);
double get_minus_percent(double total, float percent);

BOOL is_date(char* range);
void print_report(char* report);
void SetFocusInternal(HWND h);
#define SetFocus(x) SetFocusInternal(x)
void create_calc_window(void);

LRESULT APIENTRY CALCwindow_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);