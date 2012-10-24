/*

Port Congiguration:
-Encoder A: GPIO1_6 - Header P8, pin 3 - GPIO 38
-Encoder B: GPIO1_15 - Header P8, pin 15 - GPIO 47
-Encoder GND: Header P8, pin 1

-Pushbutton Switch: GPIO1_16 - Header P9, pin 15 - GPIO 48
-Pusshbuton Switch V+: Header P9, pin 3

This program keeps track of an encoder. CW rotation will
increment the ticks, and CCW will decrement it. The current
number of ticks is printed when the button is pressed.

The files gpio.h and gpio.c  consist of the GPIO methods copied
directly from:
https://www.ridgerun.com/developer/wiki/index.php/Gpio-int-test.c

*/

#include "gpio.h"
#include <signal.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define OMAP_DIR "/sys/kernel/debug/omap_mux"

int running = 1;

// Quit when ^C is pressed
void signal_handler(int sig)
{
	printf("\n");
	running = 0;
}

int main(int argc, char* argv[])
{
	// Polling variables
	int rc;
	struct pollfd fdset[3];
	int nfds = 3;
	int len;
	char* buf[MAX_BUF];

	// Keeps track of encoder ticks.
	int ticks = 0;

	// Variables used to store GPIO values
	unsigned int enc_a_val;
	unsigned int enc_b_val;
	unsigned int sw_val;

	// Handle Ctrl^C
	signal(SIGINT, signal_handler);

        // Configure the pins to use internal pull-down resistors
        FILE *mux_ptr;
        char pullup_str[10];
	char pulldown_str[10];
        strcpy(pullup_str, "0x0037");
	strcpy(pulldown_str, "0x0027");

        // Configure pull-up for the A signal
        if ((mux_ptr = fopen(OMAP_DIR "/gpmc_ad6", "rb+")) == NULL)
        {
                printf("Failed to open gpmc_ad6. Quitting.\n");
                exit(1);
        }
        else
	{
                fwrite(&pullup_str, sizeof(char), 6, mux_ptr);
                fclose(mux_ptr);
        }

        // Configure pull-up for the B signal
        if ((mux_ptr = fopen(OMAP_DIR "/gpmc_ad15", "rb+")) == NULL)
        {
                printf("Failed to open gpmc_ad15. Quitting.\n");
                exit(1);
        }
        else
	{
                fwrite(&pullup_str, sizeof(char), 6, mux_ptr);
                fclose(mux_ptr);
        }

        // Configure pull-down for the switch
        if ((mux_ptr = fopen(OMAP_DIR "/gpmc_a0", "rb+")) == NULL)
        {
                printf("Failed to open gpmc_a0. Quitting.\n");
                exit(1);
        }
        else
	{
                fwrite(&pulldown_str, sizeof(char), 6, mux_ptr);
                fclose(mux_ptr);
        }

	// Set up GPIO pins
	unsigned int gpio_a = 38;
	unsigned int gpio_b = 47;
	unsigned int gpio_sw = 48;

	int enc_a_fd;
	int enc_b_fd;
	int enc_sw_fd;

	printf("Exporting a... %d\n", gpio_export(gpio_a));
	printf("Exporting b... %d\n", gpio_export(gpio_b));
	printf("Exporting sw... %d\n", gpio_export(gpio_sw));

	printf("Setting a direction... %d\n", gpio_set_dir(gpio_a, 0));
	printf("Setting b direction... %d\n", gpio_set_dir(gpio_b, 0));
	printf("Setting sw direction... %d\n", gpio_set_dir(gpio_sw, 0));

	// Interrupts
	printf("setting edge a... %d\n", gpio_set_edge(gpio_a, "rising"));
	printf("setting edge sw... %d\n", gpio_set_edge(gpio_sw, "rising"));

	// Open the file for the encoder A signal
	enc_a_fd = gpio_fd_open(gpio_a);
	enc_sw_fd = gpio_fd_open(gpio_sw);

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
				if (enc_b_val == 0) ticks--;
				else ticks++;
			}

		}

		// Button press - print the current number of encoder ticks
		if (fdset[2].revents & POLLPRI)
		{
			lseek(fdset[2].fd, 0, SEEK_SET);
			len = read(fdset[2].fd, buf, MAX_BUF);

			// Very simple debouncing
			usleep(5000);
			gpio_get_value(gpio_sw, &sw_val);
			if (sw_val == 1)
			{
				printf("Ticks: %d\n", ticks);
			}
		}

	}

	gpio_fd_close(enc_a_fd);

	return 0;
}
