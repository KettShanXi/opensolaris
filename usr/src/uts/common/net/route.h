/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*
 * Copyright (c) 1980, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Kernel resident routing tables.
 *
 * The routing tables are initialized when interface addresses
 * are set by making entries for all directly connected interfaces.
 */

#ifndef	_NET_ROUTE_H
#define	_NET_ROUTE_H

#pragma ident	"%Z%%M%	%I%	%E% SMI"
/* from UCB 8.5 (Berkeley) 2/8/95 */

#include <sys/tsol/label.h>
#include <sys/tsol/label_macro.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route {
	struct	rtentry *ro_rt;
	struct	sockaddr ro_dst;
};

/*
 * These numbers are used by reliable protocols for determining
 * retransmission behavior and are included in the routing structure.
 *
 * rmx_rtt and rmx_rttvar are stored as microseconds;
 */
typedef struct rt_metrics {
	uint32_t	rmx_locks;	/* Kernel must leave these values */
					/* alone */
	uint32_t	rmx_mtu;	/* MTU for this path */
	uint32_t	rmx_hopcount;	/* max hops expected */
	uint32_t	rmx_expire;	/* lifetime for route, e.g. redirect */
	uint32_t	rmx_recvpipe;	/* inbound delay-bandwith product */
	uint32_t	rmx_sendpipe;	/* outbound delay-bandwith product */
	uint32_t	rmx_ssthresh;	/* outbound gateway buffer limit */
	uint32_t	rmx_rtt;	/* estimated round trip time */
	uint32_t	rmx_rttvar;	/* estimated rtt variance */
	uint32_t	rmx_pksent;	/* packets sent using this route */
} rt_metrics_t;

/*
 * OLD rtentry structure not used in the kernel. Instead the kernel
 * uses struct ire defined in <inet/ip.h>.
 *
 * We distinguish between routes to hosts and routes to networks,
 * preferring the former if available.  For each route we infer
 * the interface to use from the gateway address supplied when
 * the route was entered.  Routes that forward packets through
 * gateways are marked so that the output routines know to address the
 * gateway rather than the ultimate destination.
 */
struct rtentry {
	uint_t	rt_hash;		/* to speed lookups */
	struct	sockaddr rt_dst;	/* key */
	struct	sockaddr rt_gateway;	/* value */
	short	rt_flags;		/* up/down?, host/net */
	short	rt_refcnt;		/* # held references */
	uint_t	rt_use;			/* raw # packets forwarded */

/*
 * The kernel does not use this field, and without it the structure is
 * datamodel independent.
 */
#if !defined(_KERNEL)
	struct	ifnet *rt_ifp;		/* the answer: interface to use */
#endif /* !defined(_KERNEL) */
};

#define	RTF_UP		0x1		/* route usable */
#define	RTF_GATEWAY	0x2		/* destination is a gateway */
#define	RTF_HOST	0x4		/* host entry (net otherwise) */
#define	RTF_REJECT	0x8		/* host or net unreachable */
#define	RTF_DYNAMIC	0x10		/* created dynamically (by redirect) */
#define	RTF_MODIFIED	0x20		/* modified dynamically (by redirect) */
#define	RTF_DONE	0x40		/* message confirmed */
#define	RTF_MASK	0x80		/* subnet mask present */
#define	RTF_CLONING	0x100		/* generate new routes on use */
#define	RTF_XRESOLVE	0x200		/* external daemon resolves name */
#define	RTF_LLINFO	0x400		/* generated by ARP or ESIS */
#define	RTF_STATIC	0x800		/* manually added */
#define	RTF_BLACKHOLE	0x1000		/* just discard pkts (during updates) */
#define	RTF_PRIVATE	0x2000		/* do not advertise this route */
#define	RTF_PROTO2	0x4000		/* protocol specific routing flag */
#define	RTF_PROTO1	0x8000		/* protocol specific routing flag */
#define	RTF_MULTIRT	0x10000		/* multiroute */
#define	RTF_SETSRC	0x20000		/* set default outgoing src address */


/*
 * OLD statistics not used by the kernel. The kernel uses <inet/mib2.h>.
 *
 * Routing statistics.
 */
struct	rtstat {
	short	rts_badredirect;	/* bogus redirect calls */
	short	rts_dynamic;		/* routes created by redirects */
	short	rts_newgateway;		/* routes modified by redirects */
	short	rts_unreach;		/* lookups which failed */
	short	rts_wildcard;		/* lookups satisfied by a wildcard */
};

/*
 * Structures for routing messages.
 */
typedef struct rt_msghdr {
	ushort_t rtm_msglen;	/* to skip over non-understood messages */
	uchar_t	rtm_version;	/* future binary compatibility */
	uchar_t	rtm_type;	/* message type */
	ushort_t rtm_index;	/* index for associated ifp */
	int	rtm_flags;	/* flags, incl. kern & message, e.g. DONE */
	int	rtm_addrs;	/* bitmask identifying sockaddrs in msg */
	pid_t	rtm_pid;	/* identify sender */
	int	rtm_seq;	/* for sender to identify action */
	int	rtm_errno;	/* why failed */
	int	rtm_use;	/* from rtentry */
	uint_t	rtm_inits;	/* which metrics we are initializing */
	struct	rt_metrics rtm_rmx; /* metrics themselves */
} rt_msghdr_t;

#define	RTM_VERSION	3	/* Up the ante and ignore older versions */

#define	RTM_ADD		0x1	/* Add Route */
#define	RTM_DELETE	0x2	/* Delete Route */
#define	RTM_CHANGE	0x3	/* Change Metrics or flags */
#define	RTM_GET		0x4	/* Report Metrics */
#define	RTM_LOSING	0x5	/* Kernel Suspects Partitioning */
#define	RTM_REDIRECT	0x6	/* Told to use different route */
#define	RTM_MISS	0x7	/* Lookup failed on this address */
#define	RTM_LOCK	0x8	/* fix specified metrics */
#define	RTM_OLDADD	0x9	/* caused by SIOCADDRT */
#define	RTM_OLDDEL	0xa	/* caused by SIOCDELRT */
#define	RTM_RESOLVE	0xb	/* req to resolve dst to LL addr */
#define	RTM_NEWADDR	0xc	/* address being added to iface */
#define	RTM_DELADDR	0xd	/* address being removed from iface */
#define	RTM_IFINFO	0xe	/* iface going up/down etc. */

#define	RTV_MTU		0x1	/* init or lock _mtu */
#define	RTV_HOPCOUNT	0x2	/* init or lock _hopcount */
#define	RTV_EXPIRE	0x4	/* init or lock _expire */
#define	RTV_RPIPE	0x8	/* init or lock _recvpipe */
#define	RTV_SPIPE	0x10	/* init or lock _sendpipe */
#define	RTV_SSTHRESH	0x20	/* init or lock _ssthresh */
#define	RTV_RTT		0x40	/* init or lock _rtt */
#define	RTV_RTTVAR	0x80	/* init or lock _rttvar */

/*
 * Bitmask values for rtm_addr.
 */
#define	RTA_DST		0x1	/* destination sockaddr present */
#define	RTA_GATEWAY	0x2	/* gateway sockaddr present */
#define	RTA_NETMASK	0x4	/* netmask sockaddr present */
#define	RTA_GENMASK	0x8	/* cloning mask sockaddr present */
#define	RTA_IFP		0x10	/* interface name sockaddr present */
#define	RTA_IFA		0x20	/* interface addr sockaddr present */
#define	RTA_AUTHOR	0x40	/* sockaddr for author of redirect */
#define	RTA_BRD		0x80	/* for NEWADDR, broadcast or p-p dest addr */
#define	RTA_SRC		0x100	/* source sockaddr present */
#define	RTA_SRCIFP	0x200	/* source interface index sockaddr present */

#define	RTA_NUMBITS	10	/* Number of bits used in RTA_* */

/*
 * Index offsets for sockaddr_storage array for alternate internal encoding.
 * There should be an RTAX_* associated with each RTA_*.
 */
#define	RTAX_DST	0
#define	RTAX_GATEWAY	1
#define	RTAX_NETMASK	2
#define	RTAX_GENMASK	3
#define	RTAX_IFP	4
#define	RTAX_IFA	5
#define	RTAX_AUTHOR	6
#define	RTAX_BRD	7
#define	RTAX_SRC	8
#define	RTAX_SRCIFP	9
#define	RTAX_MAX	RTA_NUMBITS	/* size of array to allocate */

/*
 * Routing socket message extension after sockaddrs.
 */
typedef struct rtm_ext_s {
	uint32_t	rtmex_type;	/* identifier for type of extension */
	uint32_t	rtmex_len;	/* length of this extension */
} rtm_ext_t;

#define	RTMEX_GATEWAY_SECATTR	1	/* extension is tsol_rtsecattr */
#define	RTMEX_MAX	RTMEX_GATEWAY_SECATTR

/*
 * Trusted Solaris route security attributes extension.
 */
typedef struct rtsa_s {
	uint32_t	rtsa_mask;	/* see RTSA_* below */
	uint32_t	rtsa_doi;	/* domain of interpretation */
	brange_t	rtsa_slrange;	/* sensitivity label range */
} rtsa_t;

typedef struct tsol_rtsecattr_s {
	uint32_t	rtsa_cnt;	/* number of attributes */
	rtsa_t		rtsa_attr[1];
} tsol_rtsecattr_t;

#define	TSOL_RTSECATTR_SIZE(n) \
	(sizeof (tsol_rtsecattr_t) + (((n) - 1) * sizeof (struct rtsa_s)))

#define	RTSA_MINSL	0x1	/* minimum sensitivity label is valid */
#define	RTSA_MAXSL	0x2	/* maximum sensitivity label is valid */
#define	RTSA_DOI	0x4	/* domain of interpretation is valid */
#define	RTSA_CIPSO	0x100	/* CIPSO protocol */
#define	RTSA_SLRANGE (RTSA_MINSL|RTSA_MAXSL)

#ifdef	__cplusplus
}
#endif

#endif	/* _NET_ROUTE_H */
