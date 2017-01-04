TARGET = pagerank
compiler =$(shell echo $(CXX))
CXXFLAGS =-Wall -Wextra -O3
CXXFLAGS_DEBUG =-Wall -Wextra -O3
LDFLAGS=-lpthread

debug_flags = "-DDEBUG"

all: $(TARGET)_pthreads.exe $(TARGET)_serial.exe;
$(TARGET)_pthreads.exe: pagerank_threads.cpp utils.o
	$(compiler) pagerank_threads.cpp utils.o $(CXXFLAGS) $(LDFLAGS) -o $(TARGET)_pthreads.exe
$(TARGET)_serial.exe: pagerank.cpp utils.o
	$(compiler) pagerank.cpp utils.o $(CXXFLAGS) $(LDFLAGS) -o $(TARGET)_serial.exe

debug_mode_serial: pagerank.cpp utils.o
	$(compiler) pagerank.cpp utils.o $(CXXFLAGS_DEBUG) $(LDFLAGS) $(debug_flags) -o $(TARGET)_serial.exe
debug_mode_pthreads: pagerank_threads.cpp utils.o
	$(compiler) pagerank_threads.cpp utils.o $(CXXFLAGS_DEBUG) $(LDFLAGS) $(debug_flags) -o $(TARGET)_pthreads.exe

utils.o: utils.h utils.cpp
	$(compiler) -c utils.cpp $(CXXFLAGS) $(LDFLAGS)

run_serial: $(TARGET).exe
	./$(TARGET)_serial.exe
run_parallel: $(TARGET).exe
	./$(TARGET)_pthreads.exe
clean:
	rm -f *~ *.o *.out *.exe
compress:
	tar vfcz $(TARGET).tar.gz *.cpp *.h Makefile
