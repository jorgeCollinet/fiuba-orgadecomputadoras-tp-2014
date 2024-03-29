/*
 * main.c
 *
 *  Created on: 29/08/2014
 *      Author: jorge y jasmina y ....
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

#define TAM 500
static bool DEBUG = false;
static char mensaje_ayuda[]=""
 "* -s, --number-separator	[requiere argumento] (Indica el texto separador entre numero de lınea y la lınea).\n"
 "* -v, --starting-line-number	[requiere argumento] (Indica el numero de la primer lınea).\n"
 "* -i, --line-increment 		[requiere argumento] (Indica el incremento entre lıneas consecutivas).\n"
 "* -t, --non-empty		[NO requiere argumento] (Si esta presente, solo se deben numerar las lıneas NO vacias. Caso con-trario, tambien deben numerar las lıneas vacias).\n"
 "* -l, --join-blank-lines	[requiere argumento] (Indica la cantidad de lıneas vacias a agrupar en una unica lınea).\n"
 "* -h, --help			[NO requiere argumento] (Imprime el mensaje de ayuda).\n";

// optarg: opcion de argumento ej --add=gg  -> optarg = gg

static struct option long_options[] = {
		/* para cada opcion, se registra si necesita argumento y que letra devuelve getopt_long(), en caso
		 * de encontrarse con esa opcion */
		{ "number-separator", 	required_argument, 		0, 's' },
		{ "starting-line-number", required_argument, 	0, 'v' },
		{ "line-increment", 	required_argument, 		0, 'i' },
		{ "non-empty",			no_argument, 			0, 't' },
		{ "join-blank-lines", 	required_argument, 		0, 'l' },
		{ "help", 				no_argument, 			0, 'h' },
		// esto lo demanda la funcion
		{ 0, 0, 0, 0 }
};

static char short_options[] = "s:v:i:l:th";
static char number_separator[10];
static unsigned long starting_line_number = 0;
static int line_increment = 1;
static long line_number = 0;
static bool non_empty = false;
static unsigned int join_blank_lines = 1;

/*
 * Funcion encargada de la carga de opciones proveniente de la llamada al programa
 */
void init(int argc, char **argv) {
	/* getopt_long stores the option index here. */
	int option_index = 0;
	int c = getopt_long(argc, argv, short_options, long_options, &option_index);
	while (c != -1) {
		//pequeña correccion para que no tenga en cuenta el '=' como valor en el argumento
		// si se pasa el argumento de forma -s="jjoj"
		if(optarg!= NULL && optarg[0]=='=' && optarg[1]!='\0'){
			optarg++;
		}
		switch (c) {
		case 's':
			if(DEBUG)printf("option -s o number-separator con valor: %s\n", optarg);
			strcpy(number_separator, optarg);
			break;

		case 'v':
			if(DEBUG)printf("option -v o --starting-line-number con valor: %s\n",optarg);
			starting_line_number = atol(optarg);
			break;

		case 'i':
			if(DEBUG)printf("option -i o --line-increment con valor: %s\n", optarg);
			line_increment = atoi(optarg);
			break;

		case 't':
			if(DEBUG)printf("option -t o --non-empty\n");
			non_empty = true;
			break;

		case 'l':
			if(DEBUG)printf("option -l o --join-blank-lines con valor: %s\n", optarg);
			join_blank_lines = atoi(optarg);
			break;

		case 'h':
			printf("option -h o --help\n");
			printf("%s", mensaje_ayuda);
			exit(1);
			break;

		default:
			abort();
		}

		c = getopt_long(argc, argv, short_options, long_options, &option_index);
	}
}

//Devuelve true si leyo una linea entera o false en caso contrario.
bool leer_linea(FILE* fd, char* buffer){
	if (fgets(buffer, TAM, fd) == NULL){
		strcpy(buffer,"");
		return true;
	}
	char* ptr = strchr(buffer,'\n');
	if (ptr != NULL){
		return true;
	}
	return false;
}

void imprimir_espacios_acumulados(int* cantidad_espacios){
	int i;
	if (non_empty == false && *cantidad_espacios > 0){
		for(i = 0; i < *cantidad_espacios; i++){		
			printf("%lu%s\n", line_number, number_separator);
			line_number += line_increment;					
		}
		*cantidad_espacios = 0;	
	}
}

void escribir_archivo_en_stdout(FILE* fd) {
	// se hace rewind porque si es cargado desde la stdin, el puntero queda apuntando al final
	rewind(fd);
	char buffer[TAM];
	int cantidad_espacios = 0;
	bool linea_completa = false;	
	while (!feof(fd)){
		linea_completa = leer_linea(fd, buffer);
		if (!linea_completa){
			imprimir_espacios_acumulados(&cantidad_espacios);
			printf("%lu%s%s", line_number, number_separator, buffer);
			line_number += line_increment;
			while (!linea_completa && !feof(fd)){
				linea_completa = leer_linea(fd, buffer);
				printf("%s", buffer);
			}			
		}else{
			if (strcmp (buffer,"") == 0){				
				imprimir_espacios_acumulados(&cantidad_espacios);
				continue;
			}else if(strcmp(buffer, "\n") == 0){
				cantidad_espacios++;
				if (cantidad_espacios == join_blank_lines){
					cantidad_espacios = 1;
					imprimir_espacios_acumulados(&cantidad_espacios);							
				}
			}else{
				imprimir_espacios_acumulados(&cantidad_espacios);
				printf("%lu%s%s", line_number, number_separator, buffer);
				line_number += line_increment;					
			}
		}
	}
}

/* Funcion encargada de procesar la lista de archivos y los - */
void procesar_archivos(int optind, int argc, char* argv[]) {
	line_number = starting_line_number;
	if (optind < argc) {
		int aux = optind;
		if (DEBUG) {
			printf("Elementos que se consideran archivos: \n");
			while (optind < argc) {
				printf("%s\n", argv[optind++]);
			}
			printf("\n");
		}
		optind = aux;
		while (optind < argc) {
			FILE * file = NULL;
			if (argv[optind][0] == '-') {
				file = stdin;
			} else {
				file = fopen(argv[optind], "r");
				if (file == NULL) {
					fprintf(stderr, "Error al tratar de abrir el archivo:'%s' ", argv[optind]);
					perror("");
				}
			}
			if (file != NULL) {
				escribir_archivo_en_stdout(file);
				fclose(file);
			}
			optind++;
		}
		printf("\n");
	} else {
		escribir_archivo_en_stdout(stdin);
	}
}

int main(int argc, char **argv) {
	init(argc, argv);
	if (DEBUG) {
		printf("++++++++++++++++++\nDEBUG IS ON\n");
		printf("number_peratator:'%s'\n", number_separator);
		printf("starting_line_number:'%lu'\n", starting_line_number);
		printf("line_increment:'%d'\n", line_increment);
		printf("non_empty:'%s'\n", (non_empty) ? "true" : "false");
		printf("join_blank_lines:'%d'\n", join_blank_lines);
		printf("++++++++++++++++++\n");
	}
	procesar_archivos(optind,argc,argv);
	exit(0);
}
