//===============    includes

#include "qbmoveAPI/qbmove_communications.h"

//===============    your code

int main() {
	//Open port
	openRS485(&comm_settings, port);

	//Retrieve measurements
	commGetMeasurements(&comm_settings, device_id, measurements);

	//Retrieve currents
	commGetCurrents(&comm_settings, device_id, currents);

	//Set new position
	commSetInputs(&comm_settings, device_id, inputs);

	return 1;
}
