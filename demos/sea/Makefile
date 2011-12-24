CC=gcc

INCS= -I ./include -I ../../include
LIBS= -L ./lib -L ../../lib -L ../../

CFLAGS= $(INCS) -Wall -Wno-unused -fPIC
LFLAGS= $(LIBS) -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32 -llua5.1
LFLAGS_LINUX= $(LIBS) -lcorange -lGL -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

sea.exe: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c -g $(CFLAGS) -o $@ 

clean:
	rm $(OBJ_FILES)
	
linux: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS_LINUX) -o sea.exe
