# generation de programme PVM
#  TP algorithmique parallele
#  maitrise
#  LL
#  07/10/97
# 

CC = gcc -g

# executable directory
#BDIR  =  $(HOME)/bin/$(PVM_ARCH)
BDIR = $(PWD)

# Programmes maitre et esclave
PROG = upper upper_slave

PVM_ROOT = /usr/lib/pvm3
PVM_ARCH = LINUXI386
#PVM_ROOT = /home/commun_depinfo/enseignants/lemarchand/TPPVMM1/PvmLinux/pvm3
#PVM_ARCH = LINUX64


#### do not edit #####

EXEC = $(PROG:%=$(BDIR)/%)

#PVM_LIB = $(PVM_ROOT)/lib/$(PVM_ARCH)/libpvm3.a
PVM_LIB = -lpvm3
CFLAGS =  -I$(PVM_ROOT)/include -D_POSIX_C_SOURCE=2 -DEPATH=\"$(BDIR)\"
# include $(PVM_ROOT)/conf/$(PVM_ARCH).def

#### generation rules #####

all: $(EXEC)

$(BDIR)/upper: point.o upper.o 
	$(CC) -o $@ upper.o point.o $(PVM_LIB) $(ARCHLIB)

$(BDIR)/upper_slave: point.o upper_slave.o 
	$(CC) -o $@ upper_slave.o point.o $(PVM_LIB) $(ARCHLIB)

.c.o: point.h
	$(CC) $(CFLAGS) $(ARCHCFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXEC) upper_hull.pdf
