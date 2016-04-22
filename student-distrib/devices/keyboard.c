/* keyboard.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "keyboard.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"
#include "../fs.h"
#include "../process.h"
#include "../sys_calls.h"

extern void kybd_isr();

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

/* Line buffer as well as the current size of it */
static int8_t line_buf[LINE_BUF_MAX];
static uint16_t buf_count = 0;

static int reading = 0;
static int input_len = 0;

/* determine if the key is pressed */
static int r_shift_key = 0;
static int l_shift_key = 0;
static int r_ctrl_key = 0;
static int l_ctrl_key = 0;
static int r_alt_key = 0;
static int l_alt_key = 0;
static int hit_enter = 0;
/* determine if the key is on */
static int caps_lock = 0;

fops_t term_fops = {
	.read = terminal_read,
	.write = terminal_write,
	.open = terminal_open,
	.close = terminal_close
};

int32_t terminal_open(const uint8_t* filename){
  return 0;
}

int32_t terminal_close(int32_t fd){
  return 0;
}

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
	int diff, chars_read;

	/* Check for null pointer */
	if(buf == NULL)
		return -1;

	/* wait until user hit enter */
	reading = 1;
	hit_enter = 0;
	while(!hit_enter);

	cli();

	/* sanity check on nbytes */
	if(nbytes > LINE_BUF_MAX) nbytes = LINE_BUF_MAX;

	/* read nbytes from line buffer */
	diff = buf_count - nbytes;
	chars_read = nbytes > buf_count ? buf_count : nbytes;
	if(diff < 0) diff = 0;
	memcpy(buf, line_buf, nbytes);

	/* move unread bytes in line buffer to beginning */
	memmove(line_buf, line_buf + nbytes, diff);
	/* clear rest of line buffer */
	memset(line_buf + diff, NULL_CHAR, nbytes);
	buf_count = diff;

	sti();

	reading = 0;
	hit_enter = 0;
	return chars_read;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
	int i;

  /* Check for null pointer */
	if(buf == NULL)
		return -1;

	for(i = 0; i < nbytes; i++){
		putc(((int8_t *) buf)[i]);
	}
	return nbytes;
}

void kybd_init(){
	/* Populate IDT entry for keyboard */
	set_int_gate(0x21, (unsigned long) kybd_isr); //0x21 is the interrupt number in the IDT

	/* Unmask keybord interrupt on PIC */
	enable_irq(KEYBOARD_IRQ_NUM);

	/* Set all of the values in the line buffer to the null character */
	memset(line_buf, NULL_CHAR, LINE_BUF_MAX);

	add_device(TERM_FTYPE, &term_fops);
}

void update(uint16_t key){
	if(key == KEY_RETURN){
		putc(key);
		hit_enter = 1;
		input_len = 0;
		if(!reading) {
			memset(line_buf, NULL_CHAR, LINE_BUF_MAX);
			buf_count = 0;
		}
	}
	else if(key == KEY_BACKSPACE){
		if(input_len){
			backspace_fnc();
			buf_count--;
			input_len--;
			line_buf[buf_count] = NULL_CHAR;
		}
	}
	else if(buf_count < LINE_BUF_MAX - 1 - 1){  /* make room for newline at end */
		putc(key);
		line_buf[buf_count] = key;
		buf_count++;
		input_len++;
	}
}

void keyboard_handler_main(){

	unsigned char scancode;
	unsigned char status;

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
				if(scancode > MAX_SCANCODE){}//If not valid scancode do nothing
				else if(!shift && ctrl && key_out == 'l') {
					clear();
					puts(line_buf);
				}
				else if(!shift && ctrl && key_out == 'c') {
					if(are_processes()){
						send_eoi(KEYBOARD_IRQ_NUM);
						halt(1);
					}
				}
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
					update(key_out);
			  }
			}
		}
	}
	send_eoi(KEYBOARD_IRQ_NUM);
}
