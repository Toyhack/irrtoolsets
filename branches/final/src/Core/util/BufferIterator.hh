//
// $Id$
//
// Author(s): Cengiz Alaettinoglu <cengiz@isi.edu>

#ifndef BUFFER_ITERATOR_HH
#define BUFFER_ITERATOR_HH

#include "config.h"
#include "util/Buffer.hh"

class BufferIterator {
   Buffer &buf;
   char    seperator;
   Buffer  current;
public:
   BufferIterator(Buffer &b, char sep) 
      : buf(b), seperator(sep), current() {
      current.callerAllocated = true;
      first();
   }
   ~BufferIterator() {}
   Buffer *first() {
      current.size = 0;
      current.contents = (char *) NULL;
      if (!buf.contents || !buf.size)
	 return (Buffer *) NULL;
      current.contents = buf.contents;
      char *next = (char *) memchr(current.contents, seperator, buf.size);
      if (next)
	 current.size = next - current.contents;
      else
	 current.size = buf.size;
      return &current;
   }

   Buffer *next() {
      if (!current.contents)
	 return (Buffer *) NULL;
      
      current.contents += current.size + 1;
      if (current.contents >= buf.contents + buf.size) {
	 current.contents = (char *) NULL;
	 current.size = 0;
	 return (Buffer *) NULL;
      }

      char *next = (char *) memchr(current.contents, seperator, 
				   buf.size - (current.contents-buf.contents));
      if (next)
	 current.size = next - current.contents;
      else
	 current.size = buf.size - (current.contents - buf.contents);
      return &current;
   }
   BufferIterator &operator++(int) {
      next();
      return *this;
   }
   operator Buffer*() { 
      return current.contents ? &current : (Buffer *) NULL; 
   }
   Buffer* operator()(void) { 
      return current.contents ? &current : (Buffer *) NULL; 
   }
};

#endif // BUFFER_ITERATOR_HH


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

