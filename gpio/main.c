//#include "gpio_handler.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "libsoc_gpio.h"
#include "libsoc_debug.h" 

unsigned int LEDGPIO = 45;  
unsigned int BUTTON_CONNECT = 31;
unsigned int BUTTON_ARM = 115;



int callback_con(void* arg)
{

	int* btn = (int*) arg;
	
	if(BUTTON_CONNECT==*btn)	printf("\nCONNECT PRESSED...%d\n",*btn);
	else if(BUTTON_ARM==*btn)	printf("\nARM PRESSED...%d\n",*btn);
	return EXIT_SUCCESS;
}

int callback_arm(void* arg)
{

	int* btn = (int*) arg;
	
	
	if(BUTTON_ARM)	printf("\nARM PRESSED...%d\n",*btn);
	return EXIT_SUCCESS;
}


int main(int argc, char *argv[]){

	int btn_id_arm;
	int btn_id_con;
	// Create both gpio pointers
	gpio *gpio_output, *gpio_btn_con, *gpio_btn_arm;

	// Enable debug output
	libsoc_set_debug(1);

	// Request gpios
	gpio_output = libsoc_gpio_request(LEDGPIO, LS_GPIO_GREEDY);
	gpio_btn_con = libsoc_gpio_request(BUTTON_CONNECT, LS_GPIO_GREEDY);
	gpio_btn_arm = libsoc_gpio_request(BUTTON_ARM, LS_GPIO_GREEDY);

		// Ensure both gpio were successfully requested
	if (gpio_output == NULL || gpio_btn_con == NULL || gpio_btn_arm == NULL)
	{
		printf("gpio were Unsuccessfully requested\n");
	}
	
	// Set direction to OUTPUT
	libsoc_gpio_set_direction(gpio_output, OUTPUT);
	
	// Check the direction
	if (libsoc_gpio_get_direction(gpio_output) != OUTPUT)
	{
		printf("Failed to set direction to OUTPUT\n");
	}

	// Set direction to INPUT
	libsoc_gpio_set_direction(gpio_btn_con, INPUT);
	if (libsoc_gpio_get_direction(gpio_btn_con) != INPUT)printf("Failed to set direction to INPUT\n");
	

	// Set direction to INPUT
	libsoc_gpio_set_direction(gpio_btn_arm, INPUT);
	if (libsoc_gpio_get_direction(gpio_btn_arm) != INPUT) printf("Failed to set direction to INPUT\n");
	

	libsoc_gpio_set_level(gpio_output, HIGH);
	if (libsoc_gpio_get_level(gpio_output) != HIGH) printf("Failed setting gpio level HIGH\n");


	libsoc_gpio_set_edge(gpio_btn_con, RISING);
  	if (libsoc_gpio_get_edge(gpio_btn_con) != RISING) printf("Failed to set edge to RISING\n");

	libsoc_gpio_set_edge(gpio_btn_arm, RISING);
  	if (libsoc_gpio_get_edge(gpio_btn_arm) != RISING) printf("Failed to set edge to RISING\n");
	

	// Setup callback
	btn_id_con = BUTTON_CONNECT;
  	int res = libsoc_gpio_callback_interrupt(gpio_btn_con, &callback_con,(void*) &btn_id_con);
	if (res == EXIT_FAILURE) perror("Failed to set gpio callback");

	btn_id_arm = BUTTON_ARM;
	res = libsoc_gpio_callback_interrupt(gpio_btn_arm, &callback_con,(void*) &btn_id_arm);
	if (res == EXIT_FAILURE) perror("Failed to set gpio callback");

	printf("Setting off interrupts generation...\n"); 

	printf("Waiting for interrupt. Press 'q' and 'Enter' at any time to exit\n");

	char key = -1;
	while (1) {
			key = fgetc(stdin);
			if (key == 'q')
				//goto exit;
				break;
	}

	//exit:
		if (gpio_btn_con) {
			res = libsoc_gpio_free(gpio_btn_con);
			if (res == EXIT_FAILURE)perror("Could not free gpio CON");
		}

		if (gpio_btn_arm) {
			res = libsoc_gpio_free(gpio_btn_arm);
			if (res == EXIT_FAILURE)
				perror("Could not free gpio ARM");
		}

		if (gpio_output) {
			res = libsoc_gpio_free(gpio_output);
			if (res == EXIT_FAILURE)
				perror("Could not free led gpio");
		}

		return res;

}















//// ****** CODE FOR OPENING GPIOS AS FILES **** ///////

/* 
//#include "gpio_handler.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "libsoc_gpio.h"
#include "libsoc_debug.h" 

unsigned int LEDGPIO = 45;  
unsigned int BUTTON_GPIO = 30; 

int callback_test(void* arg)
{
   int* tmp_count = (int*) arg;
  
  *tmp_count = *tmp_count + 1; 

  printf("I Pressed the button");
  return EXIT_SUCCESS;
}


int main(int argc, char *argv[]){

	// Create both gpio pointers
	gpio *gpio_output, *gpio_input;

	// Enable debug output
	libsoc_set_debug(1);

	// Request gpios
	gpio_output = libsoc_gpio_request(LEDGPIO, LS_GPIO_GREEDY);
	//gpio_input = libsoc_gpio_request(BUTTON_GPIO, LS_GPIO_SHARED);

		// Ensure both gpio were successfully requested
	//if (gpio_output == NULL || gpio_input == NULL)
	if (gpio_output == NULL)
	{
		//goto fail;
		printf("gpio were Unsuccessfully requested\n");
	}
	
	// Set direction to OUTPUT
	libsoc_gpio_set_direction(gpio_output, OUTPUT);
	
	// Check the direction
	if (libsoc_gpio_get_direction(gpio_output) != OUTPUT)
	{
		printf("Failed to set direction to OUTPUT\n");
		//goto fail;
	}

	// Set direction to INPUT
	//libsoc_gpio_set_direction(gpio_input, INPUT);

		// Check the direction
	//if (libsoc_gpio_get_direction(gpio_input) != INPUT)
	//{
		printf("Failed to set direction to INPUT\n");
	// goto fail;
	//}

	libsoc_gpio_set_level(gpio_output, HIGH);
	 if (libsoc_gpio_get_level(gpio_output) != HIGH)
	{
		printf("Failed setting gpio level HIGH\n");
		//goto fail;
	}

	sleep(5);


	libsoc_gpio_free(gpio_output);











   
   
    gpio_export(LEDGPIO);    // The LED
    gpio_set_dir(LEDGPIO, OUTPUT_PIN);   // The LED is an output
	printf("Setting the flashing LED on\n");
	// Flash the LED 10 times
	for(int i=0; i<20; i++){
        gpio_set_value(LEDGPIO, HIGH);
		usleep(200000);         // on for 200ms
        gpio_set_value(LEDGPIO, LOW);
		usleep(200000);         // off for 200ms
	}
    printf("Finished Testing the GPIO Pins\n");
	gpio_unexport(LEDGPIO);     // unexport the LED 


	return 0;
}

 */