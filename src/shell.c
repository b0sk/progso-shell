#include <getopt.h>
#include <stdio.h>		// for printf, rename ...
#include <stdlib.h>		// for exit ...
#include <string.h>		// for strtok, strcmp ...
#include <sys/wait.h>	// for wait, waitpid, WEXITSTATUS ...
#include <unistd.h>		// for chdir, (fork, exec) ...

#include "utils.h"
#include "sh.h"


// The name of this program.
const char* program_name;


/*
 * Parse the command line arguments given by the user
 * and save the result in the global variables.
 */
void parse_args(int argc, char *argv[]){
	// A string listing valid short options
	const char* const short_options = "p:l:f:";
	// An array describing the valid long options
	const struct option long_options[] = {
		{ "prompt",		required_argument, NULL, 'p' },
		{ "loglevel",	required_argument, NULL, 'l' },
		{ "logfile",	required_argument, NULL, 'f' },
		{ 0, 0, 0, 0 } //Required
	};
	
	int next_opt;
	while(1){
		/* getopt_long stores the option index here. */
		int next_opt = getopt_long (argc, argv, short_options, long_options, NULL);
		/* Detect the end of the options. */
		if (next_opt == -1)
			break;
		switch(next_opt){
			case 0:
				break;
			case 'p':	// -p o --prompt
				/* Set the prompt from argument */ 
				prompt = optarg;
				break;
			case 'l':	// -l o --loglevel				
				// if arg is low set loglevel to 0
				if(strcmp(optarg, "low") == 0) 
					loglevel = 0;
				// if arg is middle set loglevel to 1
				else if(strcmp(optarg, "middle") == 0)
					loglevel = 1;
				// if arg is middle set loglevel to 2
				else if(strcmp(optarg, "high") == 0)
					loglevel = 2;
				// else print usage and exit with error
				else{
					printf("Invalid argument for option --loglevel\n");
					sh_print_usage(EXIT_FAILURE);
				}
				break;
			case 'f':	// -f o --logfile
				logfile = optarg;
				break;
			case '?':	// Opzione non valida.
				/* getopt_long prints an error message */
				sh_print_usage(EXIT_FAILURE); // Print usage and exit with error
				break;
			default:	// Opzione non riconosciuta.
				printf("Invalid option.\n");
				sh_print_usage(EXIT_FAILURE); // Print usage and exit with error
		}
	}
	
	/* Print any remaining command line arguments (not options). */
	if (optind < argc){
		printf ("non-option ARGV-elements: ");
		while (optind < argc)
			printf ("%s ", argv[optind++]);
		putchar ('\n');
		
		sh_print_usage(-1); // Print usage and return error
    }

}


int main (int argc, char* argv[]){
	/* Set the program name from argv[0]; */
	program_name = argv[0];

	/* Set default log lgevel to MIDLLE (1) */
	loglevel = 1;
	/* Set default prompt to "->" */
	prompt = "->";
	/* Set default lodfile name to "shell.log" */
	logfile = "shell.log";
	/* Set the default logging status to ON (1) */
	log_status = 1;

	/* Parse command line arguments */
	parse_args(argc, argv);

	/* Open the logfile in reading and append mode, 
	 * if there is an error in opening the file turns logging off 
	 */
	if(sh_open_logfile() != 0){
		log_status = 0;
	}

	/* Prints welcome message and status */
	printf("Interactive shell started.\n");
	printf("Prompt set to [%s]\n", prompt);
	sh_cmd_showfile();
	sh_cmd_showlevel();

    /* Main shell loop */
    while(!feof(stdin)) {
    	/* Contains the line from input */
    	char *line;
    	int exit_status; 

    	/* Print eh shell prompt */
    	sh_print_prompt(prompt);
    	
    	/* Read a line from input */
    	line = sh_read_line();

		/* Remove initial empty chars */
		char *ln = line;
    	remove_leading_spaces(&ln);

    	/* Parse the command */
    	command c = sh_parse_command(ln);

    	/* Launch the command and save the exit status */    	
    	exit_status = sh_launch(c);

    	/* If logging is active log the command */
    	if(log_status == 1){
    		sh_log_command(c, exit_status);
    	}

    	free(line);
    }

    sh_close_logfile();

	return 0;
}
