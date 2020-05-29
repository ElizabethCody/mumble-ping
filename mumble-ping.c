#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <endian.h>
#include "mumble-ping.h"

bool mumble_ping(const struct sockaddr_in* addr, struct mumble_ping_reply* reply) {
   static unsigned int seed = 0;
   seed = (seed == 0) ? (unsigned int) time(NULL) : seed;

   int sock = socket(addr->sin_family, SOCK_DGRAM, 0);

   if(sock < 0) {
      return false;
   }

   bool success = true;

   struct timeval timeout;
   memset(&timeout, 0, sizeof(timeout));
   timeout.tv_sec = 1;
   if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
      success = false;
      goto close_and_return;
   }

   // generate id
   uint64_t id;

   id = rand_r(&seed);
   id <<= 32;
   id |= rand_r(&seed);

   uint32_t buf32;
   uint64_t buf64;

   char request[12];

   // export type = 0
   buf32 = htobe32(0);
   memcpy(request, &buf32, 4);

   // export random id
   buf64 = htobe64(id);
   memcpy(request + 4, &buf64, 8);

   // send datagram
   if(sendto(sock, request, 12, MSG_CONFIRM, (const struct sockaddr*) addr,
         sizeof(*addr)) != 12) {
      success = false;
      goto close_and_return;
   }

   char response[24];
   socklen_t len = 0;

   // read response datagram
   if(recvfrom(sock, response, 24, MSG_WAITALL, (struct sockaddr*) addr,
         &len) != 24) {
      success = false;
      goto close_and_return;
   }

   // parse version
   memcpy(&reply->version, response, 4);

   // parse ping id
   memcpy(&buf64, response + 4, 8);
   buf64 = be64toh(buf64);

   // verify ping id
   if(buf64 != id) {
      success = false;
      goto close_and_return;
   }

   // parse users
   memcpy(&buf32, response + 12, 4);
   reply->users = be32toh(buf32);

   // parse slots
   memcpy(&buf32, response + 16, 4);
   reply->slots = be32toh(buf32);

   // parse bandwidth
   memcpy(&buf32, response + 20, 4);
   reply->bandwidth = be32toh(buf32);

close_and_return:
   close(sock);

   return success;
}
