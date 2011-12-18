CC=gcc

INCS= -I ./include
LIBS= -L ./lib -L ./

CFLAGS= $(INCS) -Wall -Wno-unused
LFLAGS= $(LIBS) -lmingw32 -lopengl32 -lopencl32 -lSDLmain -lSDL -llua5.1

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

corange.dll: $(OBJ_FILES)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -shared -o $@
	cp corange.dll ./demos/cello/corange.dll
	cp corange.dll ./demos/metaballs/corange.dll
	cp corange.dll ./demos/noise/corange.dll
	cp corange.dll ./demos/sea/corange.dll
	cp corange.dll ./demos/lut_gen/corange.dll

obj/%.o: src/%.c
	$(CC) $< -c  -O3 -g $(CFLAGS) -o $@

clean:
	rm $(OBJ_FILES)