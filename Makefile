CC=gcc
AR=ar

INCS= -I ./include

C_FILES= $(wildcard src/*.c) $(wildcard src/*/*.c)

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	DYNAMIC=libcorange.so
	STATIC=libcorange.a
	CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g -fPIC
	LFLAGS= -lGL -lSDLmain -lSDL -shared
	OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	DYNAMIC=libcorange.so
	STATIC=libcorange.a
	CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g -fPIC
	LFLAGS= -lGL -lSDLmain -lSDL -shared
	OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	DYNAMIC=corange.dll
	STATIC=corange.lib
	CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
	LFLAGS= -g -L ./lib -lmingw32 -lopengl32 -lSDLmain -lSDL -lSDL_mixer -shared
	OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o))) corange.res
endif

all: $(DYNAMIC) $(STATIC)

$(DYNAMIC): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(LFLAGS) -o $@
	
$(STATIC): $(OBJ_FILES)
	$(AR) rcs $@ $(OBJ_FILES)
	
obj/%.o: src/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@

obj/%.o: src/*/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@
	
obj:
	mkdir obj
	
corange.res: corange.rc
	windres $< -O coff -o $@
	
clean:
	rm $(OBJ_FILES)
