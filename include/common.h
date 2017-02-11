
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

class itemkey {
private:
	char _data [KEY_SIZE];
public:
	itemkey() {}

	itemkey(uint64_t value) {
		((uint64_t*)_data)[0] = 0;
		((uint64_t*)_data)[1] = value;
	}

	itemkey(const char * value) {
		memcpy(_data, value, KEY_SIZE);
	}

	itemkey(const itemkey & that) {
		*this = that;
	};

	inline int comp_key(const itemkey& that) {
		uint64_t * tskey = (uint64_t*)this->_data;
		uint64_t * ttkey = (uint64_t*)that._data;
		uint64_t c = tskey[0] - ttkey[0];
		if (c == 0) {
			return tskey[1] - ttkey[1];
		}else{
			return c;
		}
	}

	bool operator==(const itemkey& that) {
		return (this->comp_key(that) == 0);
	}
	bool operator<(const itemkey& that) {
		return (this->comp_key(that) < 0);
	}
	bool operator>(const itemkey& that) {
		return (this->comp_key(that) > 0);
	}
	bool operator<=(const itemkey& that) {
		return (this->comp_key(that) <= 0);
	}
	bool operator>=(const itemkey& that) {
		return (this->comp_key(that) >= 0);
	}
	bool operator!=(const itemkey& that) {
		return (this->comp_key(that) != 0);
	}

	inline char * value() { return _data; }



};

/* Range query definition (a pair of keys) */
class rquery {
public:
	itemkey start;
	itemkey end;
	rquery() {};
	rquery(const itemkey &start, const itemkey &end) :
		start(start), end(end) {};
};



#endif


