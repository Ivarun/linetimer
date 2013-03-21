/*
 * iunio_err.h: header-only simple error message printing.
 */

#ifndef IUN_ERR_INCLUDED
#define IUN_ERR_INCLUDED

#include <stdio.h>
#include <stdlib.h>

/*
 * Print error message and exit.
 */
#define iun_err(...) \
	do { \
		fprintf(stderr, "ERROR in '%s' (%s:%d): ", __func__, \
			__FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while (0)

#endif // IUN_ERR_INCLUDED

