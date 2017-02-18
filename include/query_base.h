#ifndef QUERY_BASE_H
#define QUERY_BASE_H

#include "query.h"
#include "common.h"
#include "mm.h"
#include "net.h"

#define BUFFER_IMP_SIZE		(1 << 20)

using namespace LARM::INLARM::LARM_MM;
using namespace LARM::LARM_NET;

template <typename KEY, typename VALUE, typename QUERY>
class dbint_base : public dbint<KEY, VALUE, QUERY> {
	using dbInt = dbint<KEY, VALUE, QUERY>;

protected:
	class cursor_imp : public dbInt::cursor_imp {
	protected:
		char * _buffer[2];
		int _buffer_index = 0;
		int _index = 0;
	public:
		cursor_imp(const QUERY& query) : dbInt::cursor_imp(query) {
			_buffer[0] = (char*)larm_malloc(BUFFER_IMP_SIZE);
			_buffer[1] = (char*)larm_malloc(BUFFER_IMP_SIZE);
			_buffer_index = 0;
			((rcode_t*)(_buffer[_buffer_index]))->code = 0;
			((rcode_t*)(_buffer[_buffer_index]))->num = 0;
		}
		cursor_imp(const cursor_imp& that) {
			/*
			_buffer[0] = (char*)larm_malloc(BUFFER_IMP_SIZE);
			_buffer[1] = (char*)larm_malloc(BUFFER_IMP_SIZE);
			memcpy(_buffer[0], that._buffer[0], BUFFER_IMP_SIZE);
			memcpy(_buffer[1], that._buffer[1], BUFFER_IMP_SIZE);
			*/
			this->_buffer = that._buffer;
			this->_query = that._query;
			this->_valid = that._valid;
			this->_code = that._code;
			this->_buffer_index = that._buffer_index;
			this->_index = that._index;
		}
		cursor_imp(cursor_imp &&that) { swap(*this, that); }

		friend void swap(cursor_imp& first, cursor_imp &second) {
			using std::swap;
			swap(first._buffer, second._buffer);
			swap(first._query, second._query);
			swap(first._valid, second._valid);
			swap(first._code, second._code);
			swap(first._buffer_index, second._buffer_index);
			swap(first._wait_query, second._wait_query);
		}

		cursor_imp& operator=(const cursor_imp that) {
			swap(*this, that);
			return *this;
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

		bool next() {
			_index++;
			if (_index == this->_code.num) {
				if (this->_code.has_next) {
					request_range();
					update_query();
					return true;
				}else{
					return false;
				}
			}
			return true;
		}

		~cursor_imp() {
			larm_free(_buffer[0]);
			larm_free(_buffer[1]);
		};

		inline char * buffer() {
			return _buffer[_buffer_index];
		}
	private:
		void update_query() {
			while (true) {
				rcode_t * code = (rcode_t*)this->buffer();
				while (code->code == 0 && code->num == 0) {
					
				}
				/* Check checksum */
				VALUE* values = (VALUE*)(&code[1]);
				uint64_t checksum = 0;
				for (uint32_t i = 0; i < code->num; ++i) {
					checksum ^= values->SKEY;
				}
				if (checksum == code->checksum) {
					this->_code = *code;
					break;
				}
			}
			this->_code = *(rcode_t*)(this->buffer());
			_index = 0;
		}

		void request_range() {
			uint32_t size = sizeof(this->_code.code);
			size += sizeof(packet_t);
			size += sizeof(uintptr_t);
			packet_t * packet = (packet_t*)larm_malloc(size);
			packet->tp = packet_t::GETRANGE;
			packet->size = size;
			uintptr_t * addr = (uintptr_t*)(&packet[1]);
			*addr = (uintptr_t)this->_code.code; addr++;
			*addr = (uintptr_t)this->buffer();
			send((char*)packet, size, 0);
			rcode_t * code = (rcode_t*)this->buffer();
			code->code = 0;
			code->num = 0;

		}

	};

public:
	typename dbInt::cursor inline find(const QUERY& query) {
		uint32_t size = sizeof(query);
		size += sizeof(packet_t);
		size += sizeof(char *);
		packet_t * packet = (packet_t*)larm_malloc(size);
		cursor_imp * cursor = new cursor_imp();
		packet->tp = packet_t::RANGE;
		packet->size = size;
		memcpy(&packet[1], query, sizeof(query));
		char * data = (char*)&packet[1];
		data += sizeof(query);
		*(uintptr_t*)data = (uintptr_t)cursor->buffer();
		send((char*)packet, size, 0);
		cursor->update_query();
		cursor->_valid = (cursor->_code->num == 0);
		return dbInt::cursor(cursor);
	}
	
	bool inline find(const KEY&, std::vector<VALUE>&) {
		return true;
	};

	bool insert(const KEY& key, const VALUE& value) {
		uint32_t size = value.size();
		size += sizeof(packet_t);
		size += sizeof(key);
		packet_t * packet = (packet_t*)larm_malloc(size);
		packet->tp = packet_t::INSERT;
		packet->size = size;
		memcpy(&packet[1], key, sizeof(key));
		char * data = (char*)&packet[1];
		data += sizeof(key);
		memcpy(data, value, value.size());
		send((char*)packet, size, (uint32_t)key);
		return true;
	}

	bool remove(const KEY& key) {
		uint32_t size = sizeof(key);
		size += sizeof(packet_t);
		packet_t * packet = (packet_t*)larm_malloc(size);
		packet->tp = packet_t::REMOVE;
		packet->size = size;
		memcpy(&packet[1], key, sizeof(key));
		send((char*)packet, size, (uint32_t)key);
		return true;
	}


};


#endif





