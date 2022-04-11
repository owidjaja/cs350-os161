OBJS	= disk.o fs.o main.o
SOURCE	= main.c
HEADER	=
OUT	= sfssh
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -pthread -lm

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c

disk.o: disk.c
	$(CC) $(FLAGS) disk.c

fs.o: fs.c
	$(CC) $(FLAGS) fs.c

clean:
	rm -f $(OBJS) $(OUT)
