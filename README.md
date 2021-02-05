## Bonk
This is a sample shell method with forking, redirection, and piping functionalities.

Run the following to compile:

    make

or

    make all

Then use the following to run:

    make run


A bonk shell command may look like so: 

    a hello < in.txt | b | c | f good by arguments > f.txt

Here are some sample commands to look at

    ls | grep txt > out.txt 2> err.txt
    ls | grep txt
    ./printstd < a.txt > out.txt 2> err.txt

Note that the printstd app here alternates stdin input (must be redirected into for this case) to push out to stdout and stderr.
* Run "make all" to create this program