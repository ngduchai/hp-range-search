
INC = -Iinclude -Iinclude/bwtree
LIB = -lpthread -libverbs -lrdmacm

SRC = src
OBJ = obj
TESTS = tests
INCLUDE = include

OPT_BW = -mcx16 -Wno-invalid-offsetof -frename-registers -funroll-loops -flto -march=native -DNDEBUG -DBWTREE_NODEBUG

CC = g++
STD = -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(OPT_BW) $(DEBUG) $(STD)
LFLAGS = -Wall $(OPT_BW) $(DEBUG) $(STD)

ROOT = .

vpath %.cpp $(SRC)
vpath %.h $(INCLUDE)

MAKE = $(CC) $(INC)

HEADER = $(wildcard $(INCLUDE)/*.h)
HEADER_BW = $(wildcard $(INCLUDE)/bwtree/*.h)

# Object files needed by modules
TEST_BWTREE = tests/bw.cpp $(addprefix $(OBJ)/, bwtree.o)
TEST_HUGE = tests/hugepage.cpp $(addprefix $(OBJ)/, mm.o)
SERVER = tests/server.cpp $(addprefix $(OBJ)/, mm.o procs_base.o bnet.o layout.o) 
TEST_UNI = tests/uni.cpp $(addprefix $(OBJ)/, mm.o bnet.o procs_base.o layout.o common.o)
RG_BEN = tests/rgben.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
VERBS = tests/verbs.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
STRESS = $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a

all:

test: items huge 

# Test Bw Tree functionalities
bwtree: $(TEST_BWTREE) $(HEADER_BW)
	$(MAKE) $(LFLAGS) $(TEST_BWTREE) -o $(ROOT)/$(TESTS)/run_test_bptree $(LIB)
	$(ROOT)/$(TESTS)/run_test_bptree

# Test hugepace functionalities
huge: $(TEST_HUGE)
	$(MAKE) $(LFLAGS) $(TEST_HUGE) -o $(ROOT)/$(TESTS)/run_test_huge $(LIB)
	$(ROOT)/release-hugepage.sh
	$(ROOT)/$(TESTS)/run_test_huge

# Start server
server: $(SERVER)
	$(MAKE) $(LFLAGS) $(SERVER) -o $(ROOT)/$(TESTS)/run_server $(LIB)
	$(ROOT)/release-hugepage.sh
	$(ROOT)/$(TESTS)/run_server 12345

uni: $(TEST_UNI)
	$(MAKE) $(LFLAGS) $(TEST_UNI) -o $(ROOT)/$(TESTS)/run_test_uni $(LIB)
	$(ROOT)/$(TESTS)/run_test_uni


rgben: $(RG_BEN)
	$(MAKE) $(LFLAGS) $(RG_BEN) -o $(ROOT)/$(TESTS)/run_rgben $(LIB)
	$(ROOT)/$(TESTS)/run_rgben

verb: $(VERBS)
	$(MAKE) $(LFLAGS) $(VERBS) -o $(ROOT)/$(TESTS)/run_verb $(LIB)
	$(ROOT)/$(TESTS)/run_verb

genit: $(STRESS) tests/genit.cpp tests/rand.cpp
	$(MAKE) $(LFLAGS) tests/genit.cpp tests/rand.cpp $(STRESS) -o $(ROOT)/$(TESTS)/run_genit $(LIB)
	$(ROOT)/$(TESTS)/run_genit

stress: $(STRESS) tests/stress.cpp tests/rand.cpp
	$(MAKE) $(LFLAGS) tests/stress.cpp tests/rand.cpp $(STRESS) -o $(ROOT)/$(TESTS)/run_stress $(LIB)
	#$(ROOT)/$(TESTS)/run_stress


# Create objects from sources
$(OBJ)/%.o: %.cpp ${HEADER}
	$(MAKE) $(CFLAGS) $< -o $@

clean:
	rm -f obj/*.o tests/run_*





	
