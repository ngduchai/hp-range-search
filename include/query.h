#ifndef QUERY_H
#define QUERY_H

#include <common.h>
#include <vector>

/* Query code */
class rcode_t {
public:
	uintptr_t code;
	uint64_t checksum;
	uint32_t num;
	bool has_next;
};

template <typename KEY, typename VALUE, typename QUERY>
class dbint {
protected:
	class cursor_imp {
	protected:
		QUERY _query;
		bool _valid = false;
		rcode_t _code;
	public:
		cursor_imp(const QUERY& query) : _query(query) {};
		virtual cursor_imp * clone() = 0;
		virtual const KEY key() = 0;
		virtual const VALUE value() = 0;
		virtual bool next() = 0;
		virtual bool valid() = 0;
		virtual ~cursor_imp() {};
		virtual char * buffer() = 0;
	};

public:
	class cursor {
	protected:
		cursor_imp * impl;
	public:
		cursor(cursor_imp * impl) : impl(impl) {};
		~cursor() { delete impl; }
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

	virtual cursor find(const QUERY&) = 0;
	virtual bool find(const KEY&, std::vector<VALUE>&) {
		return true;
	}
	virtual bool insert(const KEY&, const VALUE&) = 0;
	virtual bool remove(const KEY&) = 0;

};

#endif




