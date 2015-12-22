SYS := $(shell gcc -dumpmachine) 
UNAME := $(shell uname)

ifneq (, $(findstring linux, $(SYS))) 
	MACHINE=-DLINUX
	LIBS = 	# Do linux things
else 
	MACHINE=-DWIN
	LIBS = -lwsock32 -lws2_32 -lmswsock 
endif
CC=gcc
CFLAGS = -g 

all: udp-send 

udp-send: udp-send.o 
	@echo $(MACHINE)
	@echo $(SYS)
	@echo $(UNAME)
	$(CC)  -o udp-send udp-send.o $(LIBS)


udp-send.o: udp-send.c 
	@echo $(MACHINE)
	@echo $(SYS)
	@echo $(UNAME)
	$(CC) $(MACHINE) -g -c -o udp-send.o udp-send.c 

clean:
	rm -f udp-send udp-send.o udp-send.exe 
