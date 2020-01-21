#
# compiler
#
CC        = g++
#CC       	  = clang++
#CXXFLAGS 		= -Wall -Wextra -std=c++11 -O3 -DNDEBUG
CXXFLAGS 		= -Wall -Wextra -std=c++11 -Wfatal-errors -g -lstdc++fs
#CXXFLAGS                = -Wall -Wextra -std=c++11 -g

#
# source 
#
SRCROOT		= ../../../
SRCINC		= -I$(SRCROOT)/src
#SRCINCOUT	= -L$(SRCROOT)/src

#
# address of FileHandler (for linking)
#
#OBJ		= ../../src/FileHandler.o

.PHONY: factory-generate

TARGET = factory-generate

all: $(TARGET)

$(TARGET):
	$(CC) $(CXXFLAGS) $(SRCINC) $(TARGET).cpp -o $(TARGET)

#nego-test: test OBJ
#	$(CC) $(SRCINC) $(OBJ) test.o -o nego-test 


clean:
	rm  ./$(TARGET)  ./$(TARGET).dSYM
