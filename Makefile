CC=gcc
AR=ar

SRC = $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJ = $(addprefix obj/,$(notdir $(SRC:.c=.o)))

CFLAGS = -I ./include -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
LFLAGS = -lSDL2 -lSDL2_mixer -lSDL2_net -shared -g

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	DYNAMIC = libcorange.so
	STATIC = libcorange.a
	CFLAGS += -fPIC
	LFLAGS += -lGL
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	DYNAMIC = libcorange.so
	STATIC = libcorange.a
	CFLAGS += -fPIC
	LFLAGS += -framework OpenGL
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	DYNAMIC = corange.dll
	STATIC = libcorange.a
	LFLAGS = -lmingw32 -lopengl32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_net -shared -g
	OBJ += corange.res
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
	rm $(OBJ) $(STATIC) $(DYNAMIC)
  
install_unix: $(STATIC)
	cp $(STATIC) /usr/local/lib/$(STATIC)
  
install_win32: $(STATIC)
	cp $(STATIC) C:/MinGW/lib/$(STATIC)
  
install_win64: $(STATIC) $(DYNAMIC)
	cp $(STATIC) C:/MinGW64/x86_64-w64-mingw32/lib/$(STATIC)
	cp $(DYNAMIC) C:/MinGW64/x86_64-w64-mingw32/bin/$(DYNAMIC)
