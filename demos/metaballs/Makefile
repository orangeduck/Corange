CC=gcc

INCS= -I ./include -I ../../include
LIBS= -L ./lib -L ../../lib -L ../../

CFLAGS= $(INCS)
LFLAGS= $(LIBS) -lcorange -lmingw32 -lopengl32 -lopencl32 -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

metaballs.exe: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c -g $(CFLAGS) -o $@ 

clean:
	rm $(OBJ_FILES)