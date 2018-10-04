#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"
/* AUTHORS Carlos Bilbao 😎 */

extern char *use;

static const int MEM_TEMP = 5000;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 * nBytes as INT_MAX to copy all the file
 */
int copynFile(FILE* origin, FILE* destination, int nBytes, unsigned short* crc){

	int e, ret, sum = 0;
	unsigned short sum1 = 0, sum2 = 0;

	if(origin == NULL || destination == NULL || nBytes < 0) return -1;

	while((e = getc(origin)) != EOF && sum != nBytes){
		if((ret = putc((char) e, destination)) == EOF) return -1;
		sum++; //sizeof(char);
		sum1 = (sum1 + (unsigned short) e) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	*crc = (sum2 << 8) | sum1;

	fseek(origin, -sizeof(char), SEEK_CUR);

	return sum;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */
char* loadstr(FILE * file){

	int elems = 0;
	char* chars = malloc(MEM_TEMP);

	if(file == NULL) {
		free(chars);
		return NULL;
	}

	while((chars[elems] = getc(file)) != EOF && chars[elems] != '\0') ++elems;

	++elems;

	chars = realloc(chars, elems);

 	return chars;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor
 * nFiles: output parameter. Used to return the number
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry* readHeader(FILE* tarFile, int *nFiles){

	stHeaderEntry* array = NULL;
	int error = 0, nfiles = 0;
	//Read the number of files (N) from tarFile and store it in n_files

	fread(&nfiles, sizeof(int), 1, tarFile);

	if(nfiles == 0) return NULL;

	//Read the (pathname,size) pairs of the tarFile and stores it in the array

	array = malloc(MEM_TEMP);

	for(int i = 0; i < nfiles; ++i){

		if((array[i].name = loadstr(tarFile)) == NULL) error = 1;

		fread(&array[i].size, sizeof(int), 1, tarFile);

		if(error == 1){
			free(array);
			return NULL;
		}

		fread(&array[i].crc, sizeof(unsigned short), 1, tarFile);
	}


	*nFiles = nfiles;
	return array;
}

/** Creates a tarball archive
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE (macros stdlib.h).
 *
 */
int createTar(int nFiles, char *fileNames[], char tarName[]) {

	int filesSizeName = 0;
	for(int i = 0; i < nFiles; ++i) filesSizeName += strlen(fileNames[i]) + 1;
	/*MEMORY FOR: N + sizes of files + chars of the name + null chars + crc codes*/
	int headerSize = sizeof(int)  + nFiles * sizeof(int) + filesSizeName + nFiles * sizeof(unsigned short);

	/*INICIALIZO LA ESTRUCTURA */

	stHeaderEntry *headers= malloc(2 * nFiles * sizeof(int) + filesSizeName + sizeof(unsigned short));
	for(int i = 0; i < nFiles; ++i){
		 headers[i].name = malloc(strlen(fileNames[i]) + 1);
		 strcpy(headers[i].name, fileNames[i]);
	}

	FILE *file, *aux;
	if ((file = fopen(tarName, "w")) == NULL){
		 free(headers);
		 return EXIT_FAILURE;
	}

	/*AÑADO LA DATA SECTION*/

	fseek(file,headerSize, SEEK_SET);
	for(int i = 0; i < nFiles; ++i) {
		if((aux = fopen(fileNames[i], "r")) == NULL){
			free(headers);
			fclose(file);
			return EXIT_FAILURE;
		}

		if((headers[i].size = copynFile(aux,file, INT_MAX, &headers[i].crc)) == -1){
			free(headers);
			fclose(aux);
			fclose(file);
			return EXIT_FAILURE;
		}

		printf("[%i]: Adding file %s, size %i bytes, %#06x \n", i, headers[i].name, headers[i].size, headers[i].crc );
		fclose(aux);
	}

	/*AÑADO LA HEADER SECTION */

	fseek(file, 0, SEEK_SET);
	fwrite(&nFiles,sizeof(int),1,file);

	for(int i = 0; i < nFiles; ++i){
		fwrite(headers[i].name,strlen(headers[i].name) + 1 ,1,file);
		fwrite(&headers[i].size, sizeof(int), 1, file);
		fwrite(&headers[i].crc, sizeof(unsigned short), 1, file);
	}

	fclose(file);
	fclose(aux);
	free(headers);

	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE (macros stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the
 * tarball's data section. By using information from the
 * header --number of files and (file name, file size) pairs--, extract files
 * stored in the data section of the tarball.
 *
 */
int extractTar(char tarName[]){

	int nfiles = 0;
	unsigned short check;
	FILE *file, *aux;
	if((file = fopen(tarName,"r")) == NULL) return EXIT_FAILURE;

	/*REVISO EL HEADER*/

	stHeaderEntry* headers = NULL;
        if((headers = readHeader(file, &nfiles)) == NULL) return EXIT_FAILURE;

	/*CREO LOS ARCHIVOS*/

	for(int i = 0; i < nfiles; ++i){
		if((aux = fopen(headers[i].name, "w")) == NULL) return EXIT_FAILURE;
		if(copynFile(file,aux, headers[i].size,&check) == -1) return EXIT_FAILURE;
		printf("[%i]: Creating file %s, size %i Bytes, CRC %#06x ...\n", i, headers[i].name,headers[i].size, headers[i].crc);
		if(check != headers[i].crc){
			printf("The crc comparition is not working \n");
			return EXIT_FAILURE;
		}
		else {
			printf("[%i]: CRC of extracted file %#06x. File is OK \n", i, check);
		}
		fclose(aux);
	}

	fclose(file);
	free(headers);

	return EXIT_SUCCESS;
}
