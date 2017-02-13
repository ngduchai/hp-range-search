
#ifndef LARM_DEFINE_H
#define LARM_DEFINE_H

#include <stdint.h>
#include <cstdlib>

/* Namespace macros */
#define LARM		larm 		// Public namespace for the project
#define INLARM		in_larm		// Private namespace for internal
					// objects
#define LARM_INS	instance	// namespace for LARM instance
#define LARM_CORD	coordinator	// namespace for LARM coordinator
#define LARM_COMMON	common 		// namespace for shared utilities

#define LARM_MM		memory		// namespace for memory management

#define LARM_HASH	hashing		// namespace for hash utilities

#define LARM_NET	net		// namespace for network modules

/* A Global Pointer has 3 components: 
 * 	- Node ID 	--> 24 bits
 * 	- Region ID	--> 10 bits
 * 	- Local address	--> 30 bits
 * Layout:
 * 	| Node ID    | Region ID | Local address |
 * */
#define LARM_REGION_MASK	0x3ffUL		// Region ID has 10 bits long
#define LARM_NODE_MASK		0xffffffUL	// Node ID has 24 bits long

#define LARM_GET_NODE(M)	((M >> 40) & LARM_NODE_MASK)
#define LARM_GET_REGION(M)	(((M & (LARM_REGION_MASK << 30)) >> 30) - 1)
#define LARM_GET_REMOTE_ADDR(M)	(M & 0x3ffffffffffUL)

#define LARM_CACHE_SIZE		64
#define LARM_REGION_SIZE	(1UL << 30)

namespace LARM {

typedef char* BYTE;
typedef uintptr_t lptr;
typedef uint32_t nodeid_t;

}
#endif


