DEMO=teapot
CC=gcc

CFLAGS= -I../../include -std=gnu99 -Wall -Werror -Wno-unused -O3 -g

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	OUT=$(DEMO)
	LFLAGS= -lcorange -lGL -lSDLmain -lSDL -lSDL_Net -lSDL_Mixer
endif

ifeq ($(findstring Darwin,$(PLATFORM)),Darwin)
	OUT=$(DEMO)
	LFLAGS= -lcorange -lGL -lSDLmain -lSDL -lSDL_Net -lSDL_Mixer
endif

ifeq ($(findstring MINGW,$(PLATFORM)),MINGW)
	OUT=$(DEMO).exe
	LFLAGS= ../../corange.res -lcorange -lmingw32 -lSDLmain -lSDL -lSDL_Net -lSDL_Mixer -lopengl32
endif

$(OUT): teapot.c
	$(CC) $< $(CFLAGS) $(LFLAGS) -o $@
	
clean:
	rm $(OUT)

