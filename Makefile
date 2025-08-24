HEADER = de.h
TARGET = DE_CCEA_exe

OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc)) \
    problem.o solution.o util.o dataset_manager.o bridge.o
CC = g++
OPTION = -Ofast -march=native -D_GLIBCXX_USE_C99_MATH_TR1

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(OPTION) -lm 

%.o: %.cc $(HEADER)
	$(CC) $(CFLAGS) $(OPTION)  -c $<

problem.o: problem.cpp problem.h
	$(CC) $(CFLAGS) $(OPTION) -c problem.cpp

solution.o: solution.cpp solution.h
	$(CC) $(CFLAGS) $(OPTION) -c solution.cpp

util.o: util.cpp util.h
	$(CC) $(CFLAGS) $(OPTION) -c util.cpp

dataset_manager.o: dataset_manager.cpp dataset_manager.h
	$(CC) $(CFLAGS) $(OPTION) -c dataset_manager.cpp

bridge.o: bridge.cpp bridge.h
	$(CC) $(CFLAGS) $(OPTION) -c bridge.cpp

mcfp_validator: mcfp_validator.cpp
	$(CC) -o mcfp_validator mcfp_validator.cpp $(OPTION)

iterative_mcfp_validator: iterative_mcfp_validator.cpp
	$(CC) -o iterative_mcfp_validator iterative_mcfp_validator.cpp $(OPTION)

clean:
	rm -rf *.o
