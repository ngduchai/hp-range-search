#ifndef LAYOUT_H
#define LAYOUT_H

#include "index.h"
#include "common.h"
#include <iostream>
#include <vector>

typedef uint64_t PKEY;
typedef uint64_t SKEY;

#define MIN_DATA_SIZE 64

/* Use BwTree */
#define USE_BPTREE

/* The real data */
class item_t {
public:
	PKEY pkey;
	SKEY skey;
	const uint32_t size;
	char data [MIN_DATA_SIZE - sizeof(pkey) -
		sizeof(skey) - sizeof(size)];
	item_t() : size(MIN_DATA_SIZE) {}
	item_t(uint32_t size) : size(size) {}
	item_t(PKEY pkey, SKEY skey, uint32_t size) :
		pkey(pkey), skey(skey), size(size) {}
	item_t(PKEY pkey, SKEY skey) :
		item_t(pkey, skey, MIN_DATA_SIZE) {}
};

using index_t = indexstr_t<SKEY, item_t*, rquery<SKEY>>;

bool isearch(const SKEY key, std::vector<item_t*>& value);

index_t::cursor isearch(const rquery<SKEY>& query);

bool iinsert(const item_t& record);

bool iremove(const SKEY& key);

#endif







