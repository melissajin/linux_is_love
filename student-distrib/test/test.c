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
static void test_fs();

void test() {
	clear();

	exec_print("Testing terminal write. See source for info.", test_term_write);
	exec_print("Testing terminal read.", test_term_read);
	exec_print("Testing rtc read and write. You should see 0-9 listed "
		"2 per second and then 10-19 much more quickly.",
		test_rtc);
	exec_print("Testing file system, other tests need to comment in/comment out", test_fs);

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

void test_fs(){
 clear();
 dentry_t dentry;
 //uint8_t buf_text[200];
 //uint8_t buf_non_text[500];
 // uint8_t buf_large_text[6000];
 // uint8_t buf_dir[120];
 //uint32_t i;
 //uint32_t buf_len;

 /* TESTING BOOTBLOCK */
 /*
 printf("num dir entries: %d\n", bootblock->dir_entries_cnt);
 printf("num inodes: %d\n", bootblock->inode_cnt);
 printf("num data blocks: %d\n", bootblock->data_block_cnt);
 */
 
 /* TESTING READ_DENTRY_BY_NAME */
  printf("read by name ret val: %d\n", read_dentry_by_name((uint8_t*)"",&dentry));
  printf("fname: %s\n", dentry.fname);
  printf("ftype: %d\n", dentry.ftype);
  printf("inode: %d\n", dentry.inode);

  /* TESTING READ_DENTRY_BY_INDEX */
  printf("read by index ret val: %d\n", read_dentry_by_index(0, &dentry));
  printf("fname: %s\n", dentry.fname);
  printf("ftype: %d\n", dentry.ftype);
  printf("inode: %d\n", dentry.inode);
 
 
 /* TESTING FOR READ_DATA */
 /* IMPORTANT: the size of the text buffer used must be big enough to
      hold all the bytes read */

 /* test read text file */
 /*
 printf("TESTING READ TEXT FILE\n");
 buf_len = read_data(13,0,buf_text,100);
 printf("Number bytes read: %d\n", buf_len);
  for(i=0; i< buf_len; i++){
  putc(buf_text[i]);
 }
 */

 /* test read non-text file */
 /*
 printf("TESTING READ NON-TEXT FILE\n");
 buf_len = read_data(1,0,buf_non_text,5);
 printf("Number bytes read: %d\n", buf_len);
  for(i=0; i< buf_len; i++){
  putc(buf_non_text[i]);
 } 
 */

 /* test reading large file*/
 /*
 printf("TESTING READ LARGE FILE\n");
 buf_len = read_data(16,0,buf_large_text,5);
 printf("Number bytes read: %d\n", buf_len);
  for(i=0; i< buf_len; i++){
  putc(buf_large_text[i]);
 }
 */

 /* test read directory */
 /* 
 printf("TESTING READ DIRECTORY\n");
 buf_len = read_data(DIRECTORY_INODE ,2,buf_dir,10);
 puts((int8_t*)buf_dir);
 */
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
