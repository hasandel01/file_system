# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Targets
TARGETS = makeFileSystem fileSystemOper
OBJS_COMMON = filesystem.o utility.o

# Rules
all: $(TARGETS)

makeFileSystem: main.o $(OBJS_COMMON)
	$(CXX) $(CXXFLAGS) -o makeFileSystem main.o $(OBJS_COMMON)

fileSystemOper: filesystemoperations.o $(OBJS_COMMON)
	$(CXX) $(CXXFLAGS) -o fileSystemOper filesystemoperations.o $(OBJS_COMMON)

filesystem.o: filesystem.cpp filesystem.h directoryentry.h utility.h
	$(CXX) $(CXXFLAGS) -c filesystem.cpp

utility.o: utility.cpp utility.h
	$(CXX) $(CXXFLAGS) -c utility.cpp

main.o: main.cpp filesystem.h directoryentry.h utility.h
	$(CXX) $(CXXFLAGS) -c main.cpp

filesystemoperations.o: filesystemoperations.cpp filesystem.h directoryentry.h utility.h
	$(CXX) $(CXXFLAGS) -c filesystemoperations.cpp

clean:
	rm -f $(TARGETS) *.o
