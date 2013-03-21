#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>

int main(void)
{
	struct timespec reqtime;

	printf("A line first...\n");
	//fflush(stdout);

	putchar('\n');
	//fflush(stdout);

	putchar('\n');
	//fflush(stdout);

	for (int i = 0; i < 5; i++) {
		reqtime.tv_sec = 0;
		reqtime.tv_nsec = 1E9 * 0.001;
		printf("1 ms: ");
		//fflush(stdout);
		nanosleep(&reqtime, NULL);
		printf("done\n");
		//fflush(stdout);
	}

	putchar('\n');
	//fflush(stdout);

	for (int i = 0; i < 5; i++) {
		reqtime.tv_sec = 0;
		reqtime.tv_nsec = 1E9 * 0.1;
		printf("100 ms: ");
		//fflush(stdout);
		nanosleep(&reqtime, NULL);
		printf("done\n");
		//fflush(stdout);
	}

	putchar('\n');
	//fflush(stdout);

	reqtime.tv_sec = 1;
	reqtime.tv_nsec = 1E9 * 0.5;
	printf("1.5 s: ");
	//fflush(stdout);
	nanosleep(&reqtime, NULL);
	printf("done\n");
	//fflush(stdout);

	reqtime.tv_sec = 0;
	reqtime.tv_nsec = 1E9 * 0.005;
	printf("1.5 s (probably more because of printing). "
		"This line should wrap appropriately: ");
	//fflush(stdout);
	for (int i = 0; i < 300; i++) {
		nanosleep(&reqtime, NULL);
		putchar('x');
		fflush(stdout);
	}
	putchar('\n');
	//fflush(stdout);

	reqtime.tv_sec = 5;
	reqtime.tv_nsec = 1E9 * 0.555;
	printf("5.555 s: ");
	//fflush(stdout);
	nanosleep(&reqtime, NULL);
	printf("done");
	//fflush(stdout);

}

