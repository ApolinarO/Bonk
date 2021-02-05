FILENAME= bonk.c makeargv.c redirect.c
STDIOE= redirect.c out.txt err.txt
RUNNAME= bonk
RNAME = printstd.c
RPROG = printstd
CFLAGS= -Wall

main: ${FILENAME}
	gcc ${FILENAME} -c 
	gcc ${FILENAME} ${CFLAGS} -o ${RUNNAME}

all: ${FILENAME} ${RNAME}
	gcc ${FILENAME} -c 
	gcc ${FILENAME} ${CFLAGS} -o ${RUNNAME}

	gcc ${RNAME} -c 
	gcc ${RNAME} ${CFLAGS} -o ${RPROG}

clean:
	rm -rf *.o *.out *.txt ${RUNNAME} ${RPROG}

run: 
	./${RUNNAME}

runs:
	./${RUNNAME} ${STDIOE}

look:
	cat out.txt
	echo ''
	cat err.txt