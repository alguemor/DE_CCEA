HEADER = de.h
TARGET = DE_CCEA_exe

OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc)) \
    problem.o solutionGreedy.o util.o dataset_manager.o bridge.o
CC = g++
OPTION = -Ofast -march=native -D_GLIBCXX_USE_C99_MATH_TR1

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

dataset_manager.o: dataset_manager.cpp dataset_manager.h
	$(CC) $(CFLAGS) $(OPTION) -c dataset_manager.cpp

bridge.o: bridge.cpp bridge.h
	$(CC) $(CFLAGS) $(OPTION) -c bridge.cpp

test_fitness: test_fitness_evaluation.cpp dataset_manager.o bridge.o problem.o solutionGreedy.o util.o
	$(CC) -o test_fitness test_fitness_evaluation.cpp dataset_manager.o bridge.o problem.o solutionGreedy.o util.o $(OPTION) -lm

clean:
	rm -rf *.o
