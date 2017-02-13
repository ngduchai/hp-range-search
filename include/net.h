
#ifndef NET_H
#define NET_H

#include "define.h"
#include "common.h"

namespace LARM {

namespace LARM_NET {

	/* Common interface */

	/* Setup network modules an open port at [port] to listen for
	 * new request from other peers.
	 * The caller must also provide a processor for handling data
	 * request from other node that the network module does not
	 * know how to deal with.
	 * */
	void start_net(int port, LARM::BYTE region, size_t size,
			void *(*processor)(void * arg), bool ap);
	
	/* Close connections. Data on current host will not be shared with
	 * other after this function have done its job. */
	void end_net();
	
	/* Create connection to a new host */
	void add_host(std::string hostname, std::string port);

	/* Read a remote item using RDMA read. If the operation is perform
	 * perperly, the item should be place somewhere in the shared
	 * memory region and the pointer to its location is return to
	 * the caller. */
	LARM::BYTE read(LARM::lptr ptr, size_t size);

	/* Write data to a remote region using RDMA write */
	bool write(LARM::BYTE data, LARM::lptr ptr, size_t size);

	/* Send some bytes to the node [node] */
	bool send(LARM::BYTE ptr, size_t size, uint32_t code);

	/* Send some bytes to node [node] */
	LARM::BYTE exchange(LARM::BYTE ptr, size_t size, uint32_t code);

	/* LARM client. Manage connection to a host in the system */
	class client {
	protected:
		nodeid_t _nodeid;
		client() {};
		client(const client& obj) = delete;
		client& operator= (const client& obj) = delete;
		/* NOTE: Client must hold the information about the
		 * shared memory of the remote host*/
	public:
		/* Open a connection to a given host:port. After create
		 * a new client, the _nodeid must hold a valid nodeid of
		 * the remote host. */
		//client(std::string hostname, std::string port) {};
		
		/* Read shared memory on the remote host */
		virtual LARM::BYTE read(LARM::BYTE ptr, size_t size)
			const = 0;

		/* Send message to the remote host */
		virtual bool send(LARM::BYTE ptr, size_t size) = 0;

		/* Send message to the remote host and wait for response */
		virtual LARM::BYTE exchange(LARM::BYTE ptr, size_t size) = 0;

		/* Get nodeid of the remote host that the client connects to */
		inline nodeid_t get_nodeid() const { return _nodeid; };

		virtual ~client() {};


	};

}

}

#endif // NET_H


