
#include <iostream>
#include "common.h"
#include "net.h"
#include "mm.h"
#include <random>
#include "rand.h"
#include <string.h>
#include <procs.h>

#ifdef CONF_OL
#include "query_ol.h"
#else
#include "query_base.h"
#endif

#include <layout.h>

using namespace std;
using namespace LARM;
using namespace LARM::LARM_NET;
using namespace LARM::INLARM::LARM_MM;

const unsigned int DATA_SZ = 1000000;
const unsigned int NUM_DATA = 1;

const bool skw = true;

zipf_generator * zg;

uint64_t get_key();


int main(int argc, char ** argv) {
	
	if (argc != 3) {
		return 1;
	}

	uint64_t kstart = stoi(argv[1]);
	uint64_t kend = stoi(argv[2]);
	uint64_t num_items = kend - kstart;

	LARM::INLARM::LARM_MM::init_mem(2);

	zg = new zipf_generator(0, KEY_SIZE, 0.99);
	
	add_host("10.2.1.21", "12345");
	start_net(time_us() % (1 << 15) + (1 << 15), 
			(LARM::BYTE)get_base(), 2, processor, true);
	
	srand(time_us());

	cout << "Putting items to server(s)" << endl;
	double rp = 0;
	unsigned int size = 0;
	dbint_base<uint64_t, item_t, rquery<uint64_t>> dbint;
	for (uint64_t i = kstart; i <= kend; ++i) {
		dbint.insert(i, item_t(i, i));
		size++;
		if ((double)size / (double)num_items > rp + 0.1) {
			rp += 0.1;
			cout << "Put " << rp * 100 << "%" << endl;
		}
	}
	cout << "Put " << kend - kstart << " item(s)" << endl;
	
}

uint64_t get_key() {
	if (skw) {
		return zg->next();
	}else{
		std::mt19937_64 gen { std::random_device()() };
		std::uniform_int_distribution<size_t> dist {0, DATA_SZ - 1};
		uint32_t data = dist(gen);
		return data;
	}
}





