NAME = lab03
SRC = $(NAME).c
EXEC = $(NAME).exe

CC = x86_64-w64-mingw32-g++
CFLAGS = -Wall -mwindows

build_and_run: $(EXEC)
	wine $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(SRC) -o $(EXEC) $(CFLAGS)

clean:
	rm -f $(EXEC)
