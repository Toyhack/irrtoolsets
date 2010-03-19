/*
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
//  $Id$
//
//  Copyright (c) 1994 by the University of Southern California
//  All rights reserved.
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in
//    all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//    THE SOFTWARE.
//
//  Questions concerning this software should be directed to
//  irrtoolset@cs.usc.edu.
//
//  Author(s): eddy@isi.edu
*/

#include "net.hh"

//
// setName sets either the hexaddr or the hostname, then calls
// _getHostent() to fill in the hostent and copy the official
// values into the redundant vars in the ipAddr class
//
void
ipAddr::init_ipAddr () {
    bzero ((char *) &hostname, MAXHOSTNAMELEN);
    bzero ((char *) &dottedaddr, sizeof (dottedaddr));
    bzero ((char *) &maskedaddr, sizeof (maskedaddr));
    bzero ((char *) &asname, sizeof (asname));
    hp = (struct hostent *) NULL;
    mask   = 32;                // default to a host, unless changed
    hexaddr = 0;
}

int
ipAddr::setAddress (const char *addr)
{
    hp = (struct hostent *) NULL;
    hexaddr = (in_addr_t) inet_addr (addr);
    if (hexaddr != INADDR_NONE) {
	hp = gethostbyaddr ((char *) &hexaddr, sizeof (hexaddr), AF_INET);
    } else {
	hp = gethostbyname ((char *) addr);
    }   
    if (hp) {
	bcopy (hp->h_addr, (caddr_t) &hexaddr, (size_t) hp->h_length);
	strcpy (hostname, hp->h_name);
    }
    if (hp == NULL && hexaddr == INADDR_NONE) 
	error.fatal ("Could not resolve %s\0", addr);
    return (error());
}

int
ipAddr::setAddress (u_long hex)
{
    hp = (struct hostent *) NULL;
    hexaddr = hex;
    hp = gethostbyaddr ((char *) &hexaddr, sizeof (hexaddr), AF_INET);
    if (hp) {
	bcopy (hp->h_addr, (caddr_t) &hexaddr, (size_t) hp->h_length);
	strcpy (hostname, hp->h_name);
    }
    // we should do a range check on hexaddr and throw an exception
    if (!hp)
	error.fatal ("Could not set ipaddr for 0x%x", hex);
    return (error());
}

////////////////////////////////////////////////////////////////////////
Socket::Socket (u_int f, u_int t, u_int p)
{
    bzero ((char *) &sockdst, sizeof (sockdst));
    bzero ((char *) &socksrc, sizeof (socksrc));
    send_flags = 0;
    sockdst.sin_family = family = f;
    sockdst.sin_port = proto = p;
    socksrc.sin_family = family = f;
    socksrc.sin_port = proto = p;
    in = out = NULL;
    type   = t;
    sock = socket (family, type, proto);
    if (sock < 0) 
	error.fatal ("socket");
}

int
Socket::setsockopt (int level, int optname, char *data, int dlen)
{
    int on = 1;
    switch (level) {
      case SOL_SOCKET :
	if (optname & SO_DEBUG) {
	    (void) ::setsockopt (sock, level, SO_DEBUG,
				 (char *) &on, sizeof (on));
	}
	if (optname & SO_DONTROUTE) {
	    (void) ::setsockopt (sock, level, SO_DONTROUTE,
				 (char *) &on, sizeof (on));
	}
	if (optname & SO_SNDBUF) {
	    error = ::setsockopt(sock, level, SO_SNDBUF,
				 (char *)&data, dlen);
	    if (error())
		return error.fatal ("setsocketopt: SO_SNDBUF");
	}
	break;			// SOL_SOCKET
      case IPPROTO_IP:
	switch (optname) {
#ifdef IP_HDRINCL
	 case IP_HDRINCL:
	    error = ::setsockopt (sock, level, optname, (char *)&on, sizeof(on));
	    if (error())
		error("setsockopt: IP_HDRINCL");
	    return error();
	    break;		// IP_HDRINCL
#endif // IP_HDRINCL
	}
#ifdef IP_OPTIONS
      default:
	int err = ::setsockopt (sock, level, optname, (char *) data, dlen);
	if (err)
	    error.fatal ("setsockopt: level: %d, optname: %d\n", level, optname);
	return error();
	break;
#endif // IP_OPTIONS
    }
	return 0;
}

		    
int
Socket::write (char *buf, int len)
{
    char *b = (char *) buf;
    int l = len;
    while (l > 0) {
        int c = ::write (sock, b, l);
	if (c < 0) {
  	    if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
	      errno = 0;
	      continue;
	    }
	    return error.fatal ("write");
	}
	l -= c;
	b += c;
    }
    return (len - l);
}

int
Socket::read (char *buf, int len)
{
    int c = -1;
    while (c < 0) {
        c = ::read (sock, buf, len);
        if (c < 0) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                errno = 0;
                continue;
            }
            return error.fatal ("Socket::read");
        }
    }
    return c;
}
