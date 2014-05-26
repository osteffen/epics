/* 1. Include appropriate files. */
#include <stdio.h>
#include <stdlib.h>
#include <cadef.h> /* EPICS specific */
#include <../../extensions/src/ezca/ezca.h>
#include <unistd.h>  // for sleep()

int main(int argc, char **argv) {

    // allocate memory for the values
	double faradaycup = 0;
	double ionchamber = 0;	// P2
	double livetime = 0;

    // EPICS: this has to be done at the start of the program
    ezcaAutoErrorMessageOff();

    while(1) {

        // EPICS: read all PVs at once.
        //        by doing this as a "group" it is fast
        ezcaStartGroup();

        	ezcaGet("BEAM:FaradayCup", ezcaDouble, 1, &faradaycup);
        	ezcaGet("BEAM:IonChamber", ezcaDouble, 1, &ionchamber);
        	ezcaGet("TRIG:TotalLivetime", ezcaDouble, 1, &livetime);

        // end the block read and check for errors
        if (ezcaEndGroup())
            ezcaPerror(NULL);

        // print the result
        printf("Faraday Cup    = %f\n",faradaycup);
        printf("IonChamber(P2) = %f\n",ionchamber);
        printf("Total Livetime = %f\n",livetime);

        sleep(1); // wait 1 sec

    }

} /* end main() */
