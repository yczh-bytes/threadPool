CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread
TARGET = threadpool
OBJS = main.o threadpool.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp threadpool.h
	$(CXX) $(CXXFLAGS) -c main.cpp

threadpool.o: threadpool.cpp threadpool.h
	$(CXX) $(CXXFLAGS) -c threadpool.cpp

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: run clean