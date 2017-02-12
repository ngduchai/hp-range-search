#ifndef INDEX_H
#define INDEX_H

#include <vector>

/* The iterator over the index structure */

/* Index structure */
template <typename KEY, typename VALUE, typename QUERY>
class indexstr_t {
protected:
	class cursor_imp {
	protected:
		/* Return iterator must belong to a given range */
		QUERY _query;
		bool _valid = false;
		cursor_imp() = delete;
	public:
		cursor_imp(const QUERY& query) : _query(query) {};
		virtual cursor_imp * clone() const = 0;
		virtual const std::pair<KEY, VALUE>& operator*() = 0;
		virtual const std::pair<KEY, VALUE>* operator->() = 0;
		virtual const KEY key() = 0;
		virtual const VALUE value() = 0;
		virtual bool next() = 0;
		virtual bool valid() = 0;
		virtual ~cursor_imp() {};
	};

public:
	
	class cursor {
	protected:
		cursor_imp * impl;
	public:
		cursor(cursor_imp * impl) : impl(impl) {};
		~cursor() { delete impl; };
		cursor(const cursor &that) : impl(that.impl->clone()) {}
		cursor(cursor &&that) { swap(*this, that); }

		friend void swap(cursor& first, cursor& second) {
			using std::swap;
			swap(first.impl, second.impl);
		}

		cursor& operator=(const cursor that) {
			swap(*this, that);
			return *this;
		}
		
		inline const std::pair<KEY, VALUE> &operator*() const {
			return *impl;
		}

		inline bool next() { return impl->next(); }
		inline bool valid() { return impl->valid(); }
		inline KEY key() { return impl->key(); }
		inline VALUE value() { return impl->value(); }

	};

	/* Return a iterator that point to a item whose key belongs to
	 * the given range. */
	virtual cursor find(const QUERY&) = 0;
	
	/* Return a list of items whose key equal to the given key */
	virtual bool find(const KEY& key, std::vector<VALUE>& values) = 0;

	/* Insert a key-value pair */
	virtual bool insert(const KEY&, const VALUE&) = 0;

	/* Delete an item given a key */
	virtual bool remove(const KEY&) = 0;


};

#endif





