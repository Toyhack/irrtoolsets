//
// Copyright (c) 2001,2002                        RIPE NCC
//
// All Rights Reserved
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose and without fee is hereby granted,
// provided that the above copyright notice appear in all copies and that
// both that copyright notice and this permission notice appear in
// supporting documentation, and that the name of the author not be
// used in advertising or publicity pertaining to distribution of the
// software without specific, written prior permission.
//
// THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
// ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS; IN NO EVENT SHALL
// AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
// DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$
//
// Author(s): Ramesh Govindan

#ifndef _ICMProbe_hh
#define _ICMProbe_hh

#include "util/Handler.hh"
#include "sys/Time.hh"

// Classes defined elsewhere
class Handler;
class IP;

enum ICMProbeResult {
    ICMProbeResultNone,
    ICMProbeResultTimeExpired,
    ICMProbeResultUnreachable,
    ICMProbeResultReachedNode,
    ICMProbeResultSourceRouteFailed,
    ICMProbeResultDontCare
};

class ICMProbe : public DListNode {
  private:
    ICMProbe(const ICMProbe&);

  public:
    // Constructor
    ICMProbe(Address*,		// In: address to traceroute
	     Handler&,		// In: handler to call back on reply
	     int,		// In: number of hops
	     Address* = NULL);	// In: via a specified gateway

    ~ICMProbe();

    // Handle timers
    void
    handleTimer();

    // Send one traceroute packet
    void
    send();
    
    unsigned int getProbeId() { return probeId;}
    
    TimeLong getSentAt() { return sentAt;}

    // Receive one traceroute packet
    Boolean			// Out: true if matched
    receive(IP*);

    Address		to;
    Address		via;
    int			hopcount;
    Address		responder;
    ICMProbeResult	result;
    int			residualHops;
    int			sent;
    TimeShort		lastRtt;

  private:
    Timer*		timer;
    Handler		done;
    unsigned int	probeId;
    TimeLong		sentAt;
};

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

#endif _Traceroute_hh
