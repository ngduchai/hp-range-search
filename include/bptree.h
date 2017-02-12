#ifndef BPTREE_H
#define BPTREE_H

#include "index.h"
#include "common.h"
#include "bwtree/bwtree.h"

/* The wrapper of BwTree */
template <typename KEY, typename VALUE, typename QUERY>
class bptree : public indexstr_t<KEY, VALUE, QUERY> {

	using Tree = BwTree<KEY, VALUE, key_comparator<KEY>,
	      key_equality_checker<KEY>>;
	using Index = indexstr_t<KEY, VALUE, QUERY>;
	
private:
	Tree _tree;

	class cursor_imp : public Index::cursor_imp {
	private:
		typename Tree::ForwardIterator ptr;
		Tree * _index;
	public:
		cursor_imp(const QUERY& query, Tree * index) :
				Index::cursor_imp(query), _index(index) {
			ptr = _index->Begin(query.start);
			this->_valid = !ptr.IsEnd();
		}

		cursor_imp(const cursor_imp &that) :
				Index::cursor_imp(that) {
			this->ptr = that.ptr;
			this->_index = that._index;
		}

		inline cursor_imp * clone() const { 
			return new cursor_imp(*this);
		}

		inline const std::pair<KEY, VALUE>& operator*() {
			return *ptr;	
		}

		inline const std::pair<KEY, VALUE>* operator->() {
			return & *ptr;
		}

		inline const KEY key() { return ptr->first; }
		inline const VALUE value() { return ptr->second; }

		inline bool next() {
			ptr++;
			this->_valid = !ptr.IsEnd() &&
				ptr->first <= this->_query.end;
			return this->_valid;
		}

		inline bool valid() { return this->_valid; }

		~cursor_imp() {};

	};

public:
	
	inline typename Index::cursor find(const QUERY& query) { 
		return typename Index::cursor(new cursor_imp(query, &_tree));
	}

	inline bool insert(const KEY& key, const VALUE& value) {
		return _tree.Insert(key, value);
	}

	inline bool remove(const KEY& key) {
		std::vector<VALUE> values;
		_tree.GetValue(key, values);
		if (values.size() == 0) {
			return false;
		}
		for (auto item : values) {
			_tree.Delete(key, item);
		}
		return true;
	}

	inline bool find(const KEY& key, std::vector<VALUE>& values) {
		_tree.GetValue(key, values);
		return (values.size() > 0);
	}

};



#endif


