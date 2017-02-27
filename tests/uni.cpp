
#include <iostream>
#include "bnet.h"
#include "mm.h"

#ifdef CONF_OL
#include "query_ol.h"
#else
#include "query_base.h"
#endif

#include "common.h"
#include "define.h"
#include "layout.h"
#include "procs.h"

using namespace std;

int main() {
	LARM::INLARM::LARM_MM::init_mem(2);
	LARM::LARM_NET::add_host("10.2.1.21", "12345");
	start_net(time_us() % (1 << 14) + (1 << 14),
		(LARM::BYTE)get_base(), 2, processor, true);
	dbint_base<uint64_t, item_t, rquery<uint64_t>> dbint;
	cout << "Insert item" << endl;
	for (uint64_t i = 0; i < 100000; ++i) {
		item_t data(i, i);
		dbint.insert(i, data);
	}
	cout << "Get item" << endl;
	rquery<uint64_t> query(120, 10000);
	//rquery<uint64_t> query(120, 1220);
	auto cursor = dbint.find(query);
	int data = 0;
	while (cursor.valid()) {
		//cout << cursor.value().skey << endl;
		cursor.next();
		data++;
	}
	cout << data << endl;
}




