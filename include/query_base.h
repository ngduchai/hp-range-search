#ifndef QUERY_BASE_H
#define QUERY_BASE_H

#include "query.h"

template <typename KEY, typename VALUE, typename QUERY>
class dbint_base : public dbint<KEY, VALUE, QUERY> {
	using dbInt = dbint<KEY, VALUE, QUERY>;

protected:
	class cursor_imp : public dbInt::cursor_imp {
	protected:
	public:
		cursor_imp(const QUERY& query) : dbInt::cursor_imp(query) {}
		cursor_imp(const cursor_imp& that) {
		
		}

		inline cursor_imp * clone() const {
			return new cursor_imp(*this);
		}
		inline const std::pair<KEY, VALUE>& operator*() {
		
		}
		inline const std::pair<KEY, VALUE>& operator->() {
		
		}
		const KEY inline key() {}
		const VALUE inline value() {}
		bool inline valid() { return this->_valid; }
		bool next();
		~cursor_imp() {};

	};

public:
	typename dbInt::cursor inline find(const QUERY& query) {
	
	}
	
	bool inline find(const KEY&, std::vector<VALUE>&) {
		return true;
	};

	bool insert(const KEY& key, const VALUE& value) {
	
	}

	bool remove(const KEY& key) {
	
	}


};


#endif





