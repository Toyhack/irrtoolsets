//
// $Id$
//
// Signal.hh
// Author(s): Ramesh Govindan

#ifndef _Signal_hh
#define _Signal_hh

#include "util/List.hh"
#include "util/Handler.hh"

// Classes defined elsewhere
//class Handler;

// This class encapsulates Unix signals and signal handling.
// Signals are known by a textual representation of the action or event:
//	e.g. "terminate", "restart", "dump" etc.
//

class Signal : public ListNode {
	//  friend Dispatcher::SignalList;
    
public:
    // Create a new signal
    Signal(const char*,		// In: textual representation of signal
           const Handler*);	// In: handler for signal
    ~Signal();
	Boolean pending() const { return pending_; }
	void pending(Boolean p) { pending_= p; }
	void handle(void *vp) { handler_.callBack(vp); }
	const char* name() {return name_;}
private:
    const char* name_;
    Boolean	pending_;
    Handler	handler_;
};

// 
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
//  info-ra@isi.edu.
//
//  Author(s): Ramesh Govindan <govindan@isi.edu>

#endif // _Signal_hh
