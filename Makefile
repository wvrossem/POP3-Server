TITLE=pop3server
CC=g++
CFLAGS=-c -Wall
LDFLAGS=
LDLIBS= -L/usr/local/lib -ldvnet -ldvthread -ldvutil
SOURCES=command.cpp maildrop.cpp maildrops.cpp manager.cpp message.cpp player.cpp pop3server.cpp
HFILES=command.h maildrop.h maildrops.h manager.h message.h player.h 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=pop3
FILES=$(SOURCES) $(HFILES) Makefile pop3.config pop3.log

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) make.depend

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

dist:	$(FILES)
	mkdir $(TITLE) && cp $(FILES) $(TITLE) && \
	tar cvf $(TITLE).tar $(TITLE) && rm -fr $(TITLE)

