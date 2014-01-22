// --- INCLUDE ---
#include "qbmoveAPI/qbmove_communications.h"
#include "definitions.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

// function declaration
int open_port();
int initMemory();
void printMainMenu();
void printParamMenu();
int calibrate();

// global variables
char get_or_set;
comm_settings comm_settings_t;


// --- MAIN ---
int main() {
	char choice;
	int device_id;
	int aux_int;
	short int aux_sint;
	unsigned char aux_uchar;
	float pid_control[3];
	short int offsets[NUM_OF_SENSORS];


	assert(open_port());

	printMainMenu();
	scanf("%c", &choice);
	printf("choice: %c\n", choice);

	switch(choice) {
		case 'g':
			get_or_set = choice;
			break;
		case 's':
			get_or_set = choice;
			break;
		case 'm':
			initMemory();
			break;
		case 'c':
			calibrate();
			break;

		default:
			break;
	}

	if (get_or_set == 'g' || get_or_set == 's') {
		printParamMenu();

		scanf(" %c", &choice);

		if (get_or_set == 's') {
			switch(choice) {
				// Set new ID
				case 'i':
					printf("Choose a new ID: ");
					scanf("%d", &device_id);

					commSetParam(&comm_settings_t, BROADCAST_ID,
	            		PARAM_ID, &device_id, 1);
					usleep(100000);
					commStoreDefaultParams(&comm_settings_t, BROADCAST_ID);
					usleep(500000);
					break;

				// Set new PID parameters
				case 'k':
					printf("Set the new values for the PID controller:\nK_p: ");
		            scanf("%f", pid_control);
		            printf("K_i: ");
		            scanf("%f", pid_control + 1);
		            printf("K_d: ");
		            scanf("%f", pid_control + 2);

		            commSetParam(&comm_settings_t, BROADCAST_ID,
		            	PARAM_PID_CONTROL, pid_control, 3);
			        usleep(100000);
			        commStoreDefaultParams(&comm_settings_t, BROADCAST_ID);
			        usleep(500000);
					break;
				// Set startup activation flag
				case 'a':
					printf("Do you want the motor to be ON [1] or OFF [0] at startup?: ");
					scanf("%d", &aux_int);

					if (aux_int == 0) {
						aux_uchar = 0;
						commSetParam(&comm_settings_t, BROADCAST_ID,
            				PARAM_STARTUP_ACTIVATION, &aux_uchar, 1);
					} else {
						aux_uchar = 3;
						commSetParam(&comm_settings_t, BROADCAST_ID,
            				PARAM_STARTUP_ACTIVATION, &aux_uchar, 1);
					}
					commStoreDefaultParams(&comm_settings_t, BROADCAST_ID);
					break;


				case 'm':
					break;
				case 's':
					break;
				case 'o':
					printf("Set the new offsets:\n");
					printf("Offset 1: ");
					scanf("%hd", &aux_sint);
					offsets[0] = aux_sint;
					printf("Offset 2: ");
					scanf("%hd", &aux_sint);
					offsets[1] = aux_sint;
					printf("Offset 3: ");
					scanf("%hd", &aux_sint);
					offsets[2] = aux_sint;

					commSetParam(&comm_settings_t, BROADCAST_ID, PARAM_MEASUREMENT_OFFSET,
				            offsets, NUM_OF_SENSORS);
				    usleep(100000);
				    commStoreDefaultParams(&comm_settings_t, BROADCAST_ID);
				    usleep(1000000);
					break;
				case 'u':
					break;
				case 'f':
					break;
				case 'l':
					break;
			}
		} else {
			switch(choice) {
				case 'i':
					break;
				case 'k':
					break;
				case 'a':
					break;
				case 'm':
					break;
				case 's':
					break;
				case 'o':
					break;
				case 'u':
					break;
				case 'f':
					break;
				case 'l':
					break;
			}

		}

	}

	return 1;
}


int open_port() {
	FILE *file;
	char port[255];

	file = fopen(QBMOVE_FILE, "r");

    fscanf(file, "serialport %s\n", port);

    fclose(file);


    openRS485(&comm_settings_t, port);
    
    if(comm_settings_t.file_handle == INVALID_HANDLE_VALUE)
    {
        puts("Couldn't connect to the serial port.");
        return 0;
    }
    usleep(500000);

    return 1;
}

int initMemory() {
	char choice;

	getchar();
	printf("WARNING: Restore initial memory settings? [y/N]\n");
	choice = getchar();

	if (choice != 'y' && choice != 'Y') {
		return 0;
	}

	printf("Initializing memory...");

	if (!commInitMem(&comm_settings_t, BROADCAST_ID)) {
		printf("DONE\n");
		return 1;
	}

	printf("Failed\n");
	return 0;
}

int calibrate() {
	printf("Calibrating...");
	fflush(stdout);
	if(!commCalibrate(&comm_settings_t, BROADCAST_ID)) {
		printf("DONE\n");
		return 1;
	} else {
		printf("FAILED\n");
		return 0;
	}
}

void printMainMenu() {
	printf("g: getParam\n");
	printf("s: setParam\n");
	printf("m: initMemory\n");
	printf("c: calibrate\n");
}

void printParamMenu() {
	printf("i: ID\n");
	printf("k: PID params\n");
	printf("a: startup activation flag\n");
	printf("m: input mode\n");
	printf("s: resolution\n");
	printf("o: measurements offset\n");
	printf("u: multipliers\n");
	printf("f: position limit flag\n");
	printf("l: position limits\n");
}

/* END OF FILE */