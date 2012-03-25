DEMO=lut_gen
CC=gcc

INCS= -I ./include -I ../../include

CFLAGS= $(INCS) -std=gnu99 -Wall -Werror -Wno-unused -O3 -g
C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

ifndef OSTYPE
  OSTYPE = $(shell uname -s|awk '{print tolower($$0)}')
endif

ifeq ($(OSTYPE),linux)
	OUT=$(DEMO)
	LFLAGS= -L./lib -lcorange -lGL -lSDLmain -lSDL
	CORANGE_LIB= ../../libcorange.so
	CORANGE_TAR= ./lib/libcorange.a
else
	OUT=$(DEMO).exe
	LFLAGS= ../../corange.res -L./ -L../../lib -lcorange -lmingw32 -lSDLmain -lSDL -lopengl32
	CORANGE_LIB= ../../corange.dll
	CORANGE_TAR= ./corange.dll
endif

# Windows

$(OUT): $(OBJ_FILES) corange
	$(CC) -g $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c $(CFLAGS) -o $@

corange: $(CORANGE_LIB)
	cp $(CORANGE_LIB) $(CORANGE_TAR)
	
clean:
	rm $(OBJ_FILES)	

