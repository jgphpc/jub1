CXX     = CC
FFLAGS = -DSYNC_WINDOW -DMAX_BUFF_SIZE_PER_PROC=64
# -Wl,"-zmuldefs"
H := $(shell echo $(HOST) | tr [a-z] [A-Z] | tr -d [0-9] )



# add -openmp compiler flag will define _OPENMP
ifeq ($(PE_ENV),INTEL)
        FFLAGS += -I. -O2
        CFLAGS += $(FFLAGS)
endif

ifeq ($(PE_ENV),PGI)
        FFLAGS += -mp -w -O0
        CFLAGS += $(FFLAGS)
endif

ifeq ($(PE_ENV),GNU)
        FFLAGS += -fopenmp -ggdb
        CFLAGS += $(FFLAGS)
endif

ifeq ($(PE_ENV),CRAY)
        FFLAGS += -G0 -homp
        CFLAGS += $(FFLAGS) # -h msglevel_4 
endif

LDFLAGS        = $(FFLAGS)
LIB            = 
SRC     = $(wildcard *.cc)
OBJ     = $(SRC:.cc=.o)
.SUFFIXES: .o .cc

%.o: %.cc
	$(CXX) $(CFLAGS) -c $< -o $@

$(PE_ENV).$(H): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LIB) -o $@
	@echo $(PE_ENV) / $(H) / c executable ready

clean:
	rm -f *.o o_* runme.slurm.* *.$(H) 
