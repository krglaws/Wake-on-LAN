
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define PACKETSIZE (17 * 6)

uint8_t magic[PACKETSIZE];


void usage(char *name) {
  fprintf(stderr, "usage: %s -m <mac_address> [-i <ip_address>]\n", name);
  exit(EXIT_FAILURE);
}


int valid_mac(const char* mac) {

  if (strlen(mac) != 17) {
    return 0;
  }

  for (int i = 0; i < 17; i += 3) {

    if (!isxdigit(mac[i]) || !isxdigit(mac[i+1])) {
      return 0;
    }

    if (i < 15 && mac[i + 2] != ':') {
      return 0;
    }
  }

  return 1;
}


int main(int argc, char** argv) {

  /* Destination IP is broadcast address by default */
  char* ip = "255.255.255.255";
  char* mac = NULL;

  /* Get args */
  int c;
  while ((c = getopt(argc, argv, "i:m:")) != -1) {
    switch (c) {
    case 'i':
      ip = optarg;
      break;
    case 'm':
      mac = optarg;
      break;
    case '?':
      usage(argv[0]);
    default:
      usage(argv[0]);
    }
  }

  if (optind < argc && argv[optind]) {
    fprintf(stderr, "%s: unknown argument -- '%s'\n", argv[0], argv[optind]);
    usage(argv[0]);
  }

  if (mac == NULL) {
    usage(argv[0]);
  }

  if (!valid_mac(mac)) {
    fprintf(stderr, "%s: Invalid MAC Address\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Prepare magic packet */
  for (int i = 0; i < 6; i++) magic[i] = 0xFF;

  uint8_t byte = strtol(mac, &mac, 16);
  mac++; //skip ':'
  for (int i = 6; i < PACKETSIZE - 5; i += 6) magic[i] = byte;

  byte = strtol(mac, &mac, 16);
  mac++; //skip ':'
  for (int i = 7; i < PACKETSIZE - 4; i += 6) magic[i] = byte;
  
  byte = strtol(mac, &mac, 16);
  mac++; //skip ':'
  for (int i = 8; i < PACKETSIZE - 3; i += 6) magic[i] = byte;
 
  byte = strtol(mac, &mac, 16);
  mac++; //skip ':'
  for (int i = 9; i < PACKETSIZE - 2; i += 6) magic[i] = byte;

  byte = strtol(mac, &mac, 16);
  mac++; //skip ':'
  for (int i = 10; i < PACKETSIZE - 1; i += 6) magic[i] = byte;

  byte = strtol(mac, &mac, 16);
  mac++; //skip ':'
  for (int i = 11; i < PACKETSIZE; i += 6) magic[i] = byte;
 
  /* Create socket */
  int sock;
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror(argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Set broadcast permissions */
  int broadperm = 1; 
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadperm,
      sizeof(broadperm)) < 0) {
    perror(argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Init destination address */
  struct sockaddr_in destaddr;
  memset(&destaddr, 0, sizeof(destaddr));

  destaddr.sin_family = AF_INET;
  destaddr.sin_port = htons(42069);
  if (inet_aton(ip, &(destaddr.sin_addr)) < 0) {
    fprintf(stderr, "%s: Invalid IP\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Send it */
  if (sendto(sock, (const char *)magic, PACKETSIZE, MSG_CONFIRM,
         (const struct sockaddr *) &destaddr, sizeof(destaddr)) < 0)
  {
    perror(argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Magic packet sent.\n"); 

  close(sock); 
  return 0;
}

