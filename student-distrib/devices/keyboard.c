/* keyboard.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "keyboard.h"
#include "pit.h"
#include "../i8259.h"
#include "../isr.h"
#include "../lib.h"
#include "../fs.h"
#include "../process.h"
#include "../sys_calls.h"
#include "../virtualmem.h"
#include "../x86_desc.h"

static int32_t terminal_open(const uint8_t* filename);
static int32_t terminal_close(int32_t fd);
static int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
static int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

// Scancode for keyboard keys
// Source: http://www.brokenthorn.com/Resources/OSDev19.html
static uint16_t kybd_keys [] = {

	KEY_UNKNOWN,	//0
	KEY_ESCAPE,		//1
	KEY_1,			//2
	KEY_2,			//3
	KEY_3,			//4
	KEY_4,			//5
	KEY_5,			//6
	KEY_6,			//7
	KEY_7,			//8
	KEY_8,			//9
	KEY_9,			//0xa
	KEY_0,			//0xb
	KEY_MINUS,		//0xc
	KEY_EQUAL,		//0xd
	KEY_BACKSPACE,	//0xe
	KEY_TAB,		//0xf
	KEY_Q,			//0x10
	KEY_W,			//0x11
	KEY_E,			//0x12
	KEY_R,			//0x13
	KEY_T,			//0x14
	KEY_Y,			//0x15
	KEY_U,			//0x16
	KEY_I,			//0x17
	KEY_O,			//0x18
	KEY_P,			//0x19
	KEY_LEFTBRACKET,//0x1a
	KEY_RIGHTBRACKET,//0x1b
	KEY_RETURN,		//0x1c
	KEY_LCTRL,		//0x1d
	KEY_A,			//0x1e
	KEY_S,			//0x1f
	KEY_D,			//0x20
	KEY_F,			//0x21
	KEY_G,			//0x22
	KEY_H,			//0x23
	KEY_J,			//0x24
	KEY_K,			//0x25
	KEY_L,			//0x26
	KEY_SEMICOLON,	//0x27
	KEY_QUOTE,		//0x28
	KEY_GRAVE,		//0x29
	KEY_LSHIFT,		//0x2a
	KEY_BACKSLASH,	//0x2b
	KEY_Z,			//0x2c
	KEY_X,			//0x2d
	KEY_C,			//0x2e
	KEY_V,			//0x2f
	KEY_B,			//0x30
	KEY_N,			//0x31
	KEY_M,			//0x32
	KEY_COMMA,		//0x33
	KEY_DOT,		//0x34
	KEY_SLASH,		//0x35
	KEY_RSHIFT,		//0x36
	KEY_KP_ASTERISK,//0x37
	KEY_RALT,		//0x38
	KEY_SPACE,		//0x39
	KEY_CAPSLOCK,	//0x3a
	KEY_F1,			//0x3b
	KEY_F2,			//0x3c
	KEY_F3,			//0x3d
	KEY_F4,			//0x3e
	KEY_F5,			//0x3f
	KEY_F6,			//0x40
	KEY_F7,			//0x41
	KEY_F8,			//0x42
	KEY_F9,			//0x43
	KEY_F10,		//0x44
	KEY_KP_NUMLOCK,	//0x45
	KEY_SCROLLLOCK,	//0x46
	KEY_HOME,		//0x47
	KEY_KP_8,		//0x48	//keypad up arrow
	KEY_PAGEUP,		//0x49
	KEY_KP_2,		//0x50	//keypad down arrow
	KEY_KP_3,		//0x51	//keypad page down
	KEY_KP_0,		//0x52	//keypad insert key
	KEY_KP_DECIMAL,	//0x53	//keypad delete key
	KEY_UNKNOWN,	//0x54
	KEY_UNKNOWN,	//0x55
	KEY_UNKNOWN,	//0x56
	KEY_F11,		//0x57
	KEY_F12			//0x58
};

static terminal_t terminals[MAX_TERMINALS];

/* determine if the key is pressed */
static int r_shift_key = 0;
static int l_shift_key = 0;
static int r_ctrl_key = 0;
static int l_ctrl_key = 0;
static int r_alt_key = 0;
static int l_alt_key = 0;

/* determine if the key is on */
static int caps_lock = 0;

static int32_t current_terminal = -1;

static fops_t term_fops = {
	.read = terminal_read,
	.write = terminal_write,
	.open = terminal_open,
	.close = terminal_close
};

/* terminal_open
 * DESC: Function that always returns 0 when someone tries to open terminal
 * 			 because the terminal is always open
 * INPUT: None - none of the parameters are used
 * OUTPUT: None
 * RETURN: 0
 * SIDE EFFECTS: None
 */
int32_t terminal_open(const uint8_t* filename){
  return 0;
}

/* terminal_close
 * DESC: Function that always returns 0 when someone tries to close the terminal
 * 			 because the terminal is always open
 * INPUT: None - none of the parameters are used
 * OUTPUT: None
 * RETURN: 0
 * SIDE EFFECTS: None
 */
int32_t terminal_close(int32_t fd){
  return 0;
}

/* terminal_read
 * DESC: returns data from one line that has been terminated by pressing
 * Enter, or as much as fits in the buffer from one such line
 * INPUT: The current terminal buffer from the curent pcb, number of bytes to read
 * OUTPUT: Moves nbytes number of bytes from terminal buffer to the passed in buffer
 * RETURN: Number of bytes read from the terminal buffer
 * SIDE EFFECTS: Moves the unread bytes to the beginning of the buffer and
 * clears the rest of the buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
	int diff, chars_read;
	pcb_t * pcb;
	int32_t term_num;

	/* Check for null pointer */
	if(buf == NULL)
		return -1;

	pcb(pcb);
	term_num = pcb -> term_num;

	/* wait until user hit enter */
	terminals[term_num].reading = 1;
	terminals[term_num].hit_enter = 0;

	while(!terminals[term_num].hit_enter);

	cli();

	/* sanity check on nbytes */
	if(nbytes > LINE_BUF_MAX) nbytes = LINE_BUF_MAX;

	/* read nbytes from line buffer */
	diff = terminals[term_num].buf_count - nbytes;
	chars_read = nbytes > terminals[term_num].buf_count ?
			terminals[term_num].buf_count : nbytes;
	if(diff < 0) diff = 0;
	memcpy(buf, terminals[term_num].line_buf, nbytes);

	/* move unread bytes in line buffer to beginning */
	memmove(terminals[term_num].line_buf, terminals[term_num].line_buf + nbytes, diff);
	/* clear rest of line buffer */
	memset(terminals[term_num].line_buf + diff, NULL_CHAR, nbytes);
	terminals[term_num].buf_count = diff;

	sti();

	terminals[term_num].reading = 0;
	terminals[term_num].hit_enter = 0;
	return chars_read;
}

/* terminal_write
 * DESC: Prints nbytes number of characters onto to the terminal
 * INPUT: nbytes to determine the number of bytes, buffer that contains the characters
 * to print
 * OUTPUT: Prints nbytes number of characters to the terminal
 * RETURN: Number of bytes printed
 * SIDE EFFECTS: Could move the terminal up
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
	int i;
	pcb_t * pcb;
	int32_t term_num;

  /* Check for null pointer */
	if(buf == NULL)
		return -1;

	pcb(pcb);
	term_num = pcb -> term_num;

	for(i = 0; i < nbytes; i++){
		putc_in_terminal(((int8_t *) buf)[i], &(terminals[term_num].screen));
	}
	if(pcb -> term_num == current_terminal) {
		move_cursor(term_num, &(terminals[term_num].screen), PAGE_SIZE);
	}

	return nbytes;
}

/* kybd_init
 * DESC: Initializes the keyboard interrupt handler which takes inputs from
 * the keyboard and starts all of the terminals
 * INPUT: The keyboard handler
 * OUTPUT: Sets the IDT entry for keyboard
 * RETURN: None
 * SIDE EFFECTS: Initializes all of the terminals to position (0,0) and
 * all of the buffers are full of null characters
 */
void kybd_init(){
	int i;

	/* Populate IDT entry for keyboard */
	add_irq(KEYBOARD_IRQ_NUM, (uint32_t) keyboard_handler_main);

	/* Unmask keybord interrupt on PIC */
	enable_irq(KEYBOARD_IRQ_NUM);

	/* Set all of the values in the line buffer to the null character */
	for(i = 0; i < MAX_TERMINALS; i++) {
		memset(terminals[i].line_buf, NULL_CHAR, LINE_BUF_MAX);
		terminals[i].buf_count = 0;
		terminals[i].reading = 0;
		terminals[i].screen.x = 0;
		terminals[i].screen.y = 0;
		terminals[i].screen.video_mem = get_video_mem();
	}

	add_device(TERM_FTYPE, &term_fops);
}

/* update
 * DESC: Determines what to do with the terminal buffer and what is printed on the
 * terminal based on the key pressed
 * INPUT: The ascii value of the key that is pressed from the keyboard
 * OUTPUT: None
 * RETURN: None
 * SIDE EFFECTS: Changes terminal buffer and what is printed to the terminal
 */
void update(uint16_t key){
	terminal_t * curr_term = &(terminals[current_terminal]);

	if(key == KEY_RETURN){
		curr_term -> screen.video_mem += PAGE_SIZE * (current_terminal + 1);
		/* put '/r' as the last character in the buffer */
		putc_in_terminal(key, &(curr_term -> screen));
		curr_term -> screen.video_mem -= PAGE_SIZE * (current_terminal + 1);
		/* Moves the cursor to the next line start at x position 0 */
		move_cursor(current_terminal, &(curr_term -> screen), PAGE_SIZE);
		curr_term -> line_buf[curr_term -> buf_count++] = '\n';
		curr_term -> hit_enter = 1;
		curr_term -> input_len = 0;
		/* If the terminal is not being read from then just clear the terminal buffer */
		if(!curr_term -> reading) {
			memset(curr_term -> line_buf, NULL_CHAR, LINE_BUF_MAX);
			curr_term -> buf_count = 0;
		}
	}
	/* If backspace is pressed then delete the character from the screen and
		 delete the character from the terminal buffer */
	else if(key == KEY_BACKSPACE){
		if(curr_term -> input_len){
			curr_term -> screen.video_mem += PAGE_SIZE * (current_terminal + 1);
			backspace_fnc(&(curr_term -> screen));
			curr_term -> screen.video_mem -= PAGE_SIZE * (current_terminal + 1);
			move_cursor(current_terminal, &(curr_term -> screen), PAGE_SIZE);
			curr_term -> buf_count--;
			curr_term -> input_len--;
			curr_term -> line_buf[curr_term -> buf_count] = NULL_CHAR;
		}
	}
	/* If the terminal buffer is already full then do not print anything and
		 stop filling the buffer */
	else if(curr_term -> buf_count < LINE_BUF_MAX - 1 - 1){  /* make room for newline at end */
		curr_term -> screen.video_mem += PAGE_SIZE * (current_terminal + 1);
		putc_in_terminal(key, &(curr_term -> screen));
		curr_term -> screen.video_mem -= PAGE_SIZE * (current_terminal + 1);
		move_cursor(current_terminal, &(curr_term -> screen), PAGE_SIZE);
		curr_term -> line_buf[curr_term -> buf_count] = key;
		curr_term -> buf_count++;
		curr_term -> input_len++;
	}
}

/* keyboard_handler_main
 * DESC: Keyboard interrupt function that gets the key pressed from the keyboard
 * and determines what to do with it
 * INPUT: The keyboard data from the keyboard ports
 * OUTPUT: None
 * RETURN: None
 * SIDE EFFECTS: Changes what is printed to the terminal as well as switches
 * between terminals
 */
void keyboard_handler_main(){

	unsigned char scancode;
	unsigned char status;
	int i;

	status = inb(KEYBOARD_PORT);
	if(status & 1){
		/* Read from keyboard's data buffer */
		scancode = inb(KEYBOARD_PORT_DATA);
		uint16_t key_out;

		if((scancode & MASK_KEY_PRESS)){ //0x80 is a flag for key release check
			scancode -= MASK_KEY_PRESS; // Get the actual scancode value
			key_out = kybd_keys[scancode];
			if(key_out == KEY_RSHIFT) r_shift_key = 0;
			else if(key_out == KEY_LSHIFT) l_shift_key = 0;
			else if(key_out == KEY_LCTRL) r_ctrl_key = 0;
			else if(key_out == KEY_RCTRL) l_ctrl_key = 0;
			else if(key_out == KEY_LALT) l_alt_key = 0;
			else if(key_out == KEY_RALT) r_alt_key = 0;
		}
		/* If key not released, print to screen */
		else if(!(scancode & MASK_KEY_PRESS)){ //0x80 is a flag for key release check
			key_out = kybd_keys[scancode];
			int shift = l_shift_key | r_shift_key;
			int alt = l_alt_key | r_alt_key;
			int ctrl = l_ctrl_key | r_ctrl_key;
			if(key_out == KEY_CAPSLOCK) caps_lock = !caps_lock;
			else if(key_out == KEY_RSHIFT) r_shift_key = 1;
			else if(key_out == KEY_LSHIFT) l_shift_key = 1;
			else if(key_out == KEY_LCTRL) r_ctrl_key = 1;
			else if(key_out == KEY_RCTRL) l_ctrl_key = 1;
			else if(key_out == KEY_LALT) l_alt_key = 1;
			else if(key_out == KEY_RALT) r_alt_key = 1;
			else{
				if(!shift && ctrl && key_out == 'l') {
					terminals[current_terminal].screen.video_mem += PAGE_SIZE * (current_terminal + 1);
					clear_terminal(&(terminals[current_terminal].screen));
					for(i = 0; i < terminals[current_terminal].buf_count; i++){
						putc_in_terminal(((int8_t *) terminals[current_terminal].line_buf)[i],
							&(terminals[current_terminal].screen)
						);
					}
					terminals[current_terminal].screen.video_mem -= PAGE_SIZE * (current_terminal + 1);
					move_cursor(current_terminal, &(terminals[current_terminal].screen), PAGE_SIZE);
				}
				else if(!ctrl && !shift && alt && key_out == KEY_F1) {
					send_eoi(KEYBOARD_IRQ_NUM);
					start_terminal(0);
					return;
				}
				else if(!ctrl && !shift && alt && key_out == KEY_F2) {
					send_eoi(KEYBOARD_IRQ_NUM);
					start_terminal(1);
					return;
				}
				else if(!ctrl && !shift && alt && key_out == KEY_F3) {
					send_eoi(KEYBOARD_IRQ_NUM);
					start_terminal(2);
					return;
				}
				else if(scancode > MAX_SCANCODE){} //If not valid scancode do nothing
				else{
					if((caps_lock ^ shift) && (!alt && !ctrl)
							&& (key_out >= 'a' && key_out <= 'z'))
						key_out -= 'a' - 'A';  /* offset for capital chars */
					else if(shift && !ctrl && !alt) {
						switch(key_out) {
							case '0': key_out = KEY_RIGHTPARENTHESIS;
								break;
							case '1': key_out = KEY_EXCLAMATION;
								break;
							case '2': key_out = KEY_AT;
								break;
							case '3': key_out = KEY_HASH;
								break;
							case '4': key_out = KEY_DOLLAR;
								break;
							case '5': key_out = KEY_PERCENT;
								break;
							case '6': key_out = KEY_CARRET;
								break;
							case '7': key_out = KEY_AMPERSAND;
								break;
							case '8': key_out = KEY_ASTERISK;
								break;
							case '9': key_out = KEY_LEFTPARENTHESIS;
								break;
							case KEY_COMMA: key_out = KEY_LESS;
								break;
							case KEY_DOT: key_out = KEY_GREATER;
								break;
							case KEY_SLASH: key_out = KEY_QUESTION;
								break;
							case KEY_SEMICOLON: key_out = KEY_COLON;
								break;
							case KEY_QUOTE: key_out = KEY_QUOTEDOUBLE;
								break;
							case KEY_LEFTBRACKET: key_out = KEY_LEFTCURL;
								break;
							case KEY_RIGHTBRACKET: key_out = KEY_RIGHTCURL;
								break;
							case KEY_GRAVE: key_out = KEY_TILDE;
								break;
							case KEY_MINUS: key_out = KEY_UNDERSCORE;
								break;
							case KEY_EQUAL: key_out = KEY_PLUS;
								break;
							case KEY_BACKSLASH: key_out = KEY_BAR;
								break;
						}
					}
					/* if the key is not supposed to print to screen then ignore it */
					if((key_out >= ' ' && key_out <= '~') ||
							(key_out == KEY_BACKSPACE) ||
							(key_out == KEY_RETURN))
						update(key_out);
			  }
			}
		}
	}
	send_eoi(KEYBOARD_IRQ_NUM);
}

/* start_terminal
 * DESC: Sets the vga to the terminals video memory location and restores the
 * state the terminal was in before it was left
 * INPUT: The number of the terminal the user is switching to
 * OUTPUT: Restores the previous state of the terminal to the screen
 * RETURN: If not switiching to a valid terminal or switching to the same terminal
 * then return -1 otherwise return 0
 * SIDE EFFECTS: None
 */
int32_t start_terminal(uint32_t term_num){
	int32_t curr_active_process, next_active_process;

	if(term_num < 0 || term_num >= MAX_TERMINALS) return -1;
	if(term_num == current_terminal) return -1;

	curr_active_process = get_active_process(current_terminal);
	next_active_process = get_active_process(term_num);

	if(!free_procs() && next_active_process == -1) return -1;

	set_vga_start(term_num * PAGE_SIZE);

	move_cursor(term_num, &(terminals[term_num].screen), PAGE_SIZE);

	/* switch processes */
	current_terminal = term_num;
	if(next_active_process == -1){
		/* terminal not initialized */
		/* start shell in terminal */
		uint8_t shell[] = "shell";

		if(curr_active_process != -1) {
			schedule_for_execution(shell);
		} else {
			while(1)
				execute(shell);
		}
	}

	return 0;
}

/* set_curr_active_process
 * DESC: Set the active process of the current terminal
 * INPUT: The identification number of the active process
 * OUTPUT: None
 * RETURN: None
 * SIDE EFFECTS: None
 */
void set_curr_active_process(int32_t pid) {
	set_active_process(current_terminal, pid);
}

/* get_active_process
 * DESC: Get the active process of the current terminal
 * INPUT: None
 * OUTPUT: None
 * RETURN: The identification number of the active process
 * SIDE EFFECTS: None
 */
int32_t get_curr_active_process() {
	return get_active_process(current_terminal);
}

/* curr_terminal_running_process
 * DESC: Determines if the terminal you are on is running a process
 * INPUT: The current terminal the user is on
 * OUTPUT: None
 * RETURN: identification of the active process if there is one
 * SIDE EFFECTS: None
 */
int32_t curr_terminal_running_process(){
	return get_active_process(current_terminal) != -1;
}

/* get_current_terminal
 * DESC: Return the terminal number the user is on
 * INPUT: None
 * OUTPUT: None
 * RETURN: current terminal number
 * SIDE EFFECTS: None
 */
uint32_t get_current_terminal() {
	return current_terminal;
}

/* get_terminal
 * DESC: Get access to a terminals structure
 * INPUT: The terminal number you are accessing
 * OUTPUT: None
 * RETURN: The address terminal structure
 * SIDE EFFECTS: None
 */
terminal_t * get_terminal(int32_t term_num) {
	return &(terminals[term_num]);
}
