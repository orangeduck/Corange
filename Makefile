CC=gcc
AR=ar

SRC = $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJ = $(addprefix obj/,$(notdir $(SRC:.c=.o)))

CFLAGS = -I ./include -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
LFLAGS = -lSDLmain -lSDL -lSDL_mixer -shared

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	DYNAMIC = libcorange.so
	STATIC = libcorange.a
	CFLAGS += -fPIC
	LFLAGS += -lGL
	LIBS = /usr/lib
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	DYNAMIC = libcorange.so
	STATIC = libcorange.a
	CFLAGS += -fPIC
	LFLAGS += -lGL
	LIBS = /usr/lib
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	DYNAMIC = corange.dll
	STATIC = libcorange.a
	LFLAGS = -lmingw32 -lopengl32 -lSDLmain -lSDL -lSDL_mixer -shared
	OBJ += corange.res
	LIBS = C:/MinGW64/x86_64-w64-mingw32/lib
endif

all: $(DYNAMIC) $(STATIC)

$(DYNAMIC): $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o $@
	
$(STATIC): $(OBJ)
	$(AR) rcs $@ $(OBJ)
	
obj/%.o: src/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@

obj/%.o: src/*/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@
	
obj:
	mkdir obj
	
corange.res: corange.rc
	windres $< -O coff -o $@
	
clean:
	rm $(OBJ)
  
install: $(STATIC)
	cp $(STATIC) $(LIBS)/$(STATIC)
