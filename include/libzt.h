/*
 * ZeroTier SDK - Network Virtualization Everywhere
 * Copyright (C) 2011-2019  ZeroTier, Inc.  https://www.zerotier.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial closed-source software that incorporates or links
 * directly against ZeroTier software without disclosing the source code
 * of your own application.
 */

/**
 * @file
 *
 * ZeroTier socket API
 */

#ifndef LIBZT_H
#define LIBZT_H

#ifdef _WIN32
	#ifdef ADD_EXPORTS
		#define ZT_SOCKET_API __declspec(dllexport)
	#else
		#define ZT_SOCKET_API __declspec(dllimport)
	#endif
	#define ZTCALL __cdecl
#else
	#define ZT_SOCKET_API
	#define ZTCALL
#endif

#include <stdint.h>
#include <vector>

#if !defined(_WIN32) && !defined(__ANDROID__)
typedef unsigned int socklen_t;
//#include <sys/socket.h>
#else
typedef int socklen_t;
//#include <sys/socket.h>
#endif

#if defined(_WIN32)
#include <WinSock2.h>
#include <stdint.h>
#include <WS2tcpip.h>
#include <Windows.h>
#endif

/*
#ifdef _USING_LWIP_DEFINITIONS_
#include "lwip/sockets.h"
#endif
*/

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

namespace ZeroTier {

#ifdef __cplusplus
extern "C" {
#endif

// Custom errno to prevent conflicts with platform's own errno
extern int zts_errno;
typedef int zts_err_t;

#ifdef __cplusplus
}
#endif

/**
 * The system port upon which ZT traffic is sent and received
 */
#define ZTS_DEFAULT_PORT 9994

//////////////////////////////////////////////////////////////////////////////
// Control API error codes                                                  //
//////////////////////////////////////////////////////////////////////////////

#define ZTS_ERR_OK                          0 // Everything is ok
#define ZTS_ERR_INVALID_ARG                -1 // A parameter provided by the user application is invalid (e.g. our of range, NULL, etc)
#define ZTS_ERR_SERVICE                    -2 // The service isn't initialized or is for some other reason currently unavailable
#define ZTS_ERR_INVALID_OP                 -3 // For some reason this API operation is not permitted (perhaps the service is still starting?)

//////////////////////////////////////////////////////////////////////////////
// Control API event codes                                                  //
//////////////////////////////////////////////////////////////////////////////

#define ZTS_EVENT_NONE                     -1
#define ZTS_EVENT_NODE_UP                   0
// Standard node events
#define ZTS_EVENT_NODE_OFFLINE              1
#define ZTS_EVENT_NODE_ONLINE               2
#define ZTS_EVENT_NODE_DOWN                 3
#define ZTS_EVENT_NODE_IDENTITY_COLLISION   4
// libzt node events
#define ZTS_EVENT_NODE_UNRECOVERABLE_ERROR  16
#define ZTS_EVENT_NODE_NORMAL_TERMINATION   17
// Network-specific events
#define ZTS_EVENT_NETWORK_NOT_FOUND         32
#define ZTS_EVENT_NETWORK_CLIENT_TOO_OLD    33
#define ZTS_EVENT_NETWORK_REQUESTING_CONFIG 34
#define ZTS_EVENT_NETWORK_OK                35
#define ZTS_EVENT_NETWORK_ACCESS_DENIED     36
#define ZTS_EVENT_NETWORK_READY_IP4         37
#define ZTS_EVENT_NETWORK_READY_IP6         38
#define ZTS_EVENT_NETWORK_DOWN              39
//
#define ZTS_EVENT_NETWORK_STACK_UP          48
#define ZTS_EVENT_NETWORK_STACK_DOWN        49

// lwIP netif events
#define ZTS_EVENT_NETIF_UP_IP4              64
#define ZTS_EVENT_NETIF_UP_IP6              65
#define ZTS_EVENT_NETIF_DOWN_IP4            66
#define ZTS_EVENT_NETIF_DOWN_IP6            67
#define ZTS_EVENT_NETIF_REMOVED             68
#define ZTS_EVENT_NETIF_LINK_UP             69
#define ZTS_EVENT_NETIF_LINK_DOWN           70
#define ZTS_EVENT_NETIF_NEW_ADDRESS         71
// Peer events
#define ZTS_EVENT_PEER_P2P                  96
#define ZTS_EVENT_PEER_RELAY                97
#define ZTS_EVENT_PEER_UNREACHABLE          98

//////////////////////////////////////////////////////////////////////////////
// Common definitions and structures for interacting with the ZT socket API //
// This is a subset of lwip/sockets.h, lwip/arch.h, and lwip/inet.h         //
//                                                                          //
// These re-definitions exist here so that the user application's usage     //
// of the API is internally consistent with the underlying network stack.   //
// They have an attached prefix so that they can co-exist with the native   //
// platform's own definitions and structures.                               // 
//////////////////////////////////////////////////////////////////////////////

// Socket protocol types
#define ZTS_SOCK_STREAM     0x0001
#define ZTS_SOCK_DGRAM      0x0002
#define ZTS_SOCK_RAW        0x0003
// Socket family types
#define ZTS_AF_UNSPEC       0x0000
#define ZTS_AF_INET         0x0002
#define ZTS_AF_INET6        0x000a
#define ZTS_PF_INET         ZTS_AF_INET
#define ZTS_PF_INET6        ZTS_AF_INET6
#define ZTS_PF_UNSPEC       ZTS_AF_UNSPEC
// Protocol command types
#define ZTS_IPPROTO_IP      0x0000
#define ZTS_IPPROTO_ICMP    0x0001
#define ZTS_IPPROTO_TCP     0x0006
#define ZTS_IPPROTO_UDP     0x0011
#define ZTS_IPPROTO_IPV6    0x0029
#define ZTS_IPPROTO_ICMPV6  0x003a
#define ZTS_IPPROTO_UDPLITE 0x0088
#define ZTS_IPPROTO_RAW     0x00ff
// send() and recv() flags
#define ZTS_MSG_PEEK        0x0001
#define ZTS_MSG_WAITALL     0x0002 // NOT YET SUPPORTED
#define ZTS_MSG_OOB         0x0004 // NOT YET SUPPORTED
#define ZTS_MSG_DONTWAIT    0x0008
#define ZTS_MSG_MORE        0x0010
// fnctl() commands
#define ZTS_F_GETFL         0x0003
#define ZTS_F_SETFL         0x0004
// fnctl() flags
#define ZTS_O_NONBLOCK      0x0001
#define ZTS_O_NDELAY        0x0001
// Shutdown commands
#define ZTS_SHUT_RD         0x0000
#define ZTS_SHUT_WR         0x0001
#define ZTS_SHUT_RDWR       0x0002
// Socket level option number
#define ZTS_SOL_SOCKET      0x0fff
// Socket options
#define ZTS_SO_DEBUG        0x0001 // NOT YET SUPPORTED
#define ZTS_SO_ACCEPTCONN   0x0002
#define ZTS_SO_REUSEADDR    0x0004
#define ZTS_SO_KEEPALIVE    0x0008
#define ZTS_SO_DONTROUTE    0x0010 // NOT YET SUPPORTED
#define ZTS_SO_BROADCAST    0x0020
#define ZTS_SO_USELOOPBACK  0x0040 // NOT YET SUPPORTED
#define ZTS_SO_LINGER       0x0080
#define ZTS_SO_DONTLINGER   ((int)(~ZTS_SO_LINGER))
#define ZTS_SO_OOBINLINE    0x0100 // NOT YET SUPPORTED
#define ZTS_SO_REUSEPORT    0x0200 // NOT YET SUPPORTED
#define ZTS_SO_SNDBUF       0x1001 // NOT YET SUPPORTED
#define ZTS_SO_RCVBUF       0x1002
#define ZTS_SO_SNDLOWAT     0x1003 // NOT YET SUPPORTED
#define ZTS_SO_RCVLOWAT     0x1004 // NOT YET SUPPORTED
#define ZTS_SO_SNDTIMEO     0x1005
#define ZTS_SO_RCVTIMEO     0x1006
#define ZTS_SO_ERROR        0x1007
#define ZTS_SO_TYPE         0x1008
#define ZTS_SO_CONTIMEO     0x1009
#define ZTS_SO_NO_CHECK     0x100a
// IPPROTO_IP options
#define ZTS_IP_TOS          0x0001
#define ZTS_IP_TTL          0x0002
// IPPROTO_TCP options
#define ZTS_TCP_NODELAY     0x0001
#define ZTS_TCP_KEEPALIVE   0x0002
#define ZTS_TCP_KEEPIDLE    0x0003
#define ZTS_TCP_KEEPINTVL   0x0004
#define ZTS_TCP_KEEPCNT     0x0005
// IPPROTO_IPV6 options
#define ZTS_IPV6_CHECKSUM   0x0007  // RFC3542
#define ZTS_IPV6_V6ONLY     0x001b  // RFC3493
// Macro's for defining ioctl() command values
#define ZTS_IOCPARM_MASK    0x7fU
#define ZTS_IOC_VOID        0x20000000UL
#define ZTS_IOC_OUT         0x40000000UL
#define ZTS_IOC_IN          0x80000000UL
#define ZTS_IOC_INOUT       (ZTS_IOC_IN   | ZTS_IOC_OUT)
#define ZTS_IO(x,y)         (ZTS_IOC_VOID | ((x)<<8)|(y))
#define ZTS_IOR(x,y,t)      (ZTS_IOC_OUT  | (((long)sizeof(t) & ZTS_IOCPARM_MASK)<<16) | ((x)<<8) | (y))
#define ZTS_IOW(x,y,t)      (ZTS_IOC_IN   | (((long)sizeof(t) & ZTS_IOCPARM_MASK)<<16) | ((x)<<8) | (y))
// ioctl() commands
#define ZTS_FIONREAD        ZTS_IOR('f', 127, unsigned long)
#define ZTS_FIONBIO         ZTS_IOW('f', 126, unsigned long)

/* FD_SET used for lwip_select */

#ifndef ZTS_FD_SET
#undef  ZTS_FD_SETSIZE
// Make FD_SETSIZE match NUM_SOCKETS in socket.c
#define ZTS_FD_SETSIZE    MEMP_NUM_NETCONN
#define ZTS_FDSETSAFESET(n, code) do { \
  if (((n) - LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - LWIP_SOCKET_OFFSET) >= 0)) { \
  code; }} while(0)
#define ZTS_FDSETSAFEGET(n, code) (((n) - LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - LWIP_SOCKET_OFFSET) >= 0) ?\
  (code) : 0)
#define ZTS_FD_SET(n, p)  ZTS_FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] |=  (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define ZTS_FD_CLR(n, p)  ZTS_FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] &= ~(1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define ZTS_FD_ISSET(n,p) ZTS_FDSETSAFEGET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] &   (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define ZTS_FD_ZERO(p)    memset((void*)(p), 0, sizeof(*(p)))

#elif LWIP_SOCKET_OFFSET
#error LWIP_SOCKET_OFFSET does not work with external FD_SET!
#elif ZTS_FD_SETSIZE < MEMP_NUM_NETCONN
#error "external ZTS_FD_SETSIZE too small for number of sockets"
#endif // FD_SET

#if defined(_USING_LWIP_DEFINITIONS_)

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t   u8_t;
typedef int8_t    s8_t;
typedef uint16_t  u16_t;
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;
typedef uintptr_t mem_ptr_t;

typedef u32_t zts_in_addr_t;
typedef u16_t zts_in_port_t;
typedef u8_t zts_sa_family_t;

struct zts_in_addr {
  zts_in_addr_t s_addr;
};

struct zts_in6_addr {
  union {
    u32_t u32_addr[4];
    u8_t  u8_addr[16];
  } un;
#define s6_addr  un.u8_addr
};

struct zts_sockaddr_in {
  u8_t            sin_len;
  zts_sa_family_t     sin_family;
  zts_in_port_t       sin_port;
  struct zts_in_addr  sin_addr;
#define SIN_ZERO_LEN 8
  char            sin_zero[SIN_ZERO_LEN];
};

struct zts_sockaddr_in6 {
  u8_t            sin6_len;      /* length of this structure    */
  zts_sa_family_t     sin6_family;   /* AF_INET6                    */
  zts_in_port_t       sin6_port;     /* Transport layer port #      */
  u32_t           sin6_flowinfo; /* IPv6 flow information       */
  struct zts_in6_addr sin6_addr;     /* IPv6 address                */
  u32_t           sin6_scope_id; /* Set of interfaces for scope */
};

struct zts_sockaddr {
  u8_t        sa_len;
  zts_sa_family_t sa_family;
  char        sa_data[14];
};

struct zts_sockaddr_storage {
  u8_t        s2_len;
  zts_sa_family_t ss_family;
  char        s2_data1[2];
  u32_t       s2_data2[3];
  u32_t       s2_data3[3];
};

#if !defined(zts_iovec)
struct zts_iovec {
  void  *iov_base;
  size_t iov_len;
};
#endif

struct zts_msghdr {
  void         *msg_name;
  socklen_t     msg_namelen;
  struct iovec *msg_iov;
  int           msg_iovlen;
  void         *msg_control;
  socklen_t     msg_controllen;
  int           msg_flags;
};

/*
 * Structure used for manipulating linger option.
 */
struct zts_linger {
       int l_onoff;                /* option on/off */
       int l_linger;               /* linger time in seconds */
};

/*
typedef struct fd_set
{
  unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;
*/

#ifdef __cplusplus
}
#endif

#endif // _USING_LWIP_DEFINITIONS_

//////////////////////////////////////////////////////////////////////////////
// For SOCK_RAW support, it will initially be modeled after linux's API, so //
// below are the various things we need to define in order to make this API //
// work on other platforms. Maybe later down the road we will customize	    //
// this for each different platform. Maybe.									//
//////////////////////////////////////////////////////////////////////////////

#if !defined(__linux__)
#define SIOCGIFINDEX 101
#define SIOCGIFHWADDR 102

// Normally defined in linux/if_packet.h, defined here so we can offer a linux-like
// raw socket API on non-linux platforms
struct sockaddr_ll {
		unsigned short sll_family;	/* Always AF_PACKET */
		unsigned short sll_protocol; /* Physical layer protocol */
		int	sll_ifindex;  /* Interface number */
		unsigned short sll_hatype;	/* ARP hardware type */
		unsigned char sll_pkttype;  /* Packet type */
		unsigned char sll_halen;	 /* Length of address */
		unsigned char sll_addr[8];  /* Physical layer address */
};

#endif

//////////////////////////////////////////////////////////////////////////////
// Subset of: ZeroTierOne.h                                                 //
// We redefine a few ZT structures here so that we don't need to drag the   //
// entire ZeroTierOne.h file into the user application                      //
//////////////////////////////////////////////////////////////////////////////

/**
 * Maximum address assignments per network
 */
#define ZTS_MAX_ASSIGNED_ADDRESSES 16

/**
 * Maximum routes per network
 */
#define ZTS_MAX_NETWORK_ROUTES 32

/**
 * Maximum number of direct network paths to a given peer
 */
#define ZT_MAX_PEER_NETWORK_PATHS 16

/**
 * What trust hierarchy role does this peer have?
 */
enum zts_peer_role
{
	ZTS_PEER_ROLE_LEAF = 0,       // ordinary node
	ZTS_PEER_ROLE_MOON = 1,       // moon root
	ZTS_PEER_ROLE_PLANET = 2      // planetary root
};

/**
 * A structure used to represent a virtual network route
 */
struct zts_virtual_network_route
{
	/**
	 * Target network / netmask bits (in port field) or NULL or 0.0.0.0/0 for default
	 */
	struct sockaddr_storage target;

	/**
	 * Gateway IP address (port ignored) or NULL (family == 0) for LAN-local (no gateway)
	 */
	struct sockaddr_storage via;

	/**
	 * Route flags
	 */
	uint16_t flags;

	/**
	 * Route metric (not currently used)
	 */
	uint16_t metric;
};

/**
 * A structure used to convey network-specific details to the user application
 */
struct zts_network_details
{
	/**
	 * Network ID
	 */
	uint64_t nwid;

	/**
	 * Maximum Transmission Unit size for this network
	 */
	int mtu;

	/**
	 * Number of addresses (actually) assigned to the node on this network
	 */
	short num_addresses;

	/**
	 * Array of IPv4 and IPv6 addresses assigned to the node on this network
	 */
	struct sockaddr_storage addr[ZTS_MAX_ASSIGNED_ADDRESSES];

	/**
	 * Number of routes
	 */
	unsigned int num_routes;

	/**
	 * Array of IPv4 and IPv6 addresses assigned to the node on this network
	 */
	struct zts_virtual_network_route routes[ZTS_MAX_NETWORK_ROUTES];
};

/**
 * Physical network path to a peer
 */
struct zts_physical_path
{
	/**
	 * Address of endpoint
	 */
	struct sockaddr_storage address;

	/**
	 * Time of last send in milliseconds or 0 for never
	 */
	uint64_t lastSend;

	/**
	 * Time of last receive in milliseconds or 0 for never
	 */
	uint64_t lastReceive;

	/**
	 * Is this a trusted path? If so this will be its nonzero ID.
	 */
	uint64_t trustedPathId;

	/**
	 * Is path expired?
	 */
	int expired;

	/**
	 * Is path preferred?
	 */
	int preferred;
};

/**
 * Peer status result buffer
 */
struct zts_peer_details
{
	/**
	 * ZeroTier address (40 bits)
	 */
	uint64_t address;

	/**
	 * Remote major version or -1 if not known
	 */
	int versionMajor;

	/**
	 * Remote minor version or -1 if not known
	 */
	int versionMinor;

	/**
	 * Remote revision or -1 if not known
	 */
	int versionRev;

	/**
	 * Last measured latency in milliseconds or -1 if unknown
	 */
	int latency;

	/**
	 * What trust hierarchy role does this device have?
	 */
	enum zts_peer_role role;

	/**
	 * Number of paths (size of paths[])
	 */
	unsigned int pathCount;

	/**
	 * Known network paths to peer
	 */
	zts_physical_path paths[ZT_MAX_PEER_NETWORK_PATHS];
};

/**
 * List of peers
 */
struct zts_peer_list
{
	zts_peer_details *peers;
	unsigned long peerCount;
};

//////////////////////////////////////////////////////////////////////////////
// ZeroTier Service Controls                                                //
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts the ZeroTier service and notifies user application of events via callback
 *
 * @usage Should be called at the beginning of your application. Will blocks until all of the following conditions are met:
 * - ZeroTier core service has been initialized
 * - Cryptographic identity has been generated or loaded from directory specified by `path`
 * - Virtual network is successfully joined
 * - IP address is assigned by network controller service
 * @param path path directory where cryptographic identities and network configuration files are stored and retrieved
 *              (`identity.public`, `identity.secret`)
 * @param userCallbackFunc User-specified callback for ZeroTier events
 * @return 0 if successful; or 1 if failed
 */
ZT_SOCKET_API int ZTCALL zts_start(const char *path, void (*userCallbackFunc)(uint64_t, int), int port = ZTS_DEFAULT_PORT);

/**
 * @brief Stops the ZeroTier service, brings down all virtual interfaces in order to stop all traffic processing.
 *
 * @usage This should be called when the application anticipates not needing any sort of traffic processing for a
 * prolonged period of time. The stack driver (with associated timers) will remain active in case future traffic
 * processing is required. Note that the application must tolerate a multi-second startup time if zts_start()
 * zts_startjoin() is called again. To stop this background thread and free all resources use zts_free() instead.
 * @return Returns 0 on success, -1 on failure
 */
ZT_SOCKET_API int ZTCALL zts_stop();

/**
 * @brief Stops all background services, brings down all interfaces, frees all resources. After calling this function
 * an application restart will be required before the library can be used again. This is a blocking call.
 *
 * @usage This should be called at the end of your program or when you do not anticipate communicating over ZeroTier
 * @return Returns 0 on success, -1 on failure
 */
ZT_SOCKET_API int ZTCALL zts_free();

/**
 * @brief Return whether the ZeroTier service is currently running
 *
 * @usage Call this after zts_start()
 * @return 1 if running, 0 if not running
 */
ZT_SOCKET_API int ZTCALL zts_core_running();

/**
 * @brief Return the number of networks currently joined by this node
 *
 * @usage Call this after zts_start(), zts_startjoin() and/or zts_join()
 * @return Number of networks joined by this node
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_get_num_joined_networks();

/**
 * @brief Populates a structure with details for a given network
 *
 * @usage Call this from the application thread any time after the node has joined a network
 * @param nwid A 16-digit hexidecimal virtual network ID
 * @param nd Pointer to a zts_network_details structure to populate
 * @return ZTS_ERR_SERVICE if failed, 0 if otherwise
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_get_network_details(uint64_t nwid, struct zts_network_details *nd);

/**
 * @brief Populates an array of structures with details for any given number of networks
 *
 * @usage Call this from the application thread any time after the node has joined a network
 * @param nds Pointer to an array of zts_network_details structures to populate
 * @param num Number of zts_network_details structures available to copy data into, will be updated
 * to reflect number of structures that were actually populated
 * @return ZTS_ERR_SERVICE if failed, 0 if otherwise
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_get_all_network_details(struct zts_network_details *nds, int *num);

/**
 * @brief Join a network
 *
 * @usage Call this from application thread. Only after zts_start() has succeeded
 * @param nwid A 16-digit hexidecimal virtual network ID
 * @return 0 if successful, -1 for any failure
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_join(const uint64_t nwid);

/**
 * @brief Leave a network
 *
 * @usage Call this from application thread. Only after zts_start() has succeeded
 * @param nwid A 16-digit hexidecimal virtual network ID
 * @return 0 if successful, -1 for any failure
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_leave(const uint64_t nwid);


/**
 * @brief Leaves all networks
 *
 * @usage Call this from application thread. Only after zts_start() has succeeded
 * @return 0 if successful, -1 for any failure
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_leave_all();

/**
 * @brief Orbits a given moon (user-defined root server)
 *
 * @usage Call this from application thread. Only after zts_start() has succeeded
 * @param moonWorldId A 16-digit hexidecimal world ID
 * @param moonSeed A 16-digit hexidecimal seed ID
 * @return ZTS_ERR_OK if successful, ZTS_ERR_SERVICE, ZTS_ERR_INVALID_ARG, ZTS_ERR_INVALID_OP if otherwise
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_orbit(uint64_t moonWorldId, uint64_t moonSeed);

/**
 * @brief De-orbits a given moon (user-defined root server)
 *
 * @usage Call this from application thread. Only after zts_start() has succeeded
 * @param moonWorldId A 16-digit hexidecimal world ID
 * @return ZTS_ERR_OK if successful, ZTS_ERR_SERVICE, ZTS_ERR_INVALID_ARG, ZTS_ERR_INVALID_OP if otherwise
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_deorbit(uint64_t moonWorldId);

/**
 * @brief Copies the configuration path used by ZeroTier into the provided buffer
 *
 * @usage Use this to determine where ZeroTier is storing identity files
 * @param homePath Path to ZeroTier configuration files
 * @param len Length of destination buffer
 * @return 0 if no error, -1 if invalid argument was supplied
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_get_path(char *homePath, size_t *len);

/**
 * @brief Returns the node ID of this instance
 *
 * @usage Call this after zts_start() and/or when zts_running() returns true
 * @return
 */
ZT_SOCKET_API uint64_t ZTCALL zts_get_node_id();

/**
 * @brief Returns whether any address has been assigned to the SockTap for this network
 *
 * @usage This is used as an indicator of readiness for service for the ZeroTier core and stack
 * @param nwid Network ID
 * @return
 */
ZT_SOCKET_API int ZTCALL zts_has_address(const uint64_t nwid);


/**
 * @brief Returns the number of addresses assigned to this node for the given nwid
 *
 * @param nwid Network ID
 * @return The number of addresses assigned
 */
ZT_SOCKET_API int ZTCALL zts_get_num_assigned_addresses(const uint64_t nwid);

/**
 * @brief Returns the assigned address located at the given index
 *
 * @usage The indices of each assigned address are not guaranteed and should only
 * be used for iterative purposes.
 * @param nwid Network ID
 * @param index location of assigned address
 * @return The number of addresses assigned
 */
ZT_SOCKET_API int ZTCALL zts_get_address_at_index(
	const uint64_t nwid, const int index, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Get IP address for this device on a given network
 *
 * @usage FIXME: Only returns first address found, good enough for most cases
 * @param nwid Network ID
 * @param addr Destination structure for address
 * @param addrlen size of destination address buffer, will be changed to size of returned address
 * @return 0 if an address was successfully found, -1 if failure
 */
ZT_SOCKET_API int ZTCALL zts_get_address(
	const uint64_t nwid, struct sockaddr_storage *addr, const int address_family);

/**
 * @brief Computes a 6PLANE IPv6 address for the given Network ID and Node ID
 *
 * @usage Can call any time
 * @param addr Destination structure for address
 * @param nwid Network ID 
 * @param nodeId Node ID
 * @return
 */
ZT_SOCKET_API void ZTCALL zts_get_6plane_addr(
	struct sockaddr_storage *addr, const uint64_t nwid, const uint64_t nodeId);

/**
 * @brief Computes a RFC4193 IPv6 address for the given Network ID and Node ID
 *
 * @usage Can call any time
 * @param addr Destination structure for address
 * @param nwid Network ID 
 * @param nodeId Node ID
 * @return
 */
ZT_SOCKET_API void ZTCALL zts_get_rfc4193_addr(
	struct sockaddr_storage *addr, const uint64_t nwid, const uint64_t nodeId);

/**
 * @brief Return the number of peers
 *
 * @usage Call this after zts_start() has succeeded
 * @return
 */
ZT_SOCKET_API zts_err_t zts_get_peer_count();

ZT_SOCKET_API zts_err_t zts_get_peers(struct zts_peer_details *pds, int *num);

/**
 * @brief Determines whether a peer is reachable via a P2P connection
 * or is being relayed via roots.
 *
 * @usage
 * @param nodeId The ID of the peer to check
 * @return The status of a peer
 */
ZT_SOCKET_API zts_err_t zts_get_peer_status(uint64_t nodeId);

/**
 * @brief Starts a ZeroTier service in the background
 *
 * @usage For internal use only.
 * @param
 * @return
 */
#if defined(_WIN32)
DWORD WINAPI _zts_start_service(LPVOID thread_id);
#else
void *_zts_start_service(void *thread_id);
#endif

/**
 * @brief [Should not be called from user application] This function must be surrounded by 
 * ZT service locks. It will determine if it is currently safe and allowed to operate on 
 * the service.
 * @usage Can be called at any time
 * @return 1 or 0
 */
int _zts_can_perform_service_operation();

/**
 * @brief [Should not be called from user application] Returns whether or not the node is 
 * online.
 * @usage Can be called at any time
 * @return 1 or 0
 */
int _zts_node_online();

int zts_ready();

//////////////////////////////////////////////////////////////////////////////
// Socket API                                                               //
//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create a socket
 *
 * This function will return an integer which can be used in much the same way as a
 * typical file descriptor, however it is only valid for use with libzt library calls
 * as this is merely a facade which is associated with the internal socket representation
 * of both the network stacks and drivers.
 *
 * @usage Call this after zts_start() has succeeded
 * @param socket_family Address family (AF_INET, AF_INET6)
 * @param socket_type Type of socket (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW)
 * @param protocol Protocols supported on this socket
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_socket(int socket_family, int socket_type, int protocol);

/**
 * @brief Connect a socket to a remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param addr Remote host address to connect to
 * @param addrlen Length of address
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_connect(int fd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Bind a socket to a virtual interface
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param addr Local interface address to bind to
 * @param addrlen Length of address
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_bind(int fd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Listen for incoming connections
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param backlog Number of backlogged connection allowed
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_listen(int fd, int backlog);

/**
 * @brief Accept an incoming connection
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param addr Address of remote host for accepted connection
 * @param addrlen Length of address
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_accept(int fd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Accept an incoming connection
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param addr Address of remote host for accepted connection
 * @param addrlen Length of address
 * @param flags
 * @return
 */
#if defined(__linux__)
	zts_err_t zts_accept4(int fd, struct sockaddr *addr, socklen_t *addrlen, int flags);
#endif

/**
 * @brief Set socket options
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param level Protocol level to which option name should apply
 * @param optname Option name to set
 * @param optval Source of option value to set
 * @param optlen Length of option value
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_setsockopt(
	int fd, int level, int optname, const void *optval, socklen_t optlen);

/**
 * @brief Get socket options
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param level Protocol level to which option name should apply
 * @param optname Option name to get
 * @param optval Where option value will be stored
 * @param optlen Length of value
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_getsockopt(
	int fd, int level, int optname, void *optval, socklen_t *optlen);

/**
 * @brief Get socket name
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param addr Name associated with this socket
 * @param addrlen Length of name
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Get the peer name for the remote end of a connected socket
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param addr Name associated with remote end of this socket
 * @param addrlen Length of name
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Gets current hostname
 *
 * @usage Call this after zts_start() has succeeded
 * @param name
 * @param len
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_gethostname(char *name, size_t len);

/**
 * @brief Sets current hostname
 *
 * @usage Call this after zts_start() has succeeded
 * @param name
 * @param len
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_sethostname(const char *name, size_t len);

/**
 * @brief Return a pointer to an object with the following structure describing an internet host referenced by name
 *
 * @usage Call this after zts_start() has succeeded
 * @param name
 * @return Returns pointer to hostent structure otherwise NULL if failure
 */
ZT_SOCKET_API struct hostent *zts_gethostbyname(const char *name);

/**
 * @brief Close a socket
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_close(int fd);

/**
 * @brief Waits for one of a set of file descriptors to become ready to perform I/O.
 *
 * @usage Call this after zts_start() has succeeded
 * @param fds
 * @param nfds
 * @param timeout
 * @return
 */
#if defined(__linux__)
/*
typedef unsigned int nfds_t;
int zts_poll(struct pollfd *fds, nfds_t nfds, int timeout);
*/
#endif

/**
 * @brief Monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready"
 *
 * @usage Call this after zts_start() has succeeded
 * @param nfds 
 * @param readfds
 * @param writefds
 * @param exceptfds
 * @param timeout
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_select(
	int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

/**
 * @brief Issue file control commands on a socket
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param cmd
 * @param flags
 * @return
 */
#if defined(_WIN32)
#define F_SETFL 0
#define O_NONBLOCK 0
#endif
ZT_SOCKET_API zts_err_t ZTCALL zts_fcntl(int fd, int cmd, int flags);

/**
 * @brief Control a device
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param request
 * @param argp
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_ioctl(int fd, unsigned long request, void *argp);

/**
 * @brief Send data to remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param buf Pointer to data buffer
 * @param len Length of data to write
 * @param flags
 * @return
 */
ZT_SOCKET_API ssize_t ZTCALL zts_send(int fd, const void *buf, size_t len, int flags);

/**
 * @brief Send data to remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param buf Pointer to data buffer
 * @param len Length of data to write
 * @param flags
 * @param addr Destination address
 * @param addrlen Length of destination address
 * @return
 */
ZT_SOCKET_API ssize_t ZTCALL zts_sendto(
	int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Send message to remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param msg
 * @param flags
 * @return
 */
ZT_SOCKET_API ssize_t ZTCALL zts_sendmsg(int fd, const struct msghdr *msg, int flags);

/**
 * @brief Receive data from remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param buf Pointer to data buffer
 * @param len Length of data buffer
 * @param flags
 * @return
 */
ZT_SOCKET_API ssize_t ZTCALL zts_recv(int fd, void *buf, size_t len, int flags);

/**
 * @brief Receive data from remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param buf Pointer to data buffer
 * @param len Length of data buffer
 * @param flags
 * @param addr
 * @param addrlen
 * @return
 */
ZT_SOCKET_API ssize_t ZTCALL zts_recvfrom(
	int fd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Receive a message from remote host
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param msg
 * @param flags
 * @return
 */
ZT_SOCKET_API ssize_t ZTCALL zts_recvmsg(int fd, struct msghdr *msg,int flags);

/**
 * @brief Read bytes from socket onto buffer
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param buf Pointer to data buffer
 * @param len Length of data buffer to receive data
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_read(int fd, void *buf, size_t len);

/**
 * @brief Write bytes from buffer to socket
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param buf Pointer to data buffer
 * @param len Length of buffer to write
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_write(int fd, const void *buf, size_t len);

/**
 * @brief Shut down some aspect of a socket (read, write, or both)
 *
 * @usage Call this after zts_start() has succeeded
 * @param fd File descriptor (only valid for use with libzt calls)
 * @param how Which aspects of the socket should be shut down
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_shutdown(int fd, int how);

/**
 * @brief Adds a DNS nameserver for the network stack to use
 *
 * @usage Call this after zts_start() has succeeded
 * @param addr Address for DNS nameserver
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_add_dns_nameserver(struct sockaddr *addr);

/**
 * @brief Removes a DNS nameserver
 *
 * @usage Call this after zts_start() has succeeded
 * @param addr Address for DNS nameserver
 * @return
 */
ZT_SOCKET_API zts_err_t ZTCALL zts_del_dns_nameserver(struct sockaddr *addr);

#ifdef __cplusplus
} // extern "C"
#endif

} // namespace ZeroTier

#endif // _H
