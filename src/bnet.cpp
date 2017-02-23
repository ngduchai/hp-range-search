
#include "common.h"
#include "net.h"
#include "bnet.h"
#include "layout.h"
#include "query.h"
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <mm.h>
#include <random>

using namespace LARM;
using namespace LARM::LARM_NET;
using namespace LARM::LARM_NET::LARM_NET_BASE;
using namespace LARM::INLARM::LARM_MM;
using namespace std;

#define TEST_Z(x) CHECK_RUNTIME(!(x), "Error: " #x " failed (Returned non-zero).")
#define TEST_NZ(x) CHECK_RUNTIME((x), "Error: " #x " failed (Returned null/zero).")

#define TIMEOUT_IN_MS	500
#define MAX_CODE 	(~(0x0U))

int LARM::LARM_NET::LARM_NET_BASE::num_processors;
pthread_t * LARM::LARM_NET::LARM_NET_BASE::processors;
pthread_mutex_t * LARM::LARM_NET::LARM_NET_BASE::task_mutex;
pthread_cond_t * LARM::LARM_NET::LARM_NET_BASE::task_cond_mutex;
base_task ** LARM::LARM_NET::LARM_NET_BASE::ctasks;

rdma_cm_id * LARM::LARM_NET::LARM_NET_BASE::listener = NULL;
rdma_event_channel * LARM::LARM_NET::LARM_NET_BASE::ec = NULL;
base_context * LARM::LARM_NET::LARM_NET_BASE::s_ctx = NULL;
pthread_t LARM::LARM_NET::LARM_NET_BASE::conn_processor;

bool LARM::LARM_NET::LARM_NET_BASE::base_start = false;

std::vector<base_client*> LARM::LARM_NET::LARM_NET_BASE::hosts;

std::vector<std::pair<string, string>> host_conn;

bool LARM::LARM_NET::LARM_NET_BASE::ap = false;

void LARM::LARM_NET::start_net(int port, LARM::BYTE region, size_t size,
		void *(*processor)(void * arg), bool ap) {

	sockaddr_in6 addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port  = htons(port);

	TEST_Z(ec = rdma_create_event_channel());
	TEST_NZ(rdma_create_id(ec, &listener, NULL, RDMA_PS_TCP));
	TEST_NZ(rdma_bind_addr(listener, (sockaddr*)&addr));
	TEST_NZ(rdma_listen(listener, 10));
	
	s_ctx = new base_context;
	s_ctx->num_regions = size;
	s_ctx->req_function = processor;
	s_ctx->ctx = NULL;
	
	s_ctx->data_regions = new LARM::BYTE [size];
	s_ctx->data_mr = new ibv_mr * [size]; 
	for (size_t i = 0; i < size; ++i) {
		s_ctx->data_regions[i] = region + i * LARM_REGION_SIZE;
	}

	cout << "Listening on port " << port << endl;
	
	pthread_create(&conn_processor, NULL, process_conn_req, NULL);
	
	/* Run worker threads */
	base_start = true;
	num_processors = NUM_WORKERS;
	
	processors = new pthread_t [num_processors];
	task_mutex = new pthread_mutex_t [num_processors];
	task_cond_mutex = new pthread_cond_t [num_processors];
	ctasks = new base_task * [num_processors];

	int *pti = new int [num_processors];

	LARM::LARM_NET::LARM_NET_BASE::ap = ap;

	for (int i = 0; i < num_processors; ++i) {
		pthread_mutex_init(&task_mutex[i], NULL);
		pthread_cond_init(&task_cond_mutex[i], NULL);
		ctasks[i] = NULL;
		pti[i] = i;
		pthread_create(&processors[i], NULL,
			process_task, (void*)&pti[i]);
	}
	/* Create connection to hosts */
	for (auto conn : host_conn) {
		hosts.push_back(new base_client(conn.first, conn.second));
		sleep(1);
	}
	host_conn.erase(host_conn.begin(), host_conn.end());
	
}

void LARM::LARM_NET::end_net() {
	/* Close conn_processor */
	pthread_cancel(conn_processor);
	pthread_join(conn_processor, NULL);
	rdma_destroy_id(listener);
	rdma_destroy_event_channel(ec);

	/* Clean workers */
	for (int i = 0; i < num_processors; ++i) {
		pthread_cancel(processors[i]);
		pthread_join(processors[i], NULL);
		pthread_mutex_destroy(&task_mutex[i]);
		pthread_cond_destroy(&task_cond_mutex[i]);
		base_task * task;
		while ((task = get_task(i)) != NULL) {
			delete task;
		}
	}

	/* Clean connection */
	for (auto conn : hosts) {
		delete conn;
	}
	hosts.erase(hosts.begin(), hosts.end());

	delete processors;
	delete task_mutex;
	delete task_cond_mutex;
	delete ctasks;

	/* Clean context */
	delete s_ctx->data_regions;
	delete s_ctx->data_mr;
	delete s_ctx;
}

void LARM::LARM_NET::add_host(std::string hostname, std::string port) {
	host_conn.push_back(pair<string, string>(hostname, port));
}

LARM::BYTE LARM::LARM_NET::read(LARM::lptr ptr, size_t size) {
	base_client * host = hosts[LARM_GET_NODE(ptr)];
	/*
	LARM::BYTE data = (LARM::BYTE)((ptr & 0x3fffffffUL) |
		((uintptr_t)host->data_regions & ~0x3fffffffUL));
	*/
	LARM::BYTE data = (LARM::BYTE)(ptr & 0xffffffffffUL);
	return host->read(data, size);
}

bool LARM::LARM_NET::write(LARM::BYTE data,
		LARM::lptr ptr, size_t size) {
	base_client * host = hosts[LARM_GET_NODE(ptr)];
	LARM::BYTE addr = (LARM::BYTE)(ptr & 0xffffffffffUL);
	return host->write(data, addr, size);
	
}

void LARM::LARM_NET::exchange(LARM::BYTE ptr, LARM::BYTE data,
		size_t size, uint32_t code) {
	hosts[get_nodeid(code)]->exchange(ptr, data, size);
}

bool LARM::LARM_NET::send(LARM::BYTE ptr, size_t size, uint32_t code) {
	return hosts[get_nodeid(code)]->send(ptr, size);
}

void LARM::LARM_NET::LARM_NET_BASE::post_receive(base_connection *conn) {
	ibv_recv_wr wr, *bad_wr = NULL;
	ibv_sge sge;

	wr.wr_id = (uintptr_t)conn;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;

	sge.addr = (uintptr_t)conn->recv_buffer;
	sge.length = BUFFER_SIZE;
	sge.lkey = conn->recv_mr->lkey;

	TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
}

void LARM::LARM_NET::LARM_NET_BASE::register_buffer(base_connection * conn) {
		//ibv_pd * pd) {
	/*
	conn->send_buffer = new char [BUFFER_SIZE];
	TEST_Z(conn->send_mr = ibv_reg_mr(
		pd,
		conn->send_buffer,
		BUFFER_SIZE,
		IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE
	));
	conn->recv_buffer = new char [BUFFER_SIZE];
	TEST_Z(conn->recv_mr = ibv_reg_mr(
		pd,
		conn->recv_buffer,
		BUFFER_SIZE,
		IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE
	));
	*/

	conn->send_buffer = (LARM::BYTE)larm_malloc(BUFFER_SIZE);
	conn->send_mr = s_ctx->data_mr[
		LARM_GET_REGION((uintptr_t)conn->send_buffer)];

	conn->recv_buffer = (LARM::BYTE)larm_malloc(BUFFER_SIZE);
	conn->recv_mr = s_ctx->data_mr[
		LARM_GET_REGION((uintptr_t)conn->recv_buffer)];
}

void LARM::LARM_NET::LARM_NET_BASE::build_qp_attr(
		ibv_qp_init_attr * qp_attr) {
	memset(qp_attr, 0, sizeof(*qp_attr));

	/* All the connection share the same cq */
	qp_attr->send_cq = s_ctx->cq[s_ctx->aq];
	qp_attr->recv_cq = s_ctx->cq[s_ctx->aq];
	s_ctx->aq = (s_ctx->aq + 1) % NUM_QUEUES;

	/* Using reliable transport */
	qp_attr->qp_type = IBV_QPT_RC;

	qp_attr->cap.max_send_wr = 10;
	qp_attr->cap.max_send_sge = 10;
	qp_attr->cap.max_recv_wr = 10;
	qp_attr->cap.max_recv_sge = 10;
}

void LARM::LARM_NET::LARM_NET_BASE::build_context(ibv_context * verbs) {
	if (s_ctx->ctx) {
		CHECK_RUNTIME(s_ctx->ctx != verbs,
			"Cannot handle events in more than one context");
		return;
	}
	s_ctx->ctx = verbs;
	s_ctx->pd = listener->pd;
	TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx));
	/*
	TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx));
	TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, 10, NULL,
				s_ctx->comp_channel, 0));
	TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0));
	*/
	
	for (int i = 0; i < s_ctx->num_regions; ++i) {
		TEST_Z(s_ctx->data_mr[i] = ibv_reg_mr(
			s_ctx->pd,
			s_ctx->data_regions[i],
			LARM_REGION_SIZE,
			IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ
		));
	}
	/* Begin processing request from other process */
	/*
	pthread_create(&s_ctx->req_processor, NULL,
		process_data_req, NULL);
	pthread_t req_processor[];
	pthread_create(&req_processor[0], NULL, process_data_req, NULL);
	pthread_create(&req_processor[1], NULL, process_data_req, NULL);
	pthread_create(&req_processor[2], NULL, process_data_req, NULL);
	*/
	
	if (!ap) {
		for (int i = 0; i < NUM_QUEUES; ++i) {
			TEST_Z(s_ctx->cq[i] = ibv_create_cq(s_ctx->ctx, 10,
				NULL, s_ctx->comp_channel[i], 0));
			pthread_create(&s_ctx->req_processor[i], NULL,
				process_data_req,(void*)s_ctx->cq[i]);
		}
	}

}

void LARM::LARM_NET::LARM_NET_BASE::post_send(base_connection *conn,
		size_t size) {
	ibv_send_wr wr, *bad_wr = NULL;
	ibv_sge sge;

	memset(&wr, 0, sizeof(wr));

	wr.opcode = IBV_WR_SEND;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.send_flags = IBV_SEND_SIGNALED;
	if (size < 32) {
		wr.send_flags |= IBV_SEND_INLINE;
	}

	sge.addr = (uintptr_t) conn->send_buffer;
	sge.length = size;
	sge.lkey = conn->send_mr->lkey;

	TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
}

int LARM::LARM_NET::LARM_NET_BASE::on_connect_request(rdma_cm_id * id) {
	ibv_qp_init_attr qp_attr;
	rdma_conn_param cm_params;
	base_connection * conn;

	cout << "Received a connection request. ID: " << (int*)id << endl;

	build_context(id->verbs);
	build_qp_attr(&qp_attr);

	TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr));

	id->context = conn = new base_connection;
	conn->qp = id->qp;

	register_buffer(conn); //, s_ctx->pd);
	post_receive(conn);

	memset(&cm_params, 0, sizeof(cm_params));

	cm_params.initiator_depth = cm_params.responder_resources = 1;
	cm_params.rnr_retry_count = 7;
	TEST_NZ(rdma_accept(id, &cm_params));

	return 0;
}


int LARM::LARM_NET::LARM_NET_BASE::on_established(rdma_cm_id * id) {
	/* Exchange keys */
	base_connection * conn = (base_connection *)id->context;
	cout << "Send key to " << (int*)id << endl;
	
	LARM::BYTE buffer = conn->send_buffer;
	*(uintptr_t*)buffer = (uintptr_t)s_ctx->data_regions[0];
	buffer += sizeof(uintptr_t);
	*(size_t*)buffer = s_ctx->num_regions;
	buffer += sizeof(size_t);
	int sz = sizeof(uintptr_t) + sizeof(size_t);
	for (int i = 0; i < s_ctx->num_regions; ++i) {
		*(ibv_mr*)buffer = *(s_ctx->data_mr[i]);
		buffer += sizeof(ibv_mr);
		sz += sizeof(ibv_mr);
	}
	post_send(conn, sz);

	return 0;
}

int LARM::LARM_NET::LARM_NET_BASE::on_disconnected(rdma_cm_id * id) {
	base_connection * conn = (base_connection*)id->context;
	cout << "Peer at ID " << (int*)id << " disconnected." << endl;

	rdma_destroy_qp(id);
	larm_free(conn->send_buffer);
	larm_free(conn->recv_buffer);
	delete conn;

	rdma_destroy_id(id);
	return 0;
	
}

void * LARM::LARM_NET::LARM_NET_BASE::process_conn_req(void * req) {
	rdma_cm_event * event, event_copy;

	while (true) {
		while (rdma_get_cm_event(ec, &event) == 0) {
			memcpy(&event_copy, event, sizeof(rdma_cm_event));
			rdma_ack_cm_event(event);
			int r = 0;
			switch (event_copy.event) {
				case RDMA_CM_EVENT_CONNECT_REQUEST:
					r = on_connect_request(event_copy.id);
					break;
				case RDMA_CM_EVENT_ESTABLISHED:
					r = on_established(event_copy.id);
					break;
				case RDMA_CM_EVENT_DISCONNECTED:
					r = on_disconnected(event_copy.id);
					break;
				default:
					CHECK_RUNTIME(1, "Unknow event");
			}
			if (r) {
				break;
			}
		}
	}

	return req;
}

void * LARM::LARM_NET::LARM_NET_BASE::process_data_req(void * req) {
	/*
	ibv_cq * cq;
	ibv_wc wc;

	while (1) {
		TEST_NZ(ibv_get_cq_event(s_ctx->comp_channel, &cq, &req));
		ibv_ack_cq_events(cq, 1);
		TEST_NZ(ibv_req_notify_cq(cq, 0));

		while (ibv_poll_cq(cq, 1, &wc)) {
			CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
				"Status is not IBV_WC_SUCCESS");
			if (wc.opcode & IBV_WC_RECV) {
				base_connection * conn =
					(base_connection*)wc.wr_id;
				make_product(s_ctx->req_function,
					conn, wc.byte_len);
				post_receive(conn);
			}
		}
	}
	*/
	
	ibv_cq * cq = (ibv_cq*)req;
	ibv_wc wc;
	while (true) {
		while (!ibv_poll_cq(cq, 1, &wc)) {
			/* Polling cq for a new wc */
		}
		/* Check the status of wc */
		
		CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
			"Status is not IBV_WC_SUCCESS");
		if (wc.opcode & IBV_WC_RECV) {
			base_connection * conn =
				(base_connection*)wc.wr_id;
			make_product(s_ctx->req_function,
				conn, wc.byte_len);
			post_receive(conn);
		}
	}
	return NULL;
		

}

void * LARM::LARM_NET::LARM_NET_BASE::process_task(void * task) {
	int index = *(int*)task;
	base_task * request;
	while (true) {
		pthread_mutex_lock(&task_mutex[index]);
		while (!(request = get_task(index))) {
			pthread_cond_wait(&task_cond_mutex[index],
				&task_mutex[index]);
		}
		pthread_mutex_unlock(&task_mutex[index]);
		/* Execute task */
		request->routine(&(request->arg));
		/* Get result to the host */
		if (request->arg.wrt) {
			ibv_send_wr wr, *bad_wr = NULL;
			ibv_sge sge;
			memset(&wr, 0, sizeof(wr));

			wr.opcode = IBV_WR_SEND;
			wr.sg_list = &sge;
			wr.num_sge = 1;
			wr.send_flags = IBV_SEND_SIGNALED;
			if (request->arg.size < 32) {
				wr.send_flags |= IBV_SEND_INLINE;
			}
			sge.addr = (uintptr_t)request->arg.res;
			sge.length = request->arg.size;
			sge.lkey = s_ctx->data_mr[LARM_GET_REGION(
				(uintptr_t)request->arg.res)]->lkey;
			
			TEST_NZ(ibv_post_send(request->arg.qp,
				&wr, &bad_wr));
		}
		larm_free(request->arg.res);
		larm_free(request->arg.req);
		delete request;
	}
}

int genindex() {
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(1, num_processors);
	return uni(rng) - 1;
}

void LARM::LARM_NET::LARM_NET_BASE::make_product(
		void * (*routine)(void * arg), base_connection * conn,
		size_t size) {

	int index = genindex();

	base_task * request = new base_task;
	request->routine = routine;
	request->arg.qp = conn->qp;
	request->arg.req = (LARM::BYTE)larm_malloc(size);
	memcpy(request->arg.req, conn->recv_buffer, size);
	request->arg.res = (LARM::BYTE)larm_malloc(BUFFER_SIZE);

	pthread_mutex_lock(&task_mutex[index]);
	add_task(request, index);
	pthread_cond_signal(&task_cond_mutex[index]);
	pthread_mutex_unlock(&task_mutex[index]);
}

void LARM::LARM_NET::LARM_NET_BASE::add_task(base_task * task, int position) {
	if (ctasks[position] == NULL) {
		ctasks[position] = task;
	}else{
		base_task * temp = ctasks[position];
		while (temp->next_task != NULL) {
			temp = temp->next_task;
		}
		temp->next_task = task;
		task->next_task = NULL;
	}
}

base_task * LARM::LARM_NET::LARM_NET_BASE::get_task(int position) {
	if (ctasks[position] == NULL) {
		return NULL;
	}else{
		base_task * temp = ctasks[position];
		ctasks[position] = ctasks[position]->next_task;
		temp->next_task = NULL;
		return temp;
	}
}

LARM::LARM_NET::LARM_NET_BASE::base_client::base_client(string hostname,
		string port) {

	addrinfo *addr;

	this->hostname = hostname;
	

	TEST_NZ(getaddrinfo(hostname.c_str(), port.c_str(), NULL, &addr));
	TEST_Z(_ec = rdma_create_event_channel());
	TEST_NZ(rdma_create_id(_ec, &_conn, NULL, RDMA_PS_TCP));
	TEST_NZ(rdma_resolve_addr(_conn, NULL,
			addr->ai_addr, TIMEOUT_IN_MS));

	freeaddrinfo(addr);

	
	rdma_cm_event * event, event_copy;
	while (rdma_get_cm_event(_ec, &event) == 0) {
		memcpy(&event_copy, event, sizeof(rdma_cm_event));
		rdma_ack_cm_event(event);
		int r = 0;
		switch (event_copy.event) {
		case RDMA_CM_EVENT_ADDR_RESOLVED:
			_on_addr_resolved(event_copy.id);
			break;
		case RDMA_CM_EVENT_ROUTE_RESOLVED:
			_on_route_resolved(event_copy.id);
			break;
		case RDMA_CM_EVENT_ESTABLISHED:
			_on_established(event_copy.id->context);
			r = 1;
			break;
		default:
			CHECK_RUNTIME(1, "Unknown event.");
		}
		if (r) {
			break;
		}
	}
	_send_data.erase(_send_data.begin(), _send_data.end());
	_send_size = 0;

}

void LARM::LARM_NET::LARM_NET_BASE::base_client::_on_addr_resolved(
		rdma_cm_id * id) {
	ibv_qp_init_attr qp_attr;

	build_context(id->verbs);
	_ctx = s_ctx->ctx;
	//TEST_Z(_pd = ibv_alloc_pd(_ctx));
	_pd = s_ctx->pd;
	TEST_Z(_comp_channel = ibv_create_comp_channel(_ctx));
	TEST_Z(_cq = ibv_create_cq(_ctx, 10, NULL, _comp_channel, 0));
	TEST_NZ(ibv_req_notify_cq(_cq, 0));

	memset(&qp_attr, 0, sizeof(qp_attr));

	qp_attr.send_cq = _cq;
	qp_attr.recv_cq = _cq;
	qp_attr.qp_type = IBV_QPT_RC;
	
	qp_attr.cap.max_send_wr = 10;
	qp_attr.cap.max_send_sge = 10;
	qp_attr.cap.max_recv_wr = 10;
	qp_attr.cap.max_recv_sge = 10;

	TEST_NZ(rdma_create_qp(id, _pd, &qp_attr));
	
	buffer = (LARM::BYTE)larm_malloc(BUFFER_SIZE);
	mr = s_ctx->data_mr[LARM_GET_REGION((uintptr_t)buffer)];

	_post_receive(buffer);

	
	TEST_NZ(rdma_resolve_route(id, TIMEOUT_IN_MS));
}

void LARM::LARM_NET::LARM_NET_BASE::base_client::_on_route_resolved(
		rdma_cm_id * id) {
	rdma_conn_param cm_params;

	memset(&cm_params, 0, sizeof(cm_params));
	cm_params.initiator_depth = cm_params.responder_resources = 1;
	cm_params.rnr_retry_count = 7;

	TEST_NZ(rdma_connect(id, &cm_params));

}

void LARM::LARM_NET::LARM_NET_BASE::base_client::_on_established(
		void * context) {
	/* Exchange key */
	//_post_receive(buffer);
	
	ibv_cq * cq;
	ibv_wc wc;

	TEST_NZ(ibv_get_cq_event(_comp_channel, &cq, &context));

	ibv_ack_cq_events(cq, 1);

	TEST_NZ(ibv_req_notify_cq(cq, 0));
	while (ibv_poll_cq(cq, 1, &wc)) {
		CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
			"Status is not IBV_WC_SUCCESS");
		if (wc.opcode & IBV_WC_RECV) {
			break;
		}
	}

	LARM::BYTE buff = buffer;
	data_regions = (void*)(*(uintptr_t*)buffer);
	buff += sizeof(uintptr_t);
	num_regions = *(size_t*)buff;
	buff += sizeof(size_t);
	region_akeys = new ibv_mr [num_regions];
	memcpy(region_akeys, buff, num_regions * sizeof(ibv_mr));
	cout << "Received key from " + hostname << endl;

}

void LARM::LARM_NET::LARM_NET_BASE::base_client::_post_send(
		LARM::BYTE buff, size_t size) const {
	ibv_send_wr wr, *bad_wr = NULL;
	ibv_sge sge;

	memset(&wr, 0, sizeof(wr));

	wr.opcode = IBV_WR_SEND;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.send_flags = IBV_SEND_SIGNALED;

	if (size < 32) {
		wr.send_flags |= IBV_SEND_INLINE;
	}

	sge.addr = (uintptr_t)buff;
	sge.length = size;
	sge.lkey = s_ctx->data_mr[LARM_GET_REGION((uintptr_t)buff)]->lkey;

	TEST_NZ(ibv_post_send(_conn->qp, &wr, &bad_wr));

}

void LARM::LARM_NET::LARM_NET_BASE::base_client::_post_receive(
		LARM::BYTE data) const {
	ibv_recv_wr wr, *bad_wr;
	ibv_sge sge;

	wr.wr_id = (uintptr_t)this;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	/*
	sge.addr = (uintptr_t)buffer;
	sge.length = BUFFER_SIZE;
	sge.lkey = mr->lkey;
	*/
	sge.addr = (uintptr_t)data;
	sge.length = BUFFER_SIZE;
	sge.lkey = s_ctx->data_mr[LARM_GET_REGION((uintptr_t)data)]->lkey;

	TEST_NZ(ibv_post_recv(_conn->qp, &wr, &bad_wr));
}

LARM::LARM_NET::LARM_NET_BASE::base_client::~base_client() {
	rdma_destroy_qp(_conn);
	larm_free(buffer);
	rdma_destroy_id(_conn);
	rdma_destroy_event_channel(_ec);
}

LARM::BYTE LARM::LARM_NET::LARM_NET_BASE::base_client::read(
		LARM::BYTE ptr, size_t size) const {
	
	ibv_send_wr wr, *bad_wr;
	ibv_sge sge;

	if (size > (1 << 15)) {
		throw runtime_error("size is too big");
	}

	memset(&wr, 0, sizeof(wr));
	int index = LARM_GET_REGION((uintptr_t)ptr);

	LARM::BYTE data = (LARM::BYTE)larm_new_item(size);
	
	uint8_t lsize = ((packet_t*)data)->size;

	wr.wr_id = (uintptr_t)this;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.send_flags = IBV_SEND_SIGNALED;
	wr.wr.rdma.remote_addr = (uintptr_t)ptr;
	wr.wr.rdma.rkey = region_akeys[index].rkey;

	sge.addr = (uintptr_t)data;
	sge.length = size;
	sge.lkey = s_ctx->data_mr[LARM_GET_REGION((uintptr_t)data)]->lkey;

	TEST_NZ(ibv_post_send(_conn->qp, &wr, &bad_wr));

	ibv_wc wc;

	while (true) {
		if (ibv_poll_cq(_cq, 1, &wc)) {
			CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
				"Status is not IBV_WC_SUCCESS");
			if (wc.opcode == IBV_WC_RDMA_READ) {
				while (ibv_poll_cq(_cq, 1, &wc)) {
				
				}
				break;
			}
		}
	}

	((packet_t*)data)->size = lsize;

	return data;
}

bool LARM::LARM_NET::LARM_NET_BASE::base_client::write(
		LARM::BYTE ptr, LARM::BYTE addr, size_t size) const {
	
	ibv_send_wr wr, *bad_wr;
	ibv_sge sge;

	if (size > (1 << 15)) {
		throw runtime_error("size is too big");
	}

	memset(&wr, 0, sizeof(wr));
	int index = LARM_GET_REGION((uintptr_t)addr);

	wr.wr_id = (uintptr_t)this;
	wr.opcode = IBV_WR_RDMA_WRITE;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.send_flags = IBV_SEND_SIGNALED;
	wr.wr.rdma.remote_addr = (uintptr_t)addr;
	wr.wr.rdma.rkey = region_akeys[index].rkey;

	sge.addr = (uintptr_t)ptr;
	sge.length = size;
	sge.lkey = s_ctx->data_mr[LARM_GET_REGION((uintptr_t)ptr)]->lkey;

	TEST_NZ(ibv_post_send(_conn->qp, &wr, &bad_wr));

	ibv_wc wc;

	while (true) {
		if (ibv_poll_cq(_cq, 1, &wc)) {
			CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
				"Status is not IBV_WC_SUCCESS");
			if (wc.opcode == IBV_WC_RDMA_WRITE) {
				while (ibv_poll_cq(_cq, 1, &wc)) {
				
				}
				break;
			}
		}
	}

	return true;
}

bool LARM::LARM_NET::LARM_NET_BASE::base_client::send(
		LARM::BYTE ptr, size_t size) {
	
	_post_send(ptr, size);
	
	ibv_wc wc;
	while (true) {
		if (ibv_poll_cq(_cq, 1, &wc)) {
			CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
				"Status is not IBV_WC_SUCCESS");
			if (wc.opcode == IBV_WC_SEND) {
				break;
			}
		}
	}
	return true;
	/*
	_send_data.push_back(pair<LARM::BYTE, uint32_t>(ptr, size));
	_send_size += size;
	return true;
	*/
}

void LARM::LARM_NET::LARM_NET_BASE::base_client::exchange(
		LARM::BYTE ptr, LARM::BYTE data, size_t size) {
	/*
	if (_send_size > 0) {
		size_t tsize = size + _send_size + sizeof(uint32_t);
		LARM::BYTE slots = (LARM::BYTE)larm_malloc(tsize);
		memcpy(slots, ptr, size);
		ptr = slots;
		slots += size;
		size = tsize;
		*(uint32_t*)slots = _send_data.size();
		slots += sizeof(uint32_t);
		for (auto data : _send_data) {
			memcpy(slots, data.first, data.second);
			slots += data.second;
		}
		((packet_t*)ptr)->adata = true;
	}else{
		((packet_t*)ptr)->adata = false;
	}
	*/
	_post_send(ptr, size);
	_post_receive(data);

	
	ibv_wc wc, wtc;
	
	while (true) {
		if (ibv_poll_cq(_cq, 1, &wc)) {
			CHECK_RUNTIME(wc.status != IBV_WC_SUCCESS,
				"Status is not IBV_WC_SUCCESS");
			if (wc.opcode & IBV_WC_RECV) {
				while (ibv_poll_cq(_cq, 1, &wtc)) {
				
				}
				break;
			}
		}
	}

	/*
	if (_send_size > 0) {
		for (auto data : _send_data) {
			larm_free(data.first);
		}
		larm_free(ptr);
		_send_data.erase(_send_data.begin(), _send_data.end());
		_send_size = 0;
	}
	
	size_t sz = wc.byte_len;
	if (sz == 0) {
		return NULL;
	}else{
		LARM::BYTE data = NULL;
		if (sz > sizeof(uint32_t)) {
			size_t size = get_ssz(
				((packet_t*)buffer)->size);
			data = (LARM::BYTE)larm_new_item(size);
		}else{
			data = (LARM::BYTE)larm_new_item(sz);
		}
		memcpy(data, buffer, sz);
		return data;
	}
	*/
}







