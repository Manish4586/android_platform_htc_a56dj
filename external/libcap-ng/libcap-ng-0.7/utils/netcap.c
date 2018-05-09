/*
 * netcap.c - A program that lists network apps with capabilities
 * Copyright (c) 2009-10 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 *
 * The /proc/net/tcp|udp|raw parsing code was borrowed from netstat.c
 */

#include "config.h"
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include "cap-ng.h"
#include "proc-llist.h"

static llist l;
static int perm_warn = 0, header = 0, last_uid = -1;
static char *tacct = NULL;

static void usage(void)
{
	fprintf(stderr, "usage: netcap\n");
	exit(1);
}

static int collect_process_info(void)
{
	DIR *d, *f;
	struct dirent *ent;
	d = opendir("/proc");
	if (d == NULL) {
		fprintf(stderr, "Can't open /proc: %s\n", strerror(errno));
		return 1;
	}
	while (( ent = readdir(d) )) {
		FILE *sf;
		int pid, ppid;
		capng_results_t caps;
		char buf[100];
		char *tmp, cmd[16], state, *text, *bounds;
		int fd, len, euid = -1;

		
		if(*ent->d_name<'0' || *ent->d_name>'9')
			continue;
		errno = 0;
		pid = strtol(ent->d_name, NULL, 10);
		if (errno)
			continue;

		
		snprintf(buf, 32, "/proc/%d/stat", pid);
		fd = open(buf, O_RDONLY|O_CLOEXEC, 0);
		if (fd < 0)
			continue;
		len = read(fd, buf, sizeof buf - 1);
		close(fd);
		if (len < 40)
			continue;
		buf[len] = 0;
		tmp = strrchr(buf, ')');
		if (tmp)
			*tmp = 0;
		else
			continue;
		memset(cmd, 0, sizeof(cmd));
		sscanf(buf, "%d (%15c", &ppid, cmd);
		sscanf(tmp+2, "%c %d", &state, &ppid);

		
		if (pid == 2 || ppid == 2)
			continue;

		
		capng_clear(CAPNG_SELECT_BOTH);
		capng_setpid(pid);
		if (capng_get_caps_process())
			continue;
		caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
		if (caps <= CAPNG_NONE)
			continue;
		if (caps == CAPNG_FULL)
			text = strdup("full");
		else
			text = capng_print_caps_text(CAPNG_PRINT_BUFFER,
					CAPNG_PERMITTED);

		
		snprintf(buf, 32, "/proc/%d/status", pid);
		sf = fopen(buf, "rte");
		if (sf == NULL)
			euid = 0;
		else {
			int line = 0;
			__fsetlocking(sf, FSETLOCKING_BYCALLER);
			while (fgets(buf, sizeof(buf), sf)) {
				if (line == 0) {
					line++;
					continue;
				}
				if (memcmp(buf, "Uid:", 4) == 0) {
					int id;
					sscanf(buf, "Uid: %d %d",
						&id, &euid);
					break;
				}
			}
			fclose(sf);
		}

		
		if (caps == CAPNG_PARTIAL) {
			caps = capng_have_capabilities(CAPNG_SELECT_BOUNDS);
			if (caps == CAPNG_FULL)
				bounds = strdup("+");
			else
				bounds = strdup("");
		} else
			bounds = strdup("");

		
		snprintf(buf, 32, "/proc/%d/fd", pid);
		f = opendir(buf);
		if (f == NULL) {
			if (errno == EACCES) {
				if (perm_warn == 0) {
					printf("You may need to be root to "
						"get a full report\n");
					perm_warn = 1;
				}
			} else
				fprintf(stderr, "Can't open %s: %s\n", buf,
					strerror(errno));
			free(text);
			free(bounds);
			continue;
		}
		
		while (( ent = readdir(f) )) {
			char line[256], ln[256], *s, *e;
			unsigned long inode;
			lnode node;
			int llen;

			if (ent->d_name[0] == '.')
				continue;
			snprintf(ln, 256, "%s/%s", buf, ent->d_name);
			if ((llen = readlink(ln, line, sizeof(line)-1)) < 0)
				continue;
			line[llen] = 0;
			
			
			if (memcmp(line, "socket:", 7) == 0) {
				
				s = strchr(line+7, '[');
				if (s == NULL)
					continue;
				s++;
				e = strchr(s, ']');
				if (e == NULL)
					continue;
				*e = 0;
			} else if (memcmp(line, "[0000]:", 7) == 0) {
				
				s = line + 8;
			} else
				continue;
			errno = 0;
			inode = strtoul(s, NULL, 10);
			if (errno)
				continue;
			node.ppid = ppid;
			node.pid = pid;
			node.uid = euid;
			node.cmd = strdup(cmd);
			node.inode = inode;
			node.capabilities = strdup(text);
			node.bounds = strdup(bounds);
			
			list_append(&l, &node);
		}
		closedir(f);
		free(text);
		free(bounds);
	}
	closedir(d);
	return 0;
}

static void report_finding(int port, const char *type, const char *ifc)
{
	struct passwd *p;
	lnode *n = list_get_cur(&l);
		
	
	if (header == 0) {
		printf("%-5s %-5s %-10s %-16s %-4s %-6s %s\n",
			"ppid", "pid", "acct", "command", "type", "port", 
			"capabilities");
			header = 1;
	}
	if (n->uid == 0) {
		
		tacct = "root";
		last_uid = 0;
	} else if (last_uid != (int)n->uid) {
		
		p = getpwuid(n->uid);
		last_uid = n->uid;
		if (p)
			tacct = p->pw_name;
		
	}
	if (tacct) {
		printf("%-5d %-5d %-10s", n->ppid, n->pid, tacct);
	} else
		printf("%-5d %-5d %-10d", n->ppid, n->pid, last_uid);
	printf(" %-16s %-4s", n->cmd, type);
	if (ifc)
		printf(" %-6s", ifc);
	else
		printf(" %-6d", port);
	printf(" %s %s\n", n->capabilities, n->bounds);
}

static void read_tcp(const char *proc, const char *type)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	int local_port, rem_port, d, state, timer_run, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
				proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %512s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more);
		if (list_find_inode(&l, inode))
			report_finding(local_port, type, NULL);
	}
	fclose(f);
}

static void read_udp(const char *proc, const char *type)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	int local_port, rem_port, d, state, timer_run, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
					proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %512s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more);
		if (list_find_inode(&l, inode))
			report_finding(local_port, type, NULL);
	}
	fclose(f);
}

static void read_raw(const char *proc, const char *type)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	int local_port, rem_port, d, state, timer_run, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
					proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %512s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more);
		if (list_find_inode(&l, inode))
			report_finding(0, type, NULL);
	}
	fclose(f);
}

static void get_interface(unsigned int iface, char *ifc)
{
	unsigned int line = 0;
	FILE *f;
	char buf[256], more[256];

	
	*ifc = 0;

	
	iface++;

	f = fopen("/proc/net/dev", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/dev: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == iface) {
			char *c;
			sscanf(buf, "%16s: %256s\n", ifc, more);
			c = strchr(ifc, ':');
			if (c)
				*c = 0;
			fclose(f);
			return;
		}
		line++;
	}
	fclose(f);
}

static void read_packet(void)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long sk, inode;
	unsigned int ref_cnt, type, proto, iface, r, rmem, uid;
	char more[256], ifc[32];

	f = fopen("/proc/net/packet", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/packet: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%lX %u %u %X %u %u %u %u %lu %256s\n",
			&sk, &ref_cnt, &type, &proto, &iface,
			&r, &rmem, &uid, &inode, more);
		get_interface(iface, ifc);
		if (list_find_inode(&l, inode))
			report_finding(0, "pkt", ifc);
	}
	fclose(f);
}

int main(int argc, char __attribute__((unused)) *argv[])
{
	if (argc > 1) {
		fputs("Too many arguments\n", stderr);
		usage();
	}

	list_create(&l);
	collect_process_info();

	
	read_tcp("/proc/net/tcp", "tcp");
	read_tcp("/proc/net/tcp6", "tcp6");

	
	read_udp("/proc/net/udp", "udp");
	read_udp("/proc/net/udp6", "udp6");

	
	read_raw("/proc/net/raw", "raw");
	read_raw("/proc/net/raw6", "raw6");

	
	read_packet();

	list_clear(&l);
	return 0;
}

