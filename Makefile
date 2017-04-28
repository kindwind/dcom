app := dcom.c

default: all

all: app

CC=gcc
Lib=-L/usr/X11R6/lib -lX11 -lXtst -pthread
#Lib=-pthread

app: $(app)
	${CC} $(app) -o $(patsubst %.c, %, $<) ${Lib}

clean_app: $(app)
	rm -rf $(patsubst %.c, %, $<)
	rm -rf *.o

clean: clean_app
