# compiler
# g++ for C++ code
CC= g++

# options
CFLAGS= -g -O2 `pkg-config --cflags opencv` 
LIBS= -lm `pkg-config --libs opencv`
INCLUDES= 

OBJECTS= main.o user_function.o
TARGET=  demo

# programs
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

# clean
clean:
	rm -f *.[o]
	rm $(TARGET)
	rm *~

# objects
.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $<
