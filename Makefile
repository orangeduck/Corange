CC=gcc

INCS= -I ./include

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
CFLAGS_LINUX= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g -fPIC

LFLAGS= -g -L ./lib -lmingw32 -lopengl32 -lSDLmain -lSDL -shared
LFLAGS_LINUX= -lGL -lSDLmain -lSDL

C_FILES= $(wildcard src/*.c)

OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
OBJ_FILES_LINUX= $(addprefix obj/,$(notdir $(C_FILES:.c=.ol)))

# Windows

corange.dll: $(OBJ_FILES) corange.res
	$(CC) $(OBJ_FILES) corange.res $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@
	
windows: corange.dll
	
corange.res: corange.rc
	windres $< -O coff -o $@
	
clean:
	rm $(OBJ_FILES)
	
# Linux
	
linux: $(OBJ_FILES_LINUX)
	$(CC) -g $(OBJ_FILES_LINUX) $(LFLAGS_LINUX) -shared -o libcorange.so
	
obj/%.ol: src/%.c
	$(CC) $< -c $(CFLAGS_LINUX) -o $@

cleanlinux:
	rm $(OBJ_FILES_LINUX)
