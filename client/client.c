#include "client.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../common/common.h"
#include "client.h"
#include <stdbool.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>

struct send_messages_data
{
  int sockfd;
  char username[MAX_USERNAME_LEN];
};

typedef struct
{
  int sockfd;
  char username[MAX_USERNAME_LEN];
  bool *firstMessageDisplayed;
} thread_data_t;

void *send_messages(void *arg)
{
  int sockfd = *((int *)arg);
  char buffer[MAX_MESSAGE_LEN];

  while (1)
  {
    // Get message from user

    fgets(buffer, MAX_MESSAGE_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; 

    // Send message to server
    int n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0)
    {
      perror("ERROR sending message to server");
      exit(EXIT_FAILURE);
    }
  }

  return NULL;
}

void *receive_loop(void *arg)
{
  int sockfd = *((int *)arg);
  char buffer[MAX_MESSAGE_LEN];
  int n;

  while (1)
  {
    // Read message from server
    n = recv(sockfd, buffer, MAX_MESSAGE_LEN - 1, 0);
    if (n < 0)
    {
      perror("ERROR receiving message from server");
      exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';

    printf("%s\n", buffer);
  }
}

void chat_loop(int sockfd, char *username)
{
  pthread_t receive_thread;
  int n;

  // Send username to server
  n = send(sockfd, username, strlen(username), 0);
  if (n < 0)
  {
    perror("ERROR sending username");
    exit(EXIT_FAILURE);
  }

  // Start receive thread
  if (pthread_create(&receive_thread, NULL, receive_loop, &sockfd) != 0)
  {
    perror("ERROR creating receive thread");
    exit(EXIT_FAILURE);
  }

  // Main send loop
  char buffer[MAX_MESSAGE_LEN];
  while (1)
  {
    // Get message from user
    bzero(buffer, MAX_MESSAGE_LEN);
    fgets(buffer, MAX_MESSAGE_LEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; 

    // Send message to server
    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0)
    {
      perror("ERROR sending message to server");
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char *argv[])
{
  int sockfd, n;
  char username[MAX_USERNAME_LEN];
  struct sockaddr_in serv_addr;
  struct hostent *server;

  // Check command-line arguments
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Get username from user
  printf("Enter username: ");
  fgets(username, MAX_USERNAME_LEN, stdin);
  username[strcspn(username, "\n")] = '\0'; 

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("ERROR opening socket");
    exit(EXIT_FAILURE);
  }

  // Get server address
  server = gethostbyname(argv[1]);
  if (server == NULL)
  {
    fprintf(stderr, "ERROR, no such host\n");
    exit(EXIT_FAILURE);
  }

  // Initialize server address structure
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(atoi(argv[2]));

  // Connect to server
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("ERROR connecting");
    exit(EXIT_FAILURE);
  }

  // Send username to server
  n = send(sockfd, username, strlen(username), 0);
  if (n < 0)
  {
    perror("ERROR sending username");
    exit(EXIT_FAILURE);
  }

  // Start chat loop
  chat_loop(sockfd, username);

  // Close socket
  close(sockfd);
  return 0;
}
