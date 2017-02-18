
#include "procs.h"
#include "layout.h"
#include "bnet.h"
#include "mm.h"
#include "common.h"
#include "bptree.h"
#include "query_base.h"

using namespace LARM::INLARM::LARM_MM;

struct range_ptr {
	range_ptr(rquery<SKEY>& query,
		bptree<SKEY, item_t*, rquery<SKEY>>::cursor& cursor) :
	       	query(query), cursor(cursor) {}	
	rquery<SKEY> query;
	bptree<SKEY, item_t*, rquery<SKEY>>::cursor cursor;
};

void get_items(range_ptr * ptr, char * data) {
	int size = sizeof(rcode_t);
	rcode_t * code = (rcode_t*)data;
	item_t * items =  (item_t*)(&code[1]);
	uint64_t checksum = 0;
	uint32_t num = 0;
	auto cursor = ptr->cursor;
	while (cursor.valid() && size < BUFFER_IMP_SIZE) {
		*items = *cursor.value();
		checksum ^= items->skey;
		cursor.next();
		num++; items++; size += cursor.value()->size();
	}
	code->checksum = checksum;
	code->num = num;
	if (cursor.valid()) {
		code->code = (uintptr_t)ptr;
		code->has_next = true;
	}else{
		code->has_next = false;
	}

}

void process_range(packet_t * packet, char * data) {
	rquery<SKEY> * query = (rquery<SKEY>*)(&packet[1]);
	auto cursor = isearch(*query);
	range_ptr * ptr = new range_ptr(*query, cursor);
	get_items(ptr, data);
	if (!ptr->cursor.valid()) {
		delete ptr;
	}
}

void update_range(packet_t * packet, char * data) {
	range_ptr * ptr = (range_ptr*)(&packet[1]);
	get_items(ptr, data);
	if (!ptr->cursor.valid()) {
		delete ptr;
	}
}

void * processor(void * args) {
	LARM::LARM_NET::LARM_NET_BASE::task_arg * conn =
		(LARM::LARM_NET::LARM_NET_BASE::task_arg*)args;
	packet_t * packet = (packet_t*)conn->req;
	switch (packet->tp) {
	case packet_t::INSERT: {
		SKEY * key = (SKEY*)(&packet[1]);
		item_t * value = (item_t*)larm_malloc(sizeof(item_t));
		*value = *(item_t*)(&key[1]);
		iinsert(*key, *value);
		conn->ans = true;
		break;
	}
	case packet_t::REMOVE: {
		SKEY * key = (SKEY*)(&packet[1]);
		iremove(*key);
		conn->ans = false;
		break;
	}
	case packet_t::RANGE: {
		process_range(packet, conn->res);
		conn->ans = false;
		break;
	}
	case packet_t::GETRANGE:
		update_range(packet, conn->res);
		break;
	}
	return NULL;
}


