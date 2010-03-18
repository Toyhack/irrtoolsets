//  $Id$
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

// Copyright (C) 1993 Cengiz Alaettinoglu <ca@cs.umd.edu>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You didn't receive a copy of the GNU General Public License along
// with this program; so, write to the Free Software Foundation, Inc.,
// 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Author(s): Cengiz Alaettinoglu <ca@cs.umd.edu>
//            Computer Science Department
//            University of Maryland
//            College Park, MD 20742

#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"
#ifdef DEBUG
#include <iostream.h>

#define DBG_ERR             1
#define DBG_INFO            2 

void Abort();
void copy_constructor(char *p);

class dbgstream {
public:
   dbgstream();
   dbgstream(dbgstream& a) { copy_constructor("dbgstream"); }
   int operator[](int _level) { 
      level = _level;
      return enabledp();
   }
   void dbgstream::enable();
   void dbgstream::disable();
   void dbgstream::enable(int level);
   void dbgstream::disable(int level);
   int  dbgstream::enabledp();
   int  dbgstream::enabledp(int level);
private:
   int level;
   int enabled;
};

extern dbgstream dbg;

//----------------------------------------------------------------------
// ASSERT
//      If condition is false,  print a message and dump core.
//	Useful for documenting assumptions in the code.
//
//	NOTE: needs to be a #define, to be able to print the location 
//	where the error occurred.
//----------------------------------------------------------------------
/*
#ifndef ASSERT
#define ASSERT(condition)                                                     \
    if (!(condition)) {                                                       \
        cerr << __FILE__ << ":" << __LINE__                                   \
	     << ": Assertion failed `" << #condition << "'\n";                \
        Abort();                                                              \
    }
#endif // ASSERT
*/

#define Debug(whatever) whatever
#define Channel(no) if (dbg.enabledp(no)) cerr
#define PRINT(var) cout << #var << " = " << var << endl;




#else /* DEBUG */
#define copy_constructor(p) 
#ifndef ASSERT
#define ASSERT(condition) /* do nothing */
#endif // ASSERT
#define Debug(whatever)   /* do nothing */
#define PRINT(var)        /* do nothing */

#endif /* DEBUG */

#endif /* DEBUG_H */

#ifdef DEBUG_MEMORY
#define CLASS_DEBUG_MEMORY_HH(classX)        \
   void* operator new(size_t size) {         \
      classX::count++;                             \
      return ::operator new(size);           \
   }                                         \
   void operator delete(void* p) {           \
      classX::count--;                             \
      ::delete(p);                           \
   }                                         \
   static int count                         
#define CLASS_DEBUG_MEMORY_CC(classX) \
   int classX::count = 0
#define CLASS_DEBUG_MEMORY_PRINT(classX) \
   dbg[DBG_INFO] << #classX << " remaining: " << classX::count << "\n"
#else // DEBUG_MEMORY
#define CLASS_DEBUG_MEMORY_HH(classX)
#define CLASS_DEBUG_MEMORY_CC(classX)
#define CLASS_DEBUG_MEMORY_PRINT(classX)
#endif // DEBUG_MEMORY
