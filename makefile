SOURCES = *.cpp
OBJECTS = $(SOURCES:.cpp=.o)
#OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))

CC = g++

CFLAGS = -g -Wall

PROG=fileTransmit

$(PROG): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES)

depend:
	makedepend -Y $(SOURCES)

clean:
	rm -f *.o 

# DO NOT DELETE
