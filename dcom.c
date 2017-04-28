#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	// UNIX standard function definitions
#include <errno.h>	// Error number definitions
#include <termios.h>	// POSIX terminal control definitions
#include <fcntl.h>	// File control definitions
#include <linux/input.h>
#include <pthread.h>
#include "queue.h"
#include "map.h"

#define EV_PRESSED 1
#define EV_RELEASED 0
#define EV_REPEAT 2
#define BUFFER_SIZE 100

#define DEVICE_TTYS0 "/dev/ttyS0"

#define ENTER_KEY "\r"
#define STOP_KEY "p"

int readFd;
int open_port(char *);
void WriteToUart(char *, int);

void send_keyboard_code(char ch)
{
    const char *dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    struct input_event ev;
    ssize_t n;
    int fd;

    fd = open(dev, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        //return EXIT_FAILURE;
    }
    ev.code = 0x19;
    write(fd, &ev, sizeof ev);
    // while (1) {
    //     n = read(fd, &ev, sizeof ev);
    //     if (n == (ssize_t)-1) {
    //         if (errno == EINTR)
    //             continue;
    //         else
    //             break;
    //     } else
    //     if (n != sizeof ev) {
    //         errno = EIO;
    //         break;
    //     }
    //     if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
    //         printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);

    // }
    //fflush(stdout);
    //fprintf(stderr, "%s.\n", strerror(errno));
    //return EXIT_FAILURE;
}

void* listen_keyboard_code()
{
    //struct input_event event[64];
    struct input_event event;
    int fd, rd, value, size = sizeof (struct input_event);
    // char name[256] = "Unknown";
    char          name[64];           /* RATS: Use ok, but could be better */
    // const char *device = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    const char *device = "/dev/input/event3";
    char          *tmp;
    //Setup check
    if ((getuid ()) != 0)
	printf("You are not root! This may not work...\n");

#if DISPLAY_EVENT_INFO
    char          buf[256] = { 0, };  /* RATS: Use ok */
    unsigned char mask[EV_MAX/8 + 1]; /* RATS: Use ok */
    int           version;
    int           rc;
    int           i, j;

#define test_bit(bit) (mask[(bit)/8] & (1 << ((bit)%8)))

    for (i = 0; i < 32; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) {
            ioctl(fd, EVIOCGVERSION, &version);
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            ioctl(fd, EVIOCGBIT(0, sizeof(mask)), mask);
            printf("%s\n", name);
            printf("    evdev version: %d.%d.%d\n",
                   version >> 16, (version >> 8) & 0xff, version & 0xff);
            printf("    name: %s\n", buf);
            printf("    features:");
            for (j = 0; j < EV_MAX; j++) {
                if (test_bit(j)) {
                    char *type = "unknown";
                    switch(j) {
			case EV_KEY: type = "keys/buttons"; break;
			case EV_REL: type = "relative";     break;
			case EV_ABS: type = "absolute";     break;
			case EV_MSC: type = "reserved";     break;
			case EV_LED: type = "leds";         break;
			case EV_SND: type = "sound";        break;
			case EV_REP: type = "repeat";       break;
			case EV_FF:  type = "feedback";     break;
                    }
                    printf(" %s", type);
                }
            }
            printf("\n");
            close(fd);
        }
    }
#endif


    //Open Device
    if ((fd = open (device, O_RDONLY)) == -1)
	printf("%s is not a vaild device.\n", device);

    //Print Device Name
    ioctl (fd, EVIOCGNAME (sizeof (name)), name);
    // printf ("Reading From : %s (%s)\n", device, name);

    while (1){
	if ((rd = read (fd, &event, sizeof(event))) < size)
	{
	    perror("read()");
	    exit(0);
	}
	switch (event.type)
	{
	    case EV_KEY:
		if (event.code > BTN_MISC) {
		    printf("Button %d %s",
			   event.code & 0xff,
			   event.value ? "press" : "release");
		} else {
		    //printf("===%c", input_event_map[event.code][FIRST_KEY]);
		    /*if ( event.code == KEY_SPACE || (event.code > KEY_ESC && event.code < KEY_RIGHTSHIFT ))
		    {
			//printf("%c", event.value);
		    }*/
		    printf("Key %d (0x%x) %s",
			   event.code & 0xff,
			   event.code & 0xff,
			   event.value ? "press" : "release");
		}
		break;
	    case EV_REL:
		switch (event.code) {
		    case REL_X:      tmp = "X";       break;
		    case REL_Y:      tmp = "Y";       break;
		    case REL_HWHEEL: tmp = "HWHEEL";  break;
		    case REL_DIAL:   tmp = "DIAL";    break;
		    case REL_WHEEL:  tmp = "WHEEL";   break;
		    case REL_MISC:   tmp = "MISC";    break;
		    default:         tmp = "UNKNOWN"; break;
		}
		printf("Relative %s %d", tmp, event.value);
		break;
	    case EV_ABS:
		switch (event.code) {
		    case ABS_X:        tmp = "X";        break;
		    case ABS_Y:        tmp = "Y";        break;
		    case ABS_Z:        tmp = "Z";        break;
		    case ABS_RX:       tmp = "RX";       break;
		    case ABS_RY:       tmp = "RY";       break;
		    case ABS_RZ:       tmp = "RZ";       break;
		    case ABS_THROTTLE: tmp = "THROTTLE"; break;
		    case ABS_RUDDER:   tmp = "RUDDER";   break;
		    case ABS_WHEEL:    tmp = "WHEEL";    break;
		    case ABS_GAS:      tmp = "GAS";      break;
		    case ABS_BRAKE:    tmp = "BRAKE";    break;
		    case ABS_HAT0X:    tmp = "HAT0X";    break;
		    case ABS_HAT0Y:    tmp = "HAT0Y";    break;
		    case ABS_HAT1X:    tmp = "HAT1X";    break;
		    case ABS_HAT1Y:    tmp = "HAT1Y";    break;
		    case ABS_HAT2X:    tmp = "HAT2X";    break;
		    case ABS_HAT2Y:    tmp = "HAT2Y";    break;
		    case ABS_HAT3X:    tmp = "HAT3X";    break;
		    case ABS_HAT3Y:    tmp = "HAT3Y";    break;
		    case ABS_PRESSURE: tmp = "PRESSURE"; break;
		    case ABS_DISTANCE: tmp = "DISTANCE"; break;
		    case ABS_TILT_X:   tmp = "TILT_X";   break;
		    case ABS_TILT_Y:   tmp = "TILT_Y";   break;
		    case ABS_MISC:     tmp = "MISC";     break;
		    default:           tmp = "UNKNOWN";  break;
		}
		printf("Absolute %s %d", tmp, event.value);
		break;
	    case EV_MSC:
		// printf("Misc");
		break;
	    case EV_LED: printf("Led");  break;
	    case EV_SND: printf("Snd");  break;
	    case EV_REP: printf("Rep");  break;
	    case EV_FF:  printf("FF");   break;
			 break;
	}

	/*value = event[0].value;
	// Only read the key press event
	if (value != ' ' && event[1].value == 1 && event[1].type == 1)
	{
	    // printf ("Code[%d]\n", (event[1].code));
	    switch(event[1].code)
	    {
		case 28:
		    //WriteToUart(ENTER_KEY, 1);
		    break;
		defalut:
		    break;
	    }
	}*/
    }
}



void kbhit(char ch)
{
    struct termios oldt, newt;
    //int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    //ch = getchar();

    // recover old option
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    ungetc(ch, stdin);
    //if(ch != EOF)
    //{
    //  ungetc(ch, stdin);
    //  return 1;
    //}

    //return 0;
}

/*
 * attribute
 *	c_cflag:    Control options
 *	c_lflag:    Line options
 *	c_iflag:    Input options
 *	c_oflag:    Output options
 *	c_cc:	    Control options
 *	    CBAUD:	Bit mask for baud rate
 *	    B0:		0 baud (drop DTR)
 *	    ...
 *	    B50:	50 baud
 *	    ...
 *	    B134:	134.5 baud
 *	    ...
 *	    B19200:	19200 baud
 *	    ...
 *	    B115200:	115200 baud
 *	    EXTA:	External rate clock
 *	    EXTB:	External rate clock
 *	    CSIZE:	Bit mask for data bits
 *	    ...
 *	    CLOCAL:	Local line - do not change "owner" of port
 *	c_ispeed:   Input baud (new interface)
 *	c_ospeed:   Output baud (new interface)
 */
int open_port(char *port_name)
{
    int fd;

    fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
	printf("open_port: unable to open %s\n", port_name);
	return -1;
    }
    else {
	fcntl(fd, F_SETFL, 0);
    }
    return fd;
}

void config_port(int fd)
{
    struct termios options;
    bzero(&options, sizeof(options));
    // Set the baud rate
    cfsetispeed(&options, B115200); // Control Flag Set Input Speed
    cfsetospeed(&options, B115200); // Control Flag Set Output Speed

    // set c_iflag
    /*
     * IGNPAR  : ignore bytes with parity errors
     * ICRNL   : map CR to NL (otherwise a CR input on the other computer
     *		will not terminate input)
     *		otherwise make device raw (no other input processing)
     */
    // IGNore PARity errors, Input Carriage Return to New Line
    // Ignore framing errors and parity errors | Ignore carriage return on input.
    options.c_iflag |= IGNPAR | IGNCR;
    // options.c_iflag = 0;

    // set c_oflag
    // Disable implementation-defined output processing, Raw Output
    options.c_oflag |= ~OPOST;

    // set c_cflag
    options.c_cflag |= (CLOCAL | CREAD);    // Enable the receiver and set local mode
    // Character size mask.  Values are CS5, CS6, CS7, or CS8.
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;	// Select 8 data bits
    // set parity checking
    // No parity (8N1)
    options.c_cflag &= ~PARENB;	// Disable parity checking (clear parity checking bit)
    options.c_cflag &= ~CSTOPB;	// one stop bit
    // options.c_cflag |= CSTOPB;	// two stop bits

    // Even parity (7E1)
    /*options.c_cflag |= PARENB;	// Enable parity checking
    options.c_cflag &= ~PARODD;	// clear Odd parity checking bit
    options.c_cflag |= CS7;*/

    // Odd parity (7O1)
    /*options.c_cflag |= PARENB;
    options.c_cflag |= PARODD;
    options.c_cflag |= CS7;*/

    // Space parity (7S1)
    /*options.c_cflag &= ~PARENB;	// Disable parity checking (clear parity checking bit)
    options.c_cflag |= CS8;*/

    // Clear hardware flow control
    options.c_cflag &= ~CRTSCTS;	// Request To Send, Clear To Send

    // set c_lflag

    /*
     * ICANON  : enable canonical input
     *		disable all echo functionality, and don't send signals to calling program
     */
    options.c_lflag |= ICANON;

    // set c_cc array
    /*
     * initialize all control characters
     * default values can be found in /usr/include/termios.h, and are given
     * in the comments, but we don't need them here
     */
     options.c_cc[VINTR]    = 0;     /* Ctrl-c */
     options.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
     options.c_cc[VERASE]   = 0;     /* del */
     options.c_cc[VKILL]    = 0;     /* @ */
     options.c_cc[VEOF]     = 4;     /* Ctrl-d */
     options.c_cc[VTIME]    = 0;     /* inter-character timer unused */
     options.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
     options.c_cc[VSWTC]    = 0;     /* '\0' */
     options.c_cc[VSTART]   = 0;     /* Ctrl-q */
     options.c_cc[VSTOP]    = 0;     /* Ctrl-s */
     options.c_cc[VSUSP]    = 0;     /* Ctrl-z */
     options.c_cc[VEOL]     = 0;     /* '\0' */
     options.c_cc[VREPRINT] = 0;     /* Ctrl-r */
     options.c_cc[VDISCARD] = 0;     /* Ctrl-u */
     options.c_cc[VWERASE]  = 0;     /* Ctrl-w */
     options.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
     options.c_cc[VEOL2]    = 0;     /* '\0' */

    /*
     * now clean the modem line and activate the settings for the port
    */
    tcflush(fd, TCIFLUSH);
    /*
     * Constant:
     *	TCSANOW:    Make chagnes now without waiting for data to complete
     *	TCSADRAIN:  Wait until everything has been transmitted
     *	TCSAFLUSH:  Flush input and output buffers and make the change
     */
    // TCSANOW: specifies that all changes should occur immediately without waiting for output data to finish sending or input data to finish receiving.
    tcsetattr(fd, TCSANOW, &options);	// Terminal Contorl Set Attribute
}

int OpenPort(const char* port)
{
     printf("Opening serial %s\r\n", port);
     int fd=open(port, O_RDWR | O_NOCTTY | O_NDELAY);
     if(fd==-1)
     {
          printf("Error opening serial port %s... exiting", port);
          exit(-1);
     }

     fcntl(fd, F_SETFL, 0); /* Reads will be blocking */
     struct termios options;
     tcgetattr(fd, &options);
     (void)cfsetispeed(&options, B115200); /* (void) is to stop warning in cygwin */
     (void)cfsetospeed(&options, B115200);
     options.c_cflag &= ~CSIZE;
     options.c_cflag |= CS8;  /* 8 bits */
     options.c_cflag &= ~CSTOPB; /* 1 stop bit */
     options.c_cflag &= ~PARENB; /* no parity */
     options.c_cflag &= ~PARODD;
     options.c_cflag &= ~CRTSCTS; /* HW flow control off */
     options.c_lflag =0; /* RAW input */
     options.c_iflag = 0;            /* SW flow control off, no parity checks etc */
     options.c_oflag &= ~OPOST; /* RAW output */
     options.c_cc[VTIME]=10; /* 1 sec */
     options.c_cc[VMIN]=BUFFER_SIZE;
     options.c_cflag |= (CLOCAL | CREAD);
     tcsetattr(fd, TCSAFLUSH, &options);

     return fd;
}

void WriteToUart(char *str, int len)
{
    write(readFd, str, len);
}

void* ReaderThread(void* object)
{
    ssize_t numRead;
    char buf[256];
    char *pch;
    char cmd=0x70;
    char *key_str = "Enter '1', '2', or 'p' within 2 seconds or take default";
    int stop = 0;
    while(1) {
	// kbhit('p');
	if ( (numRead = read(readFd, buf, 255)) > 0 ) {
	    /*pch = strchr(buf, '\n');
	    if (pch) {
		*pch = ' ';
	    }
	    pch = strrchr(buf, '\n');
	    if (pch) {
		*pch = '\0';
	    }*/
	    buf[numRead]='\0';
	    printf("%s", buf);
	    if (strstr(buf, key_str)) {
		WriteToUart(STOP_KEY, 1);
		WriteToUart(ENTER_KEY, 1);
		WriteToUart(ENTER_KEY, 1);
		WriteToUart(ENTER_KEY, 1);
		WriteToUart(ENTER_KEY, 1);
		//kbhit();
		//send_keyboard_code('p');
	        //printf("meet %s\n\n", key_str);
	        //write(readFd, &cmd, 1);
	    }
	}
    }
}

void* keyboard_code()
{
     int key_fd = 0;
     char input[1] = {0};
     int len;

    /* This is the keyboard device as identified using both:
    $cat /proc/bus/input/devices */
    char *device = "/dev/input/event0";

    if ( (key_fd = open(device, O_RDONLY)) > 0 )
    {
	while(1)
	{
	    if ( (len = read(key_fd, input, 1)) > 0 ) {
		printf("========%x\n", input[0]);
		//close(key_fd);
		//return input[0];
	    }
	    /*struct input_event event;

	    // Press a key (stuff the keyboard with a keypress)
	    event.type = EV_KEY;
	    event.value = EV_PRESSED;
	    event.code = KEY_UP ;
	    write(key_fd, &event, sizeof(struct input_event));

	    // Release the key
	    event.value = EV_RELEASED;
	    event.code = KEY_UP;
	    write(key_fd, &event, sizeof(struct input_event));

	    event.type = EV_SYN;
	    event.code = SYN_REPORT ;
	    write(key_fd, &event, sizeof(struct input_event));
	    close(key_fd);*/
	}

    }
    close(key_fd);
    //return input[0];
}

int main()
{
    char write_buffer[256];
    int  bytes_written  =  0 ;
    char ch;
    char *message = "/do/sc\n";
    struct termios oldtio;
    pthread_t tid;

    readFd = open_port(DEVICE_TTYS0);
    //readFd = OpenPort(DEVICE_TTYS0);

    if (readFd == -1) {
	exit(-1);
    }

    fcntl(readFd, F_SETFL, FNDELAY);    // make read to be returned immediately, return 0 if no characters are availabel
    // Get current options for the port
    tcgetattr(readFd, &oldtio);	// Terminal Control Get Attribute

    config_port(readFd);

    pthread_create(&tid, (pthread_attr_t*)(0), &ReaderThread, (void*)(0));
    pthread_create(&tid, (pthread_attr_t*)(0), &listen_keyboard_code, (void*)(0));
    pthread_join(tid, (void*)(0));
    /* restore the old port settings */
    tcsetattr(readFd,TCSANOW,&oldtio);
    close(readFd);
}
