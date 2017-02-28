
INC = -Iinclude -Iinclude/bwtree
LIB = -lpthread -libverbs -lrdmacm

SRC = src
OBJ = obj
TESTS = tests
INCLUDE = include

OPT_BW = -mcx16 -Wno-invalid-offsetof -frename-registers -Ofast -funroll-loops -flto -march=native -DNDEBUG -DBWTREE_NODEBUG

CC = g++
STD = -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(OPT_BW) $(DEBUG) $(STD)
LFLAGS = -Wall $(OPT_BW) $(DEBUG) $(STD)

ROOT = .
CONF = CONF_OL #CONF_BASE #CONF_OL
CONF_FILE = ol # base

vpath %.cpp $(SRC)
vpath %.h $(INCLUDE)

MAKE = $(CC) $(INC) -D$(CONF)

HEADER = $(wildcard $(INCLUDE)/*.h)
HEADER_BW = $(wildcard $(INCLUDE)/bwtree/*.h)

# Object files needed by modules
TEST_BWTREE = tests/bw.cpp $(addprefix $(OBJ)/, bwtree.o)
TEST_HUGE = tests/hugepage.cpp $(addprefix $(OBJ)/, mm.o)
SERVER = tests/server.cpp $(addprefix $(OBJ)/, mm.o procs_ol.o bnet.o layout.o)
TEST_UNI = tests/uni.cpp $(addprefix $(OBJ)/, mm.o procs_ol.o bnet.o layout.o common.o)
VERBS = tests/verbs.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
STRESS = $(addprefix $(OBJ)/, mm.o procs_ol.o bnet.o layout.o common.o)

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

verb: $(VERBS)
	$(MAKE) $(LFLAGS) $(VERBS) -o $(ROOT)/$(TESTS)/run_verb $(LIB)
	$(ROOT)/$(TESTS)/run_verb

genit: $(STRESS) tests/genit.cpp tests/rand.cpp
	$(MAKE) $(LFLAGS) tests/genit.cpp tests/rand.cpp $(STRESS) -o $(ROOT)/$(TESTS)/run_genit $(LIB)
	$(ROOT)/$(TESTS)/run_genit 0 1000000

stress: $(STRESS) tests/stress.cpp tests/rand.cpp
	$(MAKE) $(LFLAGS) tests/stress.cpp tests/rand.cpp $(STRESS) -o $(ROOT)/$(TESTS)/run_stress $(LIB)
	$(ROOT)/$(TESTS)/run_stress 0.1


# Create objects from sources
$(OBJ)/%.o: %.cpp ${HEADER}
	$(MAKE) $(CFLAGS) $< -o $@

clean:
	rm -f obj/*.o tests/run_*





	
