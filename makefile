LIBS = -lglut -lGLU -lGL -lm
CC = gcc
OBJECTS = parent

all: $(OBJECTS)

%: %.c
	$(CC) $< -o $@ $(LIBS)

%.o: %.c
	$(CC) $< -c -o $@ $(LIBS)

run: parent
	./parent

clean:
	rm -f *.o $(OBJECTS)
