/*
 * ttyctl.h: Header-only simple tty control using ANSI escape sequences.
 */

#ifndef TTYCTL_H_INCLUDED
#define TTYCTL_H_INCLUDED

#include <stdio.h>

#include "iun_err.h"

// Console escape sequence buffer size
#define CESB_SIZ 16

/*
 * Move cursor up n lines.
 */
void curs_up(int n)
{
	char str[CESB_SIZ];
	if (n <= 0)
		return;
	snprintf(str, CESB_SIZ, "\033[%dA", n);
	if (EOF == fputs(str, stdout))
		iun_err("output error.\n");
}

/*
 * Move cursor down n lines.
 */
void curs_down(int n)
{
	char str[CESB_SIZ];
	if (n <= 0)
		return;
	snprintf(str, CESB_SIZ, "\033[%dB", n);
	if (EOF == fputs(str, stdout))
		iun_err("output error.\n");
}

/*
 * Move cursor to horizontal position n
 */
void curs_horpos(int n)
{
	char str[CESB_SIZ];
	if (n <= 0)
		iun_err("invalid argument n=%d.\n", n);
	snprintf(str, CESB_SIZ, "\033[%dG", n);
	if (EOF == fputs(str, stdout))
		iun_err("output error.\n");
}

#endif // TTYCTL_H_INCLUDED

