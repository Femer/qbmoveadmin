//=================================================================     includes

#include "qbmoveAPI/qbmove_communications.h"
#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <termios.h>
#include <unistd.h>

//=============================================================     declarations

int mode_selection();
int port_selection(char*);
int open_port(char*);
int restore_params();
int change_id();
int set_resolution(int);
int set_pid_parameters();
int adjust_zeros();
int get_info();
int test();
int backup();
int calibrate();


//==================================================================     globals

int device_id;
comm_settings comm_settings_t;

//==============================================================================
//																			main
//==============================================================================

int main(int argc, char **argv){
	char port[255];
	device_id = BROADCAST_ID;

	assert(port_selection(port));

	assert(open_port(port));

	assert(restore_params());

	// assert(change_id());

	// assert(set_resolution(DEFAULT_RESOLUTION));

	// assert(set_pid_parameters());

	assert(adjust_zeros());

	commRestoreParams(&comm_settings_t, device_id);

	assert(get_info());

	assert(calibrate());

	assert(get_info());	

	closeRS485(&comm_settings_t); //port close

	assert(test());

	assert(backup());

	printf("===== Configuration completed. =====\n");

	return 1;
}

//==========================================================     other functions

int port_selection(char* my_port){
	int i;
	int aux_int;
	int num_ports = 0;
	char ports[10][255];
	FILE *file;

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
	        } else {
	        	puts("Choice not available");
	        	continue;
	        }

	        file = fopen(QBMOVE_FILE, "w+");
			if (file == NULL) {
				printf("Cannot open qbmove.conf\n");
			}
			fprintf(file,"serialport %s\n", my_port);
			fclose(file);
			return 1;

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

int restore_params() {
	int i;
	printf("Restoring factory settings...");
	fflush(stdout);

	i = 1;
	while(commInitMem(&comm_settings_t, device_id)) {
		printf("Try #%d\n", i++);
		usleep(500000);
	}
	
    printf("Done.\n");
    return 1;	
}


int change_id() {
	int aux = 0;
	printf("Choose a new ID for the cube: ");
	scanf("%d", &device_id);

	printf("Changing device ID...");
	fflush(stdout);
	while(1) {
		// set
		while(commSetParam(&comm_settings_t, BROADCAST_ID, PARAM_ID, &device_id, 1));

		// store	
		while(commStoreDefaultParams(&comm_settings_t, BROADCAST_ID));

	    // check
	    while(commGetParam(&comm_settings_t, BROADCAST_ID, PARAM_ID, &aux, 1));

	    if (aux == device_id)
	    	break;
	}

    printf("Done\n");
	return 1;
}


int set_resolution(int res) {
	int i,j;
	unsigned char pos_resolution[NUM_OF_SENSORS];
	unsigned char aux[NUM_OF_SENSORS];

	printf("Setting resolution...");
	fflush(stdout);

	for (i = 0; i < NUM_OF_SENSORS; i++) {
		pos_resolution[i] = res;
	}

	j = 1;
	while(j) {
		// set
		while(commSetParam(&comm_settings_t, device_id, PARAM_POS_RESOLUTION, pos_resolution, NUM_OF_SENSORS));

		// store
	    while(commStoreDefaultParams(&comm_settings_t, device_id));

	    // check
	    while(commGetParam(&comm_settings_t, device_id,
	    		PARAM_POS_RESOLUTION, aux, NUM_OF_SENSORS));

	    for (i = 0; i < NUM_OF_SENSORS; i++) {
	    	if (aux[i] != pos_resolution[i]) {
	    		j = 1;
	    		break;
	    	} else {
	    		j = 0;
	    	}
	    }
	}

    printf("Done\n");
	return 1;
}

int set_pid_parameters() {
	float pid_control[3];
	float aux[3];
	fflush(stdout);

	pid_control[0] = DEFAULT_PID_P;
	pid_control[1] = DEFAULT_PID_I;
	pid_control[2] = DEFAULT_PID_D;

	printf("Setting PID parameters: %f, %f, %f\n",
			pid_control[0], pid_control[1], pid_control[2]);
	fflush(stdout);

	while(1) {
		// set
		while(commSetParam(&comm_settings_t, device_id,
	            PARAM_PID_CONTROL, pid_control, 3));
		// store	
		while(commStoreDefaultParams(&comm_settings_t, device_id));

	    // check
	    while(commGetParam(&comm_settings_t, device_id,
	    		PARAM_PID_CONTROL, aux, 3));
	    if ((aux[0] < pid_control[0] + 0.0001) || (aux[0] > pid_control[0] - 0.0001)) {
	    	if ((aux[2] < pid_control[2] + 0.0001) || (aux[2] > pid_control[2] - 0.0001)) {
	    		break;
	    	}
	    }
	}

    printf("DONE\n");
    return 1;
}


int adjust_zeros(){
	int i,j;
	char aux_char;
	char c = ' ';
	short int measurements[NUM_OF_SENSORS];
	short int measurements_off[NUM_OF_SENSORS];
	short int aux[NUM_OF_SENSORS];
	short int current_ref[NUM_OF_MOTORS];

	// Setting current offset to zero
	for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] = 0;
    }
    j = 1;
    while(j) {
    	// set
	    while(commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	            measurements_off, NUM_OF_SENSORS));

	    // store
    	while(commStoreDefaultParams(&comm_settings_t, device_id));

	    // check
	    while(commGetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	    		aux, NUM_OF_SENSORS));

	    for (i = 0; i < NUM_OF_SENSORS; i++) {
	    	if (aux[i] != measurements_off[i]) {
	    		j = 1;
	    		break;
	    	} else {
	    		j = 0;
	    	}
	    }
	}

    // Reading current position
    while(commGetMeasurements(&comm_settings_t, device_id, measurements));

    // Prepare offsets for setting zero
    for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] = -measurements[i];
    }

    // Setting zero for current position
    j = 1;
    while(j) {
    	// set
	    while(commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	            measurements_off, NUM_OF_SENSORS));

	    // store
   		while(commStoreDefaultParams(&comm_settings_t, device_id));

	    // check
	    while(commGetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	    		aux, NUM_OF_SENSORS));

	    for (i = 0; i < NUM_OF_SENSORS; i++) {
	    	if (aux[i] != measurements_off[i]) {
	    		j = 1;
	    		break;
	    	} else {
	    		j = 0;
	    	}
	    }
	}


    // Activate motors
    while (1) {
    	// set
		commActivate(&comm_settings_t, device_id, 1);
		usleep(100000);

		// check
		commGetActivate(&comm_settings_t, device_id, &aux_char);

		if (aux_char == 0x03)
			break;
	}


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
			case 'x':
				break;
			default:
				printf("Use only q, a, w, s, e, d in lowercase\n");
				break;
		}

		commSetInputs(&comm_settings_t, device_id, current_ref);
	}

	// Restore the old tty settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);


    // Deactivate motors
    while (1) {
    	//set
		commActivate(&comm_settings_t, device_id, 0);
		usleep(100000);

		//check
		commGetActivate(&comm_settings_t, device_id, &aux_char);

		if (aux_char == 0x00)
			break;
	}

	// Setting current offset to zero
	for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] = 0;
    }
    j = 1;
    while(j) {
    	// set
	    while(commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	            measurements_off, NUM_OF_SENSORS));

	    // store
    	while(commStoreDefaultParams(&comm_settings_t, device_id));

	    // check
	    while(commGetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	    		aux, NUM_OF_SENSORS));

	    for (i = 0; i < NUM_OF_SENSORS; i++) {
	    	if (aux[i] != measurements_off[i]) {
	    		j = 1;
	    		break;
	    	} else {
	    		j = 0;
	    	}
	    }
	}

	// Reading current position
	while(commGetMeasurements(&comm_settings_t, device_id, measurements));

    // Prepare offsets for setting zero
    for (i = 0; i < NUM_OF_SENSORS; i++) {
    	measurements_off[i] = -measurements[i];
    }

    // Setting zero for current position
    printf("Setting new zero for pulleys and shaft...");
    fflush(stdout);

    j = 1;
    while(j) {
    	// set
	    while(commSetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	            measurements_off, NUM_OF_SENSORS));
	    // store
   		while(commStoreDefaultParams(&comm_settings_t, device_id));

	    //check
	    while(commGetParam(&comm_settings_t, device_id, PARAM_MEASUREMENT_OFFSET,
	    		aux, NUM_OF_SENSORS));

	    for (i = 0; i < NUM_OF_SENSORS; i++) {
	    	if (aux[i] != measurements_off[i]) {
	    		j = 1;
	    		break;
	    	} else {
	    		j = 0;
	    	}
	    }
	}

    printf("DONE\n");

    return 1;
}


int test() {
	char c;

	printf("Do you want to perform a test cycle? [Y,n]\n");
	c = getchar();
	if (c == 'n' || c == 'N') {
		return 1;
	}

	system("./bin/qbtest -r 1");

	return 1;
}

int get_info() {
	printf("Displaying informations:\n\n");
	char aux_string[2000];

	RS485GetInfo(&comm_settings_t,  aux_string);
    puts(aux_string);

    return 1;
}


int calibrate() {
	char c;

	printf("Do you want to auto-calibrate stiffness? [Y,n]\n");
	c = getchar();
	if (c == 'n' || c == 'N') {
		return 1;
	}

	printf("Calibrating...");
	fflush(stdout);
	if(!commCalibrate(&comm_settings_t, BROADCAST_ID)) {
		printf("EXECUTING: wait for the end of calibration\n");
		sleep(20);
		return 1;
	} else {
		printf("FAILED\n");
		return 0;
	}
}

int backup() {

	system("./bin/qbbackup");
	return 1;
}
/* END OF FILE */