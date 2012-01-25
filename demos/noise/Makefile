CC=gcc

INCS= -I ./include -I ../../include

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -g

LFLAGS= -L./ -L../../lib -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32
LFLAGS_LINUX= -L./lib -lcorange -lGL -lSDLmain -lSDL

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

# Windows

noise.exe: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@
	
clean:
	rm $(OBJ_FILES)	

# Linux

linux: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS_LINUX) -o noise
	

