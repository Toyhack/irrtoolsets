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

#ifndef RPTYPE_HH
#define RPTYPE_HH

#include "config.h"
#include "util/List.hh"
#include <climits>
#include <iostream.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <strstream.h>

const unsigned int UINT_MIN = 0;
const double DOUBLE_MIN = INT_MIN;
const double DOUBLE_MAX = INT_MAX;


class ItemList;
class Item;
class RPTypeNode;

class WordNode : public ListNode {
public:
   char *word;
public:
   WordNode(char *w) : word(w) {}
   WordNode(const WordNode &b) {
      word = strdup(b.word);
   }
   ~WordNode() {
      free(word);
   }
};

class RPType {
   friend class Schema;
protected:
   char *_name;
public:
   RPType() : _name(NULL) {}
   // Added by wlee
   RPType(const RPType &t) {
     if (t._name)
       _name = strdup(t._name);
     else
       _name = NULL;
   }
   virtual ~RPType() {
      if (_name) 
        // Modified by wlee
	// delete _name;
	free(_name);
      _name = NULL;
   }

   virtual bool validate(const Item     *item) const;
   virtual bool validate(ItemList *list) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const = 0;
   virtual const char *name() = 0;
   static RPType *newRPType(char *name);
   static RPType *newRPType(char *name, long long int min, long long int max);
   static RPType *newRPType(char *name, double min, double max);
   static RPType *newRPType(char *name, List<WordNode> *words);
   static RPType *newRPType(char *name, List<RPTypeNode> *types);
};

inline ostream &operator<<(ostream &out, RPType &type) {
   out << type.name();
   return out;
}

class RPTypeInt : public RPType {
   friend class Schema;
private:
   long long int min;
   long long int max;
public:
   RPTypeInt(long long int min_v = INT_MIN, long long int max_v = INT_MAX) :
      min(min_v), max(max_v) {}
   virtual ~RPTypeInt() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeInt(*this);
   }
   virtual const char *name() {
      if (!_name) {
	 if (min == INT_MIN && max == INT_MAX)
	    _name = strdup("integer");
	 else {
	    char buffer[1024];
            // Changed by wlee
	    // sprintf(buffer, "integer[%Ld, %Ld]", min, max);
            ostrstream os(buffer, 1024);
            os << "integer[" << min << ", " << max << "]" << ends;
	    _name = strdup(buffer);
	 }
      }
      return _name;
   }
};

class RPTypeDate : public RPType {
   friend class Schema;
public:
   RPTypeDate() {}
   virtual ~RPTypeDate() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeDate(*this);
   }
   virtual const char *name() {
      if (!_name)
	 _name = strdup("date");
      return _name;
   }
};

class RPTypeReal : public RPType {
   friend class Schema;
private:
   double min;
   double max;
public:
   RPTypeReal(double min_v = DOUBLE_MIN, double max_v = DOUBLE_MAX) :
      min(min_v), max(max_v) {}
   virtual ~RPTypeReal() {}
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeReal(*this);
   }
   virtual const char *name() {
      if (!_name) {
	 if (min == DOUBLE_MIN && max == DOUBLE_MAX)
	    _name = strdup("real");
	 else  {
	    char buffer[1024];
	    sprintf(buffer, "real[%f, %f]", min, max);
	    _name = strdup(buffer);
	 }
      }
      return _name;
   }
};

class RPTypeString : public RPType {
   friend class Schema;
private:
public:
   RPTypeString() {}
   virtual ~RPTypeString() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeString(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("string");
      return _name;
   }   
};

class RPTypeTimeStamp : public RPType {
   friend class Schema;
private:
public:
   RPTypeTimeStamp() {}
   virtual ~RPTypeTimeStamp() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeTimeStamp(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("time_stamp");
      return _name;
   }   
};

class RPTypeWord : public RPType {
   friend class Schema;
public:
   RPTypeWord() {}
   virtual ~RPTypeWord() {}
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeWord(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("rpsl_word");
      return _name;
   }   
};

class RPTypeBlob : public RPType {
   friend class Schema;
public:
   RPTypeBlob() {}
   virtual ~RPTypeBlob() {}
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeBlob(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("blob");
      return _name;
   }   
};

class RPTypeASName : public RPType {
   friend class Schema;
public:
   RPTypeASName() {}
   virtual ~RPTypeASName() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeASName(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("as_set_name");
      return _name;
   }   
};

class RPTypeRSName : public RPType {
   friend class Schema;
public:
   RPTypeRSName() {}
   virtual ~RPTypeRSName() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeRSName(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("route_set_name");
      return _name;
   }   
};

class RPTypeRtrsName : public RPType {
   friend class Schema;
public:
   RPTypeRtrsName() {}
   virtual ~RPTypeRtrsName() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeRtrsName(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("rtr_set_name");
      return _name;
   }   
};

class RPTypePrngName : public RPType {
   friend class Schema;
public:
   RPTypePrngName() {}
   virtual ~RPTypePrngName() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypePrngName(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("peering_set_name");
      return _name;
   }   
};

class RPTypeFltrName : public RPType {
   friend class Schema;
public:
   RPTypeFltrName() {}
   virtual ~RPTypeFltrName() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeFltrName(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("filter_set_name");
      return _name;
   }   
};

class RPTypeEMail : public RPType {
   friend class Schema;
public:
   RPTypeEMail() {}
   virtual ~RPTypeEMail() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeEMail(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("email");
      return _name;
   }   
};

class RPTypeFreeText : public RPType {
   friend class Schema;
public:
   RPTypeFreeText() {}
   virtual ~RPTypeFreeText() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeFreeText(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("free_text");
      return _name;
   }   
};

class RPTypeEnum : public RPType {
   friend class Schema;
private:
   List<WordNode> *words;
public:
   RPTypeEnum() : words(NULL) {}
   RPTypeEnum(List<WordNode> *_words) : words(_words) {}
   RPTypeEnum(const RPTypeEnum &b) {
      words = new List<WordNode>(*b.words);
   }

   virtual ~RPTypeEnum() {
      if (words)
	 delete words;
   }
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeEnum(*this);
   }
   virtual const char *name() {
      if (!_name) {
	 char buffer[1024];
	 strcpy(buffer, "enum[");
	 for (WordNode *w = words->head(); w; ) {
	    strcat(buffer, w->word);
	    w = words->next(w);
	    if (w)
	       strcat(buffer, " ");
	 }
	 strcat(buffer, "]");
	 _name = strdup(buffer);
      }
      return _name;
   }   
};

class RPTypeBoolean : public RPType {
   friend class Schema;
public:
   RPTypeBoolean() {}
   virtual ~RPTypeBoolean() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeBoolean(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("boolean");
      return _name;
   }   
};

class RPTypeASNumber : public RPType {
   friend class Schema;
public:
   RPTypeASNumber() {}
   virtual ~RPTypeASNumber() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeASNumber(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("as_number");
      return _name;
   }   
};

class RPTypeIPv4Address : public RPType {
   friend class Schema;
public:
   RPTypeIPv4Address() {}
   virtual ~RPTypeIPv4Address() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeIPv4Address(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("ipv4_address");
      return _name;
   }   
};

class RPTypeIPv4Prefix : public RPType {
   friend class Schema;
public:
   RPTypeIPv4Prefix() {}
   virtual ~RPTypeIPv4Prefix() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeIPv4Prefix(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("ipv4_prefix");
      return _name;
   }   
};

class RPTypeIPv4PrefixRange : public RPType {
   friend class Schema;
public:
   RPTypeIPv4PrefixRange() {}
   virtual ~RPTypeIPv4PrefixRange() {}
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeIPv4PrefixRange(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("ipv4_prefix_range");
      return _name;
   }   
};

class RPTypeConnection : public RPType {
   friend class Schema;
public:
   RPTypeConnection() {}
   virtual ~RPTypeConnection() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeConnection(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("connection");
      return _name;
   }   
};

class RPTypeDNSName : public RPType {
   friend class Schema;
public:
   RPTypeDNSName() {}
   virtual ~RPTypeDNSName() {}
   virtual bool validate(const Item *item) const;
   virtual RPType *dup() const {
      return new RPTypeDNSName(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("DNS_name");
      return _name;
   }   
};

class RPTypeFilter : public RPType {
   friend class Schema;
public:
   RPTypeFilter() {}
   virtual ~RPTypeFilter() {}
   virtual bool validate(const Item *item) const;
   Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeFilter(*this);
   }
   virtual const char *name() {
      if (! _name)
	 _name = strdup("filter");
      return _name;
   }   
};

class RPTypeNode : public ListNode {
public:
   RPType   *type;
public:
   RPTypeNode() : type(NULL) {}
   RPTypeNode(RPType *_type) : type(_type) {}
   RPTypeNode(const RPTypeNode &b) {
      type = b.type->dup();
   };
   virtual ~RPTypeNode() {
      if (type)
	 delete type;
   }
};

class RPTypeUnion : public RPType {
   friend class Schema;
private:
   List<RPTypeNode> *types;
public:
   RPTypeUnion() : types(NULL) {}
   RPTypeUnion(List<RPTypeNode> *_types) : types(_types) {}
   RPTypeUnion(const RPTypeUnion& b) {
      types = new List<RPTypeNode>(*b.types);
   }
   RPTypeUnion(RPType *t1, RPType *t2, RPType *t3 = NULL, RPType *t4 = NULL) {
      types = new List<RPTypeNode>;
      types->append(new RPTypeNode(t1));
      types->append(new RPTypeNode(t2));
      if (t3)
	 types->append(new RPTypeNode(t3));
      if (t4)
	 types->append(new RPTypeNode(t4));
   }
   virtual ~RPTypeUnion() {
      if (types)
	 delete types;
   }
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeUnion(*this);
   }

   virtual const char *name();
};

class RPTypeRange : public RPType {
   friend class Schema;
private:
   RPType *type;
public:
   RPTypeRange() : type(NULL) {}
   RPTypeRange(RPType *t) : type(t) {}
   virtual ~RPTypeRange() {
      if (type)
	 delete type;
   }
   RPTypeRange(const RPTypeRange &b) {
      if (b.type)
	 type = b.type->dup();
      else
	 type = NULL;
   }
   
   virtual bool validate(const Item *item) const;
   virtual Item *typeCast(const Item  *item) const;
   virtual RPType *dup() const {
      return new RPTypeRange(*this);
   }
   virtual const char *name() {
      if (!_name) {
	 char buffer[1024];
	 sprintf(buffer, "range of %s", type->name());
	 _name = strdup(buffer);
      }   
      return _name;
   }
};

class RPTypeList : public RPType {
   friend class Schema;
private:
   RPType *type;
   unsigned int min_elms;
   unsigned int max_elms;
public:
   RPTypeList() : type(NULL), min_elms(UINT_MIN), max_elms(UINT_MAX) {}
   RPTypeList(RPType *t, int min = UINT_MIN, int max = UINT_MAX) : 
      type(t), min_elms(min), max_elms(max) {}
   virtual ~RPTypeList() {
      if (type)
	 delete type;
   }
   RPTypeList(const RPTypeList &b) {
      if (b.type)
	 type = b.type->dup();
      else
	 type = NULL;
      min_elms = b.min_elms;
      max_elms = b.max_elms;
   }

   virtual bool validate(const Item *item) const;
   virtual bool validate(ItemList *l) const;
   virtual RPType *dup() const {
      return new RPTypeList(*this);
   }
   virtual const char *name() {
      if (!_name) {
	 char buffer[1024];
	 if (min_elms == UINT_MIN && max_elms == UINT_MAX) 
	    sprintf(buffer, "list of %s", type->name());
	 else
	    sprintf(buffer, "list [%d:%d] of %s", 
		    min_elms, max_elms, type->name());
	 _name = strdup(buffer);
      }   
      return _name;
   }
};

#endif   // RPTYPE_HH
