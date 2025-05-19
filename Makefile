HEADER = de.h
TARGET = DE_CCEA_exe

OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc)) \ problem.o solutionGreedy.o util.o
CC = g++
OPTION = -Ofast -march=native

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(OPTION) -lm 

%.o: %.cc $(HEADER)
	$(CC) $(CFLAGS) $(OPTION)  -c $<

problem.o: problem.cpp problem.h
    $(CC) $(CFLAGS) $(OPTION) -c problem.cpp

solutionGreedy.o: solutionGreedy.cpp solutionGreedy.h
    $(CC) $(CFLAGS) $(OPTION) -c solutionGreedy.cpp

util.o: util.cpp util.h
    $(CC) $(CFLAGS) $(OPTION) -c util.cpp

clean:
	rm -rf *.o
