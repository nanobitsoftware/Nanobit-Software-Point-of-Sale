

#define ID_CALC_INPUT 20000 // Input and answer bar
#define ID_CALC_1  10000	// BUtton:1 
#define ID_CALC_2  10001	// Button:2
#define ID_CALC_3  10002	// Button:3
#define ID_CALC_4  10003	// Button:4
#define ID_CALC_5  10004	// Button:5
#define ID_CALC_6  10005	// Button:6
#define ID_CALC_7  10006	// Button:7
#define ID_CALC_8  10007	// Button:8
#define ID_CALC_9  10008	// Button:9
#define ID_CALC_0  10009	// Button:0
#define ID_CALC_A  10010	// Button:+
#define	ID_CALC_S  10011	// Button:-
#define ID_CALC_D  10012	// Button:/
#define ID_CALC_M  10013	// Button:*
#define ID_CALC_E  10014	// Button:=
#define ID_CALC_T  10015	// Button:Tax%
#define ID_CALC_C  10016	// Button:Clear
#define ID_CALC_N  10017	// Button:NEG/POS
#define ID_CALC_DE 10018	// Button:.
#define ID_CALC_B  10019	//Button:<--
#define ID_CALC_CE 10020	// Button:CE
#define CALC_H 40
#define CALC_W  50
#define CALC_X 206//225  // X Size
#define CALC_Y 234//276	//   Y Size
#define CALC_ONTOP TRUE
#define CALC_INPUT_WIDTH 500
#define CALC_INPUT_HEIGHT 40
#define CALC_BTN_SPACE 50
#define CALC_BTN_VSPACE 40








struct calc_layout
{
	char text[128];
	int  y;
	int  x;
	int  width;
	int  heiht;
	int  id;

};






double tax_quote();
double get_add_percent(double total, float percent);
double get_minus_percent(double total, float percent);
double get_percent(double total, float percent);
double multiply_quote(void);
double divide_quote(void);
inline double subtract_quote(void);
inline double add_quote(void);
void calc_append_number(int num);