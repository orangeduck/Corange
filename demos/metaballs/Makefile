CC=gcc

INCS= -I ./include -I ../../include

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -g

LFLAGS= ../../corange.res -L./ -L./lib -L../../lib -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32 -lopencl32
LFLAGS_LINUX= -L./lib -lcorange -lGL -lSDLmain -lSDL -lOpenCL

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

# Windows

metaballs.exe: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@
	
corange:
	cp ../../corange.dll ./corange.dll
	
clean:
	rm $(OBJ_FILES)	

# Linux

linux: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS_LINUX) -o metaballs
	

