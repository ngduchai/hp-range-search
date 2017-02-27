
#include "procs.h"
#include "layout.h"
#include "bnet.h"
#include "mm.h"
#include "common.h"
#include "bptree.h"
#include "query_base.h"

#include <iostream>

using namespace LARM::INLARM::LARM_MM;

class range_ptr {
public:
	
	range_ptr(rquery<SKEY>& query,
			bptree<SKEY, item_t*, rquery<SKEY>>::cursor& cursor) :
	       		query(query), cursor(cursor), arg(NULL) {
		pthread_mutex_init(&ptr_mutex, NULL);
		pthread_cond_init(&ptr_cond_mutex, NULL);
	}
	~range_ptr() {
		pthread_mutex_destroy(&ptr_mutex);
		pthread_cond_destroy(&ptr_cond_mutex);
		if (arg != NULL) {
			larm_free(arg->res);
			delete arg;
		}
	}
	rquery<SKEY> query;
	bptree<SKEY, item_t*, rquery<SKEY>>::cursor cursor;
	pthread_mutex_t ptr_mutex;
	pthread_cond_t ptr_cond_mutex;
	LARM::LARM_NET::LARM_NET_BASE::task_arg * arg = NULL;
	bool has_items = false;
	int size = 0;
};

int get_items(range_ptr * ptr, char * data) {
	int size = sizeof(rcode_t);
	rcode_t * code = (rcode_t*)data;
	item_t * items =  (item_t*)(&code[1]);
	uint64_t checksum = 0;
	uint32_t num = 0;
	while (ptr->cursor.valid() && size < BUFFER_SIZE - 100) {
		*items = *ptr->cursor.value();
		checksum ^= items->skey;
		num++; items++; size += ptr->cursor.value()->size();
		ptr->cursor.next();
	}
	code->checksum = checksum;
	code->num = num;
	if (ptr->cursor.valid()) {
		code->code = (uintptr_t)ptr;
		code->has_next = true;
	}else{
		code->code = 1UL;
		code->has_next = false;
	}
	return size;

}

void * request_range(void * ptr) {
	LARM::LARM_NET::LARM_NET_BASE::task_arg * arg =
		(LARM::LARM_NET::LARM_NET_BASE::task_arg*)ptr;
	range_ptr * cursor = (range_ptr*)arg->req;
	pthread_mutex_lock(&cursor->ptr_mutex);
	cursor->size = get_items(cursor, cursor->arg->res);
	arg->res = (LARM::BYTE)larm_malloc(10);
	arg->req = (LARM::BYTE)larm_malloc(10);
	cursor->has_items = true;
	pthread_cond_signal(&cursor->ptr_cond_mutex);
	pthread_mutex_unlock(&cursor->ptr_mutex);
	return NULL;
}

int process_range(packet_t * packet, char * data) {
	rquery<SKEY> * query = (rquery<SKEY>*)(&packet[1]);
	auto cursor = isearch(*query);
	range_ptr * ptr = new range_ptr(*query, cursor);
	int size = get_items(ptr, data);
	if (!ptr->cursor.valid()) {
		delete ptr;
	}else{
		ptr->arg = new LARM::LARM_NET::LARM_NET_BASE::task_arg();
		ptr->arg->req = (LARM::BYTE)ptr;
		ptr->arg->res = (LARM::BYTE)larm_malloc(BUFFER_SIZE);
		ptr->arg->wrt = false;
		ptr->arg->size = 0;
		request_task(request_range, ptr->arg);
	}
	return size;
}

int update_range(packet_t * packet,
		LARM::LARM_NET::LARM_NET_BASE::task_arg * data) {
	range_ptr * ptr = (range_ptr*)(*(char**)(&packet[1]));
	pthread_mutex_lock(&ptr->ptr_mutex);
	while (!ptr->has_items) {
		pthread_cond_wait(&ptr->ptr_cond_mutex, &ptr->ptr_mutex);
	}
	ptr->has_items = false;
	LARM::BYTE temp = ptr->arg->res;
	ptr->arg->res = data->res;
	data->res = temp;
	pthread_mutex_unlock(&ptr->ptr_mutex);
	if (!ptr->cursor.valid()) {
		delete ptr;
	}else{
		request_task(request_range, ptr->arg);
	}
	return ptr->size;
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
		conn->size = update_range(packet, conn);
		conn->wrt = true;
		conn->addr = (uintptr_t)
			((char*)(&packet[1]) + sizeof(rquery<SKEY>));
		break;
	}
	return NULL;
}


