# compiler
COMPILER = gcc

# flags
CFLAGS = -c -Wall 
# PFLAGS = -framework Phidget21 -I /Library/Frameworks/Phidget21.framework/Headers

all:qbmove qbmoveadmin qbinit qbtest qb_pos_stiff_demo
    
qbmove:objs/qbmove.o objs/qbmove_communications.o bin
	$(COMPILER) $(PFLAGS) objs/qbmove.o objs/qbmove_communications.o -o bin/qbmove

qbmoveadmin:objs/qbmoveadmin.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbmoveadmin.o objs/qbmove_communications.o -o bin/qbmoveadmin

qbinit:objs/qbinit.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbinit.o objs/qbmove_communications.o -o bin/qbinit

qbtest:objs/qbtest.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbtest.o objs/qbmove_communications.o -o bin/qbtest

qb_pos_stiff_demo:objs/qb_pos_stiff_demo.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qb_pos_stiff_demo.o objs/qbmove_communications.o -o bin/qb_pos_stiff_demo

objs/qbmove.o:qbmove.c objs
	$(COMPILER) $(CFLAGS) $(PFLAGS) qbmove.c -o objs/qbmove.o 

objs/qbmoveadmin.o:qbmoveadmin.c objs
	$(COMPILER) $(CFLAGS) qbmoveadmin.c -o objs/qbmoveadmin.o

objs/qbmove_communications.o:qbmoveAPI/qbmove_communications.c objs
	$(COMPILER) $(CFLAGS) qbmoveAPI/qbmove_communications.c -o objs/qbmove_communications.o

objs/qbinit.o:qbinit.c objs
	$(COMPILER) $(CFLAGS) qbinit.c -o objs/qbinit.o

objs/qbtest.o:qbtest.c objs
	$(COMPILER) $(CFLAGS) qbtest.c -o objs/qbtest.o

objs/qb_pos_stiff_demo.o:qb_pos_stiff_demo.c objs
	$(COMPILER) $(CFLAGS) qb_pos_stiff_demo.c -o objs/qb_pos_stiff_demo.o

clean:
	rm -rf *.o bin objs

objs:
	mkdir objs

bin:
	mkdir bin
