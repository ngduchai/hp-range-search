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
private:
	uint32_t sz = MIN_DATA_SIZE;
public:
	PKEY pkey;
	SKEY skey;
	char data [MIN_DATA_SIZE - sizeof(pkey) -
		sizeof(skey) - sizeof(sz)];
	item_t() : sz(MIN_DATA_SIZE) {}
	item_t(uint32_t size) : sz(size) {}
	item_t(PKEY pkey, SKEY skey, uint32_t size) :
		sz(size), pkey(pkey), skey(skey) {}
	item_t(PKEY pkey, SKEY skey) :
		item_t(pkey, skey, MIN_DATA_SIZE) {}
	item_t& operator=(const item_t& that) {
		pkey = that.pkey;
		skey = that.skey;
		sz = that.sz;
		return *this;
	}
	uint32_t size() const { return sz; }
};

using index_t = indexstr_t<SKEY, item_t*, rquery<SKEY>>;

bool isearch(const SKEY key, std::vector<item_t*>& value);

index_t::cursor isearch(const rquery<SKEY>& query);

bool iinsert(SKEY& key, const item_t& record);

bool iremove(const SKEY& key);



#endif







