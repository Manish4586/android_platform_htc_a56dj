/***********************************************************************
*
* discovery.c
*
* Perform PPPoE discovery
*
* Copyright (C) 1999 by Roaring Penguin Software Inc.
*
***********************************************************************/

static char const RCSID[] =
"$Id: discovery.c,v 1.6 2008/06/15 04:35:50 paulus Exp $";

#define _GNU_SOURCE 1
#include "pppoe.h"
#include "pppd/pppd.h"
#include "pppd/fsm.h"
#include "pppd/lcp.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef USE_LINUX_PACKET
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include <signal.h>

static int time_left(struct timeval *diff, struct timeval *exp)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) < 0) {
	error("gettimeofday: %m");
	return 0;
    }

    if (now.tv_sec > exp->tv_sec
	|| (now.tv_sec == exp->tv_sec && now.tv_usec >= exp->tv_usec))
	return 0;

    diff->tv_sec = exp->tv_sec - now.tv_sec;
    diff->tv_usec = exp->tv_usec - now.tv_usec;
    if (diff->tv_usec < 0) {
	diff->tv_usec += 1000000;
	--diff->tv_sec;
    }

    return 1;
}

static void
parseForHostUniq(UINT16_t type, UINT16_t len, unsigned char *data,
		 void *extra)
{
    int *val = (int *) extra;
    if (type == TAG_HOST_UNIQ && len == sizeof(pid_t)) {
	pid_t tmp;
	memcpy(&tmp, data, len);
	if (tmp == getpid()) {
	    *val = 1;
	}
    }
}

static int
packetIsForMe(PPPoEConnection *conn, PPPoEPacket *packet)
{
    int forMe = 0;

    
    if (memcmp(packet->ethHdr.h_dest, conn->myEth, ETH_ALEN)) return 0;

    
    if (!conn->useHostUniq) return 1;

    parsePacket(packet, parseForHostUniq, &forMe);
    return forMe;
}

static void
parsePADOTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    struct PacketCriteria *pc = (struct PacketCriteria *) extra;
    PPPoEConnection *conn = pc->conn;
    UINT16_t mru;
    int i;

    switch(type) {
    case TAG_AC_NAME:
	pc->seenACName = 1;
	if (conn->printACNames) {
	    info("Access-Concentrator: %.*s", (int) len, data);
	}
	if (conn->acName && len == strlen(conn->acName) &&
	    !strncmp((char *) data, conn->acName, len)) {
	    pc->acNameOK = 1;
	}
	break;
    case TAG_SERVICE_NAME:
	pc->seenServiceName = 1;
	if (conn->serviceName && len == strlen(conn->serviceName) &&
	    !strncmp((char *) data, conn->serviceName, len)) {
	    pc->serviceNameOK = 1;
	}
	break;
    case TAG_AC_COOKIE:
	conn->cookie.type = htons(type);
	conn->cookie.length = htons(len);
	memcpy(conn->cookie.payload, data, len);
	break;
    case TAG_RELAY_SESSION_ID:
	conn->relayId.type = htons(type);
	conn->relayId.length = htons(len);
	memcpy(conn->relayId.payload, data, len);
	break;
    case TAG_PPP_MAX_PAYLOAD:
	if (len == sizeof(mru)) {
	    memcpy(&mru, data, sizeof(mru));
	    mru = ntohs(mru);
	    if (mru >= ETH_PPPOE_MTU) {
		if (lcp_allowoptions[0].mru > mru)
		    lcp_allowoptions[0].mru = mru;
		if (lcp_wantoptions[0].mru > mru)
		    lcp_wantoptions[0].mru = mru;
		conn->seenMaxPayload = 1;
	    }
	}
	break;
    case TAG_SERVICE_NAME_ERROR:
	error("PADO: Service-Name-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_AC_SYSTEM_ERROR:
	error("PADO: System-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_GENERIC_ERROR:
	error("PADO: Generic-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    }
}

static void
parsePADSTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    PPPoEConnection *conn = (PPPoEConnection *) extra;
    UINT16_t mru;
    switch(type) {
    case TAG_SERVICE_NAME:
	dbglog("PADS: Service-Name: '%.*s'", (int) len, data);
	break;
    case TAG_PPP_MAX_PAYLOAD:
	if (len == sizeof(mru)) {
	    memcpy(&mru, data, sizeof(mru));
	    mru = ntohs(mru);
	    if (mru >= ETH_PPPOE_MTU) {
		if (lcp_allowoptions[0].mru > mru)
		    lcp_allowoptions[0].mru = mru;
		if (lcp_wantoptions[0].mru > mru)
		    lcp_wantoptions[0].mru = mru;
		conn->seenMaxPayload = 1;
	    }
	}
	break;
    case TAG_SERVICE_NAME_ERROR:
	error("PADS: Service-Name-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_AC_SYSTEM_ERROR:
	error("PADS: System-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_GENERIC_ERROR:
	error("PADS: Generic-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_RELAY_SESSION_ID:
	conn->relayId.type = htons(type);
	conn->relayId.length = htons(len);
	memcpy(conn->relayId.payload, data, len);
	break;
    }
}

static void
sendPADI(PPPoEConnection *conn)
{
    PPPoEPacket packet;
    unsigned char *cursor = packet.payload;
    PPPoETag *svc = (PPPoETag *) (&packet.payload);
    UINT16_t namelen = 0;
    UINT16_t plen;
    int omit_service_name = 0;

    if (conn->serviceName) {
	namelen = (UINT16_t) strlen(conn->serviceName);
	if (!strcmp(conn->serviceName, "NO-SERVICE-NAME-NON-RFC-COMPLIANT")) {
	    omit_service_name = 1;
	}
    }

    
    memset(packet.ethHdr.h_dest, 0xFF, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADI;
    packet.session = 0;

    if (!omit_service_name) {
	plen = TAG_HDR_SIZE + namelen;
	CHECK_ROOM(cursor, packet.payload, plen);

	svc->type = TAG_SERVICE_NAME;
	svc->length = htons(namelen);

	if (conn->serviceName) {
	    memcpy(svc->payload, conn->serviceName, strlen(conn->serviceName));
	}
	cursor += namelen + TAG_HDR_SIZE;
    } else {
	plen = 0;
    }

    
    if (conn->useHostUniq) {
	PPPoETag hostUniq;
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	CHECK_ROOM(cursor, packet.payload, sizeof(pid) + TAG_HDR_SIZE);
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
    }

    
    if (MIN(lcp_allowoptions[0].mru, lcp_wantoptions[0].mru) > ETH_PPPOE_MTU) {
	PPPoETag maxPayload;
	UINT16_t mru = htons(MIN(lcp_allowoptions[0].mru, lcp_wantoptions[0].mru));
	maxPayload.type = htons(TAG_PPP_MAX_PAYLOAD);
	maxPayload.length = htons(sizeof(mru));
	memcpy(maxPayload.payload, &mru, sizeof(mru));
	CHECK_ROOM(cursor, packet.payload, sizeof(mru) + TAG_HDR_SIZE);
	memcpy(cursor, &maxPayload, sizeof(mru) + TAG_HDR_SIZE);
	cursor += sizeof(mru) + TAG_HDR_SIZE;
	plen += sizeof(mru) + TAG_HDR_SIZE;
    }

    packet.length = htons(plen);

    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));
}

void
waitForPADO(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    struct timeval expire_at;

    PPPoEPacket packet;
    int len;

    struct PacketCriteria pc;
    pc.conn          = conn;
    pc.acNameOK      = (conn->acName)      ? 0 : 1;
    pc.serviceNameOK = (conn->serviceName) ? 0 : 1;
    pc.seenACName    = 0;
    pc.seenServiceName = 0;
    conn->seenMaxPayload = 0;
    conn->error = 0;

    if (gettimeofday(&expire_at, NULL) < 0) {
	error("gettimeofday (waitForPADO): %m");
	return;
    }
    expire_at.tv_sec += timeout;

    do {
	if (BPF_BUFFER_IS_EMPTY) {
	    if (!time_left(&tv, &expire_at))
		return;		

	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
		r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
		if (r >= 0 || errno != EINTR) break;
	    }
	    if (r < 0) {
		error("select (waitForPADO): %m");
		return;
	    }
	    if (r == 0)
		return;		
	}

	
	receivePacket(conn->discoverySocket, &packet, &len);

	
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	
	if (!packetIsForMe(conn, &packet)) continue;

	if (packet.code == CODE_PADO) {
	    if (NOT_UNICAST(packet.ethHdr.h_source)) {
		error("Ignoring PADO packet from non-unicast MAC address");
		continue;
	    }
	    if (conn->req_peer
		&& memcmp(packet.ethHdr.h_source, conn->req_peer_mac, ETH_ALEN) != 0) {
		warn("Ignoring PADO packet from wrong MAC address");
		continue;
	    }
	    if (parsePacket(&packet, parsePADOTags, &pc) < 0)
		return;
	    if (conn->error)
		return;
	    if (!pc.seenACName) {
		error("Ignoring PADO packet with no AC-Name tag");
		continue;
	    }
	    if (!pc.seenServiceName) {
		error("Ignoring PADO packet with no Service-Name tag");
		continue;
	    }
	    conn->numPADOs++;
	    if (pc.acNameOK && pc.serviceNameOK) {
		memcpy(conn->peerEth, packet.ethHdr.h_source, ETH_ALEN);
		conn->discoveryState = STATE_RECEIVED_PADO;
		break;
	    }
	}
    } while (conn->discoveryState != STATE_RECEIVED_PADO);
}

static void
sendPADR(PPPoEConnection *conn)
{
    PPPoEPacket packet;
    PPPoETag *svc = (PPPoETag *) packet.payload;
    unsigned char *cursor = packet.payload;

    UINT16_t namelen = 0;
    UINT16_t plen;

    if (conn->serviceName) {
	namelen = (UINT16_t) strlen(conn->serviceName);
    }
    plen = TAG_HDR_SIZE + namelen;
    CHECK_ROOM(cursor, packet.payload, plen);

    memcpy(packet.ethHdr.h_dest, conn->peerEth, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADR;
    packet.session = 0;

    svc->type = TAG_SERVICE_NAME;
    svc->length = htons(namelen);
    if (conn->serviceName) {
	memcpy(svc->payload, conn->serviceName, namelen);
    }
    cursor += namelen + TAG_HDR_SIZE;

    
    if (conn->useHostUniq) {
	PPPoETag hostUniq;
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	CHECK_ROOM(cursor, packet.payload, sizeof(pid)+TAG_HDR_SIZE);
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
    }

    
    if (MIN(lcp_allowoptions[0].mru, lcp_wantoptions[0].mru) > ETH_PPPOE_MTU) {
	PPPoETag maxPayload;
	UINT16_t mru = htons(MIN(lcp_allowoptions[0].mru, lcp_wantoptions[0].mru));
	maxPayload.type = htons(TAG_PPP_MAX_PAYLOAD);
	maxPayload.length = htons(sizeof(mru));
	memcpy(maxPayload.payload, &mru, sizeof(mru));
	CHECK_ROOM(cursor, packet.payload, sizeof(mru) + TAG_HDR_SIZE);
	memcpy(cursor, &maxPayload, sizeof(mru) + TAG_HDR_SIZE);
	cursor += sizeof(mru) + TAG_HDR_SIZE;
	plen += sizeof(mru) + TAG_HDR_SIZE;
    }

    
    if (conn->cookie.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->cookie.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->cookie, ntohs(conn->cookie.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->cookie.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->cookie.length) + TAG_HDR_SIZE;
    }

    if (conn->relayId.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->relayId, ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
    }

    packet.length = htons(plen);
    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));
}

static void
waitForPADS(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    struct timeval expire_at;

    PPPoEPacket packet;
    int len;

    if (gettimeofday(&expire_at, NULL) < 0) {
	error("gettimeofday (waitForPADS): %m");
	return;
    }
    expire_at.tv_sec += timeout;

    conn->error = 0;
    do {
	if (BPF_BUFFER_IS_EMPTY) {
	    if (!time_left(&tv, &expire_at))
		return;		

	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
		r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
		if (r >= 0 || errno != EINTR) break;
	    }
	    if (r < 0) {
		error("select (waitForPADS): %m");
		return;
	    }
	    if (r == 0)
		return;		
	}

	
	receivePacket(conn->discoverySocket, &packet, &len);

	
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	
	if (memcmp(packet.ethHdr.h_source, conn->peerEth, ETH_ALEN)) continue;

	
	if (!packetIsForMe(conn, &packet)) continue;

	
	if (packet.code == CODE_PADS) {
	    
	    if (parsePacket(&packet, parsePADSTags, conn) < 0)
		return;
	    if (conn->error)
		return;
	    conn->discoveryState = STATE_SESSION;
	    break;
	}
    } while (conn->discoveryState != STATE_SESSION);

    
    conn->session = packet.session;

    info("PPP session is %d", (int) ntohs(conn->session));

    
    if (ntohs(conn->session) == 0 || ntohs(conn->session) == 0xFFFF) {
	error("Access concentrator used a session value of %x -- the AC is violating RFC 2516", (unsigned int) ntohs(conn->session));
    }
}

void
discovery(PPPoEConnection *conn)
{
    int padiAttempts = 0;
    int padrAttempts = 0;
    int timeout = conn->discoveryTimeout;

    do {
	padiAttempts++;
	if (padiAttempts > MAX_PADI_ATTEMPTS) {
	    warn("Timeout waiting for PADO packets");
	    close(conn->discoverySocket);
	    conn->discoverySocket = -1;
	    return;
	}
	sendPADI(conn);
	conn->discoveryState = STATE_SENT_PADI;
	waitForPADO(conn, timeout);

	timeout *= 2;
    } while (conn->discoveryState == STATE_SENT_PADI);

    timeout = conn->discoveryTimeout;
    do {
	padrAttempts++;
	if (padrAttempts > MAX_PADI_ATTEMPTS) {
	    warn("Timeout waiting for PADS packets");
	    close(conn->discoverySocket);
	    conn->discoverySocket = -1;
	    return;
	}
	sendPADR(conn);
	conn->discoveryState = STATE_SENT_PADR;
	waitForPADS(conn, timeout);
	timeout *= 2;
    } while (conn->discoveryState == STATE_SENT_PADR);

    if (!conn->seenMaxPayload) {
	
	if (lcp_allowoptions[0].mru > ETH_PPPOE_MTU)
	    lcp_allowoptions[0].mru = ETH_PPPOE_MTU;
	if (lcp_wantoptions[0].mru > ETH_PPPOE_MTU)
	    lcp_wantoptions[0].mru = ETH_PPPOE_MTU;
    }

    
    conn->discoveryState = STATE_SESSION;
    return;
}