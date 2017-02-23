
#include <string>
#include <iostream>
#include <unistd.h>
#include "larmdata.h"
#include "net.h"
#include "intf.h"
#include "mm.h"
#include "common.h"

#include "indata.h"

#include <chrono>
#include <rand.h>
#include <string.h>

using namespace std;
using namespace LARM;
using namespace LARM::LARM_COMMON;
using namespace LARM::LARM_NET;
using namespace LARM::INLARM::LARM_MM;

using namespace chrono;

const unsigned int MAX_REQ = 1000000;
const unsigned int MAX_CHAIN = 10;
const unsigned int READ_RATE = 100;
const unsigned int MAX_RATE = 100;

const unsigned int KEY_SIZE = (80000000);

const bool skw = false;

const unsigned int SIZE_CHANGE[4] = {
	0, 	// add new attributes
	35, 	// change data
	35, 	// change links
	0	// remove an item
};

enum data_operations { ADDATTR, CHDATA, CHLINK, REIT };

string const default_chars =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ0123456789";

string gen_string(int len, string const &allowed_char = default_chars) {
	std::mt19937_64 gen {std::random_device()() };
	std::uniform_int_distribution<size_t> dist
		{0, allowed_char.length() - 1};
	if (len <= 10) {
		len = 100;
	}else{
		std::normal_distribution<> ldist(len, len >> 2);
		do {
			len = ldist(gen);
		}while(len <= 0);
	}
	string str;
	std::generate_n(std::back_inserter(str), len,
		[&] { return allowed_char[dist(gen)]; });
	return str;

}

uint32_t gen_slen(uint32_t len) {
	if (len <= 10) {
		return 100;
	}else{
		std::mt19937_64 gen {std::random_device()() };
		std::normal_distribution<> ldist(len, len >> 2);
		do {
			len = ldist(gen);
		}while (len <= 0);
		return len;
	}
}

zipf_generator * zg;

uint64_t get_key();
bool make_change();
data_operations change_method();

int main(int argc, char ** argv) {
	
	LARM::INLARM::LARM_MM::init_mem(2);
	
	zg = new zipf_generator(0, KEY_SIZE, 0.99);

	add_host("10.2.1.21", "12345");
	start_net(time_us() % (1 << 14) + (1 << 14),
			(LARM::BYTE)get_base(), 2, processor, true);

	unsigned long num_chase = 0;
	time_t time_chase = 0;
	//double rp = 0.1;
	unsigned long num_chain = 0;

	cout << time_us() << endl;

	for (unsigned int i = 0; i < MAX_REQ; ++i) {
		uint64_t k = get_key();
		lkey key((LARM::BYTE)&k, sizeof(k));
		litem item;
		//cout << i << endl;
		if (!get_item(key, item)) {
			if (make_change()) {
				litem::builder b;
				b.set_key(lkey((BYTE)&k, sizeof(int)));
				b.set_int("num", 2);
				k = get_key();
				b.set_link("0", lkey((BYTE)&k, sizeof(int)));
				k = get_key();
				b.set_link("1", lkey((BYTE)&k, sizeof(int)));
				b.set_str("data", gen_string(100));
				put_item(b.build());
			}else{
				i--;
			}
			continue;
		}
		if (make_change()) {
			int num = *(int*)item.get_attr("num");
			litem::builder builder(item);
			uint64_t tk = get_key();
			lkey data((LARM::BYTE)&tk, sizeof(tk));
			switch (change_method()) {
			case CHDATA: {
				int len = string((char*)
					item.get_attr("data")).length();
				len = gen_slen(len);
				char * data = new char [len];
				builder.set_str("data", string(data, len));
				put_item(builder.build());
				delete data;
				break;
			}
			case REIT:
				remove_item(item.get_key());
				break;
			case CHLINK:
				builder.set_link(to_string(
					time_us() % num), data);
				put_item(builder.build());
				break;
			case ADDATTR:
				if (num > 1000) {
					i--;
					break;
				}
				builder.set_link(to_string(num), data);
				builder.set_int("num", num + 1);
				put_item(builder.build());
				break;
			}
		}else{
			for (unsigned int j = 0; j < MAX_CHAIN; ++j) {
				int num = *(int*)item.get_attr("num");
				litem record;
				string index = to_string(time_us() % num);
				//cout << index << " " << num << endl;
				auto start = high_resolution_clock::now();
				bool chase = item.chase_link(index, record);
				auto end = high_resolution_clock::now();
				auto dur = duration_cast<nanoseconds>
					(end - start);
				time_chase += dur.count();
				num_chase++;	
				if (chase) {
					item = record;
				}else{
					break;
				}
			}
			num_chain++;
		}
		/*
		if ((double)i / (double)MAX_REQ > rp) {
			cout << "Processed: " << (rp * 100) << "%" << endl;
			rp += 0.1;
		}
		*/
	}

	cout << time_us() << endl;
	/*
	cout << "Time: " << time_chase << " ns" << endl;
	cout << "Number of chains: " << num_chain << endl;
	cout << "Number of chases: " << num_chase << endl;
	*/
	cout << time_chase << endl;
	cout << num_chain << endl;
	cout << num_chase << endl;
	cout << "Throughput: " << ((double)num_chase / (double)time_chase *
		1e9) << " chases/s" << endl;
	
	end_net();

}

uint64_t get_key() {
	if (skw) {
		return zg->next();
	}else{
		std::mt19937_64 gen {std::random_device()() };
		std::uniform_int_distribution<size_t> dist {0, KEY_SIZE - 1};
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
	unsigned int method = time_us() % (SIZE_CHANGE[0] + SIZE_CHANGE[1] +
		SIZE_CHANGE[2] + SIZE_CHANGE[3]);
	data_operations data = ADDATTR;
	if (method < SIZE_CHANGE[0]) {
		data = ADDATTR;
	}else if (method < SIZE_CHANGE[0] + SIZE_CHANGE[1]) {
		data = CHDATA;
	}else if (method < SIZE_CHANGE[0] + SIZE_CHANGE[1] + SIZE_CHANGE[2]) {
		data = CHLINK;
	}else {
		cout << "data" << endl;
		data = REIT;
	}
	return data;
}









