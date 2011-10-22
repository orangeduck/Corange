CC=gcc

INCS= -I ./include -I ../../include
LIBS= -L ./lib -L ../../lib -L ../../

CFLAGS= $(INCS)
LFLAGS= $(LIBS) -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32 -llua5.1

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

sea.dll: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -shared -o $@

obj/%.o: src/%.c
	$(CC) $< -c -g $(CFLAGS) -o $@ 

clean:
	rm $(OBJ_FILES)