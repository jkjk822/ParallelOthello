ARCH = $(shell uname|tr '[A-Z]' '[a-z]')
#CC = /u/cs458/$(ARCH)/bin/mpicc
CC = mpic++ -g
#########################################################################
# At the bare minimum, you'll need to edit the following two lines
# all objects and binaries will be in the directory ${ARCH}
#########################################################################
SRCS = parallel_player.cpp
BIN = ${ARCH}/sor

#########################################################################
# Compiler flags for different archs
#########################################################################
CFLAGS = -O3
CFLAGS_irix64 =
CFLAGS_osf1 =
CFLAGS_sunos =
CFLAGS_linux =

CFLAGS += $(CFLAGS_${ARCH})

# add linking flags here
#LDFLAGS = -L/u/cs458/${ARCH}/lib
#LDFLAGS = -lmpi -lrt -lpthread -lm
#LDFLAGS = -lrt -lpthread -lm
LDFLAGS = -lm

#
# Add runtime linking path so we don't have to worry about LD_LIBRARY_PATH
#
#LDFLAGS_irix64 = -Wl,-rpath,/u/cs458/${ARCH}/lib
#LDFLAGS_osf1 = -Wl,-rpath,/u/cs458/${ARCH}/lib
#LDFLAGS_sunos = -R/u/cs458/${ARCH}/lib
#LDFLAGS_linux = -Wl,-rpath,/u/cs458/${ARCH}/lib
LDFLAGS_linux = -Wl,-rpath,/usr/lib64/mpich/lib

LDFLAGS += $(LDFLAGS_${ARCH})

# add include flags here
INCFLAGS = -I/u/cs458/${ARCH}/include
INCFLAGS =
INCFLAGS_irix64 =
INCFLAGS_osf1 =
INCFLAGS_sunos =
INCFLAGS_linux =

INCFLAGS += $(INCFLAGS_${ARCH})
#########################################################################

.PHONY:all clean

# #########################################################################
# # Generate object files
# #########################################################################
${ARCH}/%.o : %.c
	-@if [ ! -d ${ARCH} ] ; then mkdir ${ARCH} ; fi
	$(COMPILE.c) $< $(OUTPUT_OPTION)

# For C++ files the following works:
${ARCH}/%.o : %.cc
	-@if [ ! -d ${ARCH} ] ; then mkdir ${ARCH} ; fi
	$(COMPILE.C) $< $(OUTPUT_OPTION)

# Generate dependency files
${ARCH}/%.d : %.c
	-@if [ ! -d ${ARCH} ] ; then mkdir ${ARCH} ; fi
	$(CC) $(INCFLAGS) -M $< | sed 's|$*.o[ :]|${ARCH}/$*.o ${ARCH}/$*.d:|g' > $@
#########################################################################

OBJS = $(SRCS:%.c=${ARCH}/%.o)
	DEPS = $(SRCS:%.c=${ARCH}/%.d)

all: $(BIN)

# You may have to customize the following (LINK.C for c++)
$(BIN): $(OBJS)
	$(LINK.c) $(OUTPUT_OPTION) $(OBJS) $(LFLAGS) $(LDFLAGS)
                 
clean:
	-rm -rf $(OBJS) $(BIN) $(DEPS)
