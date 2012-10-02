/*

Port Congiguration:
-Encoder A: GPIO1_6 - Header P8, pin 3 - GPIO 38
-Encoder B: GPIO1_15 - Header P8, pin 15 - GPIO 47
-Pushbutton Switch: GPIO1_16 - Header P9, pin 15 - GPIO 48

This program keeps track of an encoder. CW rotation will
increment the ticks, and CCW will decrement it. The current
number of ticks is printed when the button is pressed.

The gpio.h header consists of the GPIO methods copied directly from:
https://www.ridgerun.com/developer/wiki/index.php/Gpio-int-test.c

*/


#include <signal.h>
#include <poll.h>
#include "gpio.h"

int running = 1;

void signal_handler(int sig)
{
	running = 0;
}

int main(int argc, char* argv[])
{
	int rc;
	struct pollfd fdset[3];
	int nfds = 3;
	int len;
	char* buf[MAX_BUF];
	int i;

	int pos = 0;

	unsigned int enc_a_val;
	unsigned int enc_b_val;

	// Handle Ctrl^C
	signal(SIGINT, signal_handler);

	// Set up GPIO pins
	unsigned int gpio_a = 38;
	unsigned int gpio_b = 47;
	unsigned int gpio_sw = 48;

	int enc_a_fd;
	int enc_b_fd;
	int enc_sw_fd;

	gpio_export(gpio_a);
	gpio_export(gpio_b);
	gpio_export(gpio_sw);

	gpio_set_dir(gpio_a, 0);
	gpio_set_dir(gpio_b, 0);
	gpio_set_dir(gpio_sw, 0);

	// Interrupts
	gpio_set_edge(gpio_a, "rising");
	gpio_set_edge(gpio_sw, "rising");

	// Open the file for the encoder A signal
	enc_a_fd = gpio_fd_open(gpio_a);

	// Main loop
	while (running == 1)
	{
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;

		fdset[1].fd = enc_a_fd;
		fdset[1].events = POLLPRI;

		fdset[2].fd = enc_sw_fd;
		fdset[2].events = POLLPRI;

		rc = poll(fdset, nfds, -1);

		if (rc < 0)
		{
//			printf("poll() failed.\n");
		}

		if (rc == 0)
		{
//			printf(".");
		}

		if (fdset[0].revents & POLLIN)
		{
			(void) read(fdset[0].fd, buf, 1);
		}

		// Encoder click

		if (fdset[1].revents & POLLPRI)
		{
			lseek(fdset[1].fd, 0, SEEK_SET);
			len = read(fdset[1].fd, buf, MAX_BUF);

			enc_a_val = atoi((const char*) buf);
			gpio_get_value(gpio_b, &enc_b_val);

			if (enc_a_val == 1) // rising edge
			{
				if (enc_b_val == 0) pos--;
				else pos++;
			}

		}

		if (fdset[2].revents & POLLPRI)
		{
			lseek(fdset[2].fd, 0, SEEK_SET);
			len = read(fdset[2].fd, buf, MAX_BUF);

			printf("Ticks: %d\n", pos);
		}

	}

	gpio_fd_close(enc_a_fd);

	return 0;
}
