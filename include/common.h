
#ifndef COMMON_H
#define COMMON_H

/* Key size */
#define KEY_SIZE 16

/* Item size */
#define ITEM_SIZE 64

/* Data size */
#define DATA_SIZE (ITEM_SIZE - KEY_SIZE - KEY_SIZE)

#include <iostream>
#include "string.h"

typedef char * BYTE;

/* Item wrapper, just points to item's address, does not actually
 * contain data */


typedef uint64_t key64_t;

class key128_t {
private:
	char _data [128];
public:
	key128_t() {}

	key128_t(uint64_t value) {
		((uint64_t*)_data)[0] = 0;
		((uint64_t*)_data)[1] = value;
	}

	key128_t(const char * value) {
		memcpy(_data, value, 16);
	}

	key128_t(const key128_t & that) {
		*this = that;
	};

	inline int comp_key(const key128_t& that) {
		uint64_t * tskey = (uint64_t*)this->_data;
		uint64_t * ttkey = (uint64_t*)that._data;
		uint64_t c = tskey[0] - ttkey[0];
		if (c == 0) {
			return tskey[1] - ttkey[1];
		}else{
			return c;
		}
	}

	bool operator==(const key128_t& that) {
		return (this->comp_key(that) == 0);
	}
	bool operator<(const key128_t& that) {
		return (this->comp_key(that) < 0);
	}
	bool operator>(const key128_t& that) {
		return (this->comp_key(that) > 0);
	}
	bool operator<=(const key128_t& that) {
		return (this->comp_key(that) <= 0);
	}
	bool operator>=(const key128_t& that) {
		return (this->comp_key(that) >= 0);
	}
	bool operator!=(const key128_t& that) {
		return (this->comp_key(that) != 0);
	}

	inline char * value() { return _data; }

};

/* Range query definition (a pair of keys) */
template <typename KEY>
class rquery {
public:
	KEY start;
	KEY end;
	rquery() {};
	rquery(const KEY &start, const KEY &end) :
		start(start), end(end) {};
};

/* Compare two key. */
template <typename KEY>
class key_comparator {
public:
	inline bool operator() (const KEY& k1, const KEY& k2) const {
		return k1 < k2;
	}
	key_comparator() = delete;
};

/* Check if two key is equal */
template <typename KEY>
class key_equality_checker {
public:
	inline bool operator() (const KEY& k1, const KEY& k2) const {
		return (k1 == k2);
	}
	key_equality_checker() = delete;
};



#endif


