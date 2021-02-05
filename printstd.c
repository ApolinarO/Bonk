#include "dup2.h"

int main()
{
	// values for reading from stdin and writing
	int toggle = 0; 		// Says where to write to
	char readInput[255];	// Read input from stdin

	// Prints input from stdin to stdout and stderr alternating
	while(fgets(readInput, 255, stdin))
	{
		if(toggle++ % 2 == 0)
			fprintf(stdout, readInput);
		else
			fprintf(stderr, readInput);
	}
	return 0;
}