#ifndef BPTREE_H
#define BPTREE_H

#include "index.h"
#include "common.h"
#include "bwtree/bwtree.h"

/* The wrapper of BwTree */
template <typename KEY, typename VALUE, typename RANGE>
class bptree : public indexstr_t<KEY, VALUE, RANGE, typename
	       BwTree<KEY, VALUE, key_comparator<KEY>,
	       key_equality_checker<KEY>>::ForwardIterator> {

	using Tree = BwTree<KEY, VALUE, key_comparator<KEY>,
	      key_equality_checker<KEY>>;
	typedef typename Tree::index_iter iter;
	
private:
	Tree _tree;
public:
	iter inline find(const RANGE& range) {
		return iter(range, _tree.Begin(range.start));
	}

	bool inline find_next(iter& fi) {
		fi.iter++;
		return fi.iter.isEnd() || (fi.iter->first <= fi.range.end);
	}
	
	bool inline insert(const KEY& key, const VALUE& value) {
		_tree.Insert(key, value);
	}

	bool inline remove(const KEY& key) {
		std::vector<VALUE> values;
		_tree.GetValue(key, values);
		for (auto item : values) {
			_tree.Delete(key, item);
		}
	}

};



#endif


