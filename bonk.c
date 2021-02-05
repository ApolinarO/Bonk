// Apolinar Ortega
// cat file | sort | uniq

// To detect errno
#include "dup2.h"
#include "makeargv.h"

void program();
void getRedirect(char*, char**, int*, char*, char*, char*);
int getRedirectValue(char*);
void runProgram(char*);
void getPipeCommands(char*, char**, int*);
void runPipe(char*);
void runInnerPipe(char**, int, int[], int[]);

// Sample programs; for testing
void samplePipe();
void samplePipeM();
void sampleFork(char**, int, char*, char*, char*);

int main (int argc, char * argv[])
{
	program();
	//samplePipeM();
	//runProgram("sort file");
	//printf("Done");
	return 0;
}

// Runs a loop that gets values for redirection and then runs fork
void program()
{
	char input[255];		// Direct User Input

	// As long as input is not EOF or "logout", continue shell
	printf(">");
	while(fgets(input, 255, stdin) && strcmp(input, "logout\n"))
	{
		//runProgram(input);
		runPipe(input);

		printf(">");
	}
}

// Splits up the commands by pipes, pipes them together, and begins the right-most pipe
void runPipe(char* input)
{
	char ** inputs;		// User Input Separated By Pipes into commands
	int * inputc;			// Amount of commands in user input
	int fdl[2], fdr[2];		// Right pipe and left pipe
	pid_t pid;
	int status;

	// sets command variables
	inputc = malloc(sizeof(int));
	inputs = malloc(255 * sizeof(char*));
	*inputc = 0;

	// Gets pipe commands
	getPipeCommands(input, inputs, inputc);

	/*int i;
	printf("\t%i tokens\n", *inputc);
	for(i = 0; i < *inputc; i++)
		printf(" %s", inputs[i]);
	printf("\n");*/

	pid = fork(); // Forks

	// Child
		// Starts the pipe and goes down the pipe
	if(pid == 0)
	{
		// Pipes further only when there's something to pipe
		if(*inputc > 1)
		{
			//printf("_%i: %s\n", (*inputc - 1), inputs[*inputc - 1]);

			// Pipes, then 
			pipe(fdl);
			pid = fork(); // Forks again, but to separate pipes

			// Child: runs 2nd rightmost command and pipes further down
			if(pid == 0)
			{
				runInnerPipe(inputs, *inputc - 1, fdl, fdr);
			}

			// Parent: waits for child to finish then runs the right-most command
			else if(pid > 0)
			{
				waitpid(pid, &status, 0); // Waits for Child

				close(fdl[1]); // Not piping  output
				fdl[0] = dup2(fdl[0], STDIN_FILENO); // Piping input

				runProgram(inputs[*inputc - 1]); // Runs user program
			}
		}
		else // If only 1 input, runs the command
			runProgram(inputs[0]);
	}

	// Parent: Waits for pipes to finish running
	else if(pid > 0)
		waitpid(pid, &status, 0);

	// Error forking
	else
	{
		printf("Error Forking, %s \n", strerror(errno));
		errno = 0;
	}

	free(inputs);
}

// Runs an inner pipes and ends at the left-most
void runInnerPipe(char** argv, int argc, int fdl[], int fdr[])
{
	pid_t pid;
	int status;

	// Test
	/*close(fdr[0]); // Not reading from pipe
	fdr[1] = dup2(fdr[1], STDOUT_FILENO); // pipes stdout

	argc--;
	printf("%i: %s\n", argc, argv[argc]);
	runProgram(argv[argc]);*/

	// Left-Most Command: pipes output and runs
	if(argc == 1)
	{
		argc--;
		//printf("%i: %s\n", argc,  argv[argc]);

		// Left-Most only pipes output
		fdr[0] = fdl[0];
		fdr[1] = fdl[1]; 
		close(fdr[0]);
		fdr[1] = dup2(fdr[1],STDOUT_FILENO);


		runProgram(argv[argc]);
	}

	// Middle Command: forks, runInnerPipe, pipes output & input, and runs
	else
	{
		argc--;
		//printf("%i: %s\n", argc, argv[argc]);

		// Pipes output to the right; no read from left
		fdr[1] = fdl[1]; // Makes M's right = RM's left
		fdr[1] = dup2(fdr[1], STDOUT_FILENO);
		fdr[0] = fdl[0];
		close(fdr[0]);

		pipe(fdl);

		pid = fork(); // Forks

		// Child: calls runInnerPipe on command to the left and pipes IO
		if(pid == 0)
		{
			runInnerPipe(argv, argc, fdl, fdr);
		}

		// Parent: waits for child, pipes IO, and runs
		else if(pid > 0)
		{
			waitpid(pid, &status, 0);

			// Does not write to left; pipes input
			close(fdl[1]);
			fdl[0] = dup2(fdl[0], STDIN_FILENO);

			runProgram(argv[argc]);
		}

		// Error forking
		else
		{
			printf("Error Forking, %s \n", strerror(errno));
			errno = 0;
		}
	}
}

// Breaks Pipes into a series of commands
void getPipeCommands(char * input, char ** inputs, int * inputc)
{
	// Gets input tokens from input
	char ** inputTokens;
	int tokenCount = makeargv(input, "|", &inputTokens);

	*inputc = tokenCount; // Sets amount of arguments

	// Stores tokens into inputs
	int i;	
	for(i = 0; i < tokenCount; i++)
		inputs[i] = inputTokens[i];

	free (inputTokens);
}

// Runs program from input (note that it changes to that program)
void runProgram(char* input)
{
	char** prog; int* progc;		// Arguments and Argument Count
	char in[255], out[255], err[255];	// Redirection Values
	
	// Sets defaults for argv and argc
	progc = malloc(sizeof(int));
	prog = malloc(255 * sizeof(char*));
	*progc = 0;

	// Gets values for redirections and then calls sampleFork()	
	getRedirect(input, prog, progc, in, out, err);

	/*int i;
	for(i = 0; i < *progc; i++)
		printf("%s ", prog[i]);
	printf("\n%i\n", *progc);*/
		
	/*sampleFork(prog, *progc,
		strcmp(in, "")?in:NULL, 
		strcmp(out, "")?out:NULL, 
		strcmp(err, "")?err:NULL);*/

	// Redirects and executes
	redirect(
		strcmp(in, "")?in:NULL, 
		strcmp(out, "")?out:NULL, 
		strcmp(err, "")?err:NULL);
	if(execvp(prog[0], prog) != 0)
	{
		fprintf(stderr, "Error execlp, %s\n", strerror(errno));
		errno = 0;
		exit(-1);
	}


	free(prog);
	free(progc);
}

// Returns what kind of redirection was detected and program arguments
void getRedirect(char* input, char** prog, int* progc, char* r_in, char* r_out, char* r_err)
{
	// Gets input tokens from input
	char ** inputTokens;
	int tokenCount = makeargv(input, "\t \n", &inputTokens);
	int token = 0; // Stores last non-redirect token

	// Sets output values to default values ("")
	r_in[0] = 0;
	r_out[0] = 0;
	r_err[0] = 0;

	// Goes through each token to redirect 
	int i;
	for(i = 0; i < tokenCount - 1; i++)
		switch(getRedirectValue(inputTokens[i]))
		{
			case 0: // If <, redirects stdin
				strcpy(r_in, inputTokens[++i]);
				token = (!token)?i-1:token; // Notes 1st token
				break;
			case 1: // If >, redirects stdout
				strcpy(r_out, inputTokens[++i]);
				token = (!token)?i-1:token; // Notes 1st token
				break;
			case 2: // If 2>, redirects stderr
				strcpy(r_err, inputTokens[++i]);
				token = (!token)?i-1:token; // Notes 1st token
				break;
			default: break; // If nothing, do nothing
		}
	
	token = (!token)?tokenCount:token; // If not set, sets to last token
	*progc = token;
	
	// Gets program name and arguments to execute
	//printf("Token Count: %i\n\tTokens: ", token);
	for(i = 0; i < token; i++)
		prog[i] = inputTokens[i];
		//printf("%s ", inputTokens[i]);
	prog[i] = NULL;

	/*printf("< %s\n", r_in);
	printf("> %s\n", r_out);
	printf("2> %s\n", r_err);*/

	free (inputTokens);
}

// Gets what the redirection is
int getRedirectValue(char* arg)
{
	// If not a redirection, returns -1
	if(strcmp(arg, "0<") == 0 || strcmp(arg, "<") == 0)
		return 0;
	else if(strcmp(arg, "1>") == 0 || strcmp(arg, ">") == 0)
		return 1;
	else if(strcmp(arg, "2>") == 0)
		return 2;
	else
		return -1;
}

void samplePipeM()
{
	pid_t pid;
	int status;
	int fdl[2], fdr[2]; // Pipe file descriptors

	fprintf(stdout, "Top Level\n");

	pid = fork();	// Shell is a process that will remain alive

	// 1st child called "RM" for rightmost
	if(pid == 0)
	{
		fprintf(stdout, "R\n");
		pipe(fdl);
		pid = fork();

		// RM now is parent
			// Child M for Middle
			// M and RM share pipes
		if(pid > 0) // RM
		{
			waitpid(pid, &status,0);
			
			close(fdl[1]); // RM does not write left
			fdl[0] = dup2(fdl[0], STDIN_FILENO); // Pipes

			fprintf(stdout, "R Ready to exec\n");
			execlp("uniq","uniq", NULL);

			fprintf(stdout, "R failed\n");
		}

		// Now in M
		else if(pid==0)
		{
			fprintf(stdout, "M\n");

			fdr[1] = fdl[1]; // Makes M's right = RM's left
			fdr[1] = dup2(fdr[1], STDOUT_FILENO);

			// Piping out M's stdout
			fdr[0] = fdl[0];
			close(fdr[0]);
			pipe(fdl);

			pid = fork();

			if(pid > 0) // M
			{
				waitpid(pid, &status,0);

				close(fdl[1]); // No write to ledt, but read pipe as stdin
				fdl[0] = dup2(fdl[0], STDIN_FILENO);

				fprintf(stdout, "M Ready to exec\n");
				execlp("sort","sort",NULL);
			}

			// Now in LM
				// LM has fdl and fdr
			else if(pid==0)
			{
				fprintf(stdout, "L\n");

		     	fdr[0] = fdl[0];
		     	fdr[1] = fdl[1]; 
		     	close(fdr[0]);
		     	fdr[1] = dup2(fdr[1],STDOUT_FILENO);
		     	// LM does not change stdin 
				fprintf(stdout, "L Ready to exec\n");
		     	execlp("cat","cat", "file",NULL);	
			}// LM
		}// M
	}// RM
	waitpid(pid, &status,0);
	printf("Back at top level\n");
}

void samplePipe()
{
	// Simulates "ls | sort"
	int fd[2];
	pid_t pid;

	pipe(fd);
	pid = fork();

	// Child
	if(pid == 0)
	{
		close(fd[0]); // Not reading from pipe

		// Writes before being piped
		fprintf(stdout, ".Csome charactersC.\n");

		// Stdout now writes to pipe
		fd[1] = dup2(fd[1], STDOUT_FILENO);
		fprintf(stdout, "a.Csome character2C.\n");
		execlp("ls", "ls", NULL);

		// Should not be reached
		fprintf(stdout, ".Cls failedC.\n");
	}

	// Parent
	else if (pid > 0)
	{
		close(fd[1]); // Not writing to pipe

		// Gets stdin from pipe
		fd[0] = dup2(fd[0], STDIN_FILENO);
		fprintf(stdout, ".Pin parentP.\n");

		execlp("sort", "sort", NULL);
	}

}

void sampleFork(char ** prog, int argc, char * r_in, char * r_out, char * r_err)
{
	pid_t pid = fork();

	// What to do after forking
	if(pid == 0)// If in child, run program "redirect"
	{
		//int i = execlp("./redirect", "redirect", argv[1], argv[2], argv[3], NULL);
		redirect(r_in, r_out, r_err); // Redirects std IOE
		//int i = execlp(prog[0], prog[0], NULL); // Executes a new program
		int i = execvp(prog[0], prog); // Executes a new program
		
		// If error from execlp(), then prints it
		if(i != 0)
		{
			printf("Error execlp, %s\n", strerror(errno));
			errno = 0;
		}
		//printf("Hello from child!\n");
	}
	else if(pid > 0) // In parent, wait for child and then exit
	{
		errno = 0;
		int retStat = 0;
		pid_t cpid = wait(&retStat); // Waits for child

		// If error from wait(), then prints it
		if(cpid < 0)
		{
			printf("Error waiting, %s\n", strerror(errno));
			errno = 0;
		}
		//else
		//	printf("Exited child %i; exit status: %i\n", cpid, retStat);
		
		//exit(0); // Exits program
		return;
	}
	else // If error from fork(), prints it
	{
		printf("Error Forking, %s \n", strerror(errno));
		errno = 0;
	}

	// Should not be reached
	printf("Fin %i\n", pid);
}