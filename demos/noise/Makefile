DEMO=noise
CC=gcc

INCS= -I ./include -I ../../include
LIBS= -L ./lib

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	OUT=$(DEMO)
	LFLAGS= $(LIBS) -lcorange -lGL -lSDLmain -lSDL
	CORANGE= ./lib/libcorange.a
	CORANGE_LIB= ../../libcorange.a
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	OUT=$(DEMO)
	LFLAGS= $(LIBS) -lcorange -lGL -lSDLmain -lSDL
	CORANGE= ./lib/libcorange.a
	CORANGE_LIB= ../../libcorange.a
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	OUT=$(DEMO).exe
	LFLAGS= $(LIBS) -L../../lib ../../corange.res -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32
	CORANGE= ./lib/corange.lib
	CORANGE_LIB= ../../corange.lib
endif

$(OUT): $(OBJ_FILES) $(CORANGE)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@

obj:
	mkdir obj
	
$(CORANGE): $(CORANGE_LIB) | lib
	cp $(CORANGE_LIB) $(CORANGE)
	
lib:
	mkdir lib
	
clean:
	rm $(OUT) $(CORANGE) $(OBJ_FILES)

