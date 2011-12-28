CC=gcc

INCS= -I ./include

CFLAGS= $(INCS) -Wall -Werror -Wno-unused -g
CFLAGS_LINUX= $(INCS) -Wall -Werror -Wno-unused -g -fPIC

LFLAGS= -lmingw32 -lopengl32 -lSDLmain -lSDL -llua5.1 -shared
LFLAGS_LINUX= -lGL -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)

OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
OBJ_FILES_LINUX= $(addprefix obj/,$(notdir $(C_FILES:.c=.ol)))

# Windows

corange.dll: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@
	
windows: corange.dll
	
clean:
	rm $(OBJ_FILES)
	
# Linux
	
linux: $(OBJ_FILES_LINUX)
	$(CC) -g $(OBJ_FILES_LINUX) $(LFLAGS_LINUX) -shared -o libcorange.so
	
obj/%.ol: src/%.c
	$(CC) $< -c $(CFLAGS_LINUX) -o $@

cleanlinux:
	rm $(OBJ_FILES_LINUX)
