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

#include "config.h"
#include <cstring>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>
#include <strstream.h>
#include <cctype>
#include "normalform/NE.hh"
#include "irr/irr.hh"
#include "irr/autnum.hh"
#include "RtConfig.hh"
#include "f_cisco.hh"
#include "f_junos.hh"
#include "rpsl/schema.hh"

#define DBG_JUNOS 7
#define EXPORT 0
#define IMPORT 1
#define STATIC 2

char JunosConfig::mapName[80] = "policy"; 
char JunosConfig::mapNameFormat[80] = "policy_%d_%d"; 
bool JunosConfig::useAclCaches = true;
bool JunosConfig::compressAcls = true;
bool JunosConfig::usePrefixLists = false;
int  JunosConfig::mapIncrements = 1;
int  JunosConfig::mapCount = 1;
int  JunosConfig::mapNumbersStartAt = 1;
bool JunosConfig::printRouteMap = true;
bool JunosConfig::setMEDtoIGP = false;
bool JunosConfig::exportStatics = false;

//////////////////////////// caches ////////////////////////////////

//AccessListManager<regexp_nf>         aspathMgr;
//AccessListManager<SetOfPrefix>       prefixMgr(100);
//AccessListManager<SetOfPrefix>       pktFilterMgr(100);
//AccessListManager<FilterOfCommunity> communityMgr;

AccessListManager<CommunitySet> communityMgr2;

void JunosConfig::printMartians() {
   static int done = 0;

   if (done || ! supressMartians)
      return;

   done = 1;

   // print martians
   char *martians = " 
            route-filter 127.0.0.0/8 orlonger reject;
            route-filter 10.0.0.0/8 orlonger reject;
            route-filter 172.16.0.0/12 orlonger reject;
            route-filter 192.168.0.0/16 orlonger reject;
            route-filter 192.0.2.0/24 orlonger reject;
            route-filter 128.0.0.0/16 orlonger reject;
            route-filter 191.255.0.0/16 orlonger reject;
            route-filter 192.0.0.0/24 orlonger reject;
            route-filter 223.255.255.0/24 orlonger reject;
            route-filter 224.0.0.0/3 orlonger reject;
            route-filter 169.254.0.0/16 orlonger reject;
            route-filter 0.0.0.0/0 upto /31 next policy;
            route-filter 0.0.0.0/0 upto /32 reject;
";
   
   cout << "   policy-statement supress-martians {\n"
	<< "      term martians {\n"
	<< "         from {" 
	<<              martians
	<< "         }\n"
	<< "      }\n"
	<< "   }\n\n";
}

ListOf2Ints *JunosConfig::printRoutes(SetOfPrefix& nets) {
   // return the access list number if something is printed
   ListOf2Ints *result;

   if (nets.universal())
      return NULL;

   // check to see if we already printed an identical access list, 
   if (useAclCaches && (result = prefixMgr.search(nets)))
      return result;

   result = prefixMgr.add(nets);
   int aclID = prefixMgr.newID();
   result->add(aclID, aclID);

   int allow_flag = 1;
   if (nets.negated())
      allow_flag = 0;

   char buffer[64];
   char *permitOrDeny = " reject;";
   char *negatedPermitOrDeny = " accept;";
   if (allow_flag) {
      permitOrDeny = " accept;";
      negatedPermitOrDeny = " reject;";
   }

   cout << "   policy-statement prefix-list-" << aclID << " {\n"
	<< "      term prefixes {\n"
	<< "         from {\n";

   if (compressAcls) {
      RadixSet::SortedPrefixRangeIterator itr(&nets.members);
      u_int addr;
      u_int leng;
      u_int start;
      u_int end;
      for (bool ok = itr.first(addr, leng, start, end);
	   ok;
	   ok = itr.next(addr, leng, start, end)) {
	 for (unsigned int sibling=0; sibling < (1<<(start-leng)); ++sibling) {
	    unsigned int siblingMask = sibling << (32 - start);
	    unsigned int siblingAddr = addr | siblingMask;

	    cout << "            route-filter ";
	    cout << int2quad(buffer, siblingAddr) << "/" << start;

	    if (end != start)
	       cout << " upto /" << end;
	    else
	       cout << " exact";
	 
	    cout  << permitOrDeny << "\n";
	 }
      }
   } else {
      RadixSet::SortedPrefixIterator itr(&nets.members);
      u_int addr;
      u_int leng;
      char buffer[64];

      for (bool ok = itr.first(addr, leng);
	   ok;
	   ok = itr.next(addr, leng)) {
	 cout << "            route-filter ";

	 cout << int2quad(buffer, addr) << "/" << leng;

	 cout  << " exact" << permitOrDeny << "\n";
      }
   }

   cout << "         }\n"
	<< "      }\n"
	<< "      term catch-rest {\n";
   
   // terminate the acess lis
   if (allow_flag)
      cout << "          then reject;\n";
   else
      cout << "          then accept;\n";

   cout << "      }\n"
	<< "   }\n\n";

   return result;
}

void JunosConfig::printREASno(ostream& out, const RangeList &no) {
   RangeList::Range *pi, *qi;
   int first = 1;
   int put_par = 0;

   if (no.universal())
      out << " .";
   else {
      pi = no.ranges.head();
      put_par = ! no.ranges.isSingleton() || pi->high != pi->low;
      if (put_par)
	 out << "("; 
      else
	 out << " ";

      for (; pi; pi = no.ranges.next(pi)) {
	 if (!first) 
	    out << "|";
	 else
	    first = 0;
	 out << pi->low;
	 if (pi->low != pi->high)
	    out << "-" << pi->high;
      } 
      if (put_par)
         out << ")";
   }
}

int JunosConfig::printRE_(ostream& os, const regexp& r, 
			  bool &hasBOL, bool &hasEOL) {
   int flag = 0;

   if (typeid(r) == typeid(regexp_bol)) {
      hasBOL = true;
      return flag;
   }

   if (typeid(r) == typeid(regexp_eol)) {
      hasEOL = true;
      return 1;
   }

   if (typeid(r) == typeid(regexp_symbol)) {
      printREASno(os, ((regexp_symbol &) r).asnumbers);
      return flag;
   }

   if (typeid(r) == typeid(regexp_star)) {
      bool save = reSplittable;
      reSplittable = false;
      os << "(";
      flag = printRE_(os, *((regexp_star &) r).left, hasBOL, hasEOL);
      os << ")*";
      reSplittable = save;
      return flag;
   }

   if (typeid(r) == typeid(regexp_plus)) {
      bool save = reSplittable;
      reSplittable = false;
      os << "(";
      flag = printRE_(os, *((regexp_plus &) r).left, hasBOL, hasEOL);
      os << ")+";
      reSplittable = save;
      return flag;
   }

   if (typeid(r) == typeid(regexp_tildastar)) {
      bool save = reSplittable;
      reSplittable = false;
      os << "(";
      flag = printRE_(os, *((regexp_tildastar &) r).left, hasBOL, hasEOL);
      os << ")*";
      reSplittable = save;
      return flag;
   }

   if (typeid(r) == typeid(regexp_tildaplus)) {
      bool save = reSplittable;
      reSplittable = false;
      os << "(";
      flag = printRE_(os, *((regexp_tildaplus &) r).left, hasBOL, hasEOL);
      os << ")+";
      reSplittable = save;
      return flag;
   }

   if (typeid(r) == typeid(regexp_question)) {
      os << "(";
      flag = printRE_(os, *((regexp_question &) r).left, hasBOL, hasEOL);
      os << ")?";
      return flag;
   }

   if (typeid(r) == typeid(regexp_cat)) {
      printRE_(os, *((regexp_cat &) r).left, hasBOL, hasEOL);
      flag = printRE_(os, *((regexp_cat &) r).right, hasBOL, hasEOL);
      return flag;
   }

   if (typeid(r) == typeid(regexp_or)) {
      os << "(";
      flag = printRE_(os, *((regexp_or &) r).left, hasBOL, hasEOL);
      os << "|";
      flag &= printRE_(os, *((regexp_or &) r).right, hasBOL, hasEOL);
      os  << ")";
      return flag;
   }

   assert(1);
   os << "REGEXP_UNKNOWN";
   return flag;
}

int JunosConfig::printRE(ostream& os, const regexp& r) {
   strstream sout;
   bool hasBOL = false;
   bool hasEOL = false;
   printRE_(sout, r, hasBOL, hasEOL);
   sout << ends;
   
   if (!hasBOL)
      os << ".* ";
   os << sout.str();
   if (!hasEOL)
      os << " .*";   
   sout.freeze(0);

   return 1;
}

ListOf2Ints* JunosConfig::printASPaths(FilterOfASPath &path) {
// return the access list number if something is printed
   ListOf2Ints *result;

   if (path.is_universal())
      return NULL;

   // check to see if we already printed an identical access list
   if (useAclCaches && (result = aspathMgr.search(path)))
      return result;

   result = aspathMgr.add(path);
   int aclID = aspathMgr.newID();
   result->add(aclID, aclID);

   regexp *reg = path.re->construct();
   RangeList::prefix_str = "";
   RangeList::useOrList = true;

   regexp_nf::RegexpConjunct *rc;
   regexp_nf::RegexpConjunct::ReInt *ri;
   regexp_nf::RegexpConjunct::ReInt *positive_ri = NULL;

   cout << "   as-path as-path-" << aclID << " \"";

   if (path.re->rclist.size() == 1) {
      rc = path.re->rclist.head(); 
      ri = rc->regexs.head();
      if (rc->regexs.isSingleton() && !ri->negated)
	 printRE(cout, *ri->re);
      else
	 cout << *reg;
   } else
      cout << *reg;

   cout << "\";\n\n";
   delete reg;

   return result;
}
   
inline void JunosConfig::printCommunity(ostream &os, unsigned int i) {
   if (i == COMMUNITY_INTERNET)
      os << "internet";
   else if (i == COMMUNITY_NO_EXPORT)
      os << "no-export";
   else if (i == COMMUNITY_NO_EXPORT_SUBCONFED)
      os << "no-export-subconfed";
   else if (i == COMMUNITY_NO_ADVERTISE)
      os << "no-advertise";
   else {
      int high = i >> 16;
      int low  = i & 0xFFFF;
      os << high << ":" << low;
   }
}

int JunosConfig::printCommunityList(ostream &os, ItemList *args) {
   CommunitySet set;
   for (Item *cmnty = args->head(); cmnty; cmnty = args->next(cmnty)) {
      if (typeid(*cmnty) == typeid(ItemINT)) {
	 set.add(((ItemINT *) cmnty)->i);
	 continue;
      }
      if (typeid(*cmnty) == typeid(ItemWORD)) {
	 if (!strcasecmp(((ItemWORD *)cmnty)->word, "no_advertise"))
	    set.add(COMMUNITY_NO_ADVERTISE);
	 else if (!strcasecmp(((ItemWORD *)cmnty)->word, "no_export"))
	    set.add(COMMUNITY_NO_EXPORT);
	 else if (!strcasecmp(((ItemWORD *)cmnty)->word,"no_export_subconfed"))
	    set.add(COMMUNITY_NO_EXPORT_SUBCONFED);
	 else
	    set.add(COMMUNITY_INTERNET);
	 continue;
      }

      if (typeid(*cmnty) == typeid(ItemList)) {
	 int high = ((ItemINT *) ((ItemList *) cmnty)->head())->i;
	 int low  = ((ItemINT *) ((ItemList *) cmnty)->tail())->i;
	 set.add((high << 16) + low);
      }
   }
   return printCommunitySet(os, &set, false);
}

int JunosConfig::printCommunitySet(ostream &os, CommunitySet *set, bool exact) {
   ListOf2Ints *result;

   // check to see if we already printed an identical access list,
   set->mark = exact;
   if (useAclCaches && (result = communityMgr2.search(*set)))
      return result->head()->start;

   result = communityMgr2.add(*set);
   int id = communityMgr2.newID();
   result->add(id, id);

   os << "   community community-" << id << " members ";
   if (exact)
      os << "^";
   else
      os << "[";
   for (Pix pi = set->first(); pi; ) {
      printCommunity(os, (*set)(pi));
      set->next(pi);
      if (pi)
	 os << " ";
   }
   if (exact)
      os << "$;\n";
   else
      os << "];\n";

   return id;
}

ListOf2Ints *JunosConfig::printCommunities(FilterOfCommunity& cm) {
   // return the access list numbers if something is printed
   ListOf2Ints *result;
   ostrstream lastCout;
   ostrstream midCout;
   int term = 0;
   int id;

   if (cm.is_universal())
      return 0;

   // check to see if we already printed an identical access list,
   if (useAclCaches && (result = communityMgr.search(cm)))
      return result;

   result = communityMgr.add(cm);
   int aclID = communityMgr.newID();
   result->add(aclID, aclID);

   lastCout << "   policy-statement community-pol-" << aclID << " {\n";

   // now print the communities
   // first print the conjuncts which are only positives
   id = aclID;
   bool firstTime = true;
   CommunityConjunct *cc;
   for (cc = cm.conjuncts.head(); cc; cc = cm.conjuncts.next(cc)) {
      if (cc->n->isEmpty() && cc->ne.isEmpty()) {
	 if (firstTime) {
	    lastCout << "      term comm-" << aclID << "-" << ++term << " {\n"
		     << "          from {\n";
	    lastCout << "            community [ ";
	    firstTime = false;
	 }
	 if (!cc->p->isEmpty()) 
	    id = printCommunitySet(cout, cc->p, false);
	 else
	    id = printCommunitySet(cout, cc->pe, true);
	 lastCout << "community-" << id << " ";

	 cc->marked = true;
      } else
	 cc->marked = false;
   }
   
   if (!firstTime)
      lastCout << "];\n"
	       << "         }\n"
	       << "         then accept;\n"
	       << "      }\n";

   int count = 0;
   for (cc = cm.conjuncts.head(); cc; cc = cm.conjuncts.next(cc)) {
      if (cc->marked) // this case is done above or below
         continue;

      cc->marked = true;
      // this term has ne and/or n, there may also p, but not pe
      count ++;

      if (count > 1) 
	 midCout << "   }\n\n"
		 << "   policy-statement community-pol-inner-" 
		 << aclID << "-" << count << " {\n";
      
      midCout << "      term comm-" << aclID << "-" << ++term << " {\n"
	      << "         from community [ ";

      for (CommunitySet *cs = cc->ne.head(); cs; cs = cc->ne.next(cs)) {
	 id = printCommunitySet(cout, cs, true);
	 midCout << "community-" << id << " ";
      }

      for (Pix pi = cc->n->first(); pi; cc->n->next(pi)) {
	 CommunitySet *cs = new CommunitySet((*cc->n)(pi));
	 id = printCommunitySet(cout, cs, false);
	 delete cs;
	 midCout << "community-" << id << " ";
      }

      midCout << "];\n"
	      << "         then reject;\n"
	      << "      }\n";

      bool found = false;
      for (CommunityConjunct *cc2 = cc; cc2; cc2 = cm.conjuncts.next(cc2))
	 if (*cc2->n == *cc->n && cc2->ne == cc->ne && ! cc2->p->isEmpty()) {
	    if (!found)
	       midCout << "      term comm-" << aclID << "-" << ++term <<" {\n"
		       << "         from community [ ";
	    id = printCommunitySet(cout, cc2->p, false);
	    midCout << "community-" << id << " ";
	    cc2->marked = true;
	    found = true;
	 }

      if (found)
	 midCout << "];\n"
		 << "         then accept;\n"
		 << "      }\n";
      else
	 midCout << "      term community-" << aclID << "-catch-rest {\n"
		 << "         then accept;\n"
		 << "      }\n";
   }

   if (count > 1) {
      cout << "\n"
	   << "   policy-statement community-pol-inner-" 
	   << aclID << "-" << 1 << " {\n";

      midCout << ends;
      cout << midCout.str();
      midCout.freeze(0);
      
      cout << "   }\n\n";
   
      lastCout << ends;
      cout << "\n" << lastCout.str();
      lastCout.freeze(0);

      for (int i = 1; i <= count; ++i)
	 cout << "      term comm-" << aclID << "-" << ++term << " {\n"
	      << "         from policy community-pol-inner-" 
	                           << aclID << "-" << count << ";\n"
	      << "         then accept;\n"
	      << "      }\n";
   } else {
      lastCout << ends;
      cout << "\n" << lastCout.str();
      lastCout.freeze(0);

      midCout << ends;
      cout << midCout.str();
      midCout.freeze(0);
   }      

   cout << "      term community-" << aclID << "-catch-rest {\n"
	<< "         then reject;\n"
	<< "      }\n"
	<< "   }\n\n";

   return result;
}

void JunosConfig::printActions(ostream &os, PolicyActionList *actions) {
#define UNIMPLEMENTED_METHOD \
   cerr << "Warning: unimplemented method " \
	<< actn->rp_attr->name << "." << actn->rp_method->name << endl

   PolicyAction *actn;
   for (actn = actions->head(); actn; actn = actions->next(actn)) {
      if (actn->rp_attr == dctn_rp_pref) {
	 if (actn->rp_method == dctn_rp_pref_set) {
	    int pref = ((ItemINT *) actn->args->head())->i;
	    os << "            preference " << pref << ";\n";
	 } else 
	    UNIMPLEMENTED_METHOD;
	 continue;
      }

      if (actn->rp_attr == dctn_rp_nhop) {
	 if (actn->rp_method == dctn_rp_nhop_set) {
	    char buffer[32];
	    IPAddr *ip = ((ItemIPV4 *) actn->args->head())->ipv4;
	    ip->get_text(buffer);
	    os << "            nexthop " << buffer << ";\n";
	 } else 
	    UNIMPLEMENTED_METHOD;
	 continue;
      }

      if (actn->rp_attr == dctn_rp_dpa) {
	 if (actn->rp_method == dctn_rp_dpa_set) {
	    int dpa = ((ItemINT *) actn->args->head())->i;
	    os << "            dpa " << dpa << ";\n";
	 } else 
	    UNIMPLEMENTED_METHOD;
	 continue;
      }

      if (actn->rp_attr == dctn_rp_med) {
	 if (actn->rp_method == dctn_rp_med_set) {
	    Item *item = actn->args->head();
	    if (typeid(*item) == typeid(ItemINT))
	       os << "            metric " << ((ItemINT *) item)->i << ";\n";
	    else
	       setMEDtoIGP = true;
	 } else 
	    UNIMPLEMENTED_METHOD;
	 continue;
      }

      if (actn->rp_attr == dctn_rp_community) {
	 if (actn->rp_method == dctn_rp_community_setop) {
	    os << "            community set community-" 
	       << printCommunityList(os, (ItemList *) actn->args->head())
	       << ";\n";
	 } else if (actn->rp_method == dctn_rp_community_appendop) {
	    os << "            community add community-"
	       << printCommunityList(os, (ItemList *) actn->args->head())
	       << ";\n";
	 } else if (actn->rp_method == dctn_rp_community_append) {
	    os << "            community add community-"
	       << printCommunityList(os, actn->args)
	       << ";\n";
	 } else if (actn->rp_method == dctn_rp_community_delete) {
	    os << "            community delete community-"
	       << printCommunityList(os, actn->args)
	       << ";\n";
	 } else
	    UNIMPLEMENTED_METHOD;
	 continue;
      }

      if (actn->rp_attr == dctn_rp_aspath) {
	 if (actn->rp_method == dctn_rp_aspath_prepend) {
	    os << "            as-path-prepend \"";
	    for (Item *plnd = actn->args->head();
		 plnd;
		 plnd = actn->args->next(plnd))
	       os << ((ItemASNO *) plnd)->asno << " ";
	    os << "\";\n";
	 } else
	    UNIMPLEMENTED_METHOD;
	 continue;
      }

      cerr << "Warning: unimplemented attribute " 
	   << *actn->rp_attr << endl;
   }

   os << "            accept;\n";
}

int JunosConfig::printDeclarations(NormalExpression *ne, 
				   PolicyActionList *actions, 
				   int import_flag) {
   PolicyAction *actn;
   for (actn = actions->head(); actn; actn = actions->next(actn)) {
      if (actn->rp_attr == dctn_rp_community) {
	 if (actn->rp_method == dctn_rp_community_setop) {
	    (void) printCommunityList(cout, (ItemList *) actn->args->head());
	 } else if (actn->rp_method == dctn_rp_community_appendop) {
	    (void) printCommunityList(cout, (ItemList *) actn->args->head());
	 } else if (actn->rp_method == dctn_rp_community_append) {
	    (void) printCommunityList(cout, actn->args);
	 } else if (actn->rp_method == dctn_rp_community_delete) {
	    (void) printCommunityList(cout, actn->args);
	 }
      }
   }

   for (NormalTerm *nt = ne->first(); nt; nt = ne->next()) {
      (void) printCommunities(nt->community);
      (void) printASPaths(nt->as_path);
      (void) printRoutes(nt->prfx_set);
   }

   return 1;
}

int JunosConfig::print(NormalExpression *ne, PolicyActionList *actn, 
		       int import_flag) {
   int last = 0;
   static ListOf2Ints empty_list(1);

   Debug(Channel(DBG_JUNOS) << "# ne: " << *ne << "\n");

   if (ne->isEmpty()) 
      return last;

   if (ne->is_universal()) 
      last = 1;

   if (strcmp(mapName, lastMapName)) {
      strcpy(lastMapName, mapName);
      routeMapID = mapNumbersStartAt;
   }

   for (NormalTerm *nt = ne->first(); nt; nt = ne->next()) {
      ListOf2Ints *aspath_acls = printASPaths(nt->as_path);
      ListOf2Ints *comm_acls   = printCommunities(nt->community);
      ListOf2Ints *prfx_acls   = printRoutes(nt->prfx_set);

      if (!aspath_acls)
	 aspath_acls = &empty_list;
      if (!comm_acls)
	 comm_acls = &empty_list;
      if (!prfx_acls)
	 prfx_acls = &empty_list;
      
      ListNodeOf2Ints *asp, *cmp, *prp; 
      int i;

      if (printRouteMap) {
	 for (asp = aspath_acls->head(); asp; asp = aspath_acls->next(asp)) {
	    for (cmp = comm_acls->head(); cmp; cmp = comm_acls->next(cmp)) {
	       for (prp = prfx_acls->head(); prp; prp =prfx_acls->next(prp)) {
		  cout << "      term " << mapName << "-term-" << routeMapID << " {\n"
		       << "         from {\n";
		  
		  if (import_flag == STATIC)
		     cout << "            protocol static;\n";

		  routeMapID += mapIncrements;
		  for (i = asp->start; i <= asp->end; i++)
		     cout << "            as-path as-path-" << i << ";\n";
	       
		  for (i = cmp->start; i <= cmp->end; i++)
		     cout << "            policy community-pol-" << i << ";\n";

		  for (i = prp->start; i <= prp->end; i++)
		     cout << "            policy prefix-list-" << i << ";\n";

		  cout << "         }\n"
		       << "         then {\n";
		  JunosConfig::printActions(cout, actn);
		  cout << "         }\n";
		  cout << "      }\n\n";
	       }
	    }
	 }
      }
   }

   return last;
}

bool JunosConfig::printNeighbor(int import, 
				ASt peerAS, char *neighbor, bool peerGroup) {
   if (! printRouteMap)
      return false;

   const char *direction = (import == IMPORT) ? "import" : "export";

   cout << "protocols {\n"
	<< "   bgp {\n"
	<< "      group peer-" << neighbor << " {\n"
	<< "         type external;\n"
        << "         peer-as " << peerAS << ";\n"
	<< "         neighbor " << neighbor << " {\n"
	<< "            " << direction << " ";

   if ((exportStatics && import == EXPORT) || supressMartians)
      cout << "[ ";
   if (exportStatics && import == EXPORT)
      cout << "static2bgp ";
   if (supressMartians)
      cout << "supress-martians ";
   cout << mapName << " ";
   if ((exportStatics && import == EXPORT) || supressMartians)
      cout << "]";
   cout << ";\n";
  
   if (setMEDtoIGP) {
      cout << "            metric-out igp;\n";
      setMEDtoIGP = false;
   }

   cout << "         }\n"
	<< "      }\n"
	<< "   }\n"
	<< "}\n\n";

   return true;
}

void JunosConfig::exportP(ASt asno, IPAddr *addr, 
			  ASt peerAS, IPAddr *peer_addr) {

   // Made asno part of the map name if it's not changed by users
   sprintf(mapName, mapNameFormat, asno, mapCount++);

   // get the aut-num object
   const AutNum *autnum = irr->getAutNum(asno);

   if (!autnum) {
      cerr << "Error: no object for AS" << asno << endl;
      return;
    }

   // get matching export attributes
   AutNumSelector<AttrExport> itr(autnum, "export", 
				  NULL, peerAS, peer_addr, addr);
   const FilterAction *fa = itr.first();
   if (! fa)
      cerr << "Warning: AS" << asno 
	   << " has no export policy for AS" << peerAS << endl;

   cout << "policy-options {\n";
   printMartians();

   NormalExpression *ne;
   int last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, peerAS);
      last = printDeclarations(ne, fa->action, EXPORT);
      delete ne;
   }

   cout << "   policy-statement " << mapName << " {\n";

   fa = itr.first();
   last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, peerAS);
      last = print(ne, fa->action, EXPORT);
      delete ne;
   }

   cout << "      term " << mapName << "-catch-rest {\n"
	<< "         then reject;\n"
        << "      }\n"
	<< "   }\n"
	<< "}\n\n";

   printNeighbor(EXPORT, peerAS, peer_addr->get_text(), false);
}

void JunosConfig::importP(ASt asno, IPAddr *addr, 
			 ASt peerAS, IPAddr *peer_addr) {

   // Made asno part of the map name if it's not changed by users
   sprintf(mapName, mapNameFormat, asno, mapCount++);

   // get the aut-num object
   const AutNum *autnum = irr->getAutNum(asno);

   if (!autnum) {
      cerr << "Error: no object for AS" << asno << endl;
      return;
    }

   int preAclID = prefixMgr.lastID();
   // get matching import attributes
   AutNumSelector<AttrImport> itr(autnum, "import", 
				  NULL, peerAS, peer_addr, addr);
   const FilterAction *fa = itr.first();
   if (! fa)
      cerr << "Warning: AS" << asno 
	   << " has no import policy for AS" << peerAS << endl;

   cout << "policy-options {\n";
   printMartians();

   NormalExpression *ne;
   int last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, peerAS);
      last = printDeclarations(ne, fa->action, IMPORT);
      delete ne;
   }

   cout << "   policy-statement " << mapName << " {\n";

   fa = itr.first();
   last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, peerAS);
      last = print(ne, fa->action, IMPORT);
      delete ne;
   }

   cout << "      term " << mapName << "-catch-rest {\n"
	<< "         then reject;\n"
        << "      }\n"
	<< "   }\n"
	<< "}\n\n";

   printNeighbor(IMPORT, peerAS, peer_addr->get_text(), false);
}

void JunosConfig::static2bgp(ASt asno, IPAddr *addr) {
   // Made asno part of the map name if it's not changed by users
   strcpy(mapName, "static2bgp");

   // get the aut-num object
   const AutNum *autnum = irr->getAutNum(asno);
   
   if (!autnum) {
      cerr << "Error: no object for AS" << asno << endl;
      return;
   }

   // get matching import attributes
   AutNumSelector<AttrImport> itr(autnum, "import", 
				  NULL, asno, addr, addr, "STATIC");
   const FilterAction *fa = itr.first();
   if (! fa)
      cerr << "Warning: AS" << asno 
	   << " has no static2bgp policy for AS" << asno << endl;

   cout << "policy-options {\n";
   printMartians();

   NormalExpression *ne;
   int last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, asno);
      last = printDeclarations(ne, fa->action, STATIC);
      delete ne;
   }

   cout << "   policy-statement static2bgp {\n";

   fa = itr.first();
   last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, asno);
      last = print(ne, fa->action, STATIC);
      delete ne;
   }

   cout << "      term static2bgp-catch-rest {\n"
	<< "         from protocol static;\n"
	<< "         then reject;\n"
        << "      }\n"
	<< "   }\n"
	<< "}\n\n";

   exportStatics = true;
}

void JunosConfig::networks(ASt asno) {
   static char buffer[128];
   static char buffer2[128];
   const PrefixRanges *nets = irr->expandAS(asno);

   for (int i = nets->low(); i < nets->fence(); ++i)
      cout << "   network " << int2quad(buffer, (*nets)[i].get_ipaddr())
           << " mask " << int2quad(buffer2, (*nets)[i].get_mask()) 
           << ";\n";
}

void JunosConfig::exportGroup(ASt asno, char * pset) {
   // Made asno part of the map name if it's not changed by users
   sprintf(mapName, mapNameFormat, asno, mapCount++);

   // get the aut-num object
   const AutNum *autnum = irr->getAutNum(asno);

   if (!autnum) {
      cerr << "Error: no object for AS" << asno << endl;
      return;
    }

   SymID psetID = symbols.symID(pset);
   // get matching export attributes
   AutNumSelector<AttrExport> itr(autnum, "export", psetID, ~0, NULL, NULL);
   const FilterAction *fa = itr.first();
   if (! fa)
      cerr << "Warning: AS" << asno 
	   << " has no export policy for " << pset << endl;

   cout << "policy-options {\n";
   printMartians();

   NormalExpression *ne;
   int last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, ~0);
      last = printDeclarations(ne, fa->action, EXPORT);
      delete ne;
   }

   cout << "   policy-statement " << mapName << " {\n";

   fa = itr.first();
   last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, ~0);
      last = print(ne, fa->action, EXPORT);
      delete ne;
   }

   cout << "      term " << mapName << "-catch-rest {\n"
	<< "         then reject;\n"
        << "      }\n"
	<< "   }\n"
	<< "}\n\n";

   const char *direction = "export";

   const PeeringSet *prngSet = irr->getPeeringSet(psetID);
   if (!prngSet)
      return;

   Filter *peer_as = NULL;
   for (AttrIterator<AttrPeering> itr(prngSet, "peering"); itr; itr++)
      if (typeid(*itr()->peering->peerRtrs) == typeid(FilterRouter)
	  && typeid(*itr()->peering->peerASes) == typeid(FilterASNO)) {
	 peer_as = itr()->peering->peerASes;
	 break;
      }

   cout << "protocols {\n"
	<< "   bgp {\n"
	<< "      group peers-" << pset << " {\n"
	<< "         type external;\n"
	<< "         peer-as ";

   if (peer_as)
      cout << peer_as << ";\n";
   else
      cout << "1; # this will be overriden below\n";

   cout << "         " << direction << " ";

   if (exportStatics || supressMartians)
      cout << "[ ";
   if (exportStatics)
      cout << "static2bgp ";
   if (supressMartians)
      cout << "supress-martians ";
   cout << mapName << " ";
   if (exportStatics || supressMartians)
      cout << "]";
   cout << ";\n";
  
   for (AttrIterator<AttrPeering> itr(prngSet, "peering"); itr; itr++)
      if (typeid(*itr()->peering->peerRtrs) == typeid(FilterRouter)
	  && typeid(*itr()->peering->peerASes) == typeid(FilterASNO)) {
	 cout << "         neighbor " << *itr()->peering->peerRtrs << " {\n"
	      << "            peer-as " << *itr()->peering->peerASes <<";\n"
	      << "         }\n";
      }

   cout << "      }\n"
	<< "   }\n"
	<< "}\n\n";
}

void JunosConfig::importGroup(ASt asno, char * pset) {
   // Made asno part of the map name if it's not changed by users
   sprintf(mapName, mapNameFormat, asno, mapCount++);

   // get the aut-num object
   const AutNum *autnum = irr->getAutNum(asno);

   if (!autnum) {
      cerr << "Error: no object for AS" << asno << endl;
      return;
    }

   SymID psetID = symbols.symID(pset);
   // get matching import attributes
   AutNumSelector<AttrImport> itr(autnum, "import", psetID, ~0, NULL, NULL);
   const FilterAction *fa = itr.first();
   if (! fa)
      cerr << "Warning: AS" << asno 
	   << " has no import policy for " << pset << endl;

   cout << "policy-options {\n";
   printMartians();

   NormalExpression *ne;
   int last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, ~0);
      last = printDeclarations(ne, fa->action, IMPORT);
      delete ne;
   }

   cout << "   policy-statement " << mapName << " {\n";

   fa = itr.first();
   last = 0;
   for (; fa && !last; fa = itr.next()) {
      ne = NormalExpression::evaluate(fa->filter, ~0);
      last = print(ne, fa->action, IMPORT);
      delete ne;
   }

   cout << "      term " << mapName << "-catch-rest {\n"
	<< "         then reject;\n"
        << "      }\n"
	<< "   }\n"
	<< "}\n\n";

   const char *direction = "import";

   const PeeringSet *prngSet = irr->getPeeringSet(psetID);
   if (! prngSet)
      return;

   Filter *peer_as = NULL;
   for (AttrIterator<AttrPeering> itr(prngSet, "peering"); itr; itr++)
      if (typeid(*itr()->peering->peerRtrs) == typeid(FilterRouter)
	  && typeid(*itr()->peering->peerASes) == typeid(FilterASNO)) {
	 peer_as = itr()->peering->peerASes;
	 break;
      }

   cout << "protocols {\n"
	<< "   bgp {\n"
	<< "      group peers-" << pset << " {\n"
	<< "         type external;\n"
	<< "         peer-as ";

   if (peer_as)
      cout << peer_as << ";\n";
   else
      cout << "1; # this will be overriden below\n";

   cout << "         " << direction << " ";

   if (supressMartians)
      cout << "[ supress-martians " << mapName << " ];\n";
   else
      cout << mapName << ";\n";
   
   for (AttrIterator<AttrPeering> itr(prngSet, "peering"); itr; itr++)
      if (typeid(*itr()->peering->peerRtrs) == typeid(FilterRouter)
	  && typeid(*itr()->peering->peerASes) == typeid(FilterASNO)) {
	 cout << "         neighbor " << *itr()->peering->peerRtrs << " {\n"
	      << "            peer-as " << *itr()->peering->peerASes <<";\n"
	      << "         }\n";
      }
   
   cout << "      }\n"
	<< "   }\n"
	<< "}\n\n";
}

