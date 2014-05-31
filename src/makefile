# compiler
COMPILER = g++

# flags
CFLAGS = -c -Wall
LMFLAGS = -lm

# folders
BIN_FOLDER = ../bin
OBJS_FOLDER = ../objs
LIB_FOLDER = ../../qbmoveAPI/lib


all:qbmove qbmoveadmin qbinit qbtest qb_pos_stiff_demo qbbackup qbparam


qbmove:$(OBJS_FOLDER)/qbmove.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qbmove.o      $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qbmove $(LMFLAGS)

qbmoveadmin:$(OBJS_FOLDER)/qbmoveadmin.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qbmoveadmin.o $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qbmoveadmin $(LMFLAGS)

qbinit:$(OBJS_FOLDER)/qbinit.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qbinit.o      $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qbinit $(LMFLAGS)

qbtest:$(OBJS_FOLDER)/qbtest.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qbtest.o      $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qbtest $(LMFLAGS)

qb_pos_stiff_demo:$(OBJS_FOLDER)/qb_pos_stiff_demo.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qb_pos_stiff_demo.o $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qb_pos_stiff_demo $(LMFLAGS)

qbbackup:$(OBJS_FOLDER)/qbbackup.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qbbackup.o    $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qbbackup $(LMFLAGS)

qbparam:$(OBJS_FOLDER)/qbparam.o $(BIN_FOLDER)
	$(COMPILER) $(OBJS_FOLDER)/qbparam.o     $(LIB_FOLDER)/libqbmove_comm.a -o $(BIN_FOLDER)/qbparam $(LMFLAGS)



$(OBJS_FOLDER)/qbmove.o:qbmove.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qbmove.c -o      $(OBJS_FOLDER)/qbmove.o 

$(OBJS_FOLDER)/qbmoveadmin.o:qbmoveadmin.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qbmoveadmin.c -o $(OBJS_FOLDER)/qbmoveadmin.o

$(OBJS_FOLDER)/qbinit.o:qbinit.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qbinit.c -o      $(OBJS_FOLDER)/qbinit.o

$(OBJS_FOLDER)/qbtest.o:qbtest.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qbtest.c -o      $(OBJS_FOLDER)/qbtest.o

$(OBJS_FOLDER)/qb_pos_stiff_demo.o:qb_pos_stiff_demo.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qb_pos_stiff_demo.c -o $(OBJS_FOLDER)/qb_pos_stiff_demo.o

$(OBJS_FOLDER)/qbbackup.o:qbbackup.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qbbackup.c -o    $(OBJS_FOLDER)/qbbackup.o

$(OBJS_FOLDER)/qbparam.o:qbparam.c $(OBJS_FOLDER)
	$(COMPILER) $(CFLAGS) qbparam.c -o     $(OBJS_FOLDER)/qbparam.o




clean:
ifeq "$(OS)"  "Windows_NT"
	erase /Q /S ..\objs
	erase /Q /S ..\bin
else
	rm -rf *.o $(BIN_FOLDER) $(OBJS_FOLDER)
endif

$(OBJS_FOLDER):
	mkdir $(OBJS_FOLDER)

$(BIN_FOLDER):
	mkdir $(BIN_FOLDER)
