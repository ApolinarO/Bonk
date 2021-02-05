// Apolinar Ortega
#include "dup2.h"

// Redirects stdout, stdin, and stderr (if specified)
void redirect(char * r_in, char * r_out, char * r_err)
{
	// fgets and scanf w/ stdin as argument to read input
	// outFile and errFile parameters for open
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int wFlags = O_WRONLY | O_CREAT | O_TRUNC;

	// Gets file descriptors; if name not given, then gives it '-1'
	int inFile = (r_in)?open(r_in, O_RDONLY): -1;
	int outFile = (r_out)?open(r_out, wFlags, mode): -1;
	int errFile = (r_err)?open(r_err, wFlags, mode): -1;
	
	// If a file has not been successfully opened, ends the redirection
	if(r_in && inFile < 0)
	{
		perror("\n Error opening inFile");
		errno = 0;
		return;
	}
	else if(r_out && outFile < 0)
	{
		perror("\n Error opening outFile");
		errno = 0;
		return;
	}
	else if(r_err && errFile < 0)
	{
		perror("\n Error opening errFile");
		errno = 0;
		return;
	}
	
	// Error if dup2 redirection failed
	if (r_in && dup2(inFile, STDIN_FILENO) < 0)
	{
		perror("\n Error redirecting stdin");
		errno = 0;
		return;	
	}
	if (r_out && dup2(outFile, STDOUT_FILENO) < 0)
	{
		perror("\n Error redirecting stdout");
		errno = 0;
		return;	
	}
	if (r_err && dup2(errFile, STDERR_FILENO) < 0)
	{
		perror("\n Error redirecting stderr");
		errno = 0;
		return;	
	}
}