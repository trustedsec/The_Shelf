#ifndef KERBEROS_POSIX_H_
#define KERBEROS_POSIX_H_


int removePrincipal(unsigned char* cacheName, unsigned char* principalName);
int dumpTickets(void);
int listTickets(void);
#endif
