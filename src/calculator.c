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

#define MAX_CALC_NUMBER 0xFFFFFF // Max allowed number that we can calculate. Smaller than most due to float.
#define MAX_NEST_CALC 256 // Max number of calculations we're allowed to do in a nest.

double CALC_TOTAL; // Total calculated during calc functions.
double LEFT_CALC; // Left side of quotent.
double RIGHT_CALC; // Right side of quotent.

inline double add_quote(void)
{
	return fround(LEFT_CALC + RIGHT_CALC);
}

inline double subtract_quote(void)
{
	return (CALC_TOTAL = fround(LEFT_CALC - RIGHT_CALC));
}

double divide_quote(void)
{
	if (LEFT_CALC == 0.00f || RIGHT_CALC == 0.00f)
	{
		return 0.00f;
	}
	return (CALC_TOTAL = fround(LEFT_CALC / RIGHT_CALC));
}

double multiply_quote(void)
{
	if (RIGHT_CALC == 0.00f || LEFT_CALC == 0.00f)
		return 0.00f;

	return (CALC_TOTAL = fround(LEFT_CALC * RIGHT_CALC));
}

double get_percent(double total, float percent)
{
	double answer = 0.00f;

	if (percent <= 0.00f)
		return total;

	answer = fround(((total * (percent / 100))));

	return answer;
}
double get_minus_percent(double total, float percent)
{
	if (percent <= 0.00f)
		return total;

	return  fround(total - get_percent(total, percent));
}

double get_add_percent(double total, float percent)
{
	if (percent <= 0.00f)
		return total;

	return fround(total + get_percent(total, percent));
}

double tax_quote()
{
	if (CALC_TOTAL == 0.00f || !TAX)
		return 0.00f;
	return (CALC_TOTAL = (get_add_percent(CALC_TOTAL, TAX)));
}