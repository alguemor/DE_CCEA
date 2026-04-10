HEADER = problem.h, solutionGreedy.h, util.h
TARGET = CCEA_exe

OBJS = main.o problem.o solutionGreedy.o util.o
CC = g++
CFLAGS = -Ofast -march=native

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS) -lm

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)
