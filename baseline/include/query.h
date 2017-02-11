
#ifndef QUERY_H
#define QUERY_H

#include "common.h"

/* Range query definition (a pair of keys */
class rquery {
public:
	itemkey start;
	itemkey end;
	rquery() {};
	rquery(const itemkey& start, const itemkey& end) :
		start(start), end(end) {};
};




#endif


