#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "gopt.h"
#include "ttyctl.h"
#include "iun_err.h"
#include "iun_time.h"

// Options
// Characters of padding.
int padlen = 8;
// How often to update the timer on screen.
struct timespec pad_upd_delay = {.tv_sec = 0, .tv_nsec = 33333333};
// Accumulate time instead of starting from zero on each line.
_Bool accumu = 0;
// Use exponentiial (lowercase e) mode.
_Bool expmode = 0;
// Use exponential (uppercase E) mode.
_Bool Expmode = 0;
// Force file write mode.
_Bool fmode = 0;
// Buffer length in file mode. This is the maximum length for an output line.
size_t fbuflen = (1<<20) - 1;
// Digits after the decimal point
int prec = 4;
// Set number of cols (0 == not set).
int cols_setting = 0;
// Terminal lines (may be autodetected later).
int lines_tty = 24;
// Maximum length of pad0 and pad1 strings.
#define PADBUF_LEN 64
// Separator between padding and output.
const char *padsep = "|";
// Use UTC timezone in date mode instead of local time.
_Bool utc_mode = 0;
// Format for wcsftime in date mode, before and after fractional seconds.
// NULL == date mode disabled.
const char *tfmt_pre = NULL;
const char *tfmt_post = NULL;
// String lengths required for pre and post formats
int tfmt_pre_l = 0;
int tfmt_post_l = 0;
// Suppress output (only output timing information).
_Bool suppress_output = 0;

// Number of output lines printed for current line.
size_t lins_printed = 0;
// Number of characters printed on current output line.
size_t cs_printed = 0;

char pad1[PADBUF_LEN];

/*
 * Parses the command line options in a format meant to comply with the POSIX
 * utility syntax guidelines. Also sets and prints the help message and checks
 * for some error conditions.
 */
void parse_args(int *argc, const char **argv)
{
	void *options;
	const char *str;
	char descr[] =
"\
Usage: linetimer [OPTION]...\
\n\n\
DESCRIPTION\n\
\tReads standard input and prints the read lines with times in seconds on\n\
\tthe left margin. Times are measured between the creation of a line and\n\
\ta following newline or EOF.\n\
\n\
\t-a, --accumulate\n\
\t\tAccumulate times instead of starting from zero for each line.\n\
\n\
\t-c, --columns=INTEGER\n\
\t\tSet the number of columns to assume for the output terminal or\n\
\t\tfile. The default is to read the width of the terminal or to\n\
\t\tuse padlen + 80 columns when writing to a file. If set to 0\n\
\t\tthen essentially infinite (INT_MAX) columns are assumed, which\n\
\t\talso turns on file-mode.\n\
\n\
\t-d, --date=STRING\n\
\t\tOutput date of each line's completion instead of time elapsed.\n\
\t\tAutomatically sets padlen appropriately. STRING is one of:\n\
\t\t\t'l[ocal]' use local time\n\
\t\t\t'u[tc]' use UTC\n\
\t\tThe output format is hardcoded to the only sane format in\n\
\t\texistence.\n\
\n\
\t-e, --exp-mode\n\
\t\tPrint times using exponential mode with lowercase 'e'.\n\
\n\
\t-E, --Exp-mode\n\
\t\tSame as --exp-mode, but with uppercase 'E'.\n\
\n\
\t-f, --file-mode\n\
\t\tDon't output terminal escape sequences. Lines will not appear on\n\
\t\tscreen before they can be printed in full.\n\
\n\
\t-h, --help\n\
\t\tPrint this help message.\n\
\n\
\t-l, --padlen=INTEGER\n\
\t\tSet the number of characters to pad on the left with. The\n\
\t\tdefault is 8. It may be advantageous to use a multiple of the\n\
\t\ttab size of the terminal to avoid ugly output from programs\n\
\t\tthat use tabs for alignment.\n\
\n\
\t-p, --precision=INTEGER\n\
\t\tSet the desired number of digits after the decimal point. If\n\
\t\tpadlen is not large enough to accomodate the number then the\n\
\t\tprecision is automatically decreased. The default is 4, which\n\
\t\talso seems to be roughly the accuracy of the timing (but the\n\
\t\taccuracy can of course vary from system to system).\n\
\n\
\t-s, --separator=STRING\n\
\t\tSet the separator between the timing or padding and the output.\n\
\n\
\t-t, --time-only\n\
\t\tOutput timing only. Also sets the separator to an empty string\n\
\t\tunless it is overridden with -s. Useful for example to store\n\
\t\ttiming info in a separate file by doing something like:\n\
\t\t\tsomecommand | tee out.txt | linetimer -t >timing.txt\n\
";

	options = gopt_sort(argc, argv, gopt_start(
	gopt_option('a', 0,        gopt_shorts('a'), gopt_longs("accumulate")),
	gopt_option('c', GOPT_ARG, gopt_shorts('c'), gopt_longs("columns")),
	gopt_option('d', GOPT_ARG, gopt_shorts('d'), gopt_longs("date")),
	gopt_option('e', 0,        gopt_shorts('e'), gopt_longs("exp-mode")),
	gopt_option('E', 0,        gopt_shorts('E'), gopt_longs("Exp-mode")),
	gopt_option('f', 0,        gopt_shorts('f'), gopt_longs("file-mode")),
	gopt_option('h', 0,        gopt_shorts('h'), gopt_longs("help")),
	gopt_option('l', GOPT_ARG, gopt_shorts('l'), gopt_longs("padlen")),
	gopt_option('p', GOPT_ARG, gopt_shorts('p'), gopt_longs("precision")),
	gopt_option('s', GOPT_ARG, gopt_shorts('s'), gopt_longs("separator")),
	gopt_option('t', 0,        gopt_shorts('t'), gopt_longs("time-only"))
	));

	// Write help message if requested.
	if (gopt(options, 'h')) {
		fputs(descr, stdout);
		exit(EXIT_SUCCESS);
	}

	accumu = gopt(options, 'a');

	if (gopt_arg(options, 'c', &str)) {
		cols_setting = strtol(str, NULL, 10);
		if (0 == cols_setting)
			cols_setting = INT_MAX;
	}

	// Must be set before 'd'.
	if (gopt_arg(options, 'p', &str))
		prec = (int)strtoul(str, NULL, 10);

	if (gopt_arg(options, 'd', &str)) {
		tfmt_pre = "%FT%T";
		tfmt_post = "%z";
		tfmt_pre_l = 19;
		tfmt_post_l = 5;
		if (str[0] == 'u')
			utc_mode = 1;
		else if (str[0] != 'l')
			iun_err("option '-d' requires either 'l' or 'u'.\n");
		// NB: reusing str
		if (!gopt_arg(options, 'l', &str)) {
			int len_req = tfmt_pre_l + tfmt_post_l + 1 + prec
				+ strlen(padsep);
			padlen = (len_req / 8) * 8 + (len_req % 8 ? 8 : 0);
		}
	}

	expmode = gopt(options, 'e');

	Expmode = gopt(options, 'E');

	fmode = gopt(options, 'f');

	if (gopt_arg(options, 'l', &str)) {
		padlen = (int)strtoul(str, NULL, 10);
	}

	if (gopt_arg(options, 's', &str))
		padsep = str;

	if (gopt(options, 't')) {
		suppress_output = 1;
		if (!gopt(options, 's'))
			padsep = "";
	}

	// Catch some common errors.
	if ((int)strlen(padsep) >= padlen)
		iun_err("Separator too long or padlen too short.\n");

	if (PADBUF_LEN <= padlen)
		iun_err("padlen exceeds maximum length.\n");

	gopt_free(options);
}

/*
 * Conditional timer: use iun_timer (best for intervals) when in timing mode,
 * but use CLOCK_REALTIME when in date mode (tfmt_pre != NULL).
 */
struct timespec timer_cond(void)
{
	if (NULL == tfmt_pre)
		return iun_timer(NULL);
	else {
		struct timespec ts;
		if (0 != clock_gettime(CLOCK_REALTIME, &ts))
			iun_err("clock_gettime error: %s\n", strerror(errno));
		return ts;
	}
}

/*
 * Writes a time (in seconds) to str using the specified precision, but
 * automatically decrease precision if needed to fit the time in the string
 * (only in seconds mode, not date mode).
 *
 * str_s:   the size of str (number of characters).
 * ts:      the time to be printed. Interpreted as an interval, not a date.
 * str:     the string to write to.
 */
void write_time(size_t str_s, struct timespec ts, char *str)
{
	int fw = str_s - 1;
	double t = ts.tv_sec + ts.tv_nsec * 1E-9;

	if (tfmt_pre == NULL) { // Seconds mode
		char *fmt = NULL;
		if (expmode)
			fmt = "%*.*e";
		else if (Expmode)
			fmt = "%*.*E";
		else
			fmt = "%*.*f";

		// Search down in precision until things fit.
		for (int i = prec; i >= 0; i--)
			if (snprintf(str, str_s, fmt, fw, i, t) == fw)
				return;

		// If that failed then try exp notation instead
		fmt = "%*.*e";
		for (int i = prec; i >= 0; i--)
			if (snprintf(str, str_s, fmt, fw, i, t) == fw)
				return;

		iun_err("swprintf error (perhaps padlen too small).\n");
	} else { // Date mode
		char pre[tfmt_pre_l + 1];
		char post[tfmt_post_l + 1];

		struct tm *ts_tm;
		if (utc_mode)
			ts_tm = gmtime(&ts.tv_sec);
		else
			ts_tm = localtime(&ts.tv_sec);
		if (NULL == ts_tm)
			iun_err("time error: %s\n", strerror(errno));

		strftime(pre, tfmt_pre_l + 1, tfmt_pre, ts_tm);
		strftime(post, tfmt_post_l + 1, tfmt_post, ts_tm);
		if ('\0' != pre[tfmt_pre_l] || '\0' != post[tfmt_post_l])
			iun_err("unexpected date format length.\n");

		// Need string of fractional seconds, hack it!
		char nsecstr[prec + 3];
		snprintf(nsecstr, prec + 3, "%.*f", prec, ts.tv_nsec * 1E-9);

		// Concatenate the strings
		int n = snprintf(str, str_s, "%*s%s%s",
			(int)(fw - (strlen(nsecstr) - 1) - strlen(post)),
			pre, &nsecstr[1], post);
		if (0 <= n && (size_t)n < str_s)
			return;
		iun_err("swprintf error (perhaps padlen too small).\n");
	}
}

void set_and_print_pad0(struct timespec t_ref, _Bool ttymode)
{
	char pad0[padlen + 1];
	char tstr[padlen + 1];
	struct timespec t_print;
	if (NULL == tfmt_pre)
		t_print = iun_dt(t_ref, timer_cond());
	else
		t_print = timer_cond();
	write_time(padlen - strlen(padsep) + 1, t_print, tstr);
	snprintf(pad0, padlen + 1, "%s%s", tstr, padsep);

	// Avoid attempting to scroll off the screen. Output may be ugly
	// and/or confusing, but at least it won't break.
	int sclines = (lins_printed > (size_t)(lines_tty - 1))
		? (size_t)(lines_tty - 1) : lins_printed;
	if (ttymode) {
		curs_horpos(1);
		curs_up(sclines);
	}
	fputs(pad0, stdout);
	if (ttymode) {
		curs_down(sclines);
		curs_horpos(strlen(pad0) + cs_printed + 1);
	}
}

void set_pad1(void)
{
	snprintf(pad1, padlen + 1, "%*s%s", (int)(padlen - strlen(padsep)),
		"", padsep);
}

void print_pad1(void)
{
	printf("%s", pad1);
}

void print_wchar(const wchar_t c, int cols)
{
	if (L'\n' == (wint_t)c) {
		lins_printed = 0;
		cs_printed = 0;
	} else {
		// Do we need a new output line first?
		int c_width = wcwidth(c);
		if (padlen + cs_printed + c_width > (size_t)cols) {
			lins_printed++;
			cs_printed = 0;
			putchar('\n');
			print_pad1();
		}
		cs_printed += c_width;
	}

	char mb[MB_CUR_MAX];
	int nmb = wctomb(mb, c);
	if ((int)MB_CUR_MAX < nmb || 0 >= nmb)
		iun_err("wctomb error.\n");
	for (int i = 0; i < nmb; i++)
		putchar(mb[i]);
}

void writing_to_terminal(int cols)
{
	wint_t c = 0;
	struct timespec t_ref = timer_cond();
	set_and_print_pad0(t_ref, 1);

	// Two threads. Thread 0 both reads and writes characters, while
	// thread 1 only updates the timer.
	#pragma omp parallel num_threads(2), default(shared)
	do {
		if (0 == omp_get_thread_num())
			c = fgetwc(stdin);
		else if (1 == omp_get_thread_num())
			iun_sleepmult(iun_dt(t_ref, timer_cond()),
				pad_upd_delay, NULL);

		// Printing done here, only one thread at a time.
		#pragma omp critical
		{
			if (1 == omp_get_thread_num() && WEOF != c) {
				set_and_print_pad0(t_ref, 1);
				fflush(stdout);
			} else if (0 == omp_get_thread_num()) {
				if (L'\n' == c) {
					set_and_print_pad0(t_ref, 1);
					print_wchar(c, cols);
					if (!accumu)
						t_ref = timer_cond();
					set_and_print_pad0(t_ref, 1);
				} else if (WEOF == c) {
					set_and_print_pad0(t_ref, 1);
					putchar('\n');
					exit(EXIT_SUCCESS);
				} else if (!suppress_output){
					print_wchar(c, cols);
				}
			}
		}
	} while (WEOF != c);
}

void writing_to_file(int cols)
{	
	wchar_t *fbuf = malloc((fbuflen + 1) * sizeof(*fbuf));
	if (NULL == fbuf)
		iun_err("malloc failure.\n");
	struct timespec t_ref = timer_cond();

	while (NULL != fgetws(fbuf, fbuflen + 1, stdin)) {
		size_t fbl = wcslen(fbuf);
		_Bool fbuf_too_small = 0;

		if (fbl == fbuflen && fbuf[fbl] != L'\n' && !feof(stdin))
			fbuf_too_small = 1;

		if (fbuf_too_small && suppress_output)
			continue;

		if (fbuf_too_small)
			printf("%-*s", padlen, "OFLOW");
		else
			set_and_print_pad0(t_ref, 0);

		if (!accumu && !fbuf_too_small)
			t_ref = timer_cond();

		if (!suppress_output) {
			for (size_t i = 0; i < fbl; i++)
				print_wchar(fbuf[i], cols);
			if (fbuf_too_small)
				print_wchar(L'\n', cols);
		} else if (!fbuf_too_small) {
			print_wchar(L'\n', cols);
		}
	};

	if (ferror(stdin))
		iun_err("input error.\n");

	free(fbuf);
}

int main(int argc, char **argv)
{
	setlocale(LC_CTYPE, "");
	setlocale(LC_TIME, "");
	parse_args(&argc, (const char **)argv);
	set_pad1();

	int cols_tty = 80;
	int cols_file = 80 + padlen;

	if (isatty(STDOUT_FILENO)) {
		#ifdef TIOCGWINSZ
		struct winsize w;
		if (-1 == ioctl(STDOUT_FILENO, TIOCGWINSZ, &w))
			iun_err("ioctl error.\n");
		cols_tty = w.ws_col;
		lines_tty = w.ws_row;
		if (cols_setting > cols_tty)
			fmode = 1;
		#endif
	}

	if (cols_setting)
		cols_tty = cols_file = cols_setting;

	if (isatty(STDOUT_FILENO) && !fmode) {
			writing_to_terminal(cols_tty);
	} else if (isatty(STDOUT_FILENO) && fmode) {
		writing_to_file(cols_tty);
	} else {
		writing_to_file(cols_file);
	}
}

