/* 1. Include appropriate files. */
#include <stdio.h>
#include <stdlib.h>
#include <cadef.h> /* EPICS specific */
#include <../../extensions/src/ezca/ezca.h>
#include <unistd.h>  // for sleep()

int main(int argc, char **argv) {

    // allocate memory for the values
    double* d = (double*)malloc(argc*sizeof(double));

    int i;

    // EPICS: this has to be done at the start of the program
    ezcaAutoErrorMessageOff();

    while(1) {

        // EPICS: read all PVs at once.
        //        by doing this as a "group" it is fast
        ezcaStartGroup();

            // read all PVs we want
            for( i=1;i<argc;++i)
                ezcaGet(argv[i], ezcaDouble, 1, &d[i]);

        // end the block read and check for errors
        if (ezcaEndGroup())
            ezcaPerror(NULL);

        // print the result
        for( i=1;i<argc;++i)
            printf("%s\t%f\n",argv[i],d[i]);

        sleep(1); // wait 1 sec

    }

    // free memory again
    free((void*)d);

} /* end main() */
