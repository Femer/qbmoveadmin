//=================================================================     includes

#include "qbmoveAPI/qbmove_communications.h"
#include "definitions.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <signal.h>

//==================================================================     defines

#define REPETITION_PER_CYCLE 3
#define BATCH_CYCLES 5 //number of cycle after which have a pause
#define INF_LIMIT DEFAULT_INF_LIMIT / (DEFAULT_RESOLUTION * 2)
#define SUP_LIMIT DEFAULT_SUP_LIMIT / (DEFAULT_RESOLUTION * 2)
#define DELAY 1500000	//in microseconds
#define LITTLE_DELAY 5000 //in microseconds
#define PAUSE 30 //in seconds


//===============================================================     structures

static const struct option longOpts[] = {
	{ "set_time", required_argument, NULL, 't'},
    { "set_repetitions", required_argument, NULL, 'r'},
    { NULL, no_argument, NULL, 0 }
};

static const char *optString = "t:r:";

struct global_var {
	int flag_set_time;
	int flag_set_repetitions;
} gv;

//=============================================================     declarations

void print_usage();
int open_port();
int cycle();
void int_handler(int sig);

//=========================================================     global variables

comm_settings comm_settings_t;
int device_id = BROADCAST_ID;

//==============================================================================
//																			main
//==============================================================================

int main(int argc, char **argv){
	int option;
	int longIndex = 0;
	int repetitions;

	assert(open_port());


	while ((option = getopt_long( argc, argv, optString, longOpts, &longIndex )) != -1) {
        switch (option) {
        	case 't':
        		gv.flag_set_time = 1;
        		break;
        	case 'r':
        		gv.flag_set_repetitions = 1;
        		sscanf(optarg, "%d", &repetitions);
        		break;
        	default:
        		printf("Insert a valid option\n");
        		print_usage();
        		break;
        }
    }

    if (optind == 1) {
    	print_usage();
    }


    if (gv.flag_set_time) {

    }

    if (gv.flag_set_repetitions) {
    	while(repetitions) {
    		if ((repetitions % BATCH_CYCLES) == 0) {
    			printf("Having a pause :)\n");
    			sleep(PAUSE);
    		}
    		repetitions--;
    		printf("Remaining cycles: %d\n", repetitions);
    		cycle();
    	}
    }

    closeRS485(&comm_settings_t);

	return 1;
}

//==========================================================     other functions

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

int cycle() {
	static int i, j;
	short int inputs[NUM_OF_MOTORS];
	short int measurements_1[NUM_OF_SENSORS];
	short int measurements_2[NUM_OF_SENSORS];

	// CTRL-C handler
    signal(SIGINT, int_handler);

	// Activate motors
    commActivate(&comm_settings_t, device_id, 1);

	// minimum stiffness
	for (i = 0; i < REPETITION_PER_CYCLE; i++) {
		inputs[0] = INF_LIMIT;
		inputs[1] = INF_LIMIT;

		commSetInputs(&comm_settings_t, device_id, inputs);
		usleep(DELAY);

		while(commGetMeasurements(&comm_settings_t, device_id, measurements_1));

		inputs[0] = SUP_LIMIT;
		inputs[1] = SUP_LIMIT;		

		commSetInputs(&comm_settings_t, device_id, inputs);
		usleep(DELAY);

		while(commGetMeasurements(&comm_settings_t, device_id, measurements_2));

		printf("difference between output shaft and pulleys:\n1: %d\t2: %d\n", measurements_1[2] - measurements_1[0], measurements_2[2] - measurements_2[0]);
	}

	// set to zero
	inputs[0] = 0;
	inputs[1] = 0;		
	commSetInputs(&comm_settings_t, device_id, inputs);
	usleep(DELAY / 2);


	// set max stiffness in lower position
	inputs[0] = INF_LIMIT;
	inputs[1] = INF_LIMIT + (DEFAULT_STIFFNESS * 2 * DEG_TICK_MULTIPLIER);
	commSetInputs(&comm_settings_t, device_id, inputs);
	usleep(DELAY / 2);

	// maximum stiffness
	for (i = 0; i < REPETITION_PER_CYCLE; i++) {
		for (j = 0; j < DEFAULT_MAX_EXCURSION - DEFAULT_STIFFNESS * 2; j++) {
			inputs[0] += 1 * DEG_TICK_MULTIPLIER;
			inputs[1] += 1 * DEG_TICK_MULTIPLIER;
			commSetInputs(&comm_settings_t, device_id, inputs);
			usleep(LITTLE_DELAY);
		}

		for (j = 0; j < DEFAULT_MAX_EXCURSION - DEFAULT_STIFFNESS * 2; j++) {
			inputs[0] -= 1 * DEG_TICK_MULTIPLIER;
			inputs[1] -= 1 * DEG_TICK_MULTIPLIER;
			commSetInputs(&comm_settings_t, device_id, inputs);
			usleep(LITTLE_DELAY);
		}
	}

	// set max stiffness in lower position (other side)
	inputs[0] = INF_LIMIT + (DEFAULT_STIFFNESS * 2 * DEG_TICK_MULTIPLIER);
	inputs[1] = INF_LIMIT;
	commSetInputs(&comm_settings_t, device_id, inputs);
	usleep(DELAY / 2);

	// maximum stiffness
	for (i = 0; i < REPETITION_PER_CYCLE; i++) {
		for (j = 0; j < DEFAULT_MAX_EXCURSION - DEFAULT_STIFFNESS * 2; j++) {
			inputs[0] += 1 * DEG_TICK_MULTIPLIER;
			inputs[1] += 1 * DEG_TICK_MULTIPLIER;
			commSetInputs(&comm_settings_t, device_id, inputs);
			usleep(LITTLE_DELAY);
		}

		for (j = 0; j < DEFAULT_MAX_EXCURSION - DEFAULT_STIFFNESS * 2; j++) {
			inputs[0] -= 1 * DEG_TICK_MULTIPLIER;
			inputs[1] -= 1 * DEG_TICK_MULTIPLIER;
			commSetInputs(&comm_settings_t, device_id, inputs);
			usleep(LITTLE_DELAY);
		}
	}

	// set to zero
	inputs[0] = 0;
	inputs[1] = 0;		
	commSetInputs(&comm_settings_t, device_id, inputs);
	usleep(DELAY / 2);

	// Deactivate motors
    commActivate(&comm_settings_t, device_id, 0);

	return 1;
}

void int_handler(int sig) {
	printf("Entrato l'handler\n");
	short int inputs[NUM_OF_MOTORS];

	inputs[0] = 0;
	inputs[1] = 0;

	commSetInputs(&comm_settings_t, device_id, inputs);

	usleep(2000000);

	commActivate(&comm_settings_t, device_id, 0);

	exit(0);
}

void print_usage() {
	printf("====================================================================\n");
	printf("qbtest - make a series of test cycles\n");
	printf("====================================================================\n");
	printf("usage: qbtest [OPTIONS]\n");
	printf("--------------------------------------------------------------------\n");
	printf("OPTIONS:\n");
	printf(" -t, --set_time <value>             Set time in minutes for the test\n");
	printf(" -r, --set_repetitions <value>      Set number of cycle to perform\n");
	printf("====================================================================\n");
	return;
}

/* END OF FILE */