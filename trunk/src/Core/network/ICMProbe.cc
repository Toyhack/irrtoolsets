//
// $Id$
//
// Author(s): Ramesh Govindan

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

extern "C" {
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
}

#include "util/Types.hh"
#include "util/Trail.hh"
#include "util/Handler.hh"
#include "util/Buffer.hh"
#include "sys/File.hh"
#include "sys/Pipe.hh"
#include "sys/Time.hh"
#include "sched/Timer.hh"
#include "sched/Dispatcher.hh"

#include "network/ICMProbe.hh"
#include "network/Headers.hh"
#include "network/Network.hh"

// Constants
static const unsigned char 	UDPProtocol = 17;

// Locals
static TraceCode	traceICMProbe("net");
static TimeShort	maxWait(10, 0);
static unsigned int	probeNumber = 0x80000000;

static void
icmprobeTimerHandler(void* ptr,
                   void*)
{
    ((ICMProbe*) ptr)->handleTimer();
}

ICMProbe::ICMProbe(Address* a,
		   Handler& cb,
		   int hc,
		   Address* v)
   : ListNode()
{
    done = cb;
    to = *a;
    hopcount = hc;
    if (v) {
        via = *v;
    }
    timer = NULL;
    result = ICMProbeResultNone;
    residualHops = 0;
    network->pendingICMProbes.append(this);
    probeId = 0;
    lastRtt = InfiniteInterval;
    sent = 0;
}

ICMProbe::~ICMProbe()
{
    network->pendingICMProbes.remove(this);
    if (timer) {
        delete timer;
    }
}

void
ICMProbe::send()
{
    Handler	th(icmprobeTimerHandler, this);
    Handler	nh(NULL, NULL);
    Buffer*	buf;
    IP*		ip;
    IPOptLSRR*	ipopt;
    ICMP*       icmp;
    RawSocket*	raw;
    U32*	t;
    TimeLong	at;
    Address	srcaddr;

    responder.set(0);
    result = ICMProbeResultNone;
    sent++;
    network->icmprobesSent++;

    // Send packet
    raw = new RawSocket(nh, nh, RawSocketICMP);
    if (via.get()) {
        buf = new Buffer(sizeof(IP) + sizeof(IPOptLSRR) + sizeof(ICMP) + 8);
    } else {
        buf = new Buffer(sizeof(IP) + sizeof(ICMP) + 8);
    }
    buf->zeroFill();

    ip = (IP*) buf->contents;
    
    
    ip->headerLength = via.get() ? 
        (sizeof(IP) + sizeof(IPOptLSRR)) >> 2 :
         sizeof(IP) >> 2;
    ip->version = 4;
         
    ip->typeOfService = 0;
    ip->totalLength = (ip->headerLength << 2) + sizeof(ICMP);
    ip->identifier = 0;
    ip->fragmentOffset = 0;
    ip->timeToLive = hopcount;
    ip->protocol = IPPROTO_ICMP;
    ip->source = (network->interfaces.head())->address.get();
    ip->destination = (via.get()) ? via.get() : to.get();
    ip->hton();
    ip->checksum = 0;
    ip->checksum = network->cksum((U16*) ip, ip->headerLength << 2);
    
    if (via.get()) {
        IPOptLSRR*	lsrr;

        lsrr = (IPOptLSRR*) (ip + 1);
        lsrr->nullopt = IPOPT_NOP;
        lsrr->lsrropt = IPOPT_LSRR;
        lsrr->length = sizeof(U32) + 3;
        lsrr->pointer = IPOPT_MINOFF;
        lsrr->gateway = to.get();
        lsrr->hton();
        icmp = (ICMP*) (lsrr + 1);
    } else {
        icmp = (ICMP*) (ip + 1);
    }

    ASSERT(probeNumber != 0xffffffff);	// Wrap around
    probeId = ++probeNumber;

    icmp->type = ICMPTypeEcho;
    icmp->code = 0;
    icmp->gateway = probeId;
    icmp->hton();
    icmp->checksum = 0;
    icmp->checksum = network->cksum((U16*) icmp, sizeof(ICMP) + 8);

    dispatcher.systemClock.sync();
    sentAt = dispatcher.systemClock;
    raw->sendTo(buf->contents, buf->size, to, 0);

    TRACE(traceICMProbe, "icmprobe to %s:%d\n", to.name(), hopcount);

    delete raw;
    delete buf;
    
    if (timer) {
        delete timer;
    }
    at = dispatcher.systemClock;
    at = at + maxWait;
    timer = new Timer(th, at);

    return;
}

void
ICMProbe::handleTimer()
{
    TRACE(traceICMProbe, "icmprobe failed to %s\n", to.name());
    
    timer = NULL;
    network->icmprobesTimedOut++;
    
    lastRtt = InfiniteInterval;
    done.callBack((void*) this);
    return;
}

Boolean
ICMProbe::receive(IP* ip)
{
    ICMP*		icmp;
    unsigned short	sum;
    IP*			iip;
    int			datalen;

    // Verify signature to see if it is for us
    icmp = (struct ICMP*) ((char*) ip + (ip->headerLength << 2));

    if (icmp->type != ICMPTypeTimeExceeded &&
        icmp->type != ICMPTypeDestinationUnreachable &&
	icmp->type != ICMPTypeEchoReply)
        return false;

    if (icmp->type == ICMPTypeTimeExceeded ||
        icmp->type == ICMPTypeDestinationUnreachable) {

       datalen = ip->totalLength - (ip->headerLength << 2) - sizeof(ICMP);
       if (datalen < sizeof(IP))
	  return false;		// Malformed response, discard

       iip = (struct IP*) (icmp + 1);
       iip->ntoh();

       if (datalen < ((iip->headerLength << 2) + 2 * sizeof(unsigned long))) {
          iip->ntoh();
	  return false;		// Timestamp not there, discard
       }

       if (iip->destination != to.get()) {
          iip->ntoh();
	  return false;
       }

       ICMP* icmpi = (ICMP*) (((char*) iip) + (iip->headerLength<<2));
       icmpi->ntoh();

       if (icmpi->gateway != probeId) { // it is not the pkt we sent
          iip->ntoh();
          icmpi->ntoh();
  	  return false;
       }
    }

    dispatcher.systemClock.sync();
    lastRtt = dispatcher.systemClock - sentAt;

    switch (icmp->type) {
    case ICMPTypeEchoReply:
       if (ip->source != to.get()) {
	  return false;
       }
       if (icmp->gateway != probeId) {
	  return false;
       }
        
       responder.set(ip->source);
       residualHops = 0;
       result = ICMProbeResultReachedNode;

       TRACE(traceICMProbe, "received icmprobe for %s:%d from %s\n",
	     to.name(), hopcount, responder.name());
       done.callBack((void*) this);
       break;

    case ICMPTypeTimeExceeded:
       if (iip->timeToLive <= 1) {
	  responder.set(ip->source);
	  result = ICMProbeResultTimeExpired;
	  TRACE(traceICMProbe, "received icmprobe for %s:%d from %s\n",
		to.name(), hopcount, responder.name());
       } else {
	  responder.set(ip->source);
	  ERROR("unexpected ttl %d in ICMP TimeExceeded from %s\n",
		iip->timeToLive, responder.name());
	  result = ICMProbeResultDontCare;
       } 
       done.callBack((void*) this);
       break;

    case ICMPTypeDestinationUnreachable:
       residualHops = iip->timeToLive;
       responder.set(ip->source);
       switch (icmp->code) {
       case ICMPUnreachNet:
       case ICMPUnreachHost:
	  result = ICMProbeResultUnreachable;
	  break;
	  
       case ICMPUnreachProto:
       case ICMPUnreachPort:
	  result = ICMProbeResultReachedNode;
	  break;
	  
       case ICMPUnreachSrtFail:
	  result = ICMProbeResultSourceRouteFailed;
	  break;
	  
       case ICMPUnreachNeedFrag:
       default:
	  result = ICMProbeResultDontCare;
	  break;
       }
       TRACE(traceICMProbe, "received icmprobe for %s:%d from %s\n",
	     to.name(), hopcount, responder.name());
       done.callBack((void*) this);
       // FallThrough
       
    default:
       break;
    }
    return true;
}

//  Copyright (c) 1994 by the University of Southern California.
//  All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software and
//  its documentation in source and binary forms for lawful
//  non-commercial purposes and without fee is hereby granted, provided
//  that the above copyright notice appear in all copies and that both
//  the copyright notice and this permission notice appear in supporting
//  documentation, and that any documentation, advertising materials,
//  and other materials related to such distribution and use acknowledge
//  that the software was developed by the University of Southern
//  California and/or Information Sciences Institute.
//  The name of the University of Southern California may not
//  be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THE UNIVERSITY OF SOUTHERN CALIFORNIA DOES NOT MAKE ANY REPRESENTATIONS
//  ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.  THIS SOFTWARE IS
//  PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
//  INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND 
//  NON-INFRINGEMENT.
//
//  IN NO EVENT SHALL USC, OR ANY OTHER CONTRIBUTOR BE LIABLE FOR ANY
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, WHETHER IN CONTRACT,
//  TORT, OR OTHER FORM OF ACTION, ARISING OUT OF OR IN CONNECTION WITH,
//  THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
//  Questions concerning this software should be directed to 
//  scan@isi.edu.
//


