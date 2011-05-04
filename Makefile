# This is zee makefile

CC=gcc

SDLI=./SDL/include
SDLL=./SDL/lib

INC=./include

INCS= -I $(SDLI) -I $(INC)
LIBS= -L $(SDLL)

CFLAGS= $(INCS)
LFLAGS= $(LIBS) -lmingw32 -lSDLmain -lSDL -lSDL_Image -lopengl32 -mwindows

C_FILES= $(wildcard src/*.c)
OBJ_FILES= $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

corange.exe: $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(LFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) $< -c -g $(CFLAGS) -o $@ 

clean:
	rm $(OBJ_FILES)