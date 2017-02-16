
#ifndef LARM_MM_H
#define LARM_MM_H

#include "define.h"

namespace LARM {

namespace INLARM {

namespace LARM_MM {
	
	#define NO_FREE_PTRS	(LARM_CACHE_SIZE / sizeof(atomic<BYTE>))
	/*
	#define NUM_SLOT_TYPES 15	// Maximum size is 2MB	
	const size_t SLOT_SIZE [NUM_SLOT_TYPES] =
		{64, 128, 256, 512, 1024, 2048, 4096, 8192, 16374,
		1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20};
	*/
	
	// 1.5
	#define NUM_SLOT_TYPES 19
	const size_t SLOT_SIZE [NUM_SLOT_TYPES] =
		{64, 96, 144, 224, 336, 512, 768, 1152, 1728,
		2592, 3888, 5840, 8768, 13152, 19728, 29600, 44400,
		66608, 99920};
	


	inline uint8_t get_lsize(size_t size) {
		for (int i = 0; i < NUM_SLOT_TYPES; ++i) {
			if (SLOT_SIZE[i] >= size) {
				return i;
			}
		}
		return NUM_SLOT_TYPES;
	}

	inline uint32_t get_ssz(uint8_t lsize) {
		return SLOT_SIZE[lsize];
	}

	/* Information of memory segment used to store data */
	struct mem_seg_t {
		BYTE seg;	// Pointer to the actual data segment
		uint32_t rkey;	// Key to remotely access to the segment
		uint32_t size;	// The actual size of the segment
		uint32_t shmid;	// the identifier of the segment in the
				// System V which is used to share this
				// segment among processes running on the
				// same computer
	};

	/* Allocate huge pages from local system and share
	 * them between local application using LARM as well
	 * as remote LARM. */
	void init_mem(unsigned int num_pages);

	/* Return a chunk of allocated memory to user */
	void * larm_malloc(size_t size);

	/* Take back a chunk of memory from user */
	void larm_free(void * ptr);
	
	/* Return a new slot for new shared item */
	void * larm_new_item(size_t size);

	/* Take back a shared item */
	void larm_delete_item(void * ptr);

	/* Release huge page back to the local system */
	void clean_mem(void);

	bool is_shared(const void * ptr);

	uintptr_t get_base();

	void check_mem();

}

}

}

#endif

