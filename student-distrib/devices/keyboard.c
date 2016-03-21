/* keyboard.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "keyboard.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"

extern void kybd_isr();

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

/* determine if the key is pressed */
int r_shift_key = 0;
int l_shift_key = 0;
int r_ctrl_key = 0;
int l_ctrl_key = 0;
/* determine if the key is on */
int caps_lock = 0;

void kybd_init(){
  // initialize by sending IRQ number (33?) to keybrd interrupt handler
  // I'm going to need the vector table of the idt
  //setvector(irq_num, kybrd_intr);

	/* Populate IDT entry for keyboard */
	set_int_gate(0x21, (unsigned long) kybd_isr); //0x21 is the interrupt number in the IDT

	/* Unmask keybord interrupt on PIC */
	enable_irq(KEYBOARD_IRQ_NUM);
}

void keyboard_handler_main(){

	unsigned char scancode;
	unsigned char status;

	status = inb(KEYBOARD_PORT);
	if(status & 1){
		/* Read from keyboard's data buffer */
		scancode = inb(KEYBOARD_PORT_DATA);
		uint16_t key_out;

		if((scancode & 0x80)){ //0x80 is a flag for key release check
			scancode -= 0x80; // Get the actual scancode value
			key_out = kybd_keys[scancode];
			if(key_out == KEY_RSHIFT) r_shift_key = 0;
			else if(key_out == KEY_LSHIFT) l_shift_key = 0;
			else if(key_out == KEY_LCTRL) r_ctrl_key = 0;
			else if(key_out == KEY_RCTRL) l_ctrl_key = 0;
		}
		/* If key not released, print to screen */
		else if(!(scancode & 0x80)){ //0x80 is a flag for key release check
			key_out = kybd_keys[scancode];
			if(key_out == KEY_CAPSLOCK) caps_lock = !caps_lock;
			else if(key_out == KEY_RSHIFT) r_shift_key = 1;
			else if(key_out == KEY_LSHIFT) l_shift_key = 1;
			else if(key_out == KEY_LCTRL) r_ctrl_key = 1;
			else if(key_out == KEY_RCTRL) l_ctrl_key = 1;
			else{
				if((caps_lock == 1 || r_shift_key == 1 || l_shift_key == 1) &&
				     (r_ctrl_key == 0 && l_ctrl_key == 0) &&
				   (key_out >= 'a' && key_out <= 'z'))
					putc(key_out - 32);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '0')
				putc(KEY_RIGHTPARENTHESIS);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '1')
				putc(KEY_EXCLAMATION);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '2')
				putc(KEY_AT);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '3')
				putc(KEY_HASH);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '4')
				putc(KEY_DOLLAR);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '5')
				putc(KEY_PERCENT);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '6')
				putc(KEY_CARRET);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '7')
				putc(KEY_AMPERSAND);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '8')
				putc(KEY_ASTERISK);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == '9')
				putc(KEY_LEFTPARENTHESIS);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_COMMA)
				putc(KEY_LESS);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_DOT)
				putc(KEY_GREATER);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_SLASH)
				putc(KEY_QUESTION);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_SEMICOLON)
				putc(KEY_COLON);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_QUOTE)
				putc(KEY_QUOTEDOUBLE);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_LEFTBRACKET)
				putc(KEY_LEFTCURL);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_RIGHTBRACKET)
				putc(KEY_RIGHTCURL);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_GRAVE)
				putc(KEY_TILDE);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_MINUS)
				putc(KEY_UNDERSCORE);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_EQUAL)
				putc(KEY_PLUS);
				else if((r_shift_key == 1 || l_shift_key == 1) && key_out == KEY_BACKSLASH)
				putc(KEY_BAR);
				else if(r_ctrl_key == 0 && l_ctrl_key == 0) putc(key_out);
			}
		}
	}
	send_eoi(KEYBOARD_IRQ_NUM);
}
