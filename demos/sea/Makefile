DEMO=sea
CC=gcc

CFLAGS= -I../../include -std=gnu99 -Wall -Werror -Wno-unused -O3 -g

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	OUT=$(DEMO)
	LFLAGS= -lcorange -lGL -lSDL2main -lSDL2 -lSDL2_net -lSDL2_mixer
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	OUT=$(DEMO)
	LFLAGS= -lcorange -lGL -lSDL2main -lSDL2 -lSDL2_net -lSDL2_mixer
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	OUT=$(DEMO).exe
	LFLAGS= ../../corange.res -lcorange -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -lSDL2_mixer -lopengl32
endif

$(OUT): sea.c
	$(CC) $< $(CFLAGS) $(LFLAGS) -o $@
	
clean:
	rm $(OUT)

