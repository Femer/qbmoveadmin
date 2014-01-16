# compiler
COMPILER = gcc

# flags
CFLAGS = -c -Wall
LMFLAGS = -lm
# PFLAGS = -framework Phidget21 -I /Library/Frameworks/Phidget21.framework/Headers

# files
#SOURCES = qbmove.c qbmoveadmin.c qbinit.c qbtest.c qb_pos_stiff_demo.c qbbackup.c


all:qbmove qbmoveadmin qbinit qbtest qb_pos_stiff_demo qbbackup qbparam

    
qbmove:objs/qbmove.o objs/qbmove_communications.o bin
	$(COMPILER) $(PFLAGS) objs/qbmove.o objs/qbmove_communications.o -o bin/qbmove $(LMFLAGS)

qbmoveadmin:objs/qbmoveadmin.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbmoveadmin.o objs/qbmove_communications.o -o bin/qbmoveadmin $(LMFLAGS)

qbinit:objs/qbinit.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbinit.o objs/qbmove_communications.o -o bin/qbinit $(LMFLAGS)

qbtest:objs/qbtest.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbtest.o objs/qbmove_communications.o -o bin/qbtest $(LMFLAGS)

qb_pos_stiff_demo:objs/qb_pos_stiff_demo.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qb_pos_stiff_demo.o objs/qbmove_communications.o -o bin/qb_pos_stiff_demo $(LMFLAGS)

qbbackup:objs/qbbackup.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbbackup.o objs/qbmove_communications.o -o bin/qbbackup $(LMFLAGS)

qbparam:objs/qbparam.o objs/qbmove_communications.o bin
	$(COMPILER) objs/qbparam.o objs/qbmove_communications.o -o bin/qbparam $(LMFLAGS)



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

objs/qbbackup.o:qbbackup.c objs
	$(COMPILER) $(CFLAGS) qbbackup.c -o objs/qbbackup.o

objs/qbparam.o:qbparam.c objs
	$(COMPILER) $(CFLAGS) qbparam.c -o objs/qbparam.o




clean:
	rm -rf *.o bin objs

objs:
	mkdir objs

bin:
	mkdir bin
