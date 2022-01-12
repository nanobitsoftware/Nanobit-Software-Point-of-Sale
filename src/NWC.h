#define MAX_CHILDREN 1024
#define MAX_CONTROLS 512

#define BUTTON     1
#define RADIO      2
#define VSCROLL    3
#define HSCROLL    4
#define RICHEDIT   5
#define COMBO      6
#define LISTBOX    7
#define EDIT       8
#define STATIC     9
#define CHECKBOX  10

typedef struct NWC_Parent NWC_PARENT;
typedef struct NWC_Child  NWC_CHILD;
typedef struct NWC_Child_attr NWC_CHILD_ATTR;
typedef struct window_controls NWC_CTRL;

NWC_PARENT** parents;
int current_parents;			    
int max_parents;

struct NWC_Parent
{
	char* name;
	int		children;
	int		index;
	int width;
	int heigth;
	int x;
	int y;
	BOOL on_top;
	BOOL created;
	HWND window_control;
	HWND window_pointer;
	HINSTANCE instance;
	LRESULT* control_proc;
	unsigned long int window_options;
	unsigned long int style_options;
	void  (*create_child)(void*, ...);
	void* destroy_child;
	void* destroy_self;
	NWC_CTRL** controls;
	int control_count;
	int max_controls;
	NWC_CHILD* child;
};

struct NWC_Child
{
	char* name;
	NWC_PARENT* parent;
	void** controls;
	NWC_CHILD_ATTR** controls_attr;
	int		index;
	LRESULT* control_proc;
	NWC_CHILD* next;
};

struct NWC_Child_attr
{
	int parent_x;
	int parent_y;
	int width;
	int heigth;
	unsigned long int type;
	unsigned long int type_attr;
};

struct window_controls
{
	char* name;
	int    type;
	HWND   handle;
	NWC_PARENT* parent;
	int x;
	int y;
	int height;
	int width;
	int list_view_items;
	int clist_index;
	DWORD style;
	DWORD  id;
};

NWC_CHILD* create_child(NWC_PARENT* p_window, char* name);
BOOL set_parent_config(NWC_PARENT* p_window, HWND hwnd, LRESULT* proc, int x, int y, int width, int heigth, HINSTANCE instance, BOOL on_top, unsigned long int window_options, unsigned long int style_options);
NWC_PARENT* create_parent(char* name);
NWC_CHILD* child_initialize(NWC_PARENT* p_window);
NWC_PARENT* parent_initialize(void);
void initialize_windows(void);
BOOL show_parent(NWC_PARENT* p_window);
LRESULT CALLBACK NWCDefaultProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL AddButton_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
NWC_CTRL* new_control(void);
void delete_control_parent(NWC_PARENT* p_window, NWC_CTRL* ctrl);
void add_control_parent(NWC_PARENT* p_window, NWC_CTRL* ctrl);
void del_parent(NWC_PARENT* p_window);
void add_parent(NWC_PARENT* p_window);
void CTRL_SetText(NWC_PARENT* p_window, char* ctrl, char* text, ...);
BOOL AddStatic_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddCheck_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddRadio_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddEdit_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddRichedit_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
void CTRL_Resize(NWC_PARENT* p_window, char* name, int x, int y, int width, int height);
NWC_CTRL* get_control(NWC_PARENT* p_window, char* name);
char* CTRL_gettext(NWC_PARENT* p_window, char* name);
void CTRL_ChangeFont(NWC_PARENT* p_window, char* name, char* fontname);
void DestroyParent(NWC_PARENT* p_window);
LRESULT CALLBACK StatusProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
NWC_PARENT* NWC_GetParent(HWND hwnd);
BOOL AddList_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
void CTRL_List_additem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_List_clearlist(NWC_PARENT* p_window, char* name);
void CTRL_List_delitem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_List_delitem_idx(NWC_PARENT* p_window, char* name, int idx);
void CTRL_ChangeFont_All(NWC_PARENT* p_window, int type, char* fontname);
int CTRL_list_get_sel_idx(NWC_PARENT* p_window, char* name);
BOOL AddCList_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
void clist_add_col(NWC_PARENT* p_window, char* name, int width, char* text);
void clist_add_data(NWC_PARENT* p_window, char* name, char** text, int count);
void clist_clear_list(NWC_PARENT* p_window, char* name);
BOOL check_get_status(NWC_PARENT* p_window, char* name);
BOOL check_set(NWC_PARENT* p_window, char* name);
BOOL parent_has_focus(NWC_PARENT* p_window);
BOOL AddCombo_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
void CTRL_combo_additem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_combo_clearlist(NWC_PARENT* p_window, char* name);
void CTRL_combo_delitem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_combo_delitem_idx(NWC_PARENT* p_window, char* name, int idx);
int CTRL_combo_get_sel_idx(NWC_PARENT* p_window, char* name);
char* combo_get_text(NWC_PARENT* p_window, char* name, int idx);
BOOL program_has_focus(void);
void print_invoice(char* str, int copies);
void get_user_date(NWC_PARENT* p, char* c, int x, int y);

BOOL CenterWindow(HWND hwnd, HWND hwndParent);
void parent_disable_all(NWC_PARENT* p_window);
int get_max_control_heigth(NWC_PARENT* p);
int get_max_control_width(NWC_PARENT* p);
NWC_CTRL* get_control_by_id(NWC_PARENT* p_window, DWORD id);