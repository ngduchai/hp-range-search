
#ifndef BNET_H
#define BNET_H

#include <rdma/rdma_cma.h>
#include <pthread.h>
#include "define.h"
#include "net.h"
#include <vector>
#include <list>
#include <iostream>

#define LARM_NET_BASE 	base
#define BUFFER_SIZE	(1 << 16)
#define NUM_WORKERS	12

#define NUM_QUEUES	8

namespace LARM {

namespace LARM_NET {

namespace LARM_NET_BASE {

	/* Baseline implementation of network modules */
	struct base_context {
		ibv_context * ctx = NULL;
		ibv_pd * pd;

		LARM::BYTE * data_regions;
		int num_regions;
		ibv_mr **data_mr;

		ibv_cq *cq [NUM_QUEUES];
		int aq = 0;
		ibv_comp_channel * comp_channel [NUM_QUEUES];

		pthread_t req_processor [NUM_QUEUES];
		void* (*req_function)(void * arg);
	};

	extern bool ap;

	struct base_connection {
		ibv_qp * qp;
		ibv_mr * send_mr;
		ibv_mr * recv_mr;
		LARM::BYTE send_buffer;
		LARM::BYTE recv_buffer;
	};
	
	struct task_arg {
		ibv_qp * qp;
		LARM::BYTE req;
		LARM::BYTE res;
		size_t size;
		bool change = false;
		bool wrt = true;
		uintptr_t addr;
	};

	struct base_task {
		void * (*routine) (void * args);
		task_arg arg;
		base_task * next_task;
	};

	extern int num_processors;
	extern pthread_t * processors;
	extern pthread_mutex_t * task_mutex;
	extern pthread_cond_t * task_cond_mutex;
	extern base_task ** ctasks;

	void add_task(base_task *task, int position);
	base_task * get_task(int position);
	void * process_task(void * task);

	void * process_conn_req(void * req);
	void * process_data_req(void * req);

	extern rdma_cm_id * listener;
	extern rdma_event_channel * ec;
	extern base_context * s_ctx;
	extern pthread_t conn_processor;
	
	extern bool base_start;

	void post_receive(base_connection *conn);
	void register_buffer(base_connection * conn); //, ibv_pd * pd);
	void build_qp_attr(ibv_qp_init_attr * attr);
	void build_context(ibv_context * verbs);
	void post_send(base_connection * conn, size_t size);
	int on_connect_request(rdma_cm_id * id);
	int on_established(rdma_cm_id * id);
	int on_disconnected(rdma_cm_id * id);

	void make_product(void *(function)(void * arg),
			base_connection * conn, size_t size);

	class base_client : public LARM::LARM_NET::client {
	protected:
		ibv_context * _ctx;
		ibv_pd * _pd;
		ibv_cq * _cq;
		ibv_comp_channel * _comp_channel;
		rdma_event_channel * _ec;
		rdma_cm_id * _conn;
		
		ibv_send_wr _rwr;
		ibv_sge _rsge;
		
		ibv_mr * region_akeys;
		size_t num_regions;
		
		LARM::BYTE buffer;
		ibv_mr * mr;

		void _on_addr_resolved(rdma_cm_id * id);
		void _on_route_resolved(rdma_cm_id * id);
		void _on_established(void * context);

		void _post_send(LARM::BYTE buff, size_t size) const;
		void _post_receive(LARM::BYTE data) const;

		std::list<std::pair<LARM::BYTE, uint32_t>> _send_data;
		size_t _send_size;

	public:
		void * data_regions;
		base_client(std::string hostname, std::string port);
		LARM::BYTE read(LARM::BYTE ptr, size_t size) const;
		bool write(LARM::BYTE data, LARM::BYTE ptr, size_t size) const;
		bool send(LARM::BYTE ptr, size_t size);
		void exchange(LARM::BYTE ptr, LARM::BYTE data, size_t size);
		~base_client();
		std::string hostname;
	};

	extern std::vector<base_client*> hosts;

	inline LARM::nodeid_t get_nodeid(uint32_t code) {
		//return code / (~((uint32_t)(0x0U)) / hosts.size());
		LARM::nodeid_t d = code % hosts.size();
		return d;
	}

}

}

}

#endif


