#include "test.h"
#include "../lib.h"
#include "../types.h"

#include "../devices/keyboard.h"
#include "../devices/rtc.h"
#include "../fs.h"

// static void exec(char * s, int (*test_fn)());
static void exec_print(char * s, void (*test_fn)());
static void wait();

/* add test declarations here */
static void test_term_write();
static void test_rtc();
static void test_term_read();

void test() {
	clear();

	exec_print("Testing terminal write. See source for info.", test_term_write);
	exec_print("Testing terminal read.", test_term_read);
	exec_print("Testing rtc read and write. You should see 0-9 listed "
		"2 per second and then 10-19 much more quickly.",
		test_rtc);

	printf("Testing complete.\n");
}

/* ================ TEST FUNCTIONS BELOW ================ */
void test_rtc() {
	int rate, i;

	rtc_open(0);
	for(i = 0; i < 10; i++) {
		printf("%d\n", i);
		rtc_read(0, NULL, 0);
	}

	rate = 8;
	rtc_write(0, &rate, 0);

	for(i = 10; i < 20; i++) {
		printf("%d\n", i);
		rtc_read(0, NULL, 0);
	}
	rtc_close(0);
}

void test_term_write() {
	terminal_write(0, "hi\n", 3);
	terminal_write(0, "world\n", 3);
	terminal_write(0, " world\n", 7);
	terminal_write(0, "hello world\n", 20);
}

void test_term_read() {
	char buf1[5], buf2[5];
	printf("Please type 4 characters and hit enter:\n");
	terminal_read(0, buf1, 5);

	terminal_write(0, "You typed: ", 11);
	terminal_write(0, buf1, 5);
	printf("\n");

	printf("Please type 9 characters and hit enter twice:\n");
	terminal_read(0, buf1, 5);
	terminal_read(0, buf2, 5);

	terminal_write(0, "First 5-byte read: ", 19);
	terminal_write(0, buf1, 5);
	printf("\n");
	terminal_write(0, "Second 5-byte read: ", 20);
	terminal_write(0, buf2, 5);
}

/* ====================================================== */

void exec(char * s, int (*test_fn)()) {
	printf(s);
	printf(": ");

	if(test_fn()) {
		printf("PASSED\n");
	} else {
		printf("FAILED\n");
	}
}

void exec_print(char * s, void (*test_fn)()) {
	clear();

	printf(s);
	wait();

	test_fn();

	wait();
}

void wait() {
	char buf[1];
	printf("\nPress enter to continue...\n");
	terminal_read(0, buf, 0);
}
