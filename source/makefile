# compiler
COMPILER = gcc

# flags
CFLAGS = -c -Wall 
PFLAGS = -framework Phidget21 -I /Library/Frameworks/Phidget21.framework/Headers

all:qbmove qbmoveadmin
    
qbmove:objs/qbmove.o objs/qbmove_packages.o objs/qbmove_communications.o bin
	$(COMPILER) $(PFLAGS) objs/qbmove.o objs/qbmove_packages.o objs/qbmove_communications.o -o bin/qbmove

qbmoveadmin:objs/qbmoveadmin.o objs/qbmove_packages.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbmoveadmin.o objs/qbmove_packages.o objs/qbmove_communications.o -o bin/qbmoveadmin

objs/qbmove.o:qbmove.c objs
	$(COMPILER) $(CFLAGS) $(PFLAGS) qbmove.c -o objs/qbmove.o 

objs/qbmoveadmin.o:qbmoveadmin.c objs
	$(COMPILER) $(CFLAGS) qbmoveadmin.c -o objs/qbmoveadmin.o

objs/qbmove_packages.o:qbmoveAPI/qbmove_packages.c objs
	$(COMPILER) $(CFLAGS) qbmoveAPI/qbmove_packages.c -o objs/qbmove_packages.o

objs/qbmove_communications.o:qbmoveAPI/qbmove_communications.c objs
	$(COMPILER) $(CFLAGS) qbmoveAPI/qbmove_communications.c -o objs/qbmove_communications.o

clean:
	rm -rf *.o bin objs

objs:
	mkdir objs

bin:
	mkdir bin
