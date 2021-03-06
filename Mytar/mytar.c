#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "mytar.h"

char use[]="Usage: ./mytar -c|x -f file_mytar [file1 file2 ...]\n";

int main(int argc, char *argv[]) {

 	int opt, nExtra, retCode = EXIT_SUCCESS;
 	flags flag=NONE;
  	char *tarName=NULL;

  	//Minimum args required = 3: mytar -xf file.mtar
  	if(argc < 2){
    		fprintf(stderr,"%s",use);
    		exit(EXIT_FAILURE);
  	}

	//Parse command-line options (can contain -cf or -xf)
  	while((opt = getopt(argc, argv, "cxf:")) != -1) {
    		switch(opt) {
      			case 'c':
        		flag = (flag == NONE) ? CREATE:ERROR;
        		break;
      			case 'x':
        		flag = (flag == NONE) ? EXTRACT:ERROR;
        		break;
      			case 'f':
        		tarName = optarg;
        		break;
      			default:
        		flag=ERROR;
    		}

		//Was an invalid option detected?
    		if(flag == ERROR){
      			fprintf(stderr,"%s",use);
      			exit(EXIT_FAILURE);
    		}
  	}

  	//Valid flag + arg + file[s]
  	if(flag==NONE || tarName==NULL) {
    		fprintf(stderr,"%s",use);
    		exit(EXIT_FAILURE);
  	}

 	//#extra args - Optind es un indice de getopt, empieza en 1
  	nExtra = argc - optind;

  	//Execute the required action
  	switch(flag) {
    		case CREATE:
	      		retCode = createTar(nExtra, &argv[optind], tarName);
 			if(retCode != EXIT_FAILURE)
				printf("Mtar file created successfuly\n");
		break;
    		case EXTRACT: // No hay mas args - archivos.
      		if(nExtra != 0){
        		fprintf(stderr,"%s",use);
        		exit(EXIT_FAILURE);
      		}
      		retCode = extractTar(tarName);
 	  	break;
    		default:
      		retCode = EXIT_FAILURE;
  	}

	exit(retCode);
}
