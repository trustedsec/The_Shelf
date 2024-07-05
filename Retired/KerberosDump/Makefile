OSVERSION:=$(shell uname)
MAINFILES:=src/kerberos_posix.c src/base64/encode.c src/base64/decode.c src/ccache.c
OUTFOLDER:=bins
CC:=gcc

all: setup
ifeq ($(OSVERSION), Linux)
	$(CC) -Wall -I include/ $(MAINFILES) -lkrb5 -o $(OUTFOLDER)/kerberos.out
endif
ifeq ($(OSVERSION), Darwin)
	$(CC) -Wall -I include/ $(MAINFILES) -framework Kerberos -o $(OUTFOLDER)/kerberos.out
	$(CC) -Wall -I include/ src/test2.c src/base64/encode.c -framework GSS -o $(OUTFOLDER).out
endif

debug: setup
ifeq ($(OSVERSION), Linux)
	$(CC) -g -DDEBUG -Wall -I include/ $(MAINFILES) -lkrb5 -o $(OUTFOLDER)/kerberos.out
endif
ifeq ($(OSVERSION), Darwin)
	$(CC) -g -DDEBUG -Wall -I include/ $(MAINFILES) -framework Kerberos -o $(OUTFOLDER)/kerberos.out
	$(CC) -g -DDEBUG -Wall -I include/ src/test2.c src/base64/encode.c -framework GSS -o $(OUTFOLDER).out
endif


osx: setup
	$(CC) -Wall -I include/ $(MAINFILES) -framework Kerberos -o $(OUTFOLDER)/kerberos.out

osxdebug: setup
	$(CC) -DDEBUG -Wall -I include/ $(MAINFILES) -framework Kerberos -o $(OUTFOLDER)/kerberos.out


setup:
	mkdir -p $(OUTFOLDER)

clean:
	#echo $(OUTFOLDER)
	rm -rf $(OUTFOLDER)/

PHONY:setup all clean osx osxdebug debug
