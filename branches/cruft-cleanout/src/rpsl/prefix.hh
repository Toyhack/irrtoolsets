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

#ifndef PREFIX_HH
#define PREFIX_HH

#include "config.h"
#include <sys/types.h>
#include <vector>
#include <sstream>
#include <ostream>

typedef unsigned long long int       ip_v6word_t;

// ipv6 address unit + operations
class ipv6_addr_t {
  public:
    ip_v6word_t high;
    ip_v6word_t low; 
    bool xbit;

  public:
    friend int operator<(ipv6_addr_t one, ipv6_addr_t two);
    friend int operator==(ipv6_addr_t one, ipv6_addr_t two);
    friend int operator!=(ipv6_addr_t one, ipv6_addr_t two);
    int operator!() const;

    friend ipv6_addr_t& operator&(ipv6_addr_t one, ipv6_addr_t two);
    friend ipv6_addr_t& operator|(ipv6_addr_t one, ipv6_addr_t two);
    ipv6_addr_t& operator|(ip_v6word_t i);
    ipv6_addr_t& operator+(ip_v6word_t i);
    ipv6_addr_t& operator=(ip_v6word_t i);
    int operator==(ip_v6word_t i);

    ipv6_addr_t& operator<<(unsigned int i);
    ipv6_addr_t& operator>>(unsigned int i);
    ipv6_addr_t& operator~() const;

    bool operator&&(bool b);
    bool is_true() const;

    ipv6_addr_t& getbits(unsigned int len);
    ipv6_addr_t& getmask(unsigned int len);

    friend std::ostream& operator<<(std::ostream& stream, const ipv6_addr_t& p);

 public:
    ipv6_addr_t() {}

    ipv6_addr_t(ip_v6word_t i, ip_v6word_t j) :
    high(i), low(j), xbit(false) {}

    ipv6_addr_t(ip_v6word_t i, ip_v6word_t j, bool _xbit) :
    high(i), low(j), xbit(_xbit) {}

    ~ipv6_addr_t() {}
    
};

#define IPV6_LENGTH 40

char* int2quad(char *buffer, unsigned int i);
char* ipv62hex(ipv6_addr_t *ip, char *buffer);
ipv6_addr_t* hex2ipv6(char *hex);
char *compact(ipv6_addr_t *ip, char *buffer);

extern class PrefixRange NullPrefixRange;
extern class IPAddr      NullIPAddr;
extern class PrefixRange MulticastPrefixRange;
extern class IPv6PrefixRange NullIPv6PrefixRange;
extern class IPv6Prefix    NullIPv6Prefix;
extern class IPv6Addr    NullIPv6Addr;
extern class ipv6_addr_t NullIPv6;
extern class IPv6PrefixRange MulticastIPv6PrefixRange;

class PrefixRange {

friend class MPPrefix;

 protected:
   static char formattingbuffer[128];

   unsigned int  ipaddr;
   unsigned char length;
   unsigned char n;
   unsigned char m;

  public:
   PrefixRange(void);
   // Added by wlee@isi.edu
   PrefixRange(const PrefixRange &p);
   PrefixRange(unsigned int ipaddr, unsigned char length, 
	  unsigned char n, unsigned char m);

   PrefixRange(const char *name);

   void define(unsigned int ipaddr, unsigned char length, 
	       unsigned char n, unsigned char m);

   // return false if it is an invalid operation and do nothing, 
   // or make the prefix more specific
   bool makeMoreSpecific(int code, int n, int m); 

   void print(void);
   int valid(void);

   bool isNull() const {
      return (*this) == NullPrefixRange;
   }

   PrefixRange& operator=(const PrefixRange& other);
   int operator<(const PrefixRange& other) const;
   int operator<=(const PrefixRange& other) const;
   int operator==(const PrefixRange& other) const;
   int operator!=(const PrefixRange& other) const {
      return ! (*this == other);
   }
   int compare(const PrefixRange& other) const;
   int contains(const PrefixRange& other) const;

   char *get_text(char *buffer = formattingbuffer) const;
   char *get_ip_text(char *buffer = formattingbuffer) const;
   unsigned int get_ipaddr() const { return ipaddr; }
   unsigned int get_length() const { return length; }
   unsigned int get_n() const { return n; }
   unsigned int get_m() const { return m; }
   unsigned int get_mask() const;
   unsigned long long int get_range() const;

   friend std::ostream& operator<<(std::ostream& stream, const PrefixRange& p);

/*
   // Added const to PrefixRange& p by wlee@isi.edu
   friend const ostream& operator<<(ostream& stream, const PrefixRange& p);
*/

   void parse(const char *name);
};

class Prefix : public PrefixRange {
public:
   Prefix() : PrefixRange(0, 0, 0, 0) {
   }
   Prefix(unsigned int ipaddr, unsigned char length) :
      PrefixRange(ipaddr, length, length, length) {
   }
   Prefix(char *name);
   void define(unsigned int ipaddr, unsigned char length) {
      PrefixRange::define(ipaddr, length, length, length);
   }
   char *get_text(char *buffer = formattingbuffer) const;
};

class IPAddr : public Prefix {
public:
   IPAddr() : Prefix(0, 0) {
   }
   IPAddr(unsigned int ipaddr) :
      Prefix(ipaddr, 32) {
   }
   IPAddr(const char *name);
   void define(unsigned int ipaddr) {
      Prefix::define(ipaddr, 32);
   }
   char *get_text(char *buffer = formattingbuffer) const;
};

class IPv6PrefixRange {

friend class MPPrefix;

 protected:
   static char formattingbuffer[256];

   ipv6_addr_t  *ipaddr;
   unsigned char length;

   unsigned char n;
   unsigned char m;

  public:
   IPv6PrefixRange(void);
   IPv6PrefixRange(const IPv6PrefixRange &p);
   IPv6PrefixRange(ipv6_addr_t *ipaddr, unsigned char length, 
	  unsigned char n, unsigned char m);

   IPv6PrefixRange(const char *name);

   void define(ipv6_addr_t *ipaddr, unsigned char length, 
	       unsigned char n, unsigned char m);

   // return false if it is an invalid operation and do nothing, 
   // or make the prefix more specific
   bool makeMoreSpecific(int code, int n, int m); 

   void print(void);
   int valid(void);

   bool isNull() const {
      return (*this) == NullIPv6PrefixRange;
   }

   IPv6PrefixRange& operator=(const IPv6PrefixRange& other);
   int operator<(const IPv6PrefixRange& other) const;
   int operator<=(const IPv6PrefixRange& other) const;
   int operator==(const IPv6PrefixRange& other) const;
   int operator!=(const IPv6PrefixRange& other) const {
      return ! (*this == other);
   }
   int compare(const IPv6PrefixRange& other) const;
   int contains(const IPv6PrefixRange& other) const;

   char *get_text(char *buffer = formattingbuffer) const;
   char *get_ip_text(char *buffer = formattingbuffer) const;
   ipv6_addr_t *get_ipaddr() const { return ipaddr; }
   unsigned int get_length() const { return length; }
   unsigned int get_n() const { return n; }
   unsigned int get_m() const { return m; }
   ipv6_addr_t get_mask() const;
   ipv6_addr_t get_range() const;

   friend std::ostream& operator<<(std::ostream& stream,
                                   const IPv6PrefixRange& p);

   void parse(const char *name);

};

class IPv6Prefix : public IPv6PrefixRange {
public:
   IPv6Prefix() : IPv6PrefixRange(NULL, 0, 0, 0) {
   }
   IPv6Prefix(ipv6_addr_t *ipaddr, unsigned char length) :
      IPv6PrefixRange(ipaddr, length, length, length) {
   }
   IPv6Prefix(const char *name);

   void define(ipv6_addr_t *ipaddr, unsigned char length) {
      IPv6PrefixRange::define(ipaddr, length, length, length);
   }
   char *get_text(char *buffer = formattingbuffer) const;

   friend std::ostream& operator<<(std::ostream& stream,
                                   const IPv6Prefix& p);
};

class IPv6Addr : public IPv6Prefix {
public:
   IPv6Addr() : IPv6Prefix(NULL, 128) {
   }
   IPv6Addr(ipv6_addr_t *ipaddr) :
      IPv6Prefix(ipaddr, 128) {
   }
   IPv6Addr(const char *name);
   void define(ipv6_addr_t *ipaddr) {
      IPv6Prefix::define(ipaddr, 128);
   }
   char *get_text(char *buffer = formattingbuffer) const;
   friend std::ostream& operator<<(std::ostream& stream, const IPv6Addr& p);
};

// metaclass for type isolation (ipv4, ipv6)

class MPPrefix { 
 public:
   PrefixRange *ipv4;
   IPv6PrefixRange *ipv6;

public: 
   MPPrefix(void) : 
      ipv4(NULL),
      ipv6(NULL)
   {
   }

   MPPrefix(PrefixRange *_ipv4 ) :
      ipv4(_ipv4),
      ipv6(NULL)
   {
   }

   MPPrefix(IPv6PrefixRange *_ipv6 ) :
      ipv4(NULL),
      ipv6(_ipv6)
   {
   }
  // create from string
  MPPrefix (char *name);

  unsigned int get_length() const;
  unsigned int get_n();
  unsigned int get_m();
  void define (unsigned int masklen);
  // Those are used by IPV6 only
  ipv6_addr_t get_mask() const;
  ipv6_addr_t get_range() const;
  ipv6_addr_t get_ipaddr() const;
  char *get_text() const;
  char *get_ip_text() const;
  char *get_afi() const;
  // interface to makeMoreSpecific
  bool makeMoreSpecific(int code, int n, int m);

  int operator==(const MPPrefix &t) const {
    if (ipv4 && t.ipv4)
      return (*ipv4 == *(t.ipv4));
    if (ipv6 && t.ipv6)
      return (*ipv6 == *(t.ipv6));
    return false;
  }
  int operator<(const MPPrefix &t) const {
    if (ipv4 && t.ipv4)
      return (*ipv4 < *(t.ipv4));
    if (ipv6 && t.ipv6)
      return (*ipv6 < *(t.ipv6));
    return false;
  }

  bool isNull() const {
    if (ipv4) {
      return (*ipv4 == NullPrefixRange);
    }
    if (ipv6) {
      return (*ipv6 == NullIPv6PrefixRange);
    }
    return true;
  }

  friend std::ostream& operator<<(std::ostream& stream, const MPPrefix& p);

};

class MPPrefixRanges : public std::vector<MPPrefix>
{
public:
  MPPrefixRanges() {};
  MPPrefixRanges(void *t) {
    this->append_list((MPPrefixRanges *) t);
  }

  void append_list(const MPPrefixRanges *src); // join
  void _and(MPPrefixRanges *src); // intersection
  void except(MPPrefixRanges *src); // exception
  bool contains(IPAddr ip) const;
  bool contains(IPv6Addr ip) const;
  bool contains(MPPrefix ip) const;

  friend std::ostream& operator<<(std::ostream& stream, const MPPrefixRanges& p);

};


#endif // PREFIX_HH
