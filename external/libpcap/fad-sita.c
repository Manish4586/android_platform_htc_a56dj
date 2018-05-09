/*
 *  fad-sita.c: Packet capture interface additions for SITA ACN devices
 *
 *  Copyright (c) 2007 Fulko Hew, SITA INC Canada, Inc <fulko.hew@sita.aero>
 *
 *  License: BSD
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. The names of the authors may not be used to endorse or promote
 *     products derived from this software without specific prior
 *     written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "pcap-int.h"

#include "pcap-sita.h"

extern pcap_if_t	*acn_if_list;								

int pcap_findalldevs_interfaces(pcap_if_t **alldevsp, char *errbuf) {

	

	*alldevsp = 0;												
	strcpy(errbuf, "");
	if (acn_parse_hosts_file(errbuf))							
		{
		
		return -1;
		}
	
	if (acn_findalldevs(errbuf))								
		{
		
		return -1;
		}
	*alldevsp = acn_if_list;
	acn_if_list = 0;											
	
	return 0;
}
