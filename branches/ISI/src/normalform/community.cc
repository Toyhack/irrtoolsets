//  $Id$
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

#include "config.h"
#include "util/debug.hh"
#include "community.hh"
#include <iostream.h>
#include "rpsl/rpsl_item.hh"

void CommunitySet::addCommunity(Item *item) {
   if (typeid(*item) == typeid(ItemINT))
      add(((ItemINT *)item)->i);
   else 
      if (typeid(*item) == typeid(ItemWORD))
	 if (!strcasecmp(((ItemWORD *)item)->word, "no_advertise"))
	    add(COMMUNITY_NO_ADVERTISE);
	 else if (!strcasecmp(((ItemWORD *)item)->word, "no_export"))
	    add(COMMUNITY_NO_EXPORT);
	 else if (!strcasecmp(((ItemWORD *)item)->word,"no_export_subconfed"))
	    add(COMMUNITY_NO_EXPORT_SUBCONFED);
	 else
	    add(COMMUNITY_INTERNET);
      else 
	 if (typeid(*item) == typeid(ItemList)) {
	    int high = ((ItemINT *) ((ItemList *) item)->head())->i;
	    int low  = ((ItemINT *) ((ItemList *) item)->tail())->i;
	    add((high << 16) + low);
	 } else
	    cerr << "Warning: Ignoring non-community value " << *item << endl;
}

ostream& operator<<(ostream &os, CommunitySet& cs) {
   for (Pix p = cs.first(); p;) {
      community_print(os, cs(p));
      cs.next(p);
      if (p)
	 os << ", ";
   }
   return os;
}
