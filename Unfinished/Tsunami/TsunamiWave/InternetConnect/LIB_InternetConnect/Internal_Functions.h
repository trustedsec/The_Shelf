#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <Windows.h>

//#include <wininet.h>

//
// access types for InternetOpen()
//

#define INTERNET_OPEN_TYPE_PRECONFIG                    0   // use registry configuration
#define INTERNET_OPEN_TYPE_DIRECT                       1   // direct to net
#define INTERNET_OPEN_TYPE_PROXY                        3   // via named proxy
#define INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY  4   // prevent using java/script/INS

//
// service types for InternetConnect()
//

#define INTERNET_SERVICE_FTP    1
#define INTERNET_SERVICE_GOPHER 2
#define INTERNET_SERVICE_HTTP   3


//
// flags for InternetCrackUrl() and InternetCreateUrl()
//

#define ICU_ESCAPE      0x80000000  // (un)escape URL characters
#define ICU_USERNAME    0x40000000  // use internal username & password


//
// flags for InternetCanonicalizeUrl() and InternetCombineUrl()
//

#define ICU_NO_ENCODE   0x20000000  // Don't convert unsafe characters to escape sequence
#define ICU_DECODE      0x10000000  // Convert %XX escape sequences to characters
#define ICU_NO_META     0x08000000  // Don't convert .. etc. meta path sequences
#define ICU_ENCODE_SPACES_ONLY 0x04000000  // Encode spaces only
#define ICU_BROWSER_MODE 0x02000000 // Special encode/decode rules for browser
#define ICU_ENCODE_PERCENT      0x00001000      // Encode any percent (ASCII25)
		// signs encountered, default is to not encode percent.


//
// string field lengths (in characters, not bytes)
//

#define INTERNET_MAX_HOST_NAME_LENGTH   256
#define INTERNET_MAX_USER_NAME_LENGTH   128
#define INTERNET_MAX_PASSWORD_LENGTH    128
#define INTERNET_MAX_PORT_NUMBER_LENGTH 5           // INTERNET_PORT is unsigned short
#define INTERNET_MAX_PORT_NUMBER_VALUE  65535       // maximum unsigned short value
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32          // longest protocol name length
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof("://") \
                                        + INTERNET_MAX_PATH_LENGTH)
#define MAX_USER_AGENT_SIZE				256



//
// values returned by InternetQueryOption() with INTERNET_OPTION_KEEP_CONNECTION:
//

#define INTERNET_KEEP_ALIVE_UNKNOWN     ((DWORD)-1)
#define INTERNET_KEEP_ALIVE_ENABLED     1
#define INTERNET_KEEP_ALIVE_DISABLED    0

//
// flags returned by InternetQueryOption() with INTERNET_OPTION_REQUEST_FLAGS
//

#define INTERNET_REQFLAG_FROM_CACHE     0x00000001  // response came from cache
#define INTERNET_REQFLAG_ASYNC          0x00000002  // request was made asynchronously
#define INTERNET_REQFLAG_VIA_PROXY      0x00000004  // request was made via a proxy
#define INTERNET_REQFLAG_NO_HEADERS     0x00000008  // orginal response contained no headers
#define INTERNET_REQFLAG_PASSIVE        0x00000010  // FTP: passive-mode connection
#define INTERNET_REQFLAG_CACHE_WRITE_DISABLED 0x00000040  // HTTPS: this request not cacheable
#define INTERNET_REQFLAG_NET_TIMEOUT    0x00000080  // w/ _FROM_CACHE: net request timed out


//
// flags for IDN enable/disable via INTERNET_OPTION_IDN
//

#define INTERNET_FLAG_IDN_DIRECT        0x00000001  // IDN enabled for direct connections
#define INTERNET_FLAG_IDN_PROXY         0x00000002  // IDN enabled for proxy

//
// flags common to open functions (not InternetOpen()):
//

#define INTERNET_FLAG_RELOAD            0x80000000  // retrieve the original item

//
// flags for InternetOpenUrl():
//

#define INTERNET_FLAG_RAW_DATA          0x40000000  // FTP/gopher find: receive the item as raw (structured) data
#define INTERNET_FLAG_EXISTING_CONNECT  0x20000000  // FTP: use existing InternetConnect handle for server if possible

//
// flags for InternetOpen():
//

#define INTERNET_FLAG_ASYNC             0x10000000  // this request is asynchronous (where supported)

//
// protocol-specific flags:
//

#define INTERNET_FLAG_PASSIVE           0x08000000  // used for FTP connections

//
// additional cache flags
//

#define INTERNET_FLAG_NO_CACHE_WRITE    0x04000000  // don't write this item to the cache
#define INTERNET_FLAG_DONT_CACHE        INTERNET_FLAG_NO_CACHE_WRITE
#define INTERNET_FLAG_MAKE_PERSISTENT   0x02000000  // make this item persistent in cache
#define INTERNET_FLAG_FROM_CACHE        0x01000000  // use offline semantics
#define INTERNET_FLAG_OFFLINE           INTERNET_FLAG_FROM_CACHE

//
// additional flags
//

#define INTERNET_FLAG_SECURE            0x00800000  // use PCT/SSL if applicable (HTTP)
#define INTERNET_FLAG_KEEP_CONNECTION   0x00400000  // use keep-alive semantics
#define INTERNET_FLAG_NO_AUTO_REDIRECT  0x00200000  // don't handle redirections automatically
#define INTERNET_FLAG_READ_PREFETCH     0x00100000  // do background read prefetch
#define INTERNET_FLAG_NO_COOKIES        0x00080000  // no automatic cookie handling
#define INTERNET_FLAG_NO_AUTH           0x00040000  // no automatic authentication handling
#define INTERNET_FLAG_RESTRICTED_ZONE   0x00020000  // apply restricted zone policies for cookies, auth
#define INTERNET_FLAG_CACHE_IF_NET_FAIL 0x00010000  // return cache file if net request fails

//
// Security Ignore Flags, Allow HttpOpenRequest to overide
//  Secure Channel (SSL/PCT) failures of the following types.
//

#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   0x00008000 // ex: https:// to http://
#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  0x00004000 // ex: http:// to https://
#define INTERNET_FLAG_IGNORE_CERT_DATE_INVALID  0x00002000 // expired X509 Cert.
#define INTERNET_FLAG_IGNORE_CERT_CN_INVALID    0x00001000 // bad common name in X509 Cert.

//
// more caching flags
//

#define INTERNET_FLAG_RESYNCHRONIZE     0x00000800  // asking wininet to update an item if it is newer
#define INTERNET_FLAG_HYPERLINK         0x00000400  // asking wininet to do hyperlinking semantic which works right for scripts
#define INTERNET_FLAG_NO_UI             0x00000200  // no cookie popup
#define INTERNET_FLAG_PRAGMA_NOCACHE    0x00000100  // asking wininet to add "pragma: no-cache"
#define INTERNET_FLAG_CACHE_ASYNC       0x00000080  // ok to perform lazy cache-write
#define INTERNET_FLAG_FORMS_SUBMIT      0x00000040  // this is a forms submit
#define INTERNET_FLAG_FWD_BACK          0x00000020  // fwd-back button op
#define INTERNET_FLAG_NEED_FILE         0x00000010  // need a file for this request
#define INTERNET_FLAG_MUST_CACHE_REQUEST INTERNET_FLAG_NEED_FILE

//
// flags for FTP
//

#define INTERNET_FLAG_TRANSFER_ASCII    FTP_TRANSFER_TYPE_ASCII     // 0x00000001
#define INTERNET_FLAG_TRANSFER_BINARY   FTP_TRANSFER_TYPE_BINARY    // 0x00000002

//
// flags field masks
//

#define SECURITY_INTERNET_MASK  (INTERNET_FLAG_IGNORE_CERT_CN_INVALID    |  \
                                 INTERNET_FLAG_IGNORE_CERT_DATE_INVALID  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   )

#define SECURITY_IGNORE_ERROR_MASK  (INTERNET_FLAG_IGNORE_CERT_CN_INVALID   |  \
                                     INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |  \
                                     SECURITY_FLAG_IGNORE_UNKNOWN_CA        |  \
                                     SECURITY_FLAG_IGNORE_REVOCATION    )

#define INTERNET_FLAGS_MASK     (INTERNET_FLAG_RELOAD               \
                                | INTERNET_FLAG_RAW_DATA            \
                                | INTERNET_FLAG_EXISTING_CONNECT    \
                                | INTERNET_FLAG_ASYNC               \
                                | INTERNET_FLAG_PASSIVE             \
                                | INTERNET_FLAG_NO_CACHE_WRITE      \
                                | INTERNET_FLAG_MAKE_PERSISTENT     \
                                | INTERNET_FLAG_FROM_CACHE          \
                                | INTERNET_FLAG_SECURE              \
                                | INTERNET_FLAG_KEEP_CONNECTION     \
                                | INTERNET_FLAG_NO_AUTO_REDIRECT    \
                                | INTERNET_FLAG_READ_PREFETCH       \
                                | INTERNET_FLAG_NO_COOKIES          \
                                | INTERNET_FLAG_NO_AUTH             \
                                | INTERNET_FLAG_CACHE_IF_NET_FAIL   \
                                | SECURITY_INTERNET_MASK            \
                                | INTERNET_FLAG_RESYNCHRONIZE       \
                                | INTERNET_FLAG_HYPERLINK           \
                                | INTERNET_FLAG_NO_UI               \
                                | INTERNET_FLAG_PRAGMA_NOCACHE      \
                                | INTERNET_FLAG_CACHE_ASYNC         \
                                | INTERNET_FLAG_FORMS_SUBMIT        \
                                | INTERNET_FLAG_NEED_FILE           \
                                | INTERNET_FLAG_RESTRICTED_ZONE     \
                                | INTERNET_FLAG_TRANSFER_BINARY     \
                                | INTERNET_FLAG_TRANSFER_ASCII      \
                                | INTERNET_FLAG_FWD_BACK            \
                                | INTERNET_FLAG_BGUPDATE            \
                                )



#define INTERNET_ERROR_MASK_INSERT_CDROM                    0x1
#define INTERNET_ERROR_MASK_COMBINED_SEC_CERT               0x2
#define INTERNET_ERROR_MASK_NEED_MSN_SSPI_PKG               0X4
#define INTERNET_ERROR_MASK_LOGIN_FAILURE_DISPLAY_ENTITY_BODY 0x8

#define INTERNET_OPTIONS_MASK   (~INTERNET_FLAGS_MASK)

//
// common per-API flags (new APIs)
//

#define WININET_API_FLAG_ASYNC          0x00000001  // force async operation
#define WININET_API_FLAG_SYNC           0x00000004  // force sync operation
#define WININET_API_FLAG_USE_CONTEXT    0x00000008  // use value supplied in dwContext (even if 0)

//
// INTERNET_NO_CALLBACK - if this value is presented as the dwContext parameter
// then no call-backs will be made for that API
//

#define INTERNET_NO_CALLBACK            0
//
// values for INTERNET_OPTION_SECURITY_FLAGS
//

// query only
#define SECURITY_FLAG_SECURE                    0x00000001 // can query only
#define SECURITY_FLAG_STRENGTH_WEAK             0x10000000
#define SECURITY_FLAG_STRENGTH_MEDIUM           0x40000000
#define SECURITY_FLAG_STRENGTH_STRONG           0x20000000
#define SECURITY_FLAG_UNKNOWNBIT                0x80000000
#define SECURITY_FLAG_FORTEZZA                  0x08000000
#define SECURITY_FLAG_NORMALBITNESS             SECURITY_FLAG_STRENGTH_WEAK



// The following are unused
#define SECURITY_FLAG_SSL                       0x00000002
#define SECURITY_FLAG_SSL3                      0x00000004
#define SECURITY_FLAG_PCT                       0x00000008
#define SECURITY_FLAG_PCT4                      0x00000010
#define SECURITY_FLAG_IETFSSL4                  0x00000020

// The following are for backwards compatability only.
#define SECURITY_FLAG_40BIT                     SECURITY_FLAG_STRENGTH_WEAK
#define SECURITY_FLAG_128BIT                    SECURITY_FLAG_STRENGTH_STRONG
#define SECURITY_FLAG_56BIT                     SECURITY_FLAG_STRENGTH_MEDIUM

// setable flags
#define SECURITY_FLAG_IGNORE_REVOCATION         0x00000080
#define SECURITY_FLAG_IGNORE_UNKNOWN_CA         0x00000100
#define SECURITY_FLAG_IGNORE_WRONG_USAGE        0x00000200

#define SECURITY_FLAG_IGNORE_CERT_CN_INVALID    INTERNET_FLAG_IGNORE_CERT_CN_INVALID
#define SECURITY_FLAG_IGNORE_CERT_DATE_INVALID  INTERNET_FLAG_IGNORE_CERT_DATE_INVALID


#define SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS  INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
#define SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP   INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP



#define SECURITY_SET_MASK       (SECURITY_FLAG_IGNORE_REVOCATION |\
                                 SECURITY_FLAG_IGNORE_UNKNOWN_CA |\
                                 SECURITY_FLAG_IGNORE_CERT_CN_INVALID |\
                                 SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |\
                                 SECURITY_FLAG_IGNORE_WRONG_USAGE)


//
// options manifests for Internet{Query|Set}Option
//

#define INTERNET_OPTION_CALLBACK                1
#define INTERNET_OPTION_CONNECT_TIMEOUT         2
#define INTERNET_OPTION_CONNECT_RETRIES         3
#define INTERNET_OPTION_CONNECT_BACKOFF         4
#define INTERNET_OPTION_SEND_TIMEOUT            5
#define INTERNET_OPTION_CONTROL_SEND_TIMEOUT    INTERNET_OPTION_SEND_TIMEOUT
#define INTERNET_OPTION_RECEIVE_TIMEOUT         6
#define INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT INTERNET_OPTION_RECEIVE_TIMEOUT
#define INTERNET_OPTION_DATA_SEND_TIMEOUT       7
#define INTERNET_OPTION_DATA_RECEIVE_TIMEOUT    8
#define INTERNET_OPTION_HANDLE_TYPE             9
#define INTERNET_OPTION_LISTEN_TIMEOUT          11
#define INTERNET_OPTION_READ_BUFFER_SIZE        12
#define INTERNET_OPTION_WRITE_BUFFER_SIZE       13

#define INTERNET_OPTION_ASYNC_ID                15
#define INTERNET_OPTION_ASYNC_PRIORITY          16

#define INTERNET_OPTION_PARENT_HANDLE           21
#define INTERNET_OPTION_KEEP_CONNECTION         22
#define INTERNET_OPTION_REQUEST_FLAGS           23
#define INTERNET_OPTION_EXTENDED_ERROR          24

#define INTERNET_OPTION_OFFLINE_MODE            26
#define INTERNET_OPTION_CACHE_STREAM_HANDLE     27
#define INTERNET_OPTION_USERNAME                28
#define INTERNET_OPTION_PASSWORD                29
#define INTERNET_OPTION_ASYNC                   30
#define INTERNET_OPTION_SECURITY_FLAGS          31
#define INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT 32
#define INTERNET_OPTION_DATAFILE_NAME           33
#define INTERNET_OPTION_URL                     34
#define INTERNET_OPTION_SECURITY_CERTIFICATE    35
#define INTERNET_OPTION_SECURITY_KEY_BITNESS    36
#define INTERNET_OPTION_REFRESH                 37
#define INTERNET_OPTION_PROXY                   38
#define INTERNET_OPTION_SETTINGS_CHANGED        39
#define INTERNET_OPTION_VERSION                 40
#define INTERNET_OPTION_USER_AGENT              41
#define INTERNET_OPTION_END_BROWSER_SESSION     42
#define INTERNET_OPTION_PROXY_USERNAME          43
#define INTERNET_OPTION_PROXY_PASSWORD          44
#define INTERNET_OPTION_CONTEXT_VALUE           45
#define INTERNET_OPTION_CONNECT_LIMIT           46
#define INTERNET_OPTION_SECURITY_SELECT_CLIENT_CERT 47
#define INTERNET_OPTION_POLICY                  48
#define INTERNET_OPTION_DISCONNECTED_TIMEOUT    49
#define INTERNET_OPTION_CONNECTED_STATE         50
#define INTERNET_OPTION_IDLE_STATE              51
#define INTERNET_OPTION_OFFLINE_SEMANTICS       52
#define INTERNET_OPTION_SECONDARY_CACHE_KEY     53
#define INTERNET_OPTION_CALLBACK_FILTER         54
#define INTERNET_OPTION_CONNECT_TIME            55
#define INTERNET_OPTION_SEND_THROUGHPUT         56
#define INTERNET_OPTION_RECEIVE_THROUGHPUT      57
#define INTERNET_OPTION_REQUEST_PRIORITY        58
#define INTERNET_OPTION_HTTP_VERSION            59
#define INTERNET_OPTION_RESET_URLCACHE_SESSION  60
#define INTERNET_OPTION_ERROR_MASK              62
#define INTERNET_OPTION_FROM_CACHE_TIMEOUT      63
#define INTERNET_OPTION_BYPASS_EDITED_ENTRY     64

#define INTERNET_OPTION_HTTP_DECODING           65

#define INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO  67
#define INTERNET_OPTION_CODEPAGE                68
#define INTERNET_OPTION_CACHE_TIMESTAMPS        69
#define INTERNET_OPTION_DISABLE_AUTODIAL        70
#define INTERNET_OPTION_MAX_CONNS_PER_SERVER     73
#define INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER 74
#define INTERNET_OPTION_PER_CONNECTION_OPTION   75
#define INTERNET_OPTION_DIGEST_AUTH_UNLOAD             76
#define INTERNET_OPTION_IGNORE_OFFLINE           77
#define INTERNET_OPTION_IDENTITY                 78
#define INTERNET_OPTION_REMOVE_IDENTITY          79
#define INTERNET_OPTION_ALTER_IDENTITY           80
#define INTERNET_OPTION_SUPPRESS_BEHAVIOR        81
#define INTERNET_OPTION_AUTODIAL_MODE            82
#define INTERNET_OPTION_AUTODIAL_CONNECTION      83
#define INTERNET_OPTION_CLIENT_CERT_CONTEXT      84
#define INTERNET_OPTION_AUTH_FLAGS               85
#define INTERNET_OPTION_COOKIES_3RD_PARTY        86
#define INTERNET_OPTION_DISABLE_PASSPORT_AUTH    87
#define INTERNET_OPTION_SEND_UTF8_SERVERNAME_TO_PROXY         88
#define INTERNET_OPTION_EXEMPT_CONNECTION_LIMIT  89
#define INTERNET_OPTION_ENABLE_PASSPORT_AUTH     90

#define INTERNET_OPTION_HIBERNATE_INACTIVE_WORKER_THREADS       91
#define INTERNET_OPTION_ACTIVATE_WORKER_THREADS                 92
#define INTERNET_OPTION_RESTORE_WORKER_THREAD_DEFAULTS          93
#define INTERNET_OPTION_SOCKET_SEND_BUFFER_LENGTH               94
#define INTERNET_OPTION_PROXY_SETTINGS_CHANGED                  95

#define INTERNET_OPTION_DATAFILE_EXT                            96

#define INTERNET_OPTION_CODEPAGE_PATH                100
#define INTERNET_OPTION_CODEPAGE_EXTRA               101
#define INTERNET_OPTION_IDN                          102
#define INTERNET_OPTION_MAX_CONNS_PER_PROXY          103
#define INTERNET_OPTION_SUPPRESS_SERVER_AUTH         104
#define INTERNET_OPTION_SERVER_CERT_CHAIN_CONTEXT    105


#define INTERNET_OPTION_ENABLE_REDIRECT_CACHE_READ   122


#define INTERNET_OPTION_ENCODE_EXTRA                 155

#define INTERNET_FIRST_OPTION                   INTERNET_OPTION_CALLBACK
#define INTERNET_LAST_OPTION                    INTERNET_OPTION_ENCODE_EXTRA


//
// HttpQueryInfo info levels. Generally, there is one info level
// for each potential RFC822/HTTP/MIME header that an HTTP server
// may send as part of a request response.
//
// The HTTP_QUERY_RAW_HEADERS info level is provided for clients
// that choose to perform their own header parsing.
//


#define HTTP_QUERY_MIME_VERSION                 0
#define HTTP_QUERY_CONTENT_TYPE                 1
#define HTTP_QUERY_CONTENT_TRANSFER_ENCODING    2
#define HTTP_QUERY_CONTENT_ID                   3
#define HTTP_QUERY_CONTENT_DESCRIPTION          4
#define HTTP_QUERY_CONTENT_LENGTH               5
#define HTTP_QUERY_CONTENT_LANGUAGE             6
#define HTTP_QUERY_ALLOW                        7
#define HTTP_QUERY_PUBLIC                       8
#define HTTP_QUERY_DATE                         9
#define HTTP_QUERY_EXPIRES                      10
#define HTTP_QUERY_LAST_MODIFIED                11
#define HTTP_QUERY_MESSAGE_ID                   12
#define HTTP_QUERY_URI                          13
#define HTTP_QUERY_DERIVED_FROM                 14
#define HTTP_QUERY_COST                         15
#define HTTP_QUERY_LINK                         16
#define HTTP_QUERY_PRAGMA                       17
#define HTTP_QUERY_VERSION                      18  // special: part of status line
#define HTTP_QUERY_STATUS_CODE                  19  // special: part of status line
#define HTTP_QUERY_STATUS_TEXT                  20  // special: part of status line
#define HTTP_QUERY_RAW_HEADERS                  21  // special: all headers as ASCIIZ
#define HTTP_QUERY_RAW_HEADERS_CRLF             22  // special: all headers
#define HTTP_QUERY_CONNECTION                   23
#define HTTP_QUERY_ACCEPT                       24
#define HTTP_QUERY_ACCEPT_CHARSET               25
#define HTTP_QUERY_ACCEPT_ENCODING              26
#define HTTP_QUERY_ACCEPT_LANGUAGE              27
#define HTTP_QUERY_AUTHORIZATION                28
#define HTTP_QUERY_CONTENT_ENCODING             29
#define HTTP_QUERY_FORWARDED                    30
#define HTTP_QUERY_FROM                         31
#define HTTP_QUERY_IF_MODIFIED_SINCE            32
#define HTTP_QUERY_LOCATION                     33
#define HTTP_QUERY_ORIG_URI                     34
#define HTTP_QUERY_REFERER                      35
#define HTTP_QUERY_RETRY_AFTER                  36
#define HTTP_QUERY_SERVER                       37
#define HTTP_QUERY_TITLE                        38
#define HTTP_QUERY_USER_AGENT                   39
#define HTTP_QUERY_WWW_AUTHENTICATE             40
#define HTTP_QUERY_PROXY_AUTHENTICATE           41
#define HTTP_QUERY_ACCEPT_RANGES                42
#define HTTP_QUERY_SET_COOKIE                   43
#define HTTP_QUERY_COOKIE                       44
#define HTTP_QUERY_REQUEST_METHOD               45  // special: GET/POST etc.
#define HTTP_QUERY_REFRESH                      46
#define HTTP_QUERY_CONTENT_DISPOSITION          47

//
// HTTP 1.1 defined headers
//

#define HTTP_QUERY_AGE                          48
#define HTTP_QUERY_CACHE_CONTROL                49
#define HTTP_QUERY_CONTENT_BASE                 50
#define HTTP_QUERY_CONTENT_LOCATION             51
#define HTTP_QUERY_CONTENT_MD5                  52
#define HTTP_QUERY_CONTENT_RANGE                53
#define HTTP_QUERY_ETAG                         54
#define HTTP_QUERY_HOST                         55
#define HTTP_QUERY_IF_MATCH                     56
#define HTTP_QUERY_IF_NONE_MATCH                57
#define HTTP_QUERY_IF_RANGE                     58
#define HTTP_QUERY_IF_UNMODIFIED_SINCE          59
#define HTTP_QUERY_MAX_FORWARDS                 60
#define HTTP_QUERY_PROXY_AUTHORIZATION          61
#define HTTP_QUERY_RANGE                        62
#define HTTP_QUERY_TRANSFER_ENCODING            63
#define HTTP_QUERY_UPGRADE                      64
#define HTTP_QUERY_VARY                         65
#define HTTP_QUERY_VIA                          66
#define HTTP_QUERY_WARNING                      67
#define HTTP_QUERY_EXPECT                       68
#define HTTP_QUERY_PROXY_CONNECTION             69
#define HTTP_QUERY_UNLESS_MODIFIED_SINCE        70



#define HTTP_QUERY_ECHO_REQUEST                 71
#define HTTP_QUERY_ECHO_REPLY                   72

// These are the set of headers that should be added back to a request when
// re-doing a request after a RETRY_WITH response.
#define HTTP_QUERY_ECHO_HEADERS                 73
#define HTTP_QUERY_ECHO_HEADERS_CRLF            74

#define HTTP_QUERY_PROXY_SUPPORT                75
#define HTTP_QUERY_AUTHENTICATION_INFO          76
#define HTTP_QUERY_PASSPORT_URLS                77
#define HTTP_QUERY_PASSPORT_CONFIG              78

#define HTTP_QUERY_X_CONTENT_TYPE_OPTIONS       79
#define HTTP_QUERY_P3P                          80
#define HTTP_QUERY_X_P2P_PEERDIST               81
#define HTTP_QUERY_TRANSLATE                    82
#define HTTP_QUERY_X_UA_COMPATIBLE              83
#define HTTP_QUERY_DEFAULT_STYLE                84
#define HTTP_QUERY_X_FRAME_OPTIONS              85
#define HTTP_QUERY_X_XSS_PROTECTION             86

#define HTTP_QUERY_SET_COOKIE2                  87

#define HTTP_QUERY_DO_NOT_TRACK                 88

#define HTTP_QUERY_KEEP_ALIVE                   89

#define HTTP_QUERY_MAX                          89
//
// HTTP_QUERY_CUSTOM - if this special value is supplied as the dwInfoLevel
// parameter of HttpQueryInfo() then the lpBuffer parameter contains the name
// of the header we are to query
//

#define HTTP_QUERY_CUSTOM                       65535

//
// HTTP_QUERY_FLAG_REQUEST_HEADERS - if this bit is set in the dwInfoLevel
// parameter of HttpQueryInfo() then the request headers will be queried for the
// request information
//

#define HTTP_QUERY_FLAG_REQUEST_HEADERS         0x80000000

//
// HTTP_QUERY_FLAG_SYSTEMTIME - if this bit is set in the dwInfoLevel parameter
// of HttpQueryInfo() AND the header being queried contains date information,
// e.g. the "Expires:" header then lpBuffer will contain a SYSTEMTIME structure
// containing the date and time information converted from the header string
//

#define HTTP_QUERY_FLAG_SYSTEMTIME              0x40000000

//
// HTTP_QUERY_FLAG_NUMBER - if this bit is set in the dwInfoLevel parameter of
// HttpQueryInfo(), then the value of the header will be converted to a 32bit
// number before being returned to the caller, if applicable
//

#define HTTP_QUERY_FLAG_NUMBER                  0x20000000

//
// HTTP_QUERY_FLAG_COALESCE - combine the values from several headers of the
// same name into the output buffer
//

#define HTTP_QUERY_FLAG_COALESCE                0x10000000

//
// HTTP_QUERY_FLAG_NUMBER64 - if this bit is set in the dwInfoLevel parameter of
// HttpQueryInfo(), then the value of the header will be converted to a 64bit
// number before being returned to the caller, if applicable
//

#define HTTP_QUERY_FLAG_NUMBER64                0x08000000


#define HTTP_QUERY_MODIFIER_FLAGS_MASK          (HTTP_QUERY_FLAG_REQUEST_HEADERS    \
                                                | HTTP_QUERY_FLAG_SYSTEMTIME        \
                                                | HTTP_QUERY_FLAG_NUMBER            \
                                                | HTTP_QUERY_FLAG_COALESCE          \
                                                | HTTP_QUERY_FLAG_NUMBER64          \
                                                )

#define HTTP_QUERY_HEADER_MASK                  (~HTTP_QUERY_MODIFIER_FLAGS_MASK)

//
// HTTP Response Status Codes:
//

#define HTTP_STATUS_CONTINUE            100 // OK to continue with request
#define HTTP_STATUS_SWITCH_PROTOCOLS    101 // server has switched protocols in upgrade header

#define HTTP_STATUS_OK                  200 // request completed
#define HTTP_STATUS_CREATED             201 // object created, reason = new URI
#define HTTP_STATUS_ACCEPTED            202 // async completion (TBS)
#define HTTP_STATUS_PARTIAL             203 // partial completion
#define HTTP_STATUS_NO_CONTENT          204 // no info to return
#define HTTP_STATUS_RESET_CONTENT       205 // request completed, but clear form
#define HTTP_STATUS_PARTIAL_CONTENT     206 // partial GET furfilled

#define HTTP_STATUS_AMBIGUOUS           300 // server couldn't decide what to return
#define HTTP_STATUS_MOVED               301 // object permanently moved
#define HTTP_STATUS_REDIRECT            302 // object temporarily moved
#define HTTP_STATUS_REDIRECT_METHOD     303 // redirection w/ new access method
#define HTTP_STATUS_NOT_MODIFIED        304 // if-modified-since was not modified
#define HTTP_STATUS_USE_PROXY           305 // redirection to proxy, location header specifies proxy to use
#define HTTP_STATUS_REDIRECT_KEEP_VERB  307 // HTTP/1.1: keep same verb

#define HTTP_STATUS_BAD_REQUEST         400 // invalid syntax
#define HTTP_STATUS_DENIED              401 // access denied
#define HTTP_STATUS_PAYMENT_REQ         402 // payment required
#define HTTP_STATUS_FORBIDDEN           403 // request forbidden
#define HTTP_STATUS_NOT_FOUND           404 // object not found
#define HTTP_STATUS_BAD_METHOD          405 // method is not allowed
#define HTTP_STATUS_NONE_ACCEPTABLE     406 // no response acceptable to client found
#define HTTP_STATUS_PROXY_AUTH_REQ      407 // proxy authentication required
#define HTTP_STATUS_REQUEST_TIMEOUT     408 // server timed out waiting for request
#define HTTP_STATUS_CONFLICT            409 // user should resubmit with more info
#define HTTP_STATUS_GONE                410 // the resource is no longer available
#define HTTP_STATUS_LENGTH_REQUIRED     411 // the server refused to accept request w/o a length
#define HTTP_STATUS_PRECOND_FAILED      412 // precondition given in request failed
#define HTTP_STATUS_REQUEST_TOO_LARGE   413 // request entity was too large
#define HTTP_STATUS_URI_TOO_LONG        414 // request URI too long
#define HTTP_STATUS_UNSUPPORTED_MEDIA   415 // unsupported media type
#define HTTP_STATUS_RETRY_WITH          449 // retry after doing the appropriate action.

#define HTTP_STATUS_SERVER_ERROR        500 // internal server error
#define HTTP_STATUS_NOT_SUPPORTED       501 // required not supported
#define HTTP_STATUS_BAD_GATEWAY         502 // error response received from gateway
#define HTTP_STATUS_SERVICE_UNAVAIL     503 // temporarily overloaded
#define HTTP_STATUS_GATEWAY_TIMEOUT     504 // timed out waiting for gateway
#define HTTP_STATUS_VERSION_NOT_SUP     505 // HTTP version not supported



//
// INTERNET_SCHEME - enumerated URL scheme type
//

typedef enum {
	INTERNET_SCHEME_PARTIAL = -2,
	INTERNET_SCHEME_UNKNOWN = -1,
	INTERNET_SCHEME_DEFAULT = 0,
	INTERNET_SCHEME_FTP,
	INTERNET_SCHEME_GOPHER,
	INTERNET_SCHEME_HTTP,
	INTERNET_SCHEME_HTTPS,
	INTERNET_SCHEME_FILE,
	INTERNET_SCHEME_NEWS,
	INTERNET_SCHEME_MAILTO,
	INTERNET_SCHEME_SOCKS,
	INTERNET_SCHEME_JAVASCRIPT,
	INTERNET_SCHEME_VBSCRIPT,
	INTERNET_SCHEME_RES,
	INTERNET_SCHEME_FIRST = INTERNET_SCHEME_FTP,
	INTERNET_SCHEME_LAST = INTERNET_SCHEME_RES
} INTERNET_SCHEME, *LPINTERNET_SCHEME;


//
// internet types
//

typedef LPVOID HINTERNET;
typedef HINTERNET * LPHINTERNET;

typedef WORD INTERNET_PORT;
typedef INTERNET_PORT * LPINTERNET_PORT;


//
// URL_COMPONENTS - the constituent parts of an URL. Used in InternetCrackUrl()
// and InternetCreateUrl()
//
// For InternetCrackUrl(), if a pointer field and its corresponding length field
// are both 0 then that component is not returned. If the pointer field is NULL
// but the length field is not zero, then both the pointer and length fields are
// returned if both pointer and corresponding length fields are non-zero then
// the pointer field points to a buffer where the component is copied. The
// component may be un-escaped, depending on dwFlags
//
// For InternetCreateUrl(), the pointer fields should be NULL if the component
// is not required. If the corresponding length field is zero then the pointer
// field is the address of a zero-terminated string. If the length field is not
// zero then it is the string length of the corresponding pointer field
//

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable : 4121 )   // disable alignment warning

typedef struct {
	DWORD   dwStructSize;       // size of this structure. Used in version check
	LPSTR   lpszScheme;         // pointer to scheme name
	DWORD   dwSchemeLength;     // length of scheme name
	INTERNET_SCHEME nScheme;    // enumerated scheme type (if known)
	LPSTR   lpszHostName;       // pointer to host name
	DWORD   dwHostNameLength;   // length of host name
	INTERNET_PORT nPort;        // converted port number
	LPSTR   lpszUserName;       // pointer to user name
	DWORD   dwUserNameLength;   // length of user name
	LPSTR   lpszPassword;       // pointer to password
	DWORD   dwPasswordLength;   // length of password
	LPSTR   lpszUrlPath;        // pointer to URL-path
	DWORD   dwUrlPathLength;    // length of URL-path
	LPSTR   lpszExtraInfo;      // pointer to extra information (e.g. ?foo or #foo)
	DWORD   dwExtraInfoLength;  // length of extra information
} URL_COMPONENTSA, *LPURL_COMPONENTSA;


//
// INTERNET_BUFFERS - combines headers and data. May be chained for e.g. file
// upload or scatter/gather operations. For chunked read/write, lpcszHeader
// contains the chunked-ext
//

typedef struct _INTERNET_BUFFERSA {
	DWORD dwStructSize;                 // used for API versioning. Set to sizeof(INTERNET_BUFFERS)
	struct _INTERNET_BUFFERSA * Next;   // chain of buffers
	LPCSTR   lpcszHeader;               // pointer to headers (may be NULL)
	DWORD dwHeadersLength;              // length of headers if not NULL
	DWORD dwHeadersTotal;               // size of headers if not enough buffer
	LPVOID lpvBuffer;                   // pointer to data buffer (may be NULL)
	DWORD dwBufferLength;               // length of data buffer if not NULL
	DWORD dwBufferTotal;                // total size of chunk, or content-length if not chunked
	DWORD dwOffsetLow;                  // used for read-ranges (only used in HttpSendRequest2)
	DWORD dwOffsetHigh;
} INTERNET_BUFFERSA, *LPINTERNET_BUFFERSA;


//
// globals
//

#define SZ_InternetConnect_Wininet "Wininet.dll"
static HMODULE g_InternetConnect_hWininet = NULL;

//
// functions
//

#define SZ_InternetConnect_InternetOpenA "InternetOpenA"
typedef HINTERNET(STDAPICALLTYPE* f_InternetConnect_InternetOpenA)
(
	__in_opt LPCSTR lpszAgent,
	__in DWORD dwAccessType,
	__in_opt LPCSTR lpszProxy,
	__in_opt LPCSTR lpszProxyBypass,
	__in DWORD dwFlags
);
extern f_InternetConnect_InternetOpenA m_InternetConnect_InternetOpenA;

#define SZ_InternetConnect_InternetCrackUrlA "InternetCrackUrlA"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_InternetCrackUrlA)
(
	__in_ecount(dwUrlLength) LPCSTR lpszUrl,
	__in DWORD dwUrlLength,
	__in DWORD dwFlags,
	__inout LPURL_COMPONENTSA lpUrlComponents
);
extern f_InternetConnect_InternetCrackUrlA m_InternetConnect_InternetCrackUrlA;

#define SZ_InternetConnect_InternetConnectA "InternetConnectA"
typedef HINTERNET(STDAPICALLTYPE* f_InternetConnect_InternetConnectA)(
	__in HINTERNET hInternet,
	__in LPCSTR lpszServerName,
	__in INTERNET_PORT nServerPort,
	__in_opt LPCSTR lpszUserName,
	__in_opt LPCSTR lpszPassword,
	__in DWORD dwService,
	__in DWORD dwFlags,
	__in_opt DWORD_PTR dwContext
);
extern f_InternetConnect_InternetConnectA m_InternetConnect_InternetConnectA;

#define SZ_InternetConnect_HttpOpenRequestA "HttpOpenRequestA"
typedef HINTERNET(STDAPICALLTYPE* f_InternetConnect_HttpOpenRequestA)(
	__in HINTERNET hConnect,
	__in_opt LPCSTR lpszVerb,
	__in_opt LPCSTR lpszObjectName,
	__in_opt LPCSTR lpszVersion,
	__in_opt LPCSTR lpszReferrer,
	__in_opt LPCSTR FAR * lplpszAcceptTypes,
	__in DWORD dwFlags,
	__in_opt DWORD_PTR dwContext
);
extern f_InternetConnect_HttpOpenRequestA m_InternetConnect_HttpOpenRequestA;

#define SZ_InternetConnect_InternetSetOptionA "InternetSetOptionA"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_InternetSetOptionA)(
	__in_opt HINTERNET hInternet,
	__in       DWORD dwOption,
	__in_opt LPVOID lpBuffer,
	__in       DWORD dwBufferLength
);
extern f_InternetConnect_InternetSetOptionA m_InternetConnect_InternetSetOptionA;

#define SZ_InternetConnect_HttpSendRequestA "HttpSendRequestA"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_HttpSendRequestA)(
	__in HINTERNET hRequest,
	__in_opt LPCSTR lpszHeaders,
	__in DWORD dwHeadersLength,
	__in_opt LPVOID lpOptional,
	__in DWORD dwOptionalLength
);
extern f_InternetConnect_HttpSendRequestA m_InternetConnect_HttpSendRequestA;

#define SZ_InternetConnect_InternetCloseHandle "InternetCloseHandle"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_InternetCloseHandle)(
	__in HINTERNET hInternet
);
extern f_InternetConnect_InternetCloseHandle m_InternetConnect_InternetCloseHandle;

#define SZ_InternetConnect_HttpQueryInfoA "HttpQueryInfoA"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_HttpQueryInfoA)(
	__in HINTERNET hRequest,
	__in DWORD dwInfoLevel,
	__inout_opt LPVOID lpBuffer,
	__inout LPDWORD lpdwBufferLength,
	__inout_opt LPDWORD lpdwIndex
);
extern f_InternetConnect_HttpQueryInfoA m_InternetConnect_HttpQueryInfoA;

#define SZ_InternetConnect_InternetReadFile "InternetReadFile"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_InternetReadFile)(
	__in HINTERNET hFile,
	__out LPVOID lpBuffer,
	__in DWORD dwNumberOfBytesToRead,
	__out LPDWORD lpdwNumberOfBytesRead
);
extern f_InternetConnect_InternetReadFile m_InternetConnect_InternetReadFile;

#define SZ_InternetConnect_InternetQueryDataAvailable "InternetQueryDataAvailable"
typedef BOOL(STDAPICALLTYPE* f_InternetConnect_InternetQueryDataAvailable)(
	__in HINTERNET hFile,
	__inout_opt LPDWORD lpdwNumberOfBytesAvailable,
	__in DWORD dwFlags,
	__in_opt DWORD_PTR dwContext
);
extern f_InternetConnect_InternetQueryDataAvailable m_InternetConnect_InternetQueryDataAvailable;



DWORD Initialize_InternetConnect_Wininet_Functions();
DWORD Finalize_InternetConnect_Wininet_Functions();



#endif