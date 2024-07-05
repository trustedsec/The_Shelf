#ifndef _CCACHE_H_
#define _CCACHE_H_

/* The Kerberos Credential Cache Binary File Format
 * Copyright (C) 2006-2013 Simon Josefsson <simon josefsson.org>
 * http://josefsson.org/shishi/ccache.txt
 * Last updated: Sat Sep 23 12:04:11 CEST 2006
 * */
#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__ ((__packed__)) _counted_octet_string {
    uint32_t length;
    uint8_t data[1];
} counted_octet_string ;

/* Going with v0x0504 here, we'll see if it works. */
typedef struct __attribute__ ((__packed__)) _header {
       uint16_t tag;                    /* 1 = DeltaTime */
       uint16_t taglen;
       uint8_t tagdata[1];
} header;

typedef struct __attribute__ ((__packed__)) _DeltaTime {
       uint32_t time_offset;
       uint32_t usec_offset;
} DeltaTime;

typedef struct __attribute__ ((__packed__)) _keyblock {
         uint16_t keytype;
         uint16_t etype;                /* only present if version 0x0503 */
         uint16_t keylen;
         uint8_t keyvalue[1];
} keyblock;

typedef struct __attribute__ ((__packed__)) _times {
      uint32_t  authtime;
      uint32_t  starttime;
      uint32_t  endtime;
      uint32_t  renew_till;
} times;

typedef struct __attribute__ ((__packed__)) _address {
        uint16_t addrtype;
        counted_octet_string addrdata;
} address;

typedef struct __attribute__ ((__packed__)) _authdata {
         uint16_t authtype;
         counted_octet_string authdata;
} authdata;

typedef struct __attribute__ ((__packed__)) _principal {
          uint32_t name_type;           /* not present if version 0x0501 */
          uint32_t num_components;      /* sub 1 if version 0x501 */
          counted_octet_string realm;
          counted_octet_string components[1];
} principal;

typedef struct __attribute__ ((__packed__)) _credential {
           principal client;
           principal server;
           keyblock key;
           times    time;
           uint8_t  is_skey;            /* 1 if skey, 0 otherwise */
           uint32_t tktflags;           /* stored in reversed byte order */
           uint32_t num_address;
           address  addrs[1];
           uint32_t num_authdata;
           authdata authdata[1];
           counted_octet_string ticket;
           counted_octet_string second_ticket;
} credential;

/* Might make using easier*/
typedef struct __attribute__ ((__packed__)) _credentialend {
           times    time;
           uint8_t  is_skey;            /* 1 if skey, 0 otherwise */
           uint32_t tktflags;           /* stored in reversed byte order */
           uint32_t num_address;
           address  addrs[1];
           uint32_t num_authdata;
           authdata authdata[1];
           counted_octet_string ticket;
           counted_octet_string second_ticket;
} credentialend;

typedef struct __attribute__ ((__packed__)) _credentialendtwo {
           uint32_t num_authdata;
           authdata authdata[1];
           counted_octet_string ticket;
           counted_octet_string second_ticket;
} credentialendtwo;




typedef struct __attribute__ ((__packed__)) _ccache {
          uint16_t file_format_version; /* 0x0504 */
          uint16_t headerlen;           /* only if version is 0x0504 */
          header headers[1];             /* only if version is 0x0504 */
          principal primary_principal;
          credential credentials[1];
} ccache;

typedef struct __attribute__ ((__packed__)) _ccache_head {
          uint16_t file_format_version; /* 0x0504 */
          uint16_t headerlen;           /* only if version is 0x0504 */
          header headers[1];             /* only if version is 0x0504 */
} ccache_head;



unsigned char* generate_ccache_binary(krb5_creds *creds, int* outdataLen);
krb5_creds* parse_ccache(unsigned char* inCcache, int ccacheSize);
int freeStructure(krb5_creds* instruct);
#endif
