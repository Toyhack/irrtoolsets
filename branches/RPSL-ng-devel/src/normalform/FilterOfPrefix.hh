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
//             Katie Petrusha <katie@ripe.net>   

#ifndef FilterOfPrefix_H
#define FilterOfPrefix_H

#include "config.h"
#include <iostream>
#include "Filter.hh"
#include "SetOfPrefix.hh"
#include "SetOfIPv6Prefix.hh"

class FilterOfPrefix : public NEFilter, public SetOfPrefix {
public:
   // FilterOfPrefix();
   // ~FilterOfPrefix();
   // FilterOfPrefix(const FilterOfPrefix& other);

   virtual int isEmpty() {
      return SetOfPrefix::isEmpty();
   }

   virtual int is_universal() {
      return SetOfPrefix::universal();
   }

   virtual void make_universal() {
      SetOfPrefix::make_universal();
   }

   virtual void make_empty() {
      SetOfPrefix::clear();
   }

   virtual void operator ~  () { // complement
      SetOfPrefix::operator~();
   }
   void operator |= (FilterOfPrefix& b) { // union
      SetOfPrefix::operator|=(b);
   }
   void operator &= (FilterOfPrefix& b) { // intersection
      SetOfPrefix::operator&=(b);
   }
   int  operator == (FilterOfPrefix& b) { // equivalance
      return SetOfPrefix::operator==(b);
   }
   void operator =  (FilterOfPrefix& b) { // assignment
      SetOfPrefix::operator=(b);
   }

   void operator = (const PrefixRanges& b) { // assignment
      SetOfPrefix::operator=(b);
   }
   void operator |= (const PrefixRanges & b) { // union
      SetOfPrefix::operator|=(b);
   }

   // below is an ugly trick
   virtual void operator |= (NEFilter& b) {
      *this |= (FilterOfPrefix&) b;
   }
   virtual void operator &= (NEFilter& b) {
      *this &= (FilterOfPrefix&) b;
   }
   virtual int  operator == (NEFilter& b) {
      return (*this == (FilterOfPrefix&) b);
   }
   virtual void operator =  (NEFilter& b) {
      *this = (FilterOfPrefix&) b;
   }

   virtual void do_print (std::ostream& stream) {
      stream << (SetOfPrefix &) *this;
   }

   CLASS_DEBUG_MEMORY_HH(FilterOfPrefix);

};

class FilterOfIPv6Prefix : public NEFilter, public SetOfIPv6Prefix {
public:

   virtual int isEmpty() {
      return SetOfIPv6Prefix::isEmpty();
   }

   virtual int is_universal() {
      return SetOfIPv6Prefix::universal();
   }

   virtual void make_universal() {
      SetOfIPv6Prefix::make_universal();
   }

   virtual void make_empty() {
      SetOfIPv6Prefix::clear();
   }

   virtual void operator ~  () { // complement
      SetOfIPv6Prefix::operator~();
   }
   void operator |= (FilterOfIPv6Prefix& b) { // union
      SetOfIPv6Prefix::operator|=(b);
   }
   void operator &= (FilterOfIPv6Prefix& b) { // intersection
      SetOfIPv6Prefix::operator&=(b);
   }
   int  operator == (FilterOfIPv6Prefix& b) { // equivalance
      return SetOfIPv6Prefix::operator==(b);
   }
   void operator =  (FilterOfIPv6Prefix& b) { // assignment
      SetOfIPv6Prefix::operator=(b);
   }

   void operator = (const MPPrefixRanges& b) { // assignment
      SetOfIPv6Prefix::operator=(b);
   }
   void operator |= (const MPPrefixRanges & b) { // union
      SetOfIPv6Prefix::operator|=(b);
   }

   // below is an ugly trick
   virtual void operator |= (NEFilter& b) {
      *this |= (FilterOfIPv6Prefix&) b;
   }
   virtual void operator &= (NEFilter& b) {
      *this &= (FilterOfIPv6Prefix&) b;
   }
   virtual int  operator == (NEFilter& b) {
      return (*this == (FilterOfIPv6Prefix&) b);
   }
   virtual void operator =  (NEFilter& b) {
      *this = (FilterOfIPv6Prefix&) b;
   }

   virtual void do_print (std::ostream& stream) {
      stream << (SetOfIPv6Prefix &) *this;
   }

   CLASS_DEBUG_MEMORY_HH(FilterOfIPv6Prefix);

};





#endif // FilterOfPrefix_H