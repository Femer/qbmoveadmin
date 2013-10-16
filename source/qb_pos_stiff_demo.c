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

int port_selection(char*);
int open_port(char*);

int pilot_pos_stiff();
int set_pos_stiff(short int*, short int*);

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

	assert(pilot_pos_stiff());

	closeRS485(&comm_settings_t);

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
			fprintf(file,"serialport1 %s\n", my_port);
			fprintf(file,"port_2_enabled %d\n", 0);
			fprintf(file,"serialport2 %s\n", my_port);
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


int pilot_pos_stiff(){
	char c = ' ';
	short int pos, stiff;

    // Activate motors
    commActivate(&comm_settings_t, device_id, 1);

    // Instructions
    printf("\nTo change the _reference_ position use LEFT and RIGHT arrows.\n");
    printf("To change the _stiffness_ use UP and DOWN arrows.\n");
    printf("To end execution press type X\n");

   	pos = 0;
   	stiff = 0;

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
			// case 27:
			// 	break;
			// case 91:
			// 	break;
			case 65:
				//printf("up\n");
				stiff += DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 66:
				//printf("down\n");
				stiff -= DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 67:
				//printf("right\n");
				pos += DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			case 68:
				//printf("left\n");
				pos -= DEFAULT_INCREMENT * DEG_TICK_MULTIPLIER;
				break;
			default:
				//do nothing
				break;
		}

		set_pos_stiff(&pos, &stiff);
		printf("Pos: %d, Stiff %d\t\t", (int)(pos/DEG_TICK_MULTIPLIER), (int)(stiff/DEG_TICK_MULTIPLIER));
		printf("Pos (ticks): %d, Stiff(ticks): %d        \r", (int)pos, (int)stiff );
		//commSetInputs(&comm_settings_t, device_id, current_ref);
	}

	pos = 0;
	stiff = 0;
	set_pos_stiff(&pos, &stiff);
	usleep(500000);

	// Deactivate motors
    commActivate(&comm_settings_t, device_id, 0);

	// Restore the old tty settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    return 1;
}

int set_pos_stiff(short int* pos, short int* stiff) {
	short int curr_ref[NUM_OF_MOTORS];

	if (*pos > (DEFAULT_SUP_LIMIT / pow(2, DEFAULT_RESOLUTION) - DEFAULT_STIFFNESS * DEG_TICK_MULTIPLIER)) {
		*pos = (DEFAULT_SUP_LIMIT / pow(2, DEFAULT_RESOLUTION) - DEFAULT_STIFFNESS * DEG_TICK_MULTIPLIER);
	} else if (*pos < (DEFAULT_INF_LIMIT / pow(2, DEFAULT_RESOLUTION) + DEFAULT_STIFFNESS * DEG_TICK_MULTIPLIER)) {
		*pos = (DEFAULT_INF_LIMIT / pow(2, DEFAULT_RESOLUTION) + DEFAULT_STIFFNESS * DEG_TICK_MULTIPLIER);
	}

	if (*stiff > DEFAULT_STIFFNESS * DEG_TICK_MULTIPLIER) {
		*stiff = DEFAULT_STIFFNESS * DEG_TICK_MULTIPLIER;
	} else if (*stiff < 0) {
		*stiff = 0;
	}
 
	curr_ref[0] = *pos - *stiff;
	curr_ref[1] = *pos + *stiff;

	commSetInputs(&comm_settings_t, device_id, curr_ref);

	return 1;
}

/* END OF FILE */