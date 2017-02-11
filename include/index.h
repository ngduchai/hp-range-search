#ifndef INDEX_H
#define INDEX_H

#include <vector>

/* The iterator over the index structure */

/* Index structure */
template <typename KEY, typename VALUE, typename RANGE, typename ITERATOR>
class indexstr_t {
public:
	class index_iter {
	private:
		/* Return iterator must belong to a given range */
		RANGE range;
		/* Iterator for retrieving data. The actual
		 * type of the paointer depends on the
		 * implemtation of the index structure that it point to */
		ITERATOR iter;
	public:
		index_iter(RANGE& range, ITERATOR iter) :
			range(range), iter(iter) {};

		inline VALUE value() { return iter->second; };
		inline VALUE key() { return iter->first; };
	
	};
	
	/* Return a iterator that point to a item whose key belongs to
	 * the given range. */
	index_iter virtual find(const RANGE&) {
		return index_iter{};
	}
	/* If there are items satisfying the range request in the iterator,
	 * move the iterator by one ahead and return true.
	 * If not, return false. */
	virtual bool find_next(index_iter&) {};

	/* Insert a key-value pair */
	virtual bool insert(const KEY&, const VALUE&) {};

	/* Delete an item given a key */
	virtual bool remove(const KEY&) {};


};

#endif





