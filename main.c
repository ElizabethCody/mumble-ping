#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "mumble-ping.h"

#define MUMBLE_DEFAULT_PORT 64738

int main(int argc, char* argv[]) {
   if(argc < 2 || argc > 3) {
      fprintf(stderr, "%s: invalid parameters.\n", argv[0]);
      fprintf(stderr, "Usage: %s <host> [port]\n", argv[0]);

      return EXIT_FAILURE;
   }

   struct sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));

   if(argc == 2) {
      addr.sin_port = htons(MUMBLE_DEFAULT_PORT);
   } else {
      uint16_t port;

      if(sscanf(argv[2], "%" SCNu16, &port) != 1) {
         fprintf(stderr, "%s: invalid port.\n", argv[0]);

         return EXIT_FAILURE;
      }

      addr.sin_port = htons(port);
   }

   struct addrinfo* result;
   if(getaddrinfo(argv[1], NULL, NULL, &result) != 0) {
      fprintf(stderr, "%s: failed DNS lookup.\n", argv[0]);

      return EXIT_FAILURE;
   }

   int status = EXIT_SUCCESS;

   struct mumble_ping_reply reply;
   for(struct addrinfo* cur = result; cur != NULL; cur = cur->ai_next) {
      if(cur->ai_family == AF_INET) {
         struct sockaddr_in* in = (struct sockaddr_in*) cur->ai_addr;
         addr.sin_family = in->sin_family;
         addr.sin_addr = in->sin_addr;

         if(mumble_ping(&addr, &reply)) {
            char ipstr[64], host[1024];
            uint16_t port = ntohs(addr.sin_port);
            inet_ntop(AF_INET, &addr.sin_addr.s_addr, ipstr, sizeof(ipstr));

            if(strcmp(ipstr, argv[1]) == 0) {
               snprintf(host, sizeof(host), "%s:%" PRIu16, argv[1], port);
            } else {
               snprintf(host, sizeof(host), "%s:%" PRIu16 " (%s:%" PRIu16 ")",
                  argv[1], port, ipstr, port);
            }

            printf("Server: %s\n", host);
            printf("Version: %" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
               reply.version[1], reply.version[2], reply.version[3]);
            printf("Users: %" PRIu32 "/%" PRIu32 "\n",
               reply.users, reply.slots);
            printf("Bandwidth: %" PRIu32 " b/s\n", reply.bandwidth);
            goto cleanup;
         }
      }
   }

   status = EXIT_FAILURE;
   fprintf(stderr, "%s: failed to query Mumble server.\n", argv[0]);

cleanup:
   freeaddrinfo(result);
   return status;
}
