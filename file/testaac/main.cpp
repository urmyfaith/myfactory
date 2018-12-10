#include "ts_test.h"
#include "generator_test.h"
#include <string.h>
#include <stdio.h>
#include<stdlib.h>
#include <signal.h>
#include <execinfo.h>

static void SegvHandler(int signum) {
    void *array[10];
    size_t size;
    char **strings;
    size_t i, j;

    signal(signum, SIG_DFL);

    size = backtrace (array, 10);
    strings = (char **)backtrace_symbols (array, size);

    fprintf(stderr, "SegvHandler received SIGSEGV! Stack trace:\n");
    for (i = 0; i < size; i++) {
        fprintf(stderr, "%d %s \n",i,strings[i]);
    }

    free (strings);
    exit(1);
}

int main()
{
	signal(SIGSEGV, SegvHandler); // SIGSEGV      11       Core    Invalid memory reference
	signal(SIGABRT, SegvHandler); // SIGABRT       6       Core    Abort signal from

//	return testts1();
//	return testts2();
//	return testts3();

//	return testaac3();

//	return testaac4();

//	return testaac5();
 
//    return testaac6();

//    return testaac7();
//    return testaac8();

//    return testaac9();
    return testaac10();
}