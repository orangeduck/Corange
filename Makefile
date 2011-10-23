CC=gcc

INCS= -I ./include
LIBS= -L ./lib -L ./

CFLAGS= $(INCS)
LFLAGS= $(LIBS) -lmingw32 -lopengl32 -lopencl32 -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

corange.exe: corange.dll corange.c
	$(CC) -g corange.c $(CFLAGS) $(LFLAGS) -lcorange -o $@
	
corange.dll: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -shared -o $@

obj/%.o: src/%.c
	$(CC) $< -c -g $(CFLAGS) -o $@ 

clean:
	rm $(OBJ_FILES)