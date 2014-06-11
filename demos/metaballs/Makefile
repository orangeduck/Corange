DEMO=metaballs
CC=gcc

INCS= -I ./include -I ../../include

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	OUT=$(DEMO)
	LFLAGS= $(LIBS) -lcorange -lGL -lSDL2main -lSDL2 -lSDL2_Mixer -lSDL2_Net -lOpenCL
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	OUT=$(DEMO)
	LFLAGS= $(LIBS) -lcorange -lGL -lSDL2main -lSDL2 -lSDL2_Mixer -lSDL2_Net -lOpenCL
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	OUT=$(DEMO).exe
	LFLAGS= $(LIBS) ../../corange.res -lcorange -lmingw32 -lSDL2main -lSDL2 -lSDL2_Mixer -lSDL2_Net -lopengl32 -lOpenCL
endif

$(OUT): $(OBJ_FILES) $(CORANGE)
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c | obj
	$(CC) $< -c $(CFLAGS) -o $@

obj:
	mkdir obj
	
lib:
	mkdir lib
	
clean:
	rm $(OUT) $(OBJ_FILES)

