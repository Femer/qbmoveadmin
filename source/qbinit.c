//=================================================================     includes

#include "qbmoveAPI/qbmove_communications.h"
#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <termios.h>
#include <unistd.h>

//==================================================================     defines

#define DEFAULT_RESOLUTION 1
#define DEFAULT_ID 0
#define DEFAULT_PROPORTIONAL_GAIN 0.1
#define DEFAULT_INCREMENT 1 //in degree
#define DEFAULT_STIFFNESS 20 //in degree

#define DEG_TICK_MULTIPLIER 65536 / (360 * (1 + DEFAULT_RESOLUTION))

//=============================================================     declarations

int port_selection(char*);
int open_port(char*);
int change_id();
int set_resolution();
int set_proportional_gain();
int adjust_zeros();

//==================================================================     globals

int device_id;
comm_settings comm_settings_t;

//==============================================================================
//																			main
//==============================================================================

int main(int argc, char **argv){
	char port[255];
	device_id = DEFAULT_ID;

	assert(port_selection(port));

	assert(open_port(port));

	assert(change_id());

	assert(set_resolution());

	assert(set_proportional_gain());

	assert(adjust_zeros());

	printf("Configuration completed\n");
	return 1;
}

//==========================================================     other functions


int port_selection(char* my_port){
	int i;
	int aux_int;
	int num_ports = 0;
	char ports[10][255];

	while(1) {
		num_ports = RS485listPorts(ports);
	      
	    if(num_ports) {
	        puts("\nChoose the serial port for your QB:\n");

	        for(i = 0; i < num_ports; ++i) {
	            printf("[%d] - %s\n\n", i+1, ports[i]);
	        }
	        printf("Serial port: ");
	        scanf("%d", &aux_int);
	        
	        if( aux_int && (aux_int <= num_ports) ) {
	            strcpy(my_port, ports[aux_int - 1]);
	            return 1;          
	        } else {
	        	puts("Choice not available");
	        }
	    } else {
	        puts("No serial port available.");
	        return 0;
	    }
	}
}


int open_port(char* port_s) {
	printf("Opening serial port...");
	fflush(stdout);

	openRS485(&comm_settings_t, port_s);
    
    if(comm_settings_t.file_handle == INVALID_HANDLE_VALUE)
    {
        puts("Couldn't connect to the serial port.");
        return 0;
    }
    usleep(500000);
    printf("Done.\n");
    return 1;
}


int change_id() {
	printf("Choose a new ID for the cube: ");
	scanf("%d", &device_id);

	printf("Changing device ID...");
	fflush(stdout);

	commSetParam(&comm_settings_t, DEFAULT_ID,
            PARAM_ID, &device_id, 1);
    commStoreParams(&comm_settings_t, DEFAULT_ID);

    usleep(500000);
    printf("Done\n");
	return 1;
}


int set_resolution() {
	int i;
	unsigned char pos_resolution[NUM_OF_SENSORS];

	printf("Setting resolution...");
	fflush(stdout);

	for (i = 0; i < NUM_OF_SENSORS; i++) {
		pos_resolution[i] = DEFAULT_RESOLUTION;
	}

	commSetParam(&comm_settings_t, device_id,
            PARAM_POS_RESOLUTION, pos_resolution, NUM_OF_SENSORS);
    commStoreParams(&comm_settings_t, device_id);

    usleep(500000);
    printf("Done\n");
	return 1;
}

int set_proportional_gain() {
	float control_k = DEFAULT_PROPORTIONAL_GAIN;
	printf("Setting proportional gain...");

	commSetParam(&comm_settings_t, device_id,
            PARAM_CONTROL_K, &control_k, 1);
    commStoreParams(&comm_settings_t, device_id);
    usleep(100000);
    printf("DONE\n");
    return 1;
}


int adjust_zeros(){
	int i;
	char c = ' ';
	short int measurements[NUM_OF_SENSORS];
	short int measurements_off[NUM_OF_SENSORS];
	short int current_ref[NUM_OF_MOTORS];

	// Setting current offset to zero
	for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] = 0;
    }
    commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
            measurements_off, NUM_OF_SENSORS);

    commStoreParams(&comm_settings_t, device_id);
    usleep(100000);

    // Reading current position
    while(commGetMeasurements(&comm_settings_t, device_id, measurements));

    // Prepare offsets for setting zero
    for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] = -measurements[i];
    }

    // Setting zero for current position
    commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
            measurements_off, NUM_OF_SENSORS);

    commStoreParams(&comm_settings_t, device_id);
    usleep(100000);

    // Activate motors
    commActivate(&comm_settings_t, device_id, 1);

    // Instructions
    printf("\nTo move the motors use letters 'q', 'a', 'w', 's'.\n");
    printf("To move the output shaft (two motors toghether) use letters 'e', 'd':\n");
    printf("When you are done type 'x' to set zero for current position\n\n");
    printf("    | M1 | M2 | OUT |\n");
    printf("----|----|----|-----|\n");
    printf(" -> | q  | w  |  e  |\n");
    printf(" <- | a  | s  |  d  |\n");

   	for (i = 0; i < NUM_OF_MOTORS; i++) {
   		current_ref[i] = 0;
   	}

   	//---- tty inizialization ---- BEGIN

	static struct termios oldt, newt;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);
    newt.c_lflag &= ~(ICANON | ECHO);       

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    
    //---- tty inizialization ---- END

    while(c != 'x') {
		c = getchar();
		switch(c) {
			case 'q':
				current_ref[0] += DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 'a':
				current_ref[0] -= DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 'w':
				current_ref[1] += DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 's':
				current_ref[1] -= DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 'e':
				current_ref[0] += DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				current_ref[1] += DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 'd':
				current_ref[0] -= DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				current_ref[1] -= DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			default:
				printf("Use only q, a, w, s, e, d in lowercase\n");
				break;
		}

		commSetInputs(&comm_settings_t, device_id, current_ref);
	}

	// Restore the old tty settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);


    // Reading current position
    while(commGetMeasurements(&comm_settings_t, device_id, measurements));

    // Prepare offsets for setting zero
    for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] -= measurements[i];
    }

    // Deactivate motors
    commActivate(&comm_settings_t, device_id, 0);

    // Setting zero for current position
    printf("Setting zero for pulleys...");
    commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
            measurements_off, NUM_OF_SENSORS);

    commStoreParams(&comm_settings_t, device_id);
    usleep(100000);
    printf("DONE\n");

    return 1;
}