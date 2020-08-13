INC=-I./vidi.h/inc
CFLAGS+=-g -O4
LINK+=-lglfw -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm

bin:
	mkdir bin

src/%.c.o: src/%.c bin
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

bin/%: src/camera_loop.c.o src/%.c.o
	$(CC) $(CFLAGS) $(INC) $^ -o $@ $(LINK)

.PHONY: clean
clean:
	rm -rf bin src/*.o


