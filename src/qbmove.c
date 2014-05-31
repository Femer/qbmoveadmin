//==================================================================     defines


//#define PHIDGETS_BRIDGE
//#define TEST_MODE

//===================================================================      const


//=================================================================     includes

#include "../../qbmoveAPI/src/qbmove_communications.h"
#include "definitions.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>


#ifdef PHIDGETS_BRIDGE
    #include <phidget21.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
	#define sleep(x) Sleep(1000 * x)
#endif

//===============================================================     structures


static const struct option longOpts[] = {
    { "set_inputs", required_argument, NULL, 's' },
    { "get_measurements", no_argument, NULL, 'g' },
    { "activate", no_argument, NULL, 'a' },    
    { "deactivate", no_argument, NULL, 'd' },
    { "ping", no_argument, NULL, 'p' }, 
    { "serial_port", no_argument, NULL, 't' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { "file", required_argument, NULL, 'f'},
    { "log", no_argument, NULL, 'l'},
    { "test_force", no_argument, NULL, 'w'},
    { "set_zeros", no_argument, NULL, 'z'},
    { "use_gen_sin", no_argument, NULL, 'k'},
    { "get_currents", no_argument, NULL, 'c'},
    { "bootloader", no_argument, NULL, 'b'},
    { "set_pos_stiff", required_argument, NULL, 'e'},
    { NULL, no_argument, NULL, 0 }
};

static const char *optString = "s:adgptvh?f:lwzkcbe:";

struct global_args {
    int device_id;
    int flag_set_inputs;            /* -s option */
    int flag_get_measurements;      /* -g option */
    int flag_activate;              /* -a option */
    int flag_deactivate;            /* -d option */    
    int flag_ping;                  /* -p option */
    int flag_serial_port;           /* -t option */
    int flag_verbose;               /* -v option */
    int flag_file;                  /* -f option */
    int flag_log;                   /* -l option */
    int flag_test;                  /* -w option */
    int flag_set_zeros;             /* -z option */
    int flag_use_gen_sin;           /* -k option */
    int flag_get_currents;          /* -c option */
    int flag_bootloader_mode;       /* -b option */
    int flag_set_pos_stiff;         /* -e option */


    short int inputs[NUM_OF_MOTORS];
    short int measurements[NUM_OF_SENSORS];
    short int measurement_offset[NUM_OF_SENSORS];
    short int currents[NUM_OF_MOTORS];
    char filename[255];
    char log_file[255];
    
    FILE* log_file_fd;
} global_args;  //multiple boards on multiple usb

struct position {
    float prec;
    float act;
} p1, p2;


//==========================================================    global variables

#ifdef PHIDGETS_BRIDGE
    CPhidgetBridgeHandle bridge;
    double m = -23009.3523;
    double q = -64.504;  

    double current_force;
#endif

float gear_ratio[NUM_OF_MOTORS];            // motor gear ratio

uint8_t resolution[NUM_OF_SENSORS];         // sensors resolution set on the board
float correction_factor[NUM_OF_SENSORS];    // correction factor calculated
                                            //   using resolution and gear ratio
float glob_measurements[NUM_OF_SENSORS];         //measurements

int ret;                                    //utility variable to store return values

comm_settings comm_settings_1;
    


//=====================================================     function definitions

/** Display program usage, and exit.
 */
void display_usage( void );


/** Parse csv input file with values to be sent to the motors
 */
float** file_parser(char*, int*, int*);

/** CTRL-c handler 1
 */
void int_handler(int sig);

/** CTRL-c handler 2
 */
void int_handler_2(int sig); 


//==================================================== phidgets functions

#ifdef PHIDGETS_BRIDGE

    int CCONV AttachHandler(CPhidgetHandle phid, void *userptr)
    {
        CPhidgetBridgeHandle bridge = (CPhidgetBridgeHandle)phid;

        CPhidgetBridge_setEnabled(bridge, 0, PTRUE);
        CPhidgetBridge_setEnabled(bridge, 1, PFALSE);
        CPhidgetBridge_setEnabled(bridge, 2, PFALSE);
        CPhidgetBridge_setEnabled(bridge, 3, PFALSE);

        CPhidgetBridge_setGain(bridge, 0, PHIDGET_BRIDGE_GAIN_128);
        CPhidgetBridge_setGain(bridge, 1, PHIDGET_BRIDGE_GAIN_16);
        CPhidgetBridge_setGain(bridge, 2, PHIDGET_BRIDGE_GAIN_32);
        CPhidgetBridge_setGain(bridge, 3, PHIDGET_BRIDGE_GAIN_64);
        CPhidgetBridge_setDataRate(bridge, 1);

        printf("Attach handler ran!\n");
        return 0;
    }

    int CCONV DetachHandler(CPhidgetHandle phid, void *userptr)
    {
        printf("Detach handler ran!\n");
        return 0;
    }

    int CCONV ErrorHandler(CPhidgetHandle phid, void *userptr, int ErrorCode, const char *errorStr)
    {
        printf("Error event: %s\n",errorStr);
        return 0;
    }

    void display_generic_properties(CPhidgetHandle phid)
    {
        int sernum, version;
        const char *deviceptr;
        CPhidget_getDeviceType(phid, &deviceptr);
        CPhidget_getSerialNumber(phid, &sernum);
        CPhidget_getDeviceVersion(phid, &version);

        printf("%s\n", deviceptr);
        printf("Version: %8d SerialNumber: %10d\n", version, sernum);
        return;
    }

    int CCONV data(CPhidgetBridgeHandle phid, void *userPtr, int index, double val)
    {   
        current_force = m*val + q;

        return 0;
    }


    void test()
    {
        //gettimeofday(&t_ref, &foo);
        
        const char *err;    
        int result;
        CPhidgetBridgeHandle bridge;

        CPhidgetBridge_create(&bridge);

        CPhidget_set_OnAttach_Handler((CPhidgetHandle)bridge, AttachHandler, NULL);
        CPhidget_set_OnDetach_Handler((CPhidgetHandle)bridge, DetachHandler, NULL);
        CPhidget_set_OnError_Handler((CPhidgetHandle)bridge, ErrorHandler, NULL);
        CPhidgetBridge_set_OnBridgeData_Handler(bridge, data, NULL);

        CPhidget_open((CPhidgetHandle)bridge, -1);

        //Wait for 1 second, otherwise exit
        if(result = CPhidget_waitForAttachment((CPhidgetHandle)bridge, 1000))
        {

            CPhidget_getErrorDescription(result, &err);
            printf("Problem waiting for attachment: %s\n", err);
            return;
        }

        display_generic_properties((CPhidgetHandle)bridge);

        return;
    }

#endif


//==============================================================================
//                                                                     main loop
//==============================================================================


/** main loop
 */
int main (int argc, char **argv)
{

    FILE *file;

    int  i = 0;             // global counters
    int  k = 0;

    int  num_ports = 0;     // variables for COMM config
    char ports[10][255];      
    int  aux_int;
    char port[255];
    
    char aux_string[10000]; // used to store PING reply
    int  aux[3];             // used to store input during set_inputs

    int  option;             // used for processing options
    int  longIndex = 0;

    char aux_char;

    // initializations

    global_args.device_id               = 0;
    global_args.flag_serial_port        = 0;
    global_args.flag_ping               = 0;
    global_args.flag_verbose            = 0;
    global_args.flag_activate           = 0;
    global_args.flag_deactivate         = 0;
    global_args.flag_get_measurements   = 0;
    global_args.flag_set_inputs         = 0;
    global_args.flag_file               = 0;
    global_args.flag_log                = 0;
    global_args.flag_test               = 0;
    global_args.flag_set_zeros          = 0;
    global_args.flag_use_gen_sin        = 0;
    global_args.flag_bootloader_mode    = 0;
    global_args.flag_set_pos_stiff      = 0;
    
    //===================================================     processing options

    while ((option = getopt_long( argc, argv, optString, longOpts, &longIndex )) != -1)
    {
        switch (option)
        {
            case 's':
                sscanf(optarg,"%d,%d", &aux[0], &aux[1]);
                global_args.inputs[0] = (short int) aux[0];
                global_args.inputs[1] = (short int) aux[1];
                global_args.flag_set_inputs = 1;
                break;
            case 'e':
                sscanf(optarg,"%d,%d", &aux[0], &aux[1]);
                global_args.inputs[0] = (short int) aux[0];
                global_args.inputs[1] = (short int) aux[1];
                global_args.flag_set_pos_stiff = 1;
                break;
            case 'g':
                global_args.flag_get_measurements = 1;
                break;
            case 'a':
                global_args.flag_activate = 1;
                break;
            case 'd':
                global_args.flag_deactivate = 1;
                break;
            case 't':
                global_args.flag_serial_port = 1;
                break;
            case 'p':
                global_args.flag_ping = 1;
                break;
            case 'v':
                global_args.flag_verbose = 1;
                break;
            case 'f':
                sscanf(optarg, "%s", global_args.filename);        
                global_args.flag_file = 1;
                break;
            case 'l':
                global_args.flag_log = 1;
                break;
            case 'k':
                global_args.flag_use_gen_sin = 1;
                break;
            case 'z':
                global_args.flag_set_zeros = 1;
                break;
            case 'w':
                global_args.flag_test = 1;
                break;
            case 'c':
                global_args.flag_get_currents = 1;
                break;
            case 'b':
                global_args.flag_bootloader_mode = 1;
                break;
            case 'h':
            case '?':
            default:
                display_usage();    
                return 0;
                break;
        }
    }

    if((optind == 1) | (global_args.flag_verbose & (optind == 2)))
    {
        display_usage();    
        return 0;
    }

    //==================================================     setting serial port

    if(global_args.flag_serial_port)
    {

        num_ports = RS485listPorts(ports);
        int tmp = -1;
        aux_int = -1;

        if(num_ports)
        {
            puts("\nChoose serial port 1:\n");
            for(i = 0; i < num_ports; ++i)
            {
                printf("[%d] - %s\n\n", i+1, ports[i]);                
            }            
            printf("Serial port: ");
            scanf("%d", &aux_int);

            if( aux_int && (aux_int <= num_ports) )
            {
                tmp = aux_int;                    
            }
            else puts("Choice not available");

        } else {
            puts("No serial port available.");
        }

        if (num_ports - 1 > 0)
        {
            puts("\nChoose serial port 2:\n");
            for(i = 0; i < num_ports; ++i)
            {
                printf("[%d] - %s ", i+1, ports[i]);
                if (i == tmp - 1) {
                    printf("[unavailable] ");
                }              
                printf("\n\n");
            }
            printf("Serial port: ");
            scanf("%d", &aux_int);
        }
        if( aux_int && (aux_int <= num_ports) )
        {
            file = fopen(QBMOVE_FILE, "w+");
            if (file == NULL) {
                printf("Cannot open qbmove.conf\n");
            }
            fprintf(file,"serialport %s\n",ports[tmp-1]);
            fclose(file);                    
        } else {
            puts("Choice not available");
        }
        
        return 0;
    }    

    //==========================================     reading configuration files

    if(global_args.flag_verbose)
        puts("Reading configuration files.");

    file = fopen(QBMOVE_FILE, "r");

    fscanf(file, "serialport %s\n", port);

    fclose(file);


    if(global_args.flag_verbose)
        printf("Port: %s\n", port);


    file = fopen(MOTOR_FILE, "r");

    fscanf(file, "gear_ratio_1 %f\n", &gear_ratio[0]);
    fscanf(file, "gear_ratio_2 %f\n", &gear_ratio[1]);

    fclose(file);

    if(global_args.flag_verbose)
        printf("Gear ratio 1: %f\nGear ratio 2: %f\n",
            gear_ratio[0], gear_ratio[1]);


    //==================================================     opening serial port

#ifndef TEST_MODE

    if(global_args.flag_verbose)
        puts("Connecting to serial port.");    

    openRS485(&comm_settings_1, port);

    if(comm_settings_1.file_handle == INVALID_HANDLE_VALUE)
    {
        puts("Couldn't connect to the serial port 1.");

        if(global_args.flag_verbose)
            puts("Closing the application.");        

        return 0;
    } else {
        if(global_args.flag_verbose)
            puts("Serial port 1 connected");        
    }


    //==========================================     calculate correction factor

    // retrieve current resolution
    // while(commGetParam(&comm_settings_1, global_args.device_id,
    //     PARAM_POS_RESOLUTION, resolution, NUM_OF_SENSORS) != 0) {}

    resolution[0] = 1;
    resolution[1] = 1;
    resolution[2] = 1;



    
    // calculate correction factor for every sensors
    for (i = 0; i < NUM_OF_SENSORS; i++) {
        correction_factor[i] = 65536.0/(360 << resolution[i]);
        if (global_args.flag_verbose) {
            printf("Corr fact %d: %f\n", i, correction_factor[i]);
        }
    }

    // add gear ratio to correction factor
    for (i = 0; i < NUM_OF_MOTORS; i++) {
        correction_factor[i] = correction_factor[i] * gear_ratio[i];
    }
    

    //====================================================     gettind device id

    if (argc - optind == 1)
    {
        sscanf(argv[optind++],"%d",&global_args.device_id);
        if(global_args.flag_verbose)
            printf("Device ID:%d\n", global_args.device_id);
    }
    else if(global_args.flag_verbose)
        puts("No device ID was chosen. Running in broadcasting mode.");

#endif

    //=================================================================     ping

    // If ping... then DOESN'T PROCESS OTHER COMMANDS

    if(global_args.flag_ping)
    {
        if(global_args.flag_verbose)
            puts("Pinging serial port.");        

        if(global_args.device_id) {
            commGetInfo(&comm_settings_1, global_args.device_id, INFO_ALL, aux_string);            
        } else {
            RS485GetInfo(&comm_settings_1,  aux_string);
            printf("len of aux_String: %d\n", (int)strlen(aux_string));
            puts(aux_string);
        }
       

        //puts(aux_string);

        if(global_args.flag_verbose)
            puts("Closing the application.");        

        return 0;
    }

    //===========================================================     set inputs

    if(global_args.flag_set_inputs)
    {
        if(global_args.flag_verbose)
            printf("Setting inputs to %d and %d.\n",
                    global_args.inputs[0], global_args.inputs[1]);

        commSetInputs(&comm_settings_1, global_args.device_id,
                global_args.inputs);

    }


    //========================================================     set pos stiff

    if(global_args.flag_set_pos_stiff)
    {
        if(global_args.flag_verbose)
            printf("Setting pos to %d and stiffness to %d.\n",
                    global_args.inputs[0], global_args.inputs[1]);

        commSetPosStiff(&comm_settings_1, global_args.device_id,
                global_args.inputs);

    }


//=========================================================     get measurements

    if(global_args.flag_get_measurements)
    {

        // short int my_values[2 + NUM_OF_SENSORS];

        // commGetCurrAndMeas(&comm_settings_1, global_args.device_id,
        //         my_values);

        // printf("currents: %d, %d\n", my_values[0], my_values[1]);
        // printf("meas: %d %d %d\n", my_values[2], my_values[3], my_values[4]);
        
        // if(global_args.flag_verbose)
        //     puts("Getting measurements.");
      
        // commGetParam(&comm_settings_1, global_args.device_id,
        //         PARAM_MEASUREMENT_OFFSET, global_args.measurements,3);

        // printf("Offsets: ");
  //       for (i = 0; i < NUM_OF_SENSORS; i++) {
  //           printf("%d\t", global_args.measurements[i]);
  //       }
  //       printf("\n");

        while(1) {
            commGetMeasurements(&comm_settings_1, global_args.device_id,
                    global_args.measurements);

            printf("measurements * 8: ");
            for (i = 0; i < NUM_OF_SENSORS; i++) {
                printf("%d\t", (short int)(global_args.measurements[i] * 8));
            }
            printf("\n");

            printf("measurements:     ");
            for (i = 0; i < NUM_OF_SENSORS; i++) {
                printf("%d\t", global_args.measurements[i]);
            }
            printf("\n");

            usleep(100000);
        }
    }

    if(global_args.flag_bootloader_mode) {
        printf("Entering bootloader mode\n");
        if(!commBootloader(&comm_settings_1, global_args.device_id)) {
            printf("DONE\n");    
        } else {
            printf("An error occurred...\n");
        }
        
    }
    
//==========================================================     get_currents

    if(global_args.flag_get_currents) {

        if(global_args.flag_verbose)
            puts("Getting currents.");

        while(1) {
            commGetCurrents(&comm_settings_1, global_args.device_id,
                global_args.currents);

            printf("Current 1: %d\t Current 2: %d\n", global_args.currents[0], global_args.currents[1]);
            fflush(stdout);
            usleep(100000);
        }
    }
    

//=================================================================     activate
    
    if(global_args.flag_activate)
    {
        if(global_args.flag_verbose)
           puts("Turning QB Move on.\n");
        commActivate(&comm_settings_1, global_args.device_id, 1);

        commGetActivate(&comm_settings_1, global_args.device_id, &aux_char);

        printf("%c %d\n", aux_char, (int)aux_char);
    }
    
//===============================================================     deactivate

    if(global_args.flag_deactivate)
    {
        if(global_args.flag_verbose)
           puts("Turning QB Move off.\n");
        commActivate(&comm_settings_1, global_args.device_id, 0);
    }    

//==============================================================     use_gen_sin

    if(global_args.flag_use_gen_sin)
    {
        //variable declaration
        float delta_t;                      // milliseconds between values
        float amplitude_1, amplitude_2;     // sinusoid amplitude
        float bias_1, bias_2;               // sinusoid bias
        float freq_1, freq_2;               // sinusoid frequency
        float period_1, period_2;           // sinusoid period = 1/frequency
        float phase_shift;                  // angular shift between sinusoids
        float total_time;                   // total execution time (if 0 takes
                                            //   number of values as parameter)
        int num_values;                     // number of values (ignored if
                                            //   total time != 0)

        float angle_1 = 0;                  // actual angle
        float angle_2 = 0;                  // actual angle
        float inc_1, inc_2;                 // angle increment for every step

        struct timeval t_act, begin, end;
        struct timezone foo;

        int error_counter = 0;

        if(global_args.flag_log) {
            strcpy(global_args.log_file, "sin_log.csv");
            global_args.log_file_fd = fopen(global_args.log_file, "w");
        }
        

        // CTRL-C handler
        signal(SIGINT, int_handler);


        if(global_args.flag_verbose) {
           puts("Generate sinusoidal inputs\n");
        }

        // opening file
        FILE* filep;
        filep = fopen(SIN_FILE, "r");
        if (filep == NULL) {
            printf("Failed opening file\n");
        }

        fscanf(filep, "delta_t %f\n", &delta_t);
        fscanf(filep, "amplitude_1 %f\n", &amplitude_1);
        fscanf(filep, "amplitude_2 %f\n", &amplitude_2);
        fscanf(filep, "bias_1 %f\n", &bias_1);
        fscanf(filep, "bias_2 %f\n", &bias_2);
        fscanf(filep, "freq_1 %f\n", &freq_1);
        fscanf(filep, "freq_2 %f\n", &freq_2);
        fscanf(filep, "phase_shift %f\n", &phase_shift);
        fscanf(filep, "total_time %f\n", &total_time);
        fscanf(filep, "num_values %d\n", &num_values);


        // closing file
        fclose(filep);

        // if total_time set, calculate num_values
        if (total_time != 0) {
            num_values = (total_time*1000)/delta_t;
            printf("Num_values: %d\n", num_values);
        }

        // calculate periods
        period_1 = 1/freq_1;
        period_2 = 1/freq_2;

        // deg to rad
        phase_shift = phase_shift*PI/180.0;

        // calculate increment for every step
        inc_1 = (2*PI)/(period_1/(delta_t/1000.0));
        inc_2 = (2*PI)/(period_2/(delta_t/1000.0));

        printf("inc1 %f, inc2 %f\n", inc_1, inc_2); //XXX

        // activate motors
        commActivate(&comm_settings_1, global_args.device_id, 1);

        // retrieve begin time
        gettimeofday(&begin, &foo);

        for(i=0; i<num_values; i++) {
            // wait for next value
            while (1) {
                gettimeofday(&t_act, &foo);
                if (timevaldiff(&begin, &t_act) >= i * delta_t * 1000) {
                    break;
                }
            }

            // update measurements
            if (commGetMeasurements(&comm_settings_1, global_args.device_id,
                    global_args.measurements)) {
                error_counter++;
            }
            for (k = 0; k < NUM_OF_SENSORS; k++) {
                glob_measurements[k] = global_args.measurements[k]/correction_factor[k];
            }
            
            // update inputs
            global_args.inputs[0] = (cos(angle_1)*amplitude_1 + bias_1)*correction_factor[0];
            global_args.inputs[1] = (cos(angle_2 + phase_shift)*amplitude_2 + bias_2)*correction_factor[1];
                
            // set new inputs
            commSetInputs(&comm_settings_1, global_args.device_id, global_args.inputs);

            // update angle position
            angle_1 += inc_1;
            angle_2 += inc_2;

            //log file
            if (global_args.flag_log) {
                for (k = 0; k < NUM_OF_SENSORS; k++) {
                    fprintf(global_args.log_file_fd, "%f,\t", glob_measurements[k]);
                }
                fprintf(global_args.log_file_fd, "%f,\t%f\n",
                    (float)global_args.inputs[0]/correction_factor[0], (float)global_args.inputs[1]/correction_factor[1]);
                // fprintf(global_args.log_file_fd, "%d,\t%d\n",
                //     global_args.currents[0], global_args.currents[1]);
            }

        }

        // get time at the end of for cycle
        gettimeofday(&end, &foo);

        // reset motor  position
        global_args.inputs[0] = 0;
        global_args.inputs[1] = 0;
        commSetInputs(&comm_settings_1, global_args.device_id,
                global_args.inputs);

        printf("total time (millisec): %f\n", timevaldiff(&begin, &end)/1000.0);
        printf("Error counter: %d\n", error_counter);

    }


//===============================================================     input file
    
    if(global_args.flag_file){
        // variable declaration
        struct timeval t_ref, t_act, begin, end;
        struct timezone foo;
        short int retrieved_values[2 + NUM_OF_SENSORS];
        int deltat = 0; //time interval in millisecs
        int num_values = 0;
        float** array;
        char filename[255];
        char* extension;
        char* name;
        int error_counter = 0;
        
        //activate phidgets bridge if needed
#ifdef PHIDGETS_BRIDGE
        test();
#endif

        // VERBOSE ONLY
        if(global_args.flag_verbose) {
            printf("Parsing file %s\n", global_args.filename);
        }

        // parsing file
        array = file_parser(global_args.filename, &deltat, &num_values);

        // VERBOSE ONLY
        if(global_args.flag_verbose)
            printf("Sending %d values with Dt = %d\n", num_values, deltat);

        //if log enabled, open file for logging
        if(global_args.flag_log) {
            strcpy(filename, global_args.filename);
            name = strtok(filename, ".");
            extension = strtok(NULL, ".");
            strcpy(global_args.log_file, name);
            strcat(global_args.log_file, "_log.");
            strcat(global_args.log_file, extension);
            global_args.log_file_fd = fopen(global_args.log_file, "w");
        }

        //retrieve current time
        gettimeofday(&t_ref, &foo);
        gettimeofday(&begin, &foo);
        
        signal(SIGINT, int_handler);
    
        // write first line of log file
        if (global_args.flag_log) {
            for (k = 0; k < NUM_OF_SENSORS; k++) {
                    fprintf(global_args.log_file_fd, "sensor_%d,\t", (k + 1));
                }
            fprintf(global_args.log_file_fd, "input_1,\tinput_2,\t");
            fprintf(global_args.log_file_fd, "current_1,\tcurrent_2\n");
        }

        for(i=0; i<num_values; i++) {     
            while (1) {
                gettimeofday(&t_act, &foo);
                if (timevaldiff(&begin, &t_act) >= i*deltat*1000) {
                    break;
                }
            }

            // update measurements
            if (commGetMeasurements(&comm_settings_1, global_args.device_id,
                    global_args.measurements)) {
                error_counter++;
            }

            // update currents
            // commGetCurrents(&comm_settings_1, global_args.device_id,
            //     global_args.currents);


            // commGetCurrAndMeas(&comm_settings_1, global_args.device_id,
            //         retrieved_values);

            // global_args.currents[0] = retrieved_values[0];
            // global_args.currents[1] = retrieved_values[1];

            // global_args.measurements[0] = retrieved_values[2];
            // global_args.measurements[1] = retrieved_values[3];
            // global_args.measurements[2] = retrieved_values[4];



            for (k = 0; k < NUM_OF_SENSORS; k++) {
                glob_measurements[k] = global_args.measurements[k]/correction_factor[k];
            }
            
            // update inputs
            global_args.inputs[0] = array[0][i]*correction_factor[0];
            global_args.inputs[1] = array[1][i]*correction_factor[1];
            
            // set new inputs
            commSetInputs(&comm_settings_1, global_args.device_id,
                    global_args.inputs);
        
            // write measurements in log file
            if (global_args.flag_log) {
                for (k = 0; k < NUM_OF_SENSORS; k++) {
                    fprintf(global_args.log_file_fd, "%f,\t", glob_measurements[k]);
                }
                fprintf(global_args.log_file_fd, "%f,\t%f,\t",
                    array[0][i], array[1][i]);
                fprintf(global_args.log_file_fd, "%d,\t%d\n",
                    global_args.currents[0], global_args.currents[1]);
            }
        }

        //get time at the end to verify correct execution
        gettimeofday(&end, &foo);
        

        //free dynamically allocated memory in parser
        free(array[0]);
        free(array[1]);
        free(array);

        //if necessary close log file
        if (global_args.flag_log) {
            fclose(global_args.log_file_fd);
        }

        //at the end, set motors to 0
        usleep(500000);
        global_args.inputs[0] = 0;
        global_args.inputs[1] = 0;
        commSetInputs(&comm_settings_1, global_args.device_id,
                global_args.inputs);

        printf("total time (usec): %d\n", (int)timevaldiff(&begin, &end));
        printf("Error counter %d\n", error_counter);
    }
    
    
    //===========================================================     test force
    if(global_args.flag_test)
    {   
        printf("Flag_test\n");

        #ifdef PHIDGETS_BRIDGE
           test();
        #endif

        getchar();
    }   


    //============================================================     set zeros
    if(global_args.flag_set_zeros)
    {
        struct timeval t_prec, t_act;
        struct timezone foo;

        signal(SIGINT, int_handler_2);
        printf("Press CTRL-C to set Zero Position\n\n");
        printf("Press return to proceed\n");
        getchar();

        // Deactivate device to avoid motor movements
        commActivate(&comm_settings_1, global_args.device_id, 0);

        // Reset all the offsets
        for (i = 0; i < NUM_OF_SENSORS; i++) {
            global_args.measurement_offset[i] = 0;    
        }

        commSetParam(&comm_settings_1, global_args.device_id,
            PARAM_MEASUREMENT_OFFSET, global_args.measurement_offset,
            NUM_OF_SENSORS);


        //Display current values until CTRL-C is pressed
        gettimeofday(&t_prec, &foo);
        gettimeofday(&t_act, &foo);
        while(1) {
            while (1) {
                gettimeofday(&t_act, &foo);
                if (timevaldiff(&t_prec, &t_act) >= 200000) {
                    break;
                }
            }
            commGetMeasurements(&comm_settings_1, global_args.device_id,
                    global_args.measurements);
            for (i = 0; i < NUM_OF_SENSORS; i++) {
                printf("%d\t", global_args.measurements[i]);
            }
            printf("\n");
            
            gettimeofday(&t_prec, &foo);
        }
    }

//==========================     closing serial port and closing the application
    

    closeRS485(&comm_settings_1);
    
    
    if(global_args.flag_verbose)
        puts("Closing the application."); 

#ifdef PHIDGETS_BRIDGE
    CPhidget_close((CPhidgetHandle)bridge);
    CPhidget_delete((CPhidgetHandle)bridge);
#endif
    
    return 0;
}


//==============================================================================
//                                                                   file_parser
//==============================================================================

/** Parse CSV file and return a pointer to a matrix of float dinamically
 *  allocated.  Remember to use free(pointer) in the caller
 */

float** file_parser( char* filename, int* deltat, int* num_values )
{
    FILE* filep;
    float** array = NULL;
    int i;
    filep = fopen(filename, "r");
    if (filep == NULL) perror ("Error opening file");
    else {
        //read first line
        fscanf(filep, "%d,%d", deltat, num_values);

        //alloc memory for the arrays
        array = (float**)malloc(2*sizeof(float*));
        array[0] = (float*)malloc(*num_values*sizeof(float));
        array[1] = (float*)malloc(*num_values*sizeof(float));

        //read num_values line of file and store them in array
        for(i=0; i<*num_values; i++) {
            fscanf(filep, "%f,%f", &array[0][i], &array[1][i]);
        }
    fclose(filep);
    }
    return array;
}

//==============================================================================
//                                                          CTRL-C interruptions
//==============================================================================

/** handle CTRL-C interruption 1
*/
void int_handler(int sig) {
    printf("\nForced quit!!!\n");
    
    //if necessary close log file
    if (global_args.flag_log) {
        fclose(global_args.log_file_fd);

        // erase last line of log file  /////////////BEGIN
        const char *tmpfilename = "tmpfile~~~";
        char line[1000];
        char command[256];
        FILE *thefile = fopen(global_args.log_file, "r");
        FILE *tmpfile = fopen(tmpfilename, "w");

        while (fgets(line, sizeof(line), thefile))
          if (!feof(thefile))
            fputs(line, tmpfile);

        fclose(tmpfile);
        fclose(thefile);

        strcpy(command, "mv ");
        strcat(command, tmpfilename);
        strcat(command, " ");
        strcat(command, global_args.log_file);

        system(command);
        // erase last line of log file  ///////////////END
    }

    // set motors to 0,0
    global_args.inputs[0] = 0;
    global_args.inputs[1] = 0;
    commSetInputs(&comm_settings_1, global_args.device_id, global_args.inputs);


    exit(1);
}

/** handle CTRL-C interruption 2
*/
void int_handler_2(int sig) {
    int i;
    printf("\n\nSetting zero position\n");

    //Set the offsets equal to minus current positions
    for (i = 0; i < NUM_OF_SENSORS; i++) {
        global_args.measurement_offset[i] = -global_args.measurements[i];
    }

    commSetParam(&comm_settings_1, global_args.device_id,
            PARAM_MEASUREMENT_OFFSET, global_args.measurement_offset,
            NUM_OF_SENSORS);

    commStoreParams(&comm_settings_1, global_args.device_id);

    sleep(1);

    // set motors to 0,0
    global_args.inputs[0] = 0;
    global_args.inputs[1] = 0;
    commSetInputs(&comm_settings_1, global_args.device_id, global_args.inputs);

    //Activate board 
    commActivate(&comm_settings_1, global_args.device_id, 1);

    exit(1);
}



//==============================================================================
//                                                                 dysplay usage
//==============================================================================

/** Display program usage, and exit.
*/

void display_usage( void )
{
    puts("================================================================================");
    puts( "qbmove - communicate with your QB Move" );
    puts("================================================================================"); 
    puts( "usage: qbmove [id] [OPTIONS]" );
    puts("--------------------------------------------------------------------------------"); 
    puts("Options:");
    puts("");
    puts(" -s, --set_inputs <value,value>   Send reference inputs to the QB Move.");
    puts(" -e, --set_pos_stiff <pos,stiff>  Set position (degree) and stiffness (\%)");
    puts(" -g, --get_measurements           Get measurements from the QB Move.");
    puts(" -c, --get_currents               Get motor currents");
    puts(" -a, --activate                   Activate the QB Move.");
    puts(" -d, --deactivate                 Deactivate the QB Move.");
    puts(" -z, --set_zeros                  Set zero position for all sensors");
    puts("");
    puts(" -k, --use_gen_sin                Sinusoidal inputs using sin.conf file");
    puts(" -f, --file <filename>            Pass a CSV file as input");
    puts("");
    puts(" -p, --ping                       Get info on the device.");
    puts(" -t, --serial_port                Set up serial port.");
    puts(" -b, --bootloader                 Enter bootloader mode.");
    puts(" -v, --verbose                    Verbose mode.");
    puts(" -h, --help                       Shows this information.");
    puts("");
    
    puts("                                  File is in the form:");
    puts("                                  millisecs,num_rows");
    puts("                                  input1_1,input2_1");
    puts("                                  input1_2,input2_2");
    puts("                                  input1_3,input2_3");
    puts("                                  ...        ...");
    puts("                                  input1_num_rows,input2_num_rows");
    puts(" -l, --log                        Use in combination with -f to");
    puts("                                  save a log of the positions in");
    puts("                                  a file named filename_log");
    puts("--------------------------------------------------------------------------------"); 
    puts("Examples:");
    puts("");
    puts("  qbmove -p                       Get info on whatever device is connected.");
    puts("  qbmove -t                       Set up serial port.");
    puts("  qbmove 65 -s 10,10              Set inputs of device 65 to 10 and 10.");    
    puts("  qbmove 65 -g                    Get measurements from device 65.");
    puts("  qbmove 65 -g -s 10,10           Set inputs of device 65 to 10");
    puts("                                  and 10, and get measurements.");        
    puts("  qbmove 65 -a 1                  Turn device 65 on.");
    puts("  qbmove 65 -a 0                  Turn device 65 off.");
    puts("  qbmove 65 -f filename           Pilot device 65 using file 'filename'");    
    puts("================================================================================");
    /* ... */
    exit( EXIT_FAILURE );
}

/* [] END OF FILE */
