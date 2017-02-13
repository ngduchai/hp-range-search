
#include "common.h"
#include "mm.h"
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <atomic>

#ifndef SHM_HUGE_1GB
#define SHM_HUGE_1GB (30 << SHM_HUGE_SHIFT)
#endif

#define DEFAULT_ADDR (0x0000000000000000UL)

using namespace LARM::INLARM::LARM_MM;
using namespace LARM::INLARM;
using namespace LARM;
using namespace std;

int num_segs = 0;
mem_seg_t * seg_info = NULL;


struct {
	BYTE base; // base pointer of the shared address space
	BYTE limit; // limit pointer of the shared address space
	BYTE current; // Pointer to the first free byte
	int region;
	
	/* We use the following struct for reuse memory slots
	 * that are allocated by then freed by the user */
	struct {
		/* Each element of the struct should contain a full
		 * cache line to avoid false sharing */

		/* Queue of free slot with the same size. The content
		 * of the slot itself is the pointer to the next 
		 * free slot */
		char _pad [LARM_CACHE_SIZE];

		litem_header* head; // Head, for one consumer at a time
		char _pad_head [LARM_CACHE_SIZE - sizeof(head)];

		atomic<bool> consumer_lock; // Lock shared between consumers
		char _pad_consumer [LARM_CACHE_SIZE - sizeof(consumer_lock)];
		
		litem_header* tail; // Tail, for one producer at a time
		char _pad_tail [LARM_CACHE_SIZE - sizeof(tail)];

		atomic<bool> producer_lock; // Lock shared between producers
		char _pad_producer [LARM_CACHE_SIZE - sizeof(producer_lock)];
		
	} free_ptrs [NUM_SLOT_TYPES];
	
	/* The lock used for protecting larm_malloc from race condition */
	atomic<bool> mlock;

} lmem_stat;
	
void LARM::INLARM::LARM_MM::init_mem(unsigned int num_pages) {
	/* We use huge pages to store data. We assume that the system has
	 * allocated huge pages for us in advance. We just allocate all
	 * available pages.
	 * */
	FILE * fp = popen("grep HugePages_Free /proc/meminfo", "r");
	CHECK_RUNTIME(fp == NULL, "Cannot access hugepage infomation");

	/* First we get the number of pages available */
	char path[200];
	fgets(path, sizeof(path) - 1, fp);
	pclose(fp);
	//num_segs = atoi(&path[22]);
	num_segs = num_pages;
	CHECK_RUNTIME(num_segs == 0, "There is no free hugepage.");

	/* Map hugepage to local address space */
	seg_info = new mem_seg_t [num_segs];
	for (int i = 0; i < num_segs; i++) {
		/* Get huge page identifier */
		seg_info[i].shmid = shmget(
			0,		// Key
			LARM_REGION_SIZE,	// 1G in size (default)
			/* Create a new page with read and write permission
			 * and use the hugepage pool */
			IPC_CREAT | 0666 | SHM_HUGETLB
		);
		CHECK_RUNTIME(
			seg_info[i].shmid < 0,
			"Cannot allocate huge page"
		);
		/* Attach huge to private address space */
		seg_info[i].seg = (BYTE)shmat(
			seg_info[i].shmid,  
			(void*)(DEFAULT_ADDR | ((lptr)(i + 1) << 30)),
			0
		);
		CHECK_RUNTIME(
			seg_info[i].seg == (BYTE) -1,
			"Cannot attach a new huge page to address space"
		);
		seg_info[i].size = LARM_REGION_SIZE;
	};

	/* Update the lmem_stat */
	lmem_stat.base = seg_info[0].seg;
	lmem_stat.limit = lmem_stat.base + num_segs * LARM_REGION_SIZE;
	lmem_stat.current = lmem_stat.base;
	lmem_stat.region = 1;

	/* Initilize 32MB for each slot type */
	const unsigned int INIT_SIZE = 1U << 25;
	for (int i = 0; i < NUM_SLOT_TYPES; ++i) {
		lmem_stat.free_ptrs[i].head = NULL;
		for (unsigned int j = 0; j < (INIT_SIZE / get_ssz(i)); ++j) {
			litem_header* p = (litem_header*)lmem_stat.current;
			lmem_stat.current += get_ssz(i);
			p->sig = 0x0;
			p->lsize = i;
			p->next_slot.store(NULL);
			if (lmem_stat.free_ptrs[i].head == NULL) {
				lmem_stat.free_ptrs[i].head =
					lmem_stat.free_ptrs[i].tail = p;
			}else{
				lmem_stat.free_ptrs[i].tail->next_slot = p;
				lmem_stat.free_ptrs[i].tail = p;
			}
			
		}
		/*
		int num_size = 0;
		for (litem_header * p = lmem_stat.free_ptrs[i].head; p != lmem_stat.free_ptrs[i].tail; p = p->next_slot) {
			num_size++;
		}
		cout << num_size << endl;
		*/
	}
	lmem_stat.mlock = false;

	/* Pin allocated segments to the NIC for communication */

}

void * LARM::INLARM::LARM_MM::larm_new_item(size_t size) {
	void * ptr = NULL;
	
	/* Determine slot size */
	int index = get_lsize(size);
	size_t msize = get_ssz(index);
	
	CHECK_RUNTIME(index >= NUM_SLOT_TYPES, 
			string("New object is too big: ") +
			std::to_string(size) + " byte(s)");
	
	while (lmem_stat.free_ptrs[index].consumer_lock.exchange(true)) {
		/* We want to support any number of thread calling this
		 * method, and let them run concurrently as much as
		 * possible. Therefore, we get the lock exclusivity on
		 * the head of the queue */
	}
	/* If the thread can reach this line, which means it is the only
	 * thread request an entry of free_ptr[index], we could let it
	 * update the head of head of the queue without worrying about
	 * conflict. However, there is potential race condition caused by
	 * the larm_free function which could lead to an update to the
	 * head pointer if the list is currently empty. */
	//cout << (int*)lmem_stat.free_ptrs[index].head << endl;
	//cout << msize << endl;
	//cout << (lmem_stat.free_ptrs[index].head->next_slot) << endl;
	litem_header * item = lmem_stat.free_ptrs[index].head->next_slot;
	/* NOTE: we consider a list containing 1 item as empty to ease
	 * the implementation */
	if (item != NULL) {
		/* The list is not emplty, so we could touch the head
		 * item without worrying about confliting with 
		 * threads calling larm_free function. (this function
		 * now should take care of other the tail which is
		 * obviously not the head because the queue is not
		 * empty). We first move the head out of the queue */
		ptr = (void*)lmem_stat.free_ptrs[index].head;
		lmem_stat.free_ptrs[index].head = item; // Update head
		/* Release the lock for other threads which ask for
		 * the same slot size */
		lmem_stat.free_ptrs[index].consumer_lock = false;
	}else{
		/* Release the lock on the list first */
		lmem_stat.free_ptrs[index].consumer_lock = false;
		/* The list is empty so we must get free memory from the
		 * memory pool. To do so, we must first acquire the pool
		 * lock.
		 * */
		while (lmem_stat.mlock.exchange(true)) {
			/* Wait until the acquiring the lock */
		}
		if (lmem_stat.current + msize >= lmem_stat.limit) {
			/* Do not enough space */
			ptr = NULL;
		}else{
			/* Allocate a new free slot */
			if (lmem_stat.current + msize >
					seg_info[lmem_stat.region].seg) {
				lmem_stat.current =
					seg_info[lmem_stat.region].seg;
				lmem_stat.region++;
			}
			ptr = lmem_stat.current;
			/* Update item size */
			((litem_header*)ptr)->lsize = index;
			lmem_stat.current += msize;
		}
		/* Release the lock for other thread */
		lmem_stat.mlock = false;
	}
	return ptr;
}

void * LARM::INLARM::LARM_MM::larm_malloc(size_t size) {
	void * data = larm_new_item(size + sizeof(litem_header));
	if (data != NULL) {
		data = (void *)(&((litem_header*)data)[1]);
	}
	return data;
}

void LARM::INLARM::LARM_MM::larm_delete_item(void * ptr) {
	/* Determine the index of the list where we will the the new free
	 * slot
	 * */
	int index = ((litem_header*)ptr)->lsize;
	/* Critical section should only used for update the queue. Changes
	 * on the data must be done outside */
	((litem_header*)ptr)->sig = 0x0; // Indicate that this slot is free
	((litem_header*)ptr)->next_slot = NULL;	// It is the last slot
						//in the list.
	/* Process on the list */
	while (lmem_stat.free_ptrs[index].producer_lock.exchange(true)) {
		/* Acquire the lock on the tail */
	}
	/* If thread could pass to this line, it must be the only one that
	 * is executing the following lines. The only thread that could
	 * interfere its execution is threads performing larm_malloc. However
	 * those threads only change the head so we only care about the case
	 * that the list is currently empty which lead to rare condtion.
	 * Fortunately, in such case, the larm_malloc cannot change the head
	 * before us since there is no free slot available. Therefore, we
	 * are free to update the tail.
	 * */
	lmem_stat.free_ptrs[index].tail->next_slot = (litem_header*)ptr;
	lmem_stat.free_ptrs[index].tail = (litem_header*)ptr;
	/* Release the lock for other thread to update the list */
	lmem_stat.free_ptrs[index].producer_lock = false;


}

void LARM::INLARM::LARM_MM::larm_free(void * ptr) {
	larm_delete_item(&((litem_header*)ptr)[-1]);
}

void LARM::INLARM::LARM_MM::clean_mem() {
	for (int i = 0; i < num_segs; i++) {
		shmctl(seg_info[i].shmid, IPC_RMID, NULL);
	}
}

bool LARM::INLARM::LARM_MM::is_shared(const void * ptr) {
	return ((uintptr_t)ptr >= (uintptr_t)lmem_stat.base) &&
		((uintptr_t)ptr < (uintptr_t)lmem_stat.limit);
}

uintptr_t LARM::INLARM::LARM_MM::get_base() {
	return (uintptr_t)lmem_stat.base;
}

void LARM::INLARM::LARM_MM::check_mem() {
	long size = 0;
	for (int i = 0; i < NUM_SLOT_TYPES; ++i) {
		litem_header * p = lmem_stat.free_ptrs[i].head;
		while (p != NULL) {
			size += SLOT_SIZE[i];
			p = p->next_slot;
		}
	}
	size += lmem_stat.limit - lmem_stat.current;
	cout << "Size: " << size << endl;
}




