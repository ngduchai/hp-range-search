
#include <string>
#include <iostream>
#include <unistd.h>
#include "net.h"
#include "mm.h"
#include "common.h"


#include <chrono>
#include <rand.h>
#include <string.h>
#include "procs.h"
#include "layout.h"
#include "query_base.h"
#include <random>

using namespace std;
using namespace LARM;
using namespace LARM::LARM_NET;
using namespace LARM::INLARM::LARM_MM;

using namespace chrono;

const unsigned int MAX_REQ = 1000;
const unsigned int READ_RATE = 100;
const unsigned int MAX_RATE = 100;

const unsigned int DATA_SZ = (1000000);

const bool skw = false;

const unsigned int SIZE_CHANGE[4] = {
	50, 	// add new attributes
	50, 	// change data
};

enum data_operations { ADDATTR, REIT };

zipf_generator * zg;

uint64_t get_key(uint64_t band);
bool make_change();
data_operations change_method();

int main(int argc, char ** argv) {
	
	if (argc != 2) {
		return 1;
	}
	uint64_t key_range = stod(argv[1]) * DATA_SZ;

	LARM::INLARM::LARM_MM::init_mem(2);
	
	zg = new zipf_generator(0, KEY_SIZE, 0.99);

	add_host("10.2.1.21", "12345");
	start_net(time_us() % (1 << 14) + (1 << 14),
			(LARM::BYTE)get_base(), 2, processor, true);
	
	dbint_base<uint64_t, item_t, rquery<uint64_t>> dbint;

	unsigned long num_reads = 0;
	time_t time_read = 0;
	double rp = 0.1;
	unsigned long num_items = 0;

	cout << time_us() << endl;

	for (unsigned int i = 0; i < MAX_REQ; ++i) {
		if (make_change()) {
			uint64_t key = get_key(0);
			switch (change_method()) {
			case REIT:
				dbint.remove(key);			
				break;
			case ADDATTR:
				dbint.insert(key, item_t(key, key));
				break;
			}
		}else{
			uint64_t skey = get_key(key_range);
			uint64_t ekey = skey + key_range - 1;
			rquery<uint64_t> query(skey, ekey);
			auto cursor = dbint.find(query);

			while (cursor.valid()) {
				auto start = high_resolution_clock::now();
				cursor.next();	
				auto end = high_resolution_clock::now();
				auto dur = duration_cast<nanoseconds>
					(end - start);
				time_read += dur.count();
				num_items++;	
			}
			num_reads++;
		}
		
		if ((double)i / (double)MAX_REQ > rp) {
			cout << "Processed: " << (rp * 100) << "%" << endl;
			rp += 0.1;
		}
		
	}

	cout << time_us() << endl;
	
	cout << "Time: " << time_read << " ns" << endl;
	cout << "Number of items: " << num_items << endl;
	cout << "Number of reads: " << num_reads << endl;
	
	cout << time_read << endl;
	cout << num_items << endl;
	cout << num_reads << endl;
	cout << "Throughput: " << ((double)num_items / (double)time_read *
		1e9) << " items/s" << endl;
	
	end_net();

}

uint64_t get_key(uint64_t band) {
	if (skw) {
		return zg->next();
	}else{
		std::mt19937_64 gen {std::random_device()() };
		std::uniform_int_distribution<size_t>
			dist {0, DATA_SZ - band - 1};
		//return dist(gen);
		uint32_t data = dist(gen);
		return data;
	}
	//return time_us() % KEY_SIZE;
}

bool make_change() {
	return ((time_us() % MAX_RATE) > READ_RATE);
}

data_operations change_method() {
	unsigned int method = time_us() % (SIZE_CHANGE[0] + SIZE_CHANGE[1]);
	data_operations data = ADDATTR;
	if (method < SIZE_CHANGE[0]) {
		data = ADDATTR;
	}else if (method < SIZE_CHANGE[0] + SIZE_CHANGE[1]) {
		data = REIT;
	}
	return data;
}









