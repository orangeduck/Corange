CC=gcc

INCS= -I ./include -I ../../include
LIBS= -L ./lib -L ../../lib -L ../../

CFLAGS= $(INCS) -Wall -Werror -Wno-unused -g
CFLAGS_LINUX= $(INCS) -Wall -Werror -Wno-unused -g -fPIC

LFLAGS= $(LIBS) -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32 -llua5.1
LFLAGS_LINUX= -L ../../ -lcorange -lGL -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
OBJ_FILES_LINUX= $(addprefix obj/,$(notdir $(C_FILES:.c=.ol)))

# Windows

cello.exe: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@
	
clean:
	rm $(OBJ_FILES)	

# Linux

linux: $(OBJ_FILES_LINUX)
	$(CC) -g $(OBJ_FILES_LINUX) $(LFLAGS_LINUX) -o cello.exe

obj/%.ol: src/%.c
	$(CC) $< -c $(CFLAGS_LINUX) -o $@

cleanlinux:
	rm $(OBJ_FILES_LINUX)
	

