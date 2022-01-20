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


/* Nomenclature is bound to change in the naer future (parent moving to simply 'window' and the like)
 * so do not keep too much stock in the current names. */


int IDX;
BOOL INITIALIZED;

NWC_PARENT* fake_window; // So we never return NULL
NWC_CTRL* fake_control; // So we never return NULL

#undef SetFocus

void SetFocusInternal(HWND h)
{
	if (h)
		SetFocus(h);
	else
		SetFocus(mainwindow);
}
void initialize_windows(void)
{
	int i;

	if (INITIALIZED == TRUE)
	{
		return;
	}

	IDX = 0;
	INITIALIZED = TRUE;
	max_parents = 100;
	parents = (NWC_PARENT**)malloc(max_parents * sizeof(*parents));

	current_parents = 0;

	for (i = 0; i < max_parents; i++)
		parents[i] = NULL;

	if (!fake_window) // So we don't do it twice and leak.
		fake_window = create_parent("EMPTY WINDOW");
	if (!fake_control)
		fake_control = new_control();
	return;
}

NWC_PARENT* parent_initialize(void)
{
	NWC_PARENT* p_window;				    
	p_window = (NWC_PARENT*)malloc(sizeof(*p_window));
	if (!p_window)
	{
		GiveError("Allocation for window memory has failed.\nProgram will now exit gracefully.", TRUE);
	}

	p_window->name = NULL;
	p_window->self = p_window;
	p_window->child = NULL;
	p_window->children = 0;
	p_window->control_proc = NULL;
	p_window->create_child = NULL;
	p_window->create_widget = (void*)nwc_widget_create_entry(p_window);
	p_window->destroy_child = NULL;
	p_window->destroy_self = NULL;
	p_window->heigth = 0;
	p_window->width = 0;
	p_window->index = 0;
	p_window->instance = 0;
	p_window->on_top = FALSE;
	p_window->window_control = 0;
	p_window->created = FALSE;
	p_window->control_count = 0;
	p_window->controls = NULL;

	return p_window;
}

NWC_CHILD* child_initialize(NWC_PARENT* p_window)
{
	NWC_CHILD* p_child;

	if (!p_window)
	{
		GiveError("Parent of child does not exist. Cannot initialize a new child element.", FALSE);
		return NULL;
	}

	if (p_window->children > MAX_CHILDREN)
	{
		GiveError("Max child windows has been reached for parent window. Cannot create a new child element.", FALSE);
		return NULL;
	}

	p_child = (NWC_CHILD*)malloc(sizeof(*p_child));

	if (!p_child)
	{
		GiveError("Unable to allocate memory for a child element of the new window. \n\nProgram will attempt to exit gracefully.", TRUE);
		return NULL;
	}

	p_child->name = NULL;
	p_child->controls = (void**)malloc(sizeof(p_child->controls) * MAX_CONTROLS);

	p_child->controls_attr = (NWC_CHILD_ATTR**)malloc(sizeof(p_child->controls_attr) * MAX_CONTROLS);
	p_child->control_proc = NULL;
	p_child->index = 0;
	p_child->next = NULL;
	p_child->parent = p_window;

	return p_child;
}

void add_parent(NWC_PARENT* p_window)
{
	int i;
	NWC_PARENT** t_par;

	if (!p_window)
		return;

	if ((current_parents + 1) >= max_parents)
	{
		t_par = (NWC_PARENT**)malloc((max_parents + 1) * sizeof(*t_par));

		for (i = 0; i < max_parents; i++)
		{
			if (parents[i] == NULL)
				continue;
			t_par[i] = parents[i];
		}
		free(parents);
		parents = t_par;
		max_parents++;
	}

	parents[current_parents] = p_window;
	current_parents++;
}

void del_parent(NWC_PARENT* p_window)
{
	int i;

	if (!p_window)
		return;

	for (i = 0; i < max_parents; i++)
	{
		if (parents[i] == p_window)
		{
			parents[i] = NULL;
		}
	}
}

NWC_PARENT* create_parent(char* name)
{
	NWC_PARENT* p_window = NULL;
	int i;

	if (!name || name[0] == '\0')
	{
		GiveError("Window was attempted to be created without proper naming convention. Exiting program.", TRUE);
		return NULL;
	}

	if (INITIALIZED == FALSE)
	{
		initialize_windows();
	}

	if (!p_window)
	{
		p_window = parent_initialize();
		if (p_window == NULL)
		{
			GiveError("Allocation for window memory has failed.\nProgram will attempt to exit gracefully.", TRUE);
		}
	}

	p_window->name = str_dup(name);
	p_window->index = IDX;
	p_window->controls = (NWC_CTRL**)malloc(20 * sizeof(NWC_CTRL*)); // Start with a max of 10 controls. This is dynamic!
	p_window->max_controls = 10;
	for (i = 0; i < p_window->max_controls; i++)
	{
		p_window->controls[i] = NULL;
	}
	IDX++;
	add_parent(p_window);
	return p_window;
}

BOOL set_parent_config(NWC_PARENT* p_window, HWND hwnd, LRESULT* proc, int x, int y, int width, int heigth, HINSTANCE instance, BOOL on_top, unsigned long int window_options, unsigned long int style_options)
{
	// This is a dynamic function; half fill options will skip gracefully and cause those to remain as they were.

	if (!p_window)
	{
		GiveError("Window not found.", FALSE);
		return FALSE;
	}

	if (hwnd != 0)
		p_window->window_control = hwnd;
	else
	{
		p_window->window_control = 0;
	}

	if (proc)
		p_window->control_proc = proc;
	else
		p_window->control_proc = (LRESULT*)NWCDefaultProc;

	if (width > -1)
		p_window->width = width;
	if (heigth > -1)
		p_window->heigth = heigth;
	p_window->instance = instance;

	if (on_top == TRUE || on_top == FALSE)
		p_window->on_top = on_top;

	p_window->window_options = window_options | WS_EX_CLIENTEDGE;
	p_window->style_options = style_options | WS_CAPTION | DS_FIXEDSYS | WS_MINIMIZEBOX | WS_SYSMENU | DS_SETFONT;
	p_window->x = x == CW_USEDEFAULT ? CW_USEDEFAULT : x < 0 ? 0 : x;
	p_window->y = y == CW_USEDEFAULT ? CW_USEDEFAULT : y < 0 ? 0 : y;

	if (p_window->window_control == 0)
	{
		show_parent(p_window);
		ShowWindow(p_window->window_pointer, SW_HIDE);
	}

	return TRUE;
}

BOOL show_parent(NWC_PARENT* p_window)
{
	WNDCLASS wc;
	int i;

	if (!p_window)
	{
		GiveError("Window will not be shown; window has not been created nor initialized.", FALSE);
		return FALSE;
	}
	if (p_window->created == FALSE) // Create the window
	{
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wc.lpfnWndProc = (WNDPROC)p_window->control_proc;
		wc.lpszClassName = p_window->name;
		wc.lpszMenuName = NULL;
		wc.hInstance = p_window->instance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.style = CS_DBLCLKS;
		RegisterClass(&wc);

		p_window->window_pointer = CreateWindowEx((DWORD)p_window->window_options, p_window->name, p_window->name, (DWORD)p_window->style_options, p_window->x, p_window->y, p_window->width, p_window->heigth, p_window->window_control, 0, p_window->instance, 0);

		if (p_window->window_control == 0)
			p_window->window_control = p_window->window_pointer;

		if (p_window->window_control == NULL)
		{
			p_window->window_control = p_window->window_pointer;
		}

		p_window->created = TRUE;

		//		ShowWindow(p_window->window_pointer,SW_SHOW);
	}

	{
		ShowWindow(p_window->window_pointer, SW_SHOW);

		if (p_window->control_count > 0)
		{
			for (i = 0; i < p_window->max_controls; i++)
			{
				if (p_window->controls[i] != NULL)
				{
					ShowWindow(p_window->controls[i]->handle, SW_SHOW);
				}
			}
		}

		//	NWCDefaultProc(p_window->window_pointer, WM_CREATE,0,0);
	}

	p_window->window_control = p_window->window_pointer;
	return TRUE;
}

NWC_CHILD* create_child(NWC_PARENT* p_window, char* name)
{
	NWC_CHILD* p_child;

	if (!p_window)
	{
		GiveError("Create_child () attempted to add a child for a non-existance, managed, window. Attempting to exit gracefully.", TRUE);
		return NULL;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of child for creation is null or not a valid name.\nCannot create child.", FALSE);
		return NULL;
	}

	p_child = child_initialize(p_window);

	if (!p_child)
	{
		GiveError("A problem has occured creating a child window.", FALSE);
		return NULL;
	}

	p_child->index = p_window->children;
	p_child->control_proc = p_window->control_proc; // Unless changed, the parent will control the child. This will rarely change.
	p_window->children++;

	return p_child;
}

LRESULT CALLBACK NWCDefaultProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	int j;

	for (i = 0; i < max_parents; i++)
	{
		if (parents[i] != NULL)
		{
			for (j = 0; j <= parents[i]->max_controls; j++)
			{
				//				if (parents[i]->controls[j] == NULL)
					//				continue;
						//		if ((parents[i]->controls[j]) && parents[i]->controls[j]->handle == hwnd)
							//	{
								//	LOG("Control: %s called", parents[i]->controls[j]->name);
								//}
			}
		}
	}

	switch (message)
	{
	default:
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	}

	return 0;
}

void add_control_parent(NWC_PARENT* p_window, NWC_CTRL* ctrl)
{
	NWC_CTRL** t_ctrl;
	int i;

	if (!p_window || !ctrl)
	{
		GiveError("Adding control to parent resulted in a bad call.", FALSE);
		return;
	}

	if ((p_window->control_count) >= p_window->max_controls)
	{
		// Increase the control count.
		t_ctrl = (NWC_CTRL**)malloc((p_window->max_controls + 1) * sizeof(*p_window->controls));

		for (i = 0; i < p_window->max_controls; i++)
		{
			if (p_window->controls[i] == NULL)
				continue;
			t_ctrl[i] = p_window->controls[i];
		}
		free(p_window->controls);
		p_window->controls = t_ctrl;
		p_window->max_controls++;
	}

	p_window->controls[p_window->control_count] = ctrl;
	p_window->control_count++;
}

void delete_control_parent(NWC_PARENT* p_window, NWC_CTRL* ctrl)
{
	int i;

	if (!p_window || !ctrl)
	{
		GiveError("Attempted to delete a control that does not exist.", FALSE);
		return;
	}

	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == ctrl)
		{
			free(ctrl->name);
			free(ctrl);
		}
	}
}

NWC_CTRL* new_control(void)
{
	NWC_CTRL* ctrl;

	ctrl = (NWC_CTRL*)malloc(sizeof(*ctrl));

	ctrl->name = NULL;
	ctrl->handle = 0;
	ctrl->height = 0;
	ctrl->parent = 0;
	ctrl->style = 0;
	ctrl->type = 0;
	ctrl->width = 0;
	ctrl->x = 0;
	ctrl->y = 0;

	return ctrl;
}

void CTRL_Resize(NWC_PARENT* p_window, char* name, int x, int y, int width, int height)
{
	int i;

	if (!p_window || !name)
		return;

	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (strstr(p_window->controls[i]->name, name))
		{
			MoveWindow(p_window->controls[i]->handle, x, y, width, height, TRUE);
		}
	}
}

NWC_CTRL* get_control(NWC_PARENT* p_window, char* name)
{
	int i;

	if (!p_window || !name)
		return fake_control;
	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (strstr(p_window->controls[i]->name, name))
		{
			return p_window->controls[i];
		}
	}
	return fake_control; // Return Fake Window instead of null.
}

/* Same as get_control above, but searches by ID rather than name.
 * This is so we can search for thingd we didn't give names to for
 * aesthetic reasons. */
NWC_CTRL* get_control_by_id(NWC_PARENT* p_window, DWORD id)
{
	int i;

	if (!p_window)
		return fake_control;
	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (p_window->controls[i]->id == id)
		{
			return p_window->controls[i];
		}
	}
	return fake_control; // Return Fake Window instead of null.
}
BOOL parent_has_focus(NWC_PARENT* p_window)
{
	int i;
	if (!p_window)
		return FALSE;

	if (GetFocus() == p_window->window_control)
		return TRUE;
	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (GetFocus() == p_window->controls[i]->handle)
			return TRUE;
	}
	return FALSE;
}

BOOL program_has_focus(void)
{
	int i;
	int x;

	for (i = 0; i < max_parents; i++)
	{
		if (parents[i] == NULL)
			continue;
		if (GetFocus() == parents[i]->window_control)
			return TRUE;
		for (x = 0; x < parents[i]->max_controls; x++)
		{
			if (parents[i]->controls[x] == NULL)
				continue;
			if (GetFocus() == parents[i]->controls[x]->handle)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL AddButton_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddButton_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-button does not exist. Using a generic name.", FALSE);
		name = str_dup("Button");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-button", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | BS_TEXT | WS_CHILD | WS_TABSTOP;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = BUTTON;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_CLIENTEDGE, WC_BUTTON, ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	CTRL_ChangeFont(p_window, ctrl->name, "Courier New");
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

void CTRL_SetText(NWC_PARENT* p_window, char* ctrl, char* text, ...)
{
	char buf[20000];

	int i;

	va_list args;
	va_start(args, text);
	vsprintf(buf, text, args);
	va_end(args);

	if (!ctrl || ctrl[0] == '\0')
	{
		GiveError("Cannot change name of control without a name.", 0);
		return;
	}

	if (!p_window)
		return;

	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (p_window->controls[i]->name == NULL)
			continue;

		if (strstr(p_window->controls[i]->name, ctrl))
		{
			SendMessage(p_window->controls[i]->handle, WM_SETTEXT, strlen(buf), (LPARAM)(LPCSTR)buf);

			return;
		}
	}
}

BOOL AddStatic_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddStatic_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-static does not exist. Using a generic name.", FALSE);
		name = str_dup("Static");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-static", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_CHILD;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = STATIC;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_WINDOWEDGE, "STATIC", ctrl->name, ctrl->style | WS_CHILD, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	CTRL_ChangeFont(p_window, ctrl->name, "Courier New");
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

BOOL AddCheck_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddCheck_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-Check does not exist. Using a generic name.", FALSE);
		name = str_dup("Checkbox");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-Check", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = CHECKBOX;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	CTRL_ChangeFont(p_window, ctrl->name, "Courier New");

	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

BOOL check_get_status(NWC_PARENT* p_window, char* name)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("check_get_status () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Check not found.", 0);
		return FALSE;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl)
	{
		GiveError("Control failed to find properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	if (SendDlgItemMessage(p_window->window_control, ctrl->id, BM_GETCHECK, 0, 0))
		return TRUE;
	else
		return FALSE;
}

BOOL check_set(NWC_PARENT* p_window, char* name)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("check_get_status () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Check not found.", 0);
		return FALSE;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl)
	{
		GiveError("Control failed to find properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	if (SendDlgItemMessage(p_window->window_control, ctrl->id, BM_SETCHECK, BST_CHECKED, 0))
		return TRUE;
	else
		return FALSE;
}

BOOL AddRadio_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddRadio_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-Radio does not exist. Using a generic name.", FALSE);
		name = str_dup("Radio");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-radio", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = RADIO;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

BOOL AddCombo_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddRadio_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-Radio does not exist. Using a generic name.", FALSE);
		name = str_dup("Radio");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-radio", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = COMBO;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_WINDOWEDGE, "COMBOBOX", ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

BOOL AddEdit_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddEdit_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-Edit does not exist. Using a generic name.", FALSE);
		name = str_dup("Editbox");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-Edit", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = EDIT;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (ctrl->style & ES_MULTILINE)
		ctrl->style &= ~WS_TABSTOP;

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_control, (HMENU)ctrl->id, g_hInst, 0);
	//CreateWindowEx((DWORD)p_window->window_options, p_window->name, p_window->name,(DWORD)p_window->style_options, p_window->x, p_window->y, p_window->width, p_window->heigth, p_window->window_control, 0, p_window->instance, 0);
	CTRL_SetText(ctrl->parent, ctrl->name, "");
	CTRL_ChangeFont(p_window, ctrl->name, "Courier New");
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

char* CTRL_gettext(NWC_PARENT* p_window, char* name)
{
	/* Returns the text that the control holds */

	CHAR* to_ret;
	int len;
	int i;

	to_ret = NULL;
	len = 0;
	i = 0;

	if (!p_window || !name)
		return NULL;

	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;

		if (strstr(p_window->controls[i]->name, name))
		{
			len = SendMessage(p_window->controls[i]->handle, WM_GETTEXTLENGTH, 0, 0);
			if (len == 0)
				return NULL;
			to_ret = (char*)malloc((sizeof(char*) * len) + 1);

			SendMessage(p_window->controls[i]->handle, WM_GETTEXT, len + 1, (LPARAM)(LPCSTR)to_ret);

			return to_ret;
		}
	}

	return NULL;
}

void CTRL_ChangeFont(NWC_PARENT* p_window, char* name, char* fontname)
{
	int i;
	static HFONT font;
	//	RECT r;

	if (!p_window || !name || !fontname)
		return;

	if (!font)
	{
		if ((font = CreateFont(15, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, 0, fontname)) == NULL)
		{
			GiveError("Error creating font.", FALSE);
			return;
		}
	}
	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (strstr(p_window->controls[i]->name, name))
		{
			SendMessage(p_window->controls[i]->handle, WM_SETFONT, (WPARAM)font, 0);
		}
	}
}

void CTRL_ChangeFont_All(NWC_PARENT* p_window, int type, char* fontname)
{
	/* Changes all fonts of all the 'type's of a window/parent. Must match tpe*/
	int i;

	if (!p_window)
		return;

	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		if (p_window->controls[i]->type == type)
		{
			CTRL_ChangeFont(p_window, p_window->controls[i]->name, fontname);
		}
	}
}

char* combo_get_text(NWC_PARENT* p_window, char* name, int idx)
{
	NWC_CTRL* ctrl;
	int count;
	static str[1024];

	count = 0;

	if (!p_window || !name)
	{
		return NULL;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (DelitemIDXlist)", 0);
		return NULL;
	}
	if (ctrl->type != COMBO)
		return NULL;

	if ((count = SendMessage(ctrl->handle, CB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelitemIDXlist)", 0);
		return NULL;
	}

	if (idx > count || idx < 0)
		return NULL;

	SendMessage(ctrl->handle, CB_GETLBTEXT, (WPARAM)idx, (LPARAM)&str);
	return (char*)str;
}

void CTRL_List_additem(NWC_PARENT* p_window, char* name, char* item)
{
	NWC_CTRL* ctrl;

	if (!p_window || !name || !item)
	{
		GiveError("Listbox Error", 0);
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Control was not found for listview.", 0);
		return;
	}

	if (strlen(item) > 1024) // Truncate to 1024 bytes.
		item[1024] = '\0';

	if (ctrl->type != LISTBOX)
		return;

	CTRL_ChangeFont(p_window, ctrl->name, "Courier");
	SendMessage(ctrl->handle, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)item);
}

void CTRL_combo_additem(NWC_PARENT* p_window, char* name, char* item)
{
	NWC_CTRL* ctrl;

	if (!p_window || !name || !item)
	{
		GiveError("Listbox Error", 0);
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Control was not found for listview.", 0);
		return;
	}

	if (strlen(item) > 1024) // Truncate to 1024 bytes.
		item[1024] = '\0';

	if (ctrl->type != COMBO)
		return;

	if (strlen(item) < 1)
		return;

	CTRL_ChangeFont(p_window, ctrl->name, "Courier");
	SendMessage(ctrl->handle, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)item);
}

void CTRL_combo_clearlist(NWC_PARENT* p_window, char* name)
{
	NWC_CTRL* ctrl;
	int count;

	if (!p_window || !name)
	{
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (Clearlist)", 0);
		return;
	}

	if (ctrl->type != COMBO)
		return;

	if ((count = SendMessage(ctrl->handle, CB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for combo (Clearlist)", 0);
		return;
	}

	for (; count >= 0; count--) // Delete backwards.
	{
		SendMessage(ctrl->handle, CB_DELETESTRING, (WPARAM)count, 0);
	}

	return;
}

void CTRL_List_clearlist(NWC_PARENT* p_window, char* name)
{
	NWC_CTRL* ctrl;
	int count;

	if (!p_window || !name)
	{
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (Clearlist)", 0);
		return;
	}

	if (ctrl->type != LISTBOX)
		return;

	if ((count = SendMessage(ctrl->handle, LB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (Clearlist)", 0);
		return;
	}

	for (; count >= 0; count--) // Delete backwards.
	{
		SendMessage(ctrl->handle, LB_DELETESTRING, (WPARAM)count, 0);
	}

	return;
}

void CTRL_combo_delitem(NWC_PARENT* p_window, char* name, char* item)
{
	NWC_CTRL* ctrl;
	int count;
	int i;
	char buf[1024];

	i = count = 0;
	buf[0] = '\0';

	if (!p_window || !name)
	{
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (Delitemlist)", 0);
		return;
	}
	if (ctrl->type != COMBO)
		return;

	if ((count = SendMessage(ctrl->handle, CB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelItemlist)", 0);
		return;
	}

	for (i = 0; i <= count; i++)
	{
		SendMessage(ctrl->handle, CB_GETLBTEXT, (WPARAM)i, (LPARAM)(LPCSTR)buf);
		if (!strcmp(buf, item))
		{
			SendMessage(ctrl->handle, CB_DELETESTRING, (WPARAM)i, 0);
		}
	}
	return;
}

void CTRL_List_delitem(NWC_PARENT* p_window, char* name, char* item)
{
	NWC_CTRL* ctrl;
	int count;
	int i;
	char buf[1024];

	i = count = 0;
	buf[0] = '\0';

	if (!p_window || !name)
	{
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (Delitemlist)", 0);
		return;
	}
	if (ctrl->type != LISTBOX)
		return;

	if ((count = SendMessage(ctrl->handle, LB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelItemlist)", 0);
		return;
	}

	for (i = 0; i <= count; i++)
	{
		SendMessage(ctrl->handle, LB_GETTEXT, (WPARAM)i, (LPARAM)(LPCSTR)buf);
		if (!strcmp(buf, item))
		{
			SendMessage(ctrl->handle, LB_DELETESTRING, (WPARAM)i, 0);
		}
	}
	return;
}

void CTRL_combo_delitem_idx(NWC_PARENT* p_window, char* name, int idx)
{
	NWC_CTRL* ctrl;
	int count;

	count = 0;

	if (!p_window || !name)
	{
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (DelitemIDXlist)", 0);
		return;
	}
	if (ctrl->type != COMBO)
		return;

	if ((count = SendMessage(ctrl->handle, CB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelitemIDXlist)", 0);
		return;
	}

	if (idx > count || idx < 0)
		return;

	SendMessage(ctrl->handle, CB_DELETESTRING, (WPARAM)idx, 0);
	return;
}

void CTRL_List_delitem_idx(NWC_PARENT* p_window, char* name, int idx)
{
	NWC_CTRL* ctrl;
	int count;

	count = 0;

	if (!p_window || !name)
	{
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (DelitemIDXlist)", 0);
		return;
	}
	if (ctrl->type != LISTBOX)
		return;

	if ((count = SendMessage(ctrl->handle, LB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelitemIDXlist)", 0);
		return;
	}

	if (idx > count || idx < 0)
		return;

	SendMessage(ctrl->handle, LB_DELETESTRING, (WPARAM)idx, 0);
	return;
}

int CTRL_combo_get_sel_idx(NWC_PARENT* p_window, char* name)
{
	NWC_CTRL* ctrl;
	int count;

	count = -1;

	if (!p_window || !name)
	{
		return -1;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (DelitemIDXlist)", 0);
		return -1;
	}
	if (ctrl->type != COMBO)
		return -1;

	if ((count = SendMessage(ctrl->handle, CB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelitemIDXlist)", 0);
		return -1;
	}

	count = SendMessage(ctrl->handle, CB_GETCURSEL, 0, 0);

	if (count == CB_ERR)
		return -1;
	return count;
}

int CTRL_list_get_sel_idx(NWC_PARENT* p_window, char* name)
{
	NWC_CTRL* ctrl;
	int count;

	count = -1;

	if (!p_window || !name)
	{
		return -1;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Listview was not found. (DelitemIDXlist)", 0);
		return -1;
	}
	if (ctrl->type != LISTBOX)
		return -1;

	if ((count = SendMessage(ctrl->handle, LB_GETCOUNT, 0, 0)) == LB_ERR)
	{
		GiveError("Sendmessage error for listview (DelitemIDXlist)", 0);
		return -1;
	}

	count = SendMessage(ctrl->handle, LB_GETCURSEL, 0, 0);

	if (count == LB_ERR)
		return -1;
	return count;
}

void clist_add_col(NWC_PARENT* p_window, char* name, int width, char* text)
{
	NWC_CTRL* ctrl;
	LV_COLUMN pcol;

	if (!p_window || !name || !text)
	{
		GiveError("Listbox Error", 0);
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Control was not found for listview.", 0);
		return;
	}

	if (strlen(text) > 1024) // Truncate to 1024 bytes.
		text[1024] = '\0';

	if (ctrl->type != LISTBOX)
		return;

	pcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	pcol.fmt = LVCFMT_LEFT;
	pcol.cx = width;
	pcol.pszText = text;

	if (ListView_InsertColumn(ctrl->handle, ctrl->clist_index, &pcol) == -1)
	{
		GiveError("Failed to insert Column into list view.", 1);
		return;
	}
	ctrl->clist_index++;
	return;
}

void clist_clear_list(NWC_PARENT* p_window, char* name)
{
	int i;
	NWC_CTRL* ctrl;

	if (!p_window || !name)
	{
		GiveError("Listbox Error", 0);
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Control was not found for listview.", 0);
		return;
	}

	if (ctrl->type != LISTBOX)
		return;

	for (i = ctrl->clist_index - 1; i >= 0; i--)
		ListView_DeleteItem(ctrl->handle, i);

	ctrl->clist_index = 0;

	return;
}

void clist_add_data(NWC_PARENT* p_window, char* name, char** text, int count)
{
	NWC_CTRL* ctrl;
	LVITEM listItem;
	LVITEM subItem;
	int i;

	if (!p_window || !name || !text)
	{
		GiveError("Listbox Error", 0);
		return;
	}

	ctrl = get_control(p_window, name);

	if (!ctrl || ctrl == NULL)
	{
		GiveError("Control was not found for listview.", 0);
		return;
	}

	//	if (ctrl->type != LISTBOX)
		//	return;

	listItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_DI_SETITEM;
	listItem.state = 0;
	listItem.stateMask = 0;
	listItem.pszText = text[0];
	listItem.iItem = ctrl->clist_index;

	listItem.iSubItem = 0;

	listItem.lParam = (LPARAM)NULL;

	ListView_InsertItem(ctrl->handle, &listItem);

	for (i = 0; i < count; i++)
	{
		subItem.iItem = ctrl->clist_index;
		subItem.iSubItem = i;
		subItem.mask = LVIF_TEXT;
		subItem.pszText = text[i];
		ListView_SetItem(ctrl->handle, &subItem);
		//ctrl->clist_index++;
	}
	ctrl->clist_index++;

	return;
}

BOOL AddRichedit_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddEdit_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-Edit does not exist. Using a generic name.", FALSE);
		name = str_dup("Editbox");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-Edit", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = RICHEDIT;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_CLIENTEDGE, "RICHEDIT", ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

BOOL AddList_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	//	INITCOMMONCONTROLSEX listctrl;
	NWC_CTRL* ctrl;

	if (!p_window)
	{
		GiveError("AddList_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-List does not exist. Using a generic name.", FALSE);
		name = str_dup("Listbox");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-List", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL;

	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = LISTBOX;
	ctrl->id = id;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

BOOL AddCList_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show)
{
	//	INITCOMMONCONTROLSEX listctrl;
	NWC_CTRL* ctrl;
	int sstyle;

	if (!p_window)
	{
		GiveError("AddList_Parent () attempted to add a button to a non-existant window (Parent). Attempting to exit gracefully.", TRUE);
		return FALSE;
	}

	if (!name || name[0] == '\0')
	{
		GiveError("Name of parent-List does not exist. Using a generic name.", FALSE);
		name = str_dup("Listbox");
	}

	if (handle < 0)
	{
		GiveError("Handle range is too small to create parent-List", FALSE);
		return FALSE;
	}

	ctrl = new_control();

	if (!ctrl)
	{
		GiveError("Control failed to create properly. Exiting gracefully.", TRUE);
		return FALSE;
	}

	ctrl->name = str_dup(name);
	ctrl->x = x;
	ctrl->y = y;
	ctrl->width = width;
	ctrl->height = height;
	ctrl->style |= style | WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | LVS_SINGLESEL | LVS_NOSORTHEADER;

	ctrl->handle = handle;
	ctrl->parent = p_window;
	ctrl->type = LISTBOX;
	ctrl->id = id;
	ctrl->clist_index = 0;
	add_control_parent(p_window, ctrl);

	if (p_window->window_pointer == NULL)
	{
		show_parent(p_window);
	}

	ctrl->handle = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, ctrl->name, ctrl->style, ctrl->x, ctrl->y, ctrl->width, ctrl->height, p_window->window_pointer, (HMENU)ctrl->id, g_hInst, 0);
	sstyle = SendMessage(ctrl->handle, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	sstyle = style | LVS_EX_FULLROWSELECT;
	SendMessage(ctrl->handle, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, sstyle);

	ShowWindow(ctrl->handle, SW_SHOW);
	return TRUE;
}

void DestroyParent(NWC_PARENT* p_window)
{
	int i;
	int f;

	f = 0;

	if (!p_window)
	{
		GiveError("Bad Window", 0);
		return;
	}
	if (p_window == fake_window)
		return; // We don't want to get rid of our fake Window.
	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;

		free(p_window->controls[i]->name);
		DestroyWindow(p_window->controls[i]->handle);
		free(p_window->controls[i]);
		p_window->controls[i] = NULL;
		f++;
	}

	for (i = 0; i < max_parents; i++)
	{
		if (parents[i] == NULL)
			continue;
		if (parents[i] == p_window)
			parents[i] = NULL;
	}

	free(p_window->controls);
	free(p_window->name);
	//DestroyWindow(p_window->window_pointer);
	free(p_window);
	p_window = NULL;
	return;
}

void parent_disable_all(NWC_PARENT* p_window)
{
	int i;

	if (!p_window)
		return;

	for (i = 0; i < p_window->max_controls; i++)
	{
		if (p_window->controls[i] == NULL)
			continue;
		EnableWindow(p_window->controls[i]->handle, FALSE);
	}
	return;
}

NWC_PARENT* NWC_GetParent(HWND hwnd)
{
	int i;

	if (!hwnd)
		return NULL;
	for (i = 0; i <= max_parents; i++)
	{
		if (parents[i] == NULL)
			continue;
		if (parents[i]->window_pointer == hwnd)
			return parents[i];
	}
	return NULL;
}

BOOL CenterWindow(HWND hwnd, HWND hwndParent)
{
	RECT rect, rectP;
	int width, height;
	int screenwidth, screenheight;
	int x, y;

	//make the window relative to its parent

	GetWindowRect(hwnd, &rect);
	GetWindowRect(hwndParent, &rectP);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	x = ((rectP.right - rectP.left) - width) / 2 + rectP.left;
	y = ((rectP.bottom - rectP.top) - height) / 2 + rectP.top;

	screenwidth = GetSystemMetrics(SM_CXSCREEN);
	screenheight = GetSystemMetrics(SM_CYSCREEN);

	//make sure that the dialog box never moves outside of
	//the screen
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x + width > screenwidth)  x = screenwidth - width;
	if (y + height > screenheight) y = screenheight - height;

	MoveWindow(hwnd, x, y, width, height, FALSE);

	return TRUE;
}

/* Get the width of the entire window, based on the controls within
 * it. Iterate through all controls, finding the furthest width that cacn
 * be given with the widest and furthest out placed control.
 * Only use this after placing all controls in their proper location.
 * Or use it after moving controls / resizzing controls. This is useful for making
 * the window the proper width without having to guess numbers for width.
 */

int get_max_control_width(NWC_PARENT* p)
{
	int i;
	int cur_width;
	int width = 0;
	int count = 0;

	cur_width = 0; // Starting point.

	if (!p)
		return 0;

	for (i = 0; i < p->max_controls; i++)
	{
		if (p->controls[i] == NULL)
			continue;
		count++;
		width = (p->controls[i]->x + p->controls[i]->width);
		//	width = (width * .05) + width;// Total width is x pos + width of control plus 1 for good measure.
		if (width > cur_width)
			cur_width = width;
	}
	cur_width = cur_width + 15;
	return cur_width;
}
/* Get the heigth of the entire window based on the total heigth of
 * the lowest placed control. Same as get_max_control_width, but only
 * for the heigth -- Use only when all controls are placed or when
 * controls are resized and/or moved.
 */
int get_max_control_heigth(NWC_PARENT* p)
{
	int i;
	int cur_height;
	int height = 0;
	int count = 0;
	cur_height = 0; // Starting point.

	if (!p)
		return 0;

	for (i = 0; i < p->max_controls; i++)
	{
		if (p->controls[i] == NULL)
			continue;
		count++;
		height = p->controls[i]->y + p->controls[i]->height; // Total width is x pos + width of control plus 1 for good measure.
	//	height = (height * .05) + height;
		if (height > cur_height)
			cur_height = height;
	}
	cur_height = cur_height + (count * 2);
	return cur_height;
}


struct collate_ctrls
{
	char* parent_name;
	int x;
	int y;
	bool visible;
	int type;
	int ctrl_num;
	DWORD flags;
	NWC_CTRL* ctrl;

};

struct collate_window 
{
	char* name;
	int x;
	int y;
	int ctrl_count;
	bool ontop;

	NWC_PARENT* parent;
	struct collate_ctrls** ctrls;
	 
};


struct collate_window *new_collate_window(void)
{
	struct collate_window *cwin;

	cwin = malloc(sizeof(struct collate_window));
	if (!cwin)
		exit(1);

	return cwin;
}

struct collate_ctrls* new_collate_ctrl(void)
{
	struct collate_ctrls* ctrl;

	ctrl = malloc(sizeof(struct collate_ctrls));
	if (!ctrl)
		exit(1);

	return ctrl;
}

/* Go through all the parents and children, turning them in to a 
 * list that we can print out or view as a user to see what our
 * layouts are, without viewing the windows. Has no real use outside
  * of bookkeeping or showing off to others.
  */
void collate_windows(void)
{
	NWC_PARENT* p;
	NWC_CHILD* c;
	NWC_CTRL* ctrl;
	struct collate_window** window_list;
	struct collate_window *t_win;
	struct collate_ctrls t_ctrl;


	int pcount=0, ccount = 0;	  
	int i, j = 0;

	window_list = (struct collate_window**)malloc(sizeof(struct collate_window*) * (current_parents+10));
	
	t_win = new_collate_window();

	t_win->ctrls = NULL;
	t_win->ctrl_count = 0;
	t_win->name = NULL;
	t_win->ontop = FALSE;
	t_win->parent = NULL;
	t_win->x = t_win->y = 0;


	for (i = 0; i <= max_parents; i++)
	{
		// Sort through the parents first. THen we'll go to each child of each parent, then each control of each child.

		if (parents[i] == NULL)
			continue;
		if (!parents[i])
			continue;
		p = parents[i]; // We have a parent. Let's collate its info. Using the structure above that we can sort by...stuff.

		window_list[pcount] = t_win;

		pcount++;
		
		if (pcount > max_parents)
			break;
		
		window_list[pcount] = NULL;

		t_win->name = str_dup(p->name);
		t_win->parent = p;
		t_win->x = p->x;
		t_win->y = p->y;
		t_win->ontop = p->on_top;
		t_win->ctrl_count = p->control_count;
		if (t_win->ctrls == NULL)
		{
			t_win->ctrls = (struct collate_ctrls**)malloc(sizeof(struct collate_ctrls*) * (t_win->ctrl_count + 10));
			for (j = 0; j < t_win->ctrl_count; j++)
				t_win->ctrls[j] = NULL;
		}



	//	p->
	}
}

#define nwc_create_widget() ()
void* nwc_widget_create_entry(NWC_PARENT *self)
{


	
	return NULL;

}



void NWC_BoundBox(NWC_PARENT *p,unsigned int x, unsigned int y)
{
	NWC_WIDGET* w;

	if (!p)
		return;

	if (p->control_count < 1)
		return; // Why bother?

	

}