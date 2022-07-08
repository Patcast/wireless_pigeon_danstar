#include "gpio_handler.h"

unsigned int LEDGPIO = 86;   

int main(int argc, char *argv[]){

    printf("Testing the GPIO Pins\n");
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

