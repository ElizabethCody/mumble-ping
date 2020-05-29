#ifndef _MUMBLE_PING_H
#define _MUMBLE_PING_H 1

#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>

struct mumble_ping_reply {
   uint8_t version[4];
   uint32_t users;
   uint32_t slots;
   uint32_t bandwidth;
};

bool mumble_ping(const struct sockaddr_in*, struct mumble_ping_reply*);

#endif // _MUMBLE_PING_H
