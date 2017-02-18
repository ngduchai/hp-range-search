
#include "layout.h"

#ifdef USE_BPTREE
#include "bptree.h"
bptree<SKEY, item_t*, rquery<SKEY>> iindex;
#endif

bool isearch(const SKEY key, std::vector<item_t*>& value) {
	return iindex.find(key, value);
}

index_t::cursor isearch(const rquery<SKEY>& query) {
	return iindex.find(query);
}

bool iinsert(SKEY& key, const item_t& record) {
	return iindex.insert(key, (item_t* const)&record);
}

bool iremove(const SKEY& key) {
	return iindex.remove(key);
}




