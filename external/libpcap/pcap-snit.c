/*
 * Copyright (c) 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Modifications made to accommodate the new SunOS4.0 NIT facility by
 * Micky Liu, micky@cunixc.cc.columbia.edu, Columbia University in May, 1989.
 * This module now handles the STREAMS based NIT.
 */

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/pcap-snit.c,v 1.77 2008-04-14 20:40:58 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/dir.h>
#include <sys/fcntlcom.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stropts.h>

#include <net/if.h>
#include <net/nit.h>
#include <net/nit_if.h>
#include <net/nit_pf.h>
#include <net/nit_buf.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "pcap-int.h"

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#define CHUNKSIZE (2*1024)

#define BUFSPACE (4*CHUNKSIZE)

static int nit_setflags(int, int, int, char *);

struct pcap_snit {
	struct pcap_stat stat;
};

static int
pcap_stats_snit(pcap_t *p, struct pcap_stat *ps)
{
	struct pcap_snit *psn = p->priv;

	*ps = psn->stat;
	return (0);
}

static int
pcap_read_snit(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
{
	struct pcap_snit *psn = p->priv;
	register int cc, n;
	register u_char *bp, *cp, *ep;
	register struct nit_bufhdr *hdrp;
	register struct nit_iftime *ntp;
	register struct nit_iflen *nlp;
	register struct nit_ifdrops *ndp;
	register int caplen;

	cc = p->cc;
	if (cc == 0) {
		cc = read(p->fd, (char *)p->buffer, p->bufsize);
		if (cc < 0) {
			if (errno == EWOULDBLOCK)
				return (0);
			snprintf(p->errbuf, sizeof(p->errbuf), "pcap_read: %s",
				pcap_strerror(errno));
			return (-1);
		}
		bp = p->buffer;
	} else
		bp = p->bp;

	n = 0;
	ep = bp + cc;
	while (bp < ep) {
		if (p->break_loop) {
			if (n == 0) {
				p->break_loop = 0;
				return (-2);
			} else {
				p->bp = bp;
				p->cc = ep - bp;
				return (n);
			}
		}

		++psn->stat.ps_recv;
		cp = bp;

		
		hdrp = (struct nit_bufhdr *)cp;
		cp += sizeof(*hdrp);

		
		ntp = (struct nit_iftime *)cp;
		cp += sizeof(*ntp);

		ndp = (struct nit_ifdrops *)cp;
		psn->stat.ps_drop = ndp->nh_drops;
		cp += sizeof *ndp;

		
		nlp = (struct nit_iflen *)cp;
		cp += sizeof(*nlp);

		
		bp += hdrp->nhb_totlen;

		caplen = nlp->nh_pktlen;
		if (caplen > p->snapshot)
			caplen = p->snapshot;

		if (bpf_filter(p->fcode.bf_insns, cp, nlp->nh_pktlen, caplen)) {
			struct pcap_pkthdr h;
			h.ts = ntp->nh_timestamp;
			h.len = nlp->nh_pktlen;
			h.caplen = caplen;
			(*callback)(user, &h, cp);
			if (++n >= cnt && !PACKET_COUNT_IS_UNLIMITED(cnt)) {
				p->cc = ep - bp;
				p->bp = bp;
				return (n);
			}
		}
	}
	p->cc = 0;
	return (n);
}

static int
pcap_inject_snit(pcap_t *p, const void *buf, size_t size)
{
	struct strbuf ctl, data;
	
	ctl.len = sizeof(*sa);	
	ctl.buf = (char *)sa;
	data.buf = buf;
	data.len = size;
	ret = putmsg(p->fd, &ctl, &data);
	if (ret == -1) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "send: %s",
		    pcap_strerror(errno));
		return (-1);
	}
	return (ret);
}

static int
nit_setflags(pcap_t *p)
{
	bpf_u_int32 flags;
	struct strioctl si;
	u_int zero = 0;
	struct timeval timeout;

	if (p->opt.immediate) {
		si.ic_cmd = NIOCSCHUNK;
		si.ic_len = sizeof(zero);
		si.ic_dp = (char *)&zero;
		if (ioctl(p->fd, I_STR, (char *)&si) < 0) {
			snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "NIOCSCHUNK: %s",
			    pcap_strerror(errno));
			return (-1);
		}
	}
	si.ic_timout = INFTIM;
	if (p->opt.timeout != 0) {
		timeout.tv_sec = p->opt.timeout / 1000;
		timeout.tv_usec = (p->opt.timeout * 1000) % 1000000;
		si.ic_cmd = NIOCSTIME;
		si.ic_len = sizeof(timeout);
		si.ic_dp = (char *)&timeout;
		if (ioctl(p->fd, I_STR, (char *)&si) < 0) {
			snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "NIOCSTIME: %s",
			    pcap_strerror(errno));
			return (-1);
		}
	}
	flags = NI_TIMESTAMP | NI_LEN | NI_DROPS;
	if (p->opt.promisc)
		flags |= NI_PROMISC;
	si.ic_cmd = NIOCSFLAGS;
	si.ic_len = sizeof(flags);
	si.ic_dp = (char *)&flags;
	if (ioctl(p->fd, I_STR, (char *)&si) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "NIOCSFLAGS: %s",
		    pcap_strerror(errno));
		return (-1);
	}
	return (0);
}

static int
pcap_activate_snit(pcap_t *p)
{
	struct strioctl si;		
	struct ifreq ifr;		
	int chunksize = CHUNKSIZE;
	int fd;
	static char dev[] = "/dev/nit";

	if (p->opt.rfmon) {
		return (PCAP_ERROR_RFMON_NOTSUP);
	}

	if (p->snapshot < 96)
		p->snapshot = 96;

	p->fd = fd = open(dev, O_RDWR);
	if (fd < 0 && errno == EACCES)
		p->fd = fd = open(dev, O_RDONLY);
	if (fd < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "%s: %s", dev,
		    pcap_strerror(errno));
		goto bad;
	}

	
	if (ioctl(fd, I_SRDOPT, (char *)RMSGD) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "I_SRDOPT: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	if (ioctl(fd, I_PUSH, "nbuf") < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "push nbuf: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	
	si.ic_cmd = NIOCSCHUNK;
	si.ic_timout = INFTIM;
	si.ic_len = sizeof(chunksize);
	si.ic_dp = (char *)&chunksize;
	if (ioctl(fd, I_STR, (char *)&si) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "NIOCSCHUNK: %s",
		    pcap_strerror(errno));
		goto bad;
	}

	
	strncpy(ifr.ifr_name, p->opt.source, sizeof(ifr.ifr_name));
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
	si.ic_cmd = NIOCBIND;
	si.ic_len = sizeof(ifr);
	si.ic_dp = (char *)&ifr;
	if (ioctl(fd, I_STR, (char *)&si) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "NIOCBIND: %s: %s",
			ifr.ifr_name, pcap_strerror(errno));
		goto bad;
	}

	
	si.ic_cmd = NIOCSSNAP;
	si.ic_len = sizeof(p->snapshot);
	si.ic_dp = (char *)&p->snapshot;
	if (ioctl(fd, I_STR, (char *)&si) < 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "NIOCSSNAP: %s",
		    pcap_strerror(errno));
		goto bad;
	}
	if (nit_setflags(p) < 0)
		goto bad;

	(void)ioctl(fd, I_FLUSH, (char *)FLUSHR);
	p->linktype = DLT_EN10MB;

	p->bufsize = BUFSPACE;
	p->buffer = (u_char *)malloc(p->bufsize);
	if (p->buffer == NULL) {
		strlcpy(p->errbuf, pcap_strerror(errno), PCAP_ERRBUF_SIZE);
		goto bad;
	}

	p->selectable_fd = p->fd;

	p->dlt_list = (u_int *) malloc(sizeof(u_int) * 2);
	if (p->dlt_list != NULL) {
		p->dlt_list[0] = DLT_EN10MB;
		p->dlt_list[1] = DLT_DOCSIS;
		p->dlt_count = 2;
	}

	p->read_op = pcap_read_snit;
	p->inject_op = pcap_inject_snit;
	p->setfilter_op = install_bpf_program;	
	p->setdirection_op = NULL;	
	p->set_datalink_op = NULL;	
	p->getnonblock_op = pcap_getnonblock_fd;
	p->setnonblock_op = pcap_setnonblock_fd;
	p->stats_op = pcap_stats_snit;

	return (0);
 bad:
	pcap_cleanup_live_common(p);
	return (PCAP_ERROR);
}

pcap_t *
pcap_create_interface(const char *device, char *ebuf)
{
	pcap_t *p;

	p = pcap_create_common(device, ebuf, sizeof (struct pcap_snit));
	if (p == NULL)
		return (NULL);

	p->activate_op = pcap_activate_snit;
	return (p);
}

int
pcap_platform_finddevs(pcap_if_t **alldevsp, char *errbuf)
{
	return (0);
}
