CC=gcc

INCS= -I ./include

C_FILES= $(wildcard src/*.c) $(wildcard src/*/*.c)

ifndef OSTYPE
  OSTYPE = $(shell uname -s|awk '{print tolower($$0)}')
endif

ifeq ($(OSTYPE),linux)
	OUT=libcorange.so
	CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g -fPIC
	LFLAGS= -lGL -lSDLmain -lSDL -shared
	OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
else
	OUT=corange.dll
	CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
	LFLAGS= -g -L ./lib -lmingw32 -lopengl32 -lSDLmain -lSDL
	OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o))) corange.res
endif

$(OUT): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@

obj/%.o: src/*/%.c
	$(CC) $< -c $(CFLAGS) -o $@
	
corange.res: corange.rc
	windres $< -O coff -o $@
	
clean:
	rm $(OBJ_FILES)
