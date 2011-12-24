CC=gcc

INCS= -I ./include
LIBS= -L ./lib -L ./

CFLAGS= $(INCS) -Wall -Wno-unused
CFLAGS_LINUX= $(INCS) -Wall -Wno-unused -fPIC

LFLAGS= $(LIBS) -lmingw32 -lopengl32 -lSDLmain -lSDL -llua5.1
LFLAGS_LINUX= $(LIBS) -lGL -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)

OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
OBJ_FILES_LINUX= $(addprefix obj/,$(notdir $(C_FILES:.c=.ol)))

# Windows

corange.dll: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -shared -o $@
	cp $@ ./demos/cello/$@
	cp $@ ./demos/metaballs/$@
	cp $@ ./demos/noise/$@
	cp $@ ./demos/sea/$@
	cp $@ ./demos/lut_gen/$@

obj/%.o: src/%.c
	$(CC) $< -c  -O3 -g $(CFLAGS) -o $@
	
windows: corange.dll
	
clean:
	rm $(OBJ_FILES)
	
# Linux
	
linux: $(OBJ_FILES_LINUX)
	$(CC) -g $(OBJ_FILES_LINUX) $(LFLAGS_LINUX) -shared -o libcorange.a
	
obj/%.ol: src/%.c
	$(CC) $< -c  -O3 -g $(CFLAGS_LINUX) -o $@

cleanlinux:
	rm $(OBJ_FILES_LINUX)