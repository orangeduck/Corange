DEMO=sea
CC=gcc

INCS= -I ./include -I ../../include

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	OUT=$(DEMO)
	LFLAGS= -L./lib -lcorange -lGL -lSDLmain -lSDL
	CORANGE_LIB= ../../libcorange.so
	CORANGE_TAR= ./lib/libcorange.a
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	OUT=$(DEMO).exe
	LFLAGS= ../../corange.res -L./ -L../../lib -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32
	CORANGE_LIB= ../../corange.dll
	CORANGE_TAR= ./corange.dll
endif

$(OUT): $(OBJ_FILES) corange
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@

corange: $(CORANGE_LIB)
	cp $(CORANGE_LIB) $(CORANGE_TAR)
	
clean:
	rm $(OBJ_FILES)	

