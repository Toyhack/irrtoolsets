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
//

#ifndef IPV6RADIXTREE_H
#define IPV6RADIXTREE_H

#include "config.h"
#include <cstdio>
#include <sys/types.h>
#include "Stack.hh"
#include "rpsl/List.hh"
#include "rpsl/prefix.hh"

#ifndef foreachchild
#define foreachchild(x) for (int x = 0; x < 2; ++x)
#endif //foreachchild

#ifndef u_int64_t
#define u_int64_t unsigned long long int
#endif

#ifndef u_int
#define u_int unsigned int
#endif

#include "FixedSizeAllocator.hh"

extern FixedSizeAllocator IPv6RadixTreeAllocator;
extern ip_v6word_t ipv6masks[];
extern ip_v6word_t ipv6bits[];

class IPv6RadixTree {
public:
   class Iterator {
   private:
      const IPv6RadixTree* last;
      const IPv6RadixTree* root;
      BoundedStack<const IPv6RadixTree*> dfsStack;
      Iterator(const Iterator &);

   public:
      Iterator(const IPv6RadixTree* r) : last((const IPv6RadixTree *) NULL), 
	 root(r), dfsStack(65) {}
      inline const IPv6RadixTree* first();
      inline const IPv6RadixTree* next(const IPv6RadixTree* _last);
   };

public:
   ipv6_addr_t addr;
   u_int leng;
   ipv6_addr_t rngs;
   enum Direction { LEFT = 0, RIGHT = 1, HERE, UP};

private:
   IPv6RadixTree *chld[2];
  
public:

   IPv6RadixTree(ipv6_addr_t _addr, u_int _leng, ipv6_addr_t _rngs) : 
      addr(_addr), leng(_leng), rngs(_rngs) {

      foreachchild(c)
	 chld[c] = (IPv6RadixTree *) NULL;
   }

   IPv6RadixTree(const IPv6RadixTree &b) : addr(b.addr), leng(b.leng), rngs(b.rngs) {
      foreachchild(c)
	 chld[c] = b.chld[c] ? new IPv6RadixTree(*b.chld[c]) : (IPv6RadixTree *) NULL;
    //  cout << "2 " << "ip" << addr << "leng" << leng << "rng" << rngs << endl;
   }

   ~IPv6RadixTree() {
      foreachchild(c)
	 if (chld[c])
	    delete chld[c];
   }

   IPv6RadixTree *insert(ipv6_addr_t addr, u_int leng, ipv6_addr_t rngs);
   IPv6RadixTree *remove(ipv6_addr_t addr, u_int leng, ipv6_addr_t rngs);

   bool contains(ipv6_addr_t addr, u_int leng, ipv6_addr_t rngs) const;

   bool equals(const IPv6RadixTree *b) const;

   IPv6RadixTree *and_(const IPv6RadixTree *b);      // this becomes this and      b
   IPv6RadixTree *or_(const IPv6RadixTree *b);       // this becomes this or       b
   IPv6RadixTree *setminus(const IPv6RadixTree *b); // this becomes this setminus b

   void print() const;
   void *operator new(size_t s) {
      return IPv6RadixTreeAllocator.allocate();
   }
   void operator delete(void *p, size_t s) {
      IPv6RadixTreeAllocator.deallocate(p);
   }
   IPv6RadixTree *makeMoreSpecific(int code, int n, int m);

protected:
   IPv6RadixTree();

private:
   IPv6RadixTree *removeRange(ipv6_addr_t _rngs);
   Direction direction(ipv6_addr_t addr, u_int leng, ipv6_addr_t _addr, u_int _leng) const;
   void commonAnscestor(ipv6_addr_t _addr,  u_int _leng, ipv6_addr_t addr,   u_int leng, ipv6_addr_t &paddr, u_int &pleng) const;
};

class IPv6RadixSet {

public:
   static bool compressedPrint;

   class Iterator {
   private:
      IPv6RadixTree::Iterator itr;
      const IPv6RadixTree *now;     // points to current node during iteration

   public:
      Iterator(const IPv6RadixSet *s) : itr(s->root) {}
      bool first(ipv6_addr_t &_addr, u_int &_leng, ipv6_addr_t &rngs);
      bool next(ipv6_addr_t &_addr, u_int &_leng, ipv6_addr_t &rngs);
   };

   class SortedIterator {
   private:
      class PrefixLNode : public ListNode {
      public:
      	 ipv6_addr_t addr;
      	 u_int leng;
      	 ipv6_addr_t rngs;

      public:
      	 PrefixLNode(ipv6_addr_t _addr, u_int _leng, ipv6_addr_t _rngs) : 
   	    addr(_addr), leng(_leng), rngs(_rngs) {}
      };
      List<PrefixLNode> l;
      const IPv6RadixSet *set;

   public:
      SortedIterator(const IPv6RadixSet *s) : set(s) {}
      ~SortedIterator() {
	       l.clear();
      }
      bool first(ipv6_addr_t &_addr, u_int &_leng, ipv6_addr_t &_rngs);
      bool next(ipv6_addr_t &_addr, u_int &_leng, ipv6_addr_t &_rngs);
   };

   class PrefixIterator {
   private:
      IPv6RadixTree::Iterator itr;
      const IPv6RadixTree *current;
      PrefixIterator(const PrefixIterator &);
      ipv6_addr_t addr;
      u_int leng;
      ipv6_addr_t rngs;
      u_int cleng;
      ipv6_addr_t number;

   public:
      PrefixIterator(const IPv6RadixSet *s) : itr(s->root) {}
      bool first(ipv6_addr_t &_addr, u_int &_leng);
      bool next(ipv6_addr_t &_addr, u_int &_leng);
   };

   class SortedPrefixIterator {
   private:
      class PrefixLNode : public ListNode {
      public:
	 ipv6_addr_t addr;
	 u_int leng;

	 PrefixLNode(ipv6_addr_t _addr, u_int _leng) : 
	    addr(_addr), leng(_leng) {}
      };
      List<PrefixLNode> l;
      const IPv6RadixSet *set;

   public:
      SortedPrefixIterator(const IPv6RadixSet *s) : set(s) {
      }
      ~SortedPrefixIterator() {
	 l.clear();
      }
      bool first(ipv6_addr_t &_addr, u_int &_leng);
      bool next(ipv6_addr_t &_addr, u_int &_leng);
   };

   class PrefixRangeIterator {
   private:
      IPv6RadixTree::Iterator itr;
      const IPv6RadixTree *current;
      PrefixRangeIterator(const PrefixRangeIterator &);
      ipv6_addr_t addr;
      u_int leng;
      ipv6_addr_t rngs;
      u_int cleng;

   public:
      PrefixRangeIterator(const IPv6RadixSet *s) : itr(s->root) {}
      bool first(ipv6_addr_t &_addr, u_int &_leng, u_int &_start, u_int &_end);
      bool next(ipv6_addr_t &_addr, u_int &_leng, u_int &_start, u_int &_end);
   };

   class SortedPrefixRangeIterator {
   private:
      class PrefixLNode : public ListNode {
      public:
	 ipv6_addr_t addr;
	 u_int leng;
	 u_int start;
	 u_int end;

	 PrefixLNode(ipv6_addr_t _addr, u_int _leng, u_int _start, u_int _end) : 
	    addr(_addr), leng(_leng), start(_start), end(_end) {}
      };
      List<PrefixLNode> l;
      const IPv6RadixSet *set;

   public:
      SortedPrefixRangeIterator(const IPv6RadixSet *s) : set(s) {}
      ~SortedPrefixRangeIterator() {
	 l.clear();
      }
      bool first(ipv6_addr_t &_addr, u_int &_leng, u_int &_start, u_int &_end);
      bool next(ipv6_addr_t &_addr, u_int &_leng, u_int &_start, u_int &_end);
   };

private:
   IPv6RadixTree *root;


public:
   IPv6RadixSet() {
     root = (IPv6RadixTree *) NULL;
   }

   IPv6RadixSet(const IPv6RadixSet &b) {
      if (b.root) {
         root = new IPv6RadixTree(*b.root);
      } else {
         root = (IPv6RadixTree *) NULL;
      }
   }

   ~IPv6RadixSet() {
      if (root)
	 delete root;
   }

   void insert(ipv6_addr_t addr, u_int leng, ipv6_addr_t rngs) {
     root = root->insert(addr, leng, rngs);
   }
   void insert(ipv6_addr_t addr, u_int leng) {
     root = root->insert(addr, leng, addr.getbits(leng) );
   }
   void remove(ipv6_addr_t addr, u_int leng, ipv6_addr_t rngs) {
     root = root->remove(addr, leng, rngs);
   }
   void remove(ipv6_addr_t addr, u_int leng) {
     root = root->remove(addr, leng, addr.getbits(leng));
   }
   bool contains(ipv6_addr_t addr, u_int leng, ipv6_addr_t rngs) const {
      return root->contains(addr, leng, rngs);
   }
   bool contains(ipv6_addr_t addr, u_int leng) const {
      return root->contains(addr, leng, addr.getbits(leng));
   }

   void makeMoreSpecific(int code, int n, int m) {
      root = root->makeMoreSpecific(code, n, m);
   }

   void clear() {
      if (root) {
	 delete root;
	 root = (IPv6RadixTree *) NULL;
      }
   }

   bool isEmpty() const {
      return !root;
   }

   void operator |= (const IPv6RadixSet& b) {
      root = root->or_(b.root);
   }
   void operator &= (const IPv6RadixSet& b) {
      root = root->and_(b.root);
   }
   void operator -= (const IPv6RadixSet& b) {
      // TBD: fix the bug in remove (see setminus) and use setminus instead (quicker!)
      //root = root->setminus(b.root); 

      if (*this == b) {
        this->clear();
        return;
      }

      if (!b.root || !this->root)
        return;

      IPv6RadixSet::PrefixIterator itr(this);
      IPv6RadixSet *result = new IPv6RadixSet;
      ipv6_addr_t addr;
      u_int leng;
      for (bool flag = itr.first(addr, leng); flag; 
                flag = itr.next(addr, leng)) {
        if (! b.contains(addr, leng)) {
          result->insert(addr, leng);
        } 
      }
      if (result->root)
        *this = *result;
      else 
        this->clear();
   }
   int  operator == (const IPv6RadixSet& b) const {
      return root->equals(b.root);
   }
   void operator =  (const IPv6RadixSet& b) {
      if (root == b.root)
	 return;
      delete root;
      root = new IPv6RadixTree(*b.root);
   }

   friend std::ostream& operator<<(std::ostream&, const IPv6RadixSet &set);
};

#endif // IPV6RADIXTREE_H
