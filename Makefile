
INC = -Iinclude -Iinclude/hashtable
LIB = -lpthread -libverbs -lrdmacm

SRC = src
OBJ = obj
TESTS = tests
INCLUDE = include

CC = g++
STD = -std=c++11
DEBUG = -g
CFLAGS = -Wall -c -O2 $(DEBUG) $(STD)
LFLAGS = -Wall -O2 $(DEBUG) $(STD)

ROOT = .

vpath %.cpp $(SRC)
vpath %.h $(INCLUDE)

MAKE = $(CC) $(INC)

HEADER = $(wildcard $(INCLUDE)/*.h)

# Object files needed by modules
TEST_ITEMS = tests/items.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
TEST_HUGE = tests/hugepage.cpp $(addprefix $(OBJ)/, mm.o)
SERVER = tests/server.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
TEST_OPERATIONS = tests/operations.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
RG_BEN = tests/rgben.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
VERBS = tests/verbs.cpp $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a
STRESS = $(addprefix $(OBJ)/, mm.o hash.o larmdata.o common.o bnet.o intf.o hashtable.o) libs/libcityhash.a

all:

test: items huge 

# Test item builder and data retrieval methods
items: $(TEST_ITEMS)
	$(MAKE) $(LFLAGS) $(TEST_ITEMS) -o $(ROOT)/$(TESTS)/run_test_items $(LIB)
	$(ROOT)/$(TESTS)/run_test_items

# Test hugepace functionalities
huge: $(TEST_HUGE)
	$(MAKE) $(LFLAGS) $(TEST_HUGE) -o $(ROOT)/$(TESTS)/run_test_huge $(LIB)
	$(ROOT)/release-hugepage.sh
	$(ROOT)/$(TESTS)/run_test_huge

# Start server
server: $(SERVER)
	$(MAKE) $(LFLAGS) $(SERVER) -o $(ROOT)/$(TESTS)/run_server $(LIB)
	$(ROOT)/release-hugepage.sh
	$(ROOT)/$(TESTS)/run_server

operations: $(TEST_OPERATIONS)
	$(MAKE) $(LFLAGS) $(TEST_OPERATIONS) -o $(ROOT)/$(TESTS)/run_test_operations $(LIB)
	$(ROOT)/$(TESTS)/run_test_operations


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





	