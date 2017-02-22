
#include "procs.h"
#include "layout.h"
#include "bnet.h"
#include "mm.h"
#include "common.h"
#include "bptree.h"
#include "query_base.h"

#include <iostream>

using namespace LARM::INLARM::LARM_MM;

struct range_ptr {
	range_ptr(rquery<SKEY>& query,
		bptree<SKEY, item_t*, rquery<SKEY>>::cursor& cursor) :
	       	query(query), cursor(cursor) {}	
	rquery<SKEY> query;
	bptree<SKEY, item_t*, rquery<SKEY>>::cursor cursor;
};

int get_items(range_ptr * ptr, char * data) {
	int size = sizeof(rcode_t);
	rcode_t * code = (rcode_t*)data;
	item_t * items =  (item_t*)(&code[1]);
	uint64_t checksum = 0;
	uint32_t num = 0;
	auto cursor = ptr->cursor;
	while (cursor.valid() && size < BUFFER_SIZE * 0.7) {
		*items = *cursor.value();
		checksum ^= items->skey;
		num++; items++; size += cursor.value()->size();
		cursor.next();
	}
	code->checksum = checksum;
	code->num = num;
	std::cout << items[-1].size() << std::endl; 
	if (cursor.valid()) {
		code->code = (uintptr_t)ptr;
		code->has_next = true;
	}else{
		code->code = 1UL;
		code->has_next = false;
	}
	return size;

}

int process_range(packet_t * packet, char * data) {
	rquery<SKEY> * query = (rquery<SKEY>*)(&packet[1]);
	auto cursor = isearch(*query);
	range_ptr * ptr = new range_ptr(*query, cursor);
	int size = get_items(ptr, data);
	if (!ptr->cursor.valid()) {
		delete ptr;
	}
	return size;
}

int update_range(packet_t * packet, char * data) {
	range_ptr * ptr = (range_ptr*)(&packet[1]);
	int size = get_items(ptr, data);
	if (!ptr->cursor.valid()) {
		delete ptr;
	}
	return size;
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
		conn->wrt = true;
		conn->size = sizeof(uint64_t);
		*(uint64_t*)conn->res = 0UL;
		break;
	}
	case packet_t::REMOVE: {
		SKEY * key = (SKEY*)(&packet[1]);
		iremove(*key);
		conn->wrt = true;
		conn->size = sizeof(uint64_t);
		*(uint64_t*)conn->res = 0UL;
		break;
	}
	case packet_t::RANGE: {
		conn->size = process_range(packet, conn->res);
		conn->wrt = true;
		conn->addr = (uintptr_t)
			((char*)(&packet[1]) + sizeof(rquery<SKEY>));
		break;
	}
	case packet_t::GETRANGE:
		conn->size = update_range(packet, conn->res);
		conn->wrt = true;
		conn->addr = (uintptr_t)
			((char*)(&packet[1]) + sizeof(rquery<SKEY>));
		break;
	}
	return NULL;
}


