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
//  Permission to use, copy, modify, and distribute this software and its
//  documentation in source and binary forms for lawful non-commercial
//  purposes and without fee is hereby granted, provided that the above
//  copyright notice appear in all copies and that both the copyright
//  notice and this permission notice appear in supporting documentation,
//  and that any documentation, advertising materials, and other materials
//  related to such distribution and use acknowledge that the software was
//  developed by the University of Southern California, Information
//  Sciences Institute. The name of the USC may not be used to endorse or
//  promote products derived from this software without specific prior
//  written permission.
//
//  THE UNIVERSITY OF SOUTHERN CALIFORNIA DOES NOT MAKE ANY
//  REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY
//  PURPOSE.  THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
//  TITLE, AND NON-INFRINGEMENT.
//
//  IN NO EVENT SHALL USC, OR ANY OTHER CONTRIBUTOR BE LIABLE FOR ANY
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, WHETHER IN CONTRACT, TORT,
//  OR OTHER FORM OF ACTION, ARISING OUT OF OR IN CONNECTION WITH, THE USE
//  OR PERFORMANCE OF THIS SOFTWARE.
//
//  Questions concerning this software should be directed to 
//  ratoolset@isi.edu.
//
//  Author(s): Cengiz Alaettinoglu <cengiz@ISI.EDU>

// Comments:
// any dynamic memory allocated by lexer or parser 
// is claimed by the constructors here
// They are either used internally or deleted using delete!
// if a pointer points to something in the dictionary, that is not claimed

#ifndef FILTER_HH
#define FILTER_HH

#include "config.h"
#include <cassert>
extern "C" {
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif // HAVE_MALLOC_H
}
#include "util/List.hh"
#include "symbols.hh"
#include "prefix.hh"
#include "gnu/prefixranges.hh"
#include "regexp.hh"
#include "rpsl_item.hh"

// Added by wlee@isi.edu
#ifdef DEBUG
#define INDENT(indent) for (int iii = 0; iii < indent; iii++) os << " "
#endif // DEBUG

typedef unsigned int ASt;

class ostream;
class AttrRPAttr;
class AttrMethod;

class Filter {
public:
   virtual ~Filter() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const = 0;
#ifdef DEBUG
   virtual const char *className(void) const = 0;
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "(*** Need more work here ***)" << endl;
   }
#endif // DEBUG
};

inline ostream &operator<<(ostream &out, const Filter &f) {
   return f.print(out);
}

class FilterOR: public Filter {
public:
   Filter *f1;
   Filter *f2;
public:
   FilterOR(Filter *_f1, Filter *_f2) : f1(_f1), f2(_f2) {
   }
   FilterOR(const FilterOR &pt) {
      f1 = (Filter *)pt.f1->dup();
      f2 = (Filter *)pt.f2->dup();
   }
   virtual ~FilterOR() {
      delete f1;
      delete f2;
   }
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterOR(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterOR";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "f1 (" << f1->className() << " *)" << endl;
      f1->printClass(os, indent + 2);
      INDENT(indent); os << "f2 (" << f2->className() << " *)" << endl;
      f2->printClass(os, indent + 2);
   }
#endif // DEBUG
};

class FilterAND: public Filter {
public:
   Filter *f1;
   Filter *f2;
public:
   FilterAND(Filter *_f1, Filter *_f2) : f1(_f1), f2(_f2) {
   }
   FilterAND(const FilterAND &pt) {
      f1 = (Filter *)pt.f1->dup();
      f2 = (Filter *)pt.f2->dup();
   }
   virtual ~FilterAND() {
      delete f1;
      delete f2;
   }
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterAND(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterAND";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "f1" << endl;
      f1->printClass(os, indent + 2);
      INDENT(indent); os << "f2" << endl;
      f2->printClass(os, indent + 2);
   }
#endif // DEBUG
};

class FilterNOT: public Filter {
public:
   Filter *f1;
public:
   FilterNOT(Filter *_f1) : f1(_f1) {
   }
   FilterNOT(const FilterNOT &pt) {
      f1 = (Filter *)pt.f1->dup();
   }
   virtual ~FilterNOT() {
      delete f1;
   }
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterNOT(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterNOT";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "f1 (NOT)" << endl;
      f1->printClass(os, indent + 2);
   }
#endif // DEBUG
};

class FilterEXCEPT: public Filter {
public:
   Filter *f1;
   Filter *f2;
public:
   FilterEXCEPT(Filter *_f1, Filter *_f2) : f1(_f1), f2(_f2) {
   }
   FilterEXCEPT(const FilterEXCEPT &pt) {
      f1 = (Filter *)pt.f1->dup();
      f2 = (Filter *)pt.f2->dup();
   }
   virtual ~FilterEXCEPT() {
      delete f1;
      delete f2;
   }
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterEXCEPT(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterEXCEPT";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "f1" << endl;
      f1->printClass(os, indent + 2);
      INDENT(indent); os << "f2" << endl;
      f2->printClass(os, indent + 2);
   }
#endif // DEBUG
};

class FilterMS: public Filter {
public:
   Filter *f1;
   int code;			// 0 for ^-, 1 for ^+, 2 for ^n and ^n-m
   int n;
   int m;
public:
   FilterMS(int _code) : f1(NULL), code(_code), n(0), m(0) {
   }
   FilterMS(int _code, int _n, int _m) : 
      f1(NULL), code(_code), n(_n), m(_m) {
   }
   FilterMS(const FilterMS &pt) : code(pt.code), n(pt.n), m(pt.m) {
      if (pt.f1)
	 f1 = (Filter *)pt.f1->dup();
      else
	 f1 = NULL;
   }
   virtual ~FilterMS() {
      if (f1)
	 delete f1;
   }
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterMS(*this);
   } 

#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterMS";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "f1 (" << f1->className() << " *)" << endl;
      f1->printClass(os, indent + 2);
   }
#endif // DEBUG
};

class FilterASNO : public Filter {
public:
   ASt asno;
public:
   FilterASNO(ASt asn) {
      asno = asn;
   }
   virtual ~FilterASNO() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterASNO(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterASNO";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "asno = " << asno << endl;
   }
#endif // DEBUG
};

class FilterASNAME : public Filter {
public:
   SymID asname;
public:
   FilterASNAME(SymID sid) {
      asname = sid;
   }
   virtual ~FilterASNAME() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterASNAME(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterASNAME";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "asname = \"" << asname << "\"" << endl;
   }
#endif // DEBUG
};

class FilterRSNAME : public Filter {
public:
   SymID rsname;
public:
   FilterRSNAME(SymID sid) {
      rsname = sid;
   }
   virtual ~FilterRSNAME() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterRSNAME(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterRSNAME";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "rsname = \"" << rsname << "\"" << endl;
   }
#endif // DEBUG
};

class FilterRTRSNAME : public Filter {
public:
   SymID rtrsname;
public:
   FilterRTRSNAME(SymID sid) {
      rtrsname = sid;
   }
   virtual ~FilterRTRSNAME() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterRTRSNAME(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterRTRSNAME";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "rtrsname = \"" << rtrsname << "\"" << endl;
   }
#endif // DEBUG
};

class FilterFLTRNAME : public Filter {
public:
   SymID fltrname;
public:
   FilterFLTRNAME(SymID sid) {
      fltrname = sid;
   }
   virtual ~FilterFLTRNAME() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterFLTRNAME(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterFLTRNAME";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "fltrname = \"" << fltrname << "\"" << endl;
   }
#endif // DEBUG
};

class FilterANY: public Filter {
public:
   FilterANY() {
   }
   virtual ~FilterANY() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterANY(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterANY";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "ANY" << endl;
   }
#endif // DEBUG
};

class FilterPeerAS: public Filter {
public:
   FilterPeerAS() {
   }
   virtual ~FilterPeerAS() {}
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterPeerAS(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterPeerAS";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "peerAS" << endl;
   }
#endif // DEBUG
};

class FilterASPath: public Filter {
public:
   regexp *re;
public:
   FilterASPath(regexp *_re) : re(_re) {
   }
   virtual ~FilterASPath() {
      delete re;
   }
   FilterASPath(const FilterASPath &b) {
      re = b.re->dup();
   }

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterASPath(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterASPath";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "re = ..." << endl;
   }
#endif // DEBUG
};

class FilterPRFXList: public Filter, public PrefixRanges {
public:
   FilterPRFXList() {
   }
   virtual ~FilterPRFXList() {}

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterPRFXList(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterPRFXList";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << *this << endl;
   }
#endif // DEBUG
};

class FilterMPPRFXList: public Filter, public MPPrefixRanges {
public:
   FilterMPPRFXList() {
   }
   virtual ~FilterMPPRFXList() {}

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterMPPRFXList(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterMPPRFXList";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << *this << endl;
   }
#endif // DEBUG
};


class FilterRPAttribute: public Filter {
public: 
   const AttrRPAttr *rp_attr;   // into rp-attribute in the dictionary
   const AttrMethod *rp_method; // into the method
   ItemList         *args;      // arguments to the method
public:
   FilterRPAttribute(const AttrRPAttr *_rp_attr, 
		     const AttrMethod *_rp_mtd, 
		     ItemList *_args): 
      rp_attr(_rp_attr), rp_method(_rp_mtd), args(_args) {}

   FilterRPAttribute(const FilterRPAttribute &b) :
      rp_attr(b.rp_attr), rp_method(b.rp_method) {
      args = (ItemList *) b.args->dup();
   }

   virtual ~FilterRPAttribute() {
      if (args) 
	 delete args;
   }

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterRPAttribute(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterRPAttribute";
   }
   virtual void printClass(ostream &os, int indent) const {
      // For rp_attr
      INDENT(indent); os << "rp_attr" << endl;
      INDENT(indent); os << "  _name = \"" << "\"" << endl;
      INDENT(indent); os << "  methods (RPMethod *)" << endl;
      INDENT(indent); os << "    _name = \"" << "\"" << endl;
      // For rp_method
      INDENT(indent); os << "rp_method" << endl;
      INDENT(indent); os << "  _name = \"" << "\"" << endl;
      // For rp_args
      INDENT(indent); os << "args (ItemList *)" << endl;
      args->printClass(os, indent + 2);
   }
#endif // DEBUG
};

////// The following is needed by route class ///////////////////////////

class FilterHAVE_COMPONENTS: public Filter {
public:
   FilterPRFXList *prfxs;
public:
   FilterHAVE_COMPONENTS(FilterPRFXList *p) : prfxs(p) {
   }
   FilterHAVE_COMPONENTS(const FilterHAVE_COMPONENTS &b) {
      prfxs = new FilterPRFXList(*b.prfxs);
   }
   virtual ~FilterHAVE_COMPONENTS() {
      delete prfxs;
   }

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterHAVE_COMPONENTS(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterHAVE_COMPONENTS";
   }
#endif // DEBUG
};

class FilterEXCLUDE: public Filter {
public:
   FilterPRFXList *prfxs;
public:
   FilterEXCLUDE(FilterPRFXList *p) : prfxs(p) {
   }
   FilterEXCLUDE(const FilterEXCLUDE &b) {
      prfxs = new FilterPRFXList(*b.prfxs);
   }
   virtual ~FilterEXCLUDE() {
      delete prfxs;
   }

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterEXCLUDE(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterEXCLUDE";
   }
#endif // DEBUG
};


////// The following is needed by <peering> ///////////////////////////

class FilterRouter : public Filter {
public:
   IPAddr *ip;
public:
   FilterRouter(IPAddr *_ip): ip(_ip) {
   }
   FilterRouter(const FilterRouter &b) {
      ip = new IPAddr(*b.ip);
   }

   virtual ~FilterRouter() {
      delete ip;
   }

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterRouter(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterRouter";
   }
#endif // DEBUG
};

// for mp-peering

class FilterIPv6Router : public Filter {
public:
   IPv6Addr *ip;
public:
   FilterIPv6Router(IPv6Addr *_ip) {
      ip = new IPv6Addr(*_ip);
   }
   FilterIPv6Router(const FilterIPv6Router &b) {
      ip = new IPv6Addr(*b.ip);
   }

   virtual ~FilterIPv6Router() {
      delete ip;
   }

   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterIPv6Router(*this);
   }
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterIPv6Router";
   }
#endif // DEBUG
};

class FilterRouterName : public Filter {
public:
   SymID name;
public:
   FilterRouterName(SymID dns) : name(dns) {}
   FilterRouterName(const FilterRouterName &b) {
      name = b.name;
   }
   virtual ~FilterRouterName() {
   }
   virtual ostream& print(ostream &out) const;
   virtual Filter* dup() const {
      return new FilterRouterName(*this);
   } 
#ifdef DEBUG
   virtual const char *className(void) const {
      return "FilterRouterName";
   }
   virtual void printClass(ostream &os, int indent) const {
      INDENT(indent); os << "name = \"" << name << "\"" << endl;
   }
#endif // DEBUG
};




#endif   // FILTER_HH
