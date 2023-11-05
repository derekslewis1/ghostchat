#include "server.h"
#include "../common/common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void send_message(int sockfd, char *message)
{
  if (write(sockfd, message, strlen(message)) < 0)
  {
    perror("ERROR writing to socket");
  }
}

struct client clients[MAX_CLIENTS];
int num_clients = 0;

int main(int argc, char *argv[])
{
 
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int portno = atoi(argv[1]);

  // Create server socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("ERROR opening socket");
    exit(1);
  }

  // Set server socket options
  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
             sizeof(int));

  // Bind server socket to local address
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("ERROR on binding");
    exit(1);
  }

  // Listen for incoming connections
  listen(sockfd, MAX_CLIENTS);

  printf("Server started on port %d...\n", portno);

  // Start accepting client connections
  while (1)
  {
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);

    if (newsockfd < 0)
    {
      perror("ERROR on accept");
      exit(1);
    }

    // Read client username
    char username[MAX_USERNAME_LEN];
    memset(username, 0, sizeof(username));
    int n = read(newsockfd, username, MAX_USERNAME_LEN - 1);

    if (n < 0)
    {
      perror("ERROR reading from socket");
      exit(1);
    }

    // Add client to list of connected clients
    if (add_client(newsockfd, username) < 0)
    {
      fprintf(stderr, "Cannot add client '%s': Maximum client count reached\n",
              username);
      close(newsockfd);
    }
    else
    {
      // Create new thread to handle client connection
      struct client_thread_data *thread_data =
          malloc(sizeof(struct client_thread_data));
      thread_data->sockfd = newsockfd;
      thread_data->cli_addr = cli_addr;
      strcpy(thread_data->username, username);

      pthread_t tid;
      pthread_create(&tid, NULL, handle_client, thread_data);
    }
  }

  return 0;
}

void *handle_client(void *arg)
{
  struct client_thread_data *thread_data = (struct client_thread_data *)arg;

  int sockfd = thread_data->sockfd;
  struct sockaddr_in cli_addr = thread_data->cli_addr;
  char username[MAX_USERNAME_LEN];
  strcpy(username, thread_data->username);

  free(thread_data);

  printf("Client connected: %s\n", username);

  // Send welcome message to Client

  char welcome_message[MAX_MESSAGE_LEN];
  snprintf(welcome_message, MAX_MESSAGE_LEN, "Welcome to the chat room, %s", username);
  send_message(sockfd, welcome_message);

  // Read client messages and broadcast to other clients
  char message[MAX_MESSAGE_LEN];
  memset(message, 0, sizeof(message));

  while (1)
  {
    int n = read(sockfd, message, MAX_MESSAGE_LEN - 1);

    if (n < 0)
    {
      perror("ERROR reading from socket");
      break;
    }
    else if (n == 0)
    {
      // Client closed connection
      break;
    }

    // Broadcast message to all other clients
    printf("%s\n", message);
    send_to_all_clients_except(sockfd, message, sockfd);

    // Clear message buffer
    memset(message, 0, sizeof(message));
  }

  // Remove client from list of connected clients
  int index = find_client_index(sockfd);
  remove_client(index);

  printf("Client disconnected: %s\n", username);

  // Close client socket
  close(sockfd);

  return NULL;
}

void send_to_all_clients_except(int sockfd, char *message, int except_sockfd)
{
  for (int i = 0; i < num_clients; i++)
  {
    if (clients[i].sockfd != except_sockfd)
    {

      if (write(clients[i].sockfd, message, strlen(message)) < 0)
      {
        perror("ERROR writing to socket");
      }
    }
  }
}

int add_client(int sockfd, char *username)
{
  if (num_clients >= MAX_CLIENTS)
  {
    return -1;
  }

  struct client new_client = {sockfd};
  strncpy(new_client.username, username, MAX_USERNAME_LEN - 1);
  clients[num_clients++] = new_client;

  return 0;
}

int remove_client(int index)
{
  if (index < 0 || index >= num_clients)
  {
    return -1;
  }

  for (int i = index; i < num_clients - 1; i++)
  {
    clients[i] = clients[i + 1];
  }

  num_clients--;
  return 0;
}

int find_client_index(int sockfd)
{
  for (int i = 0; i < num_clients; i++)
  {
    if (clients[i].sockfd == sockfd)
    {
      return i;
    }
  }

  return -1;
}
